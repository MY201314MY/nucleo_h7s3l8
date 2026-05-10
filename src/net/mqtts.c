/*
 * Copyright (c) 2023 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
	https://www.emqx.com/en/mqtt/public-mqtt5-broker

	host / port / Certificate
*/
#include "certs.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/posix/netinet/in.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/posix/netdb.h>
#include <zephyr/posix/poll.h>

#include <zephyr/net/socket.h>
#include <zephyr/net/dns_resolve.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/data/json.h>
#include <zephyr/random/random.h>

#include <zephyr/shell/shell.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(net_mqtts_client_sample, LOG_LEVEL_DBG);

#if defined(CONFIG_MBEDTLS_MEMORY_DEBUG)
#include <mbedtls/memory_buffer_alloc.h>
#endif

#define BROKER_NAME "broker.emqx.io"
#define BROKER_PORT 8883

#define MQTT_BUFFER_SIZE 256u
#define APP_BUFFER_SIZE	 1024

#define MAX_RETRIES	        10u
#define BACKOFF_CONST_MS    120000u

static struct sockaddr_in broker_addr_in;

static uint8_t rx_buffer[MQTT_BUFFER_SIZE];
static uint8_t tx_buffer[MQTT_BUFFER_SIZE];
static uint8_t buffer[APP_BUFFER_SIZE]; /* Shared between published and received messages */

static struct mqtt_client client_ctx;

static const char mqtt_client_name[] = "nucleo_h7s3l8";
static bool do_subscribe; /* Trigger client to subscribe */

#define CA_CERTIFICATE_TAG 1

static const sec_tag_t sec_tls_tags[] = {
	CA_CERTIFICATE_TAG
};

static int setup_credentials(void)
{
	int ret;

	ret = tls_credential_add(CA_CERTIFICATE_TAG,
					TLS_CREDENTIAL_CA_CERTIFICATE,
					ca_certificate,
					sizeof(ca_certificate));
	if ((ret < 0) && (ret != -EEXIST))
	{
		LOG_ERR("Failed to add device certificate: %d", ret);
	}

	return ret;
}

static int subscribe_topic(void)
{
	int ret;
	struct mqtt_topic topics[] = {{
		.topic = {.utf8 = "down",
			  .size = strlen("down")},
		.qos = 0,
	}};
	const struct mqtt_subscription_list sub_list = {
		.list = topics,
		.list_count = ARRAY_SIZE(topics),
		.message_id = 1u,
	};

	LOG_INF("Subscribing to %hu topic(s)", sub_list.list_count);

	ret = mqtt_subscribe(&client_ctx, &sub_list);
	if (ret != 0) {
		LOG_ERR("Failed to subscribe to topics: %d", ret);
	}

	return ret;
}

static int publish_message(const char *topic, size_t topic_len, uint8_t *payload,
			   size_t payload_len)
{
	static uint32_t message_id = 1u;

	int ret;
	struct mqtt_publish_param msg;

	msg.retain_flag = 0u;
	msg.dup_flag = 0u;
	msg.message.topic.topic.utf8 = topic;
	msg.message.topic.topic.size = topic_len;
	msg.message.topic.qos = 0;
	msg.message.payload.data = payload;
	msg.message.payload.len = payload_len;
	msg.message_id = message_id++;

	ret = mqtt_publish(&client_ctx, &msg);
	if (ret != 0) {
		LOG_ERR("Failed to publish message: %d", ret);
	}

	LOG_INF("PUBLISHED on topic \"%s\" [ id: %u qos: %u ], payload: %u B", topic,
		msg.message_id, msg.message.topic.qos, payload_len);
	LOG_HEXDUMP_DBG(payload, payload_len, "Published payload:");

	return ret;
}

static ssize_t handle_published_message(const struct mqtt_publish_param *pub)
{
	int ret;
	size_t received = 0u;
	const size_t message_size = pub->message.payload.len;
	const bool discarded = message_size > APP_BUFFER_SIZE;

	LOG_INF("RECEIVED on topic \"%s\" [ id: %u qos: %u ] payload: %u / %u B",
		(const char *)pub->message.topic.topic.utf8, pub->message_id,
		pub->message.topic.qos, message_size, APP_BUFFER_SIZE);

	while (received < message_size) {
		uint8_t *p = discarded ? buffer : &buffer[received];

		ret = mqtt_read_publish_payload_blocking(&client_ctx, p, APP_BUFFER_SIZE);
		if (ret < 0) {
			return ret;
		}

		received += ret;
	}

	if (!discarded) {
		LOG_HEXDUMP_DBG(buffer, MIN(message_size, 256u), "Received payload:");
	}

	/* Send ACK */
	switch (pub->message.topic.qos) {
	case MQTT_QOS_1_AT_LEAST_ONCE: {
		struct mqtt_puback_param puback;

		puback.message_id = pub->message_id;
		mqtt_publish_qos1_ack(&client_ctx, &puback);
	} break;
	case MQTT_QOS_2_EXACTLY_ONCE:
	case MQTT_QOS_0_AT_MOST_ONCE:
	default:
		break;
	}

	return discarded ? -ENOMEM : received;
}

const char *mqtt_evt_type_to_str(enum mqtt_evt_type type)
{
	static const char *const types[] = {
		"CONNACK", "DISCONNECT", "PUBLISH", "PUBACK",	"PUBREC",
		"PUBREL",  "PUBCOMP",	 "SUBACK",  "UNSUBACK", "PINGRESP",
	};

	return (type < ARRAY_SIZE(types)) ? types[type] : "<unknown>";
}

static void mqtt_event_cb(struct mqtt_client *client, const struct mqtt_evt *evt)
{
	LOG_DBG("MQTT event: %s [%u] result: %d", mqtt_evt_type_to_str(evt->type), evt->type,
		evt->result);

	switch (evt->type) {
	case MQTT_EVT_CONNACK: {
		do_subscribe = true;
	} break;

	case MQTT_EVT_PUBLISH: {
		const struct mqtt_publish_param *pub = &evt->param.publish;

		handle_published_message(pub);
	} break;

	case MQTT_EVT_SUBACK:
	case MQTT_EVT_PUBACK:
	case MQTT_EVT_DISCONNECT:
	case MQTT_EVT_PUBREC:
	case MQTT_EVT_PUBREL:
	case MQTT_EVT_PUBCOMP:
	case MQTT_EVT_PINGRESP:
	case MQTT_EVT_UNSUBACK:
	default:
		break;
	}
}

static void mqtt_client_setup(void)
{
	mqtt_client_init(&client_ctx);

	client_ctx.broker = &broker_addr_in;
	client_ctx.evt_cb = mqtt_event_cb;

	client_ctx.client_id.utf8 = (uint8_t *)mqtt_client_name;
	client_ctx.client_id.size = sizeof(mqtt_client_name) - 1;
	client_ctx.password = NULL;
	client_ctx.user_name = NULL;

	client_ctx.keepalive = CONFIG_MQTT_KEEPALIVE;

	client_ctx.protocol_version = MQTT_VERSION_3_1_1;

	client_ctx.rx_buf = rx_buffer;
	client_ctx.rx_buf_size = MQTT_BUFFER_SIZE;
	client_ctx.tx_buf = tx_buffer;
	client_ctx.tx_buf_size = MQTT_BUFFER_SIZE;

	/* setup TLS */
	client_ctx.transport.type = MQTT_TRANSPORT_SECURE;
	struct mqtt_sec_config *const tls_config = &client_ctx.transport.tls.config;

	tls_config->peer_verify = TLS_PEER_VERIFY_REQUIRED;
	tls_config->cipher_list = NULL;
	tls_config->sec_tag_list = sec_tls_tags;
	tls_config->sec_tag_count = ARRAY_SIZE(sec_tls_tags);
	tls_config->hostname = BROKER_NAME;
	tls_config->cert_nocopy = TLS_CERT_NOCOPY_NONE;
}

static int mqtt_client_try_connect(void)
{
	int ret;
	uint8_t retry = 0;

	while (retry <= MAX_RETRIES) {
		retry++;
		ret = mqtt_connect(&client_ctx);
		if (ret == 0) {
			goto exit;
		}

		LOG_ERR("Failed to connect: %d backoff delay: %u ms", ret, BACKOFF_CONST_MS);
		k_sleep(K_MSEC(BACKOFF_CONST_MS));
	}

exit:
	return ret;
}

struct publish_payload {
	uint32_t counter;
};

static const struct json_obj_descr json_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct publish_payload, counter, JSON_TOK_NUMBER),
};

static int publish(void)
{
	static uint32_t messages_received_counter = 0;

	struct publish_payload pl = {.counter = messages_received_counter++};

	json_obj_encode_buf(json_descr, ARRAY_SIZE(json_descr), &pl, buffer, sizeof(buffer));

	return publish_message("up", strlen("up"), buffer,
			       strlen(buffer));
}

void mqtt_client_loop(void)
{
	int rc;
	int timeout;
	struct pollfd fds;

	mqtt_client_setup();

	rc = mqtt_client_try_connect();
	if (rc != 0) {
		goto cleanup;
	}

	fds.fd = client_ctx.transport.tcp.sock;
	fds.events = POLLIN;

	for (;;) {
		timeout = mqtt_keepalive_time_left(&client_ctx);
		rc = poll(&fds, 1u, timeout);
		if (rc >= 0) {
			if (fds.revents & POLLIN) {
				rc = mqtt_input(&client_ctx);
				if (rc != 0) {
					LOG_ERR("Failed to read MQTT input: %d", rc);
					break;
				}
			}

			if (fds.revents & (POLLHUP | POLLERR)) {
				LOG_ERR("Socket closed/error");
				break;
			}

			rc = mqtt_live(&client_ctx);
			if ((rc != 0) && (rc != -EAGAIN)) {
				LOG_ERR("Failed to live MQTT: %d", rc);
				break;
			}
		} else {
			LOG_ERR("poll failed: %d", rc);
			break;
		}

		if (do_subscribe) {
			do_subscribe = false;
			subscribe_topic();
		}
	}

cleanup:
	mqtt_disconnect(&client_ctx, NULL);

	close(fds.fd);
	fds.fd = -1;
}

static int resolve_broker_addr(struct sockaddr_in *broker)
{
	int ret;
	struct addrinfo *ai = NULL, *index = NULL;

	const struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = 0,
	};
	char port_string[6] = {0};

	sprintf(port_string, "%d", BROKER_PORT);
	ret = getaddrinfo(BROKER_NAME, port_string, &hints, &ai);
	if (ret == 0) {
		char addr_str[INET_ADDRSTRLEN];

		for(index=ai; index != NULL; index=index->ai_next)
		{
			memcpy(broker, index->ai_addr, MIN(index->ai_addrlen, sizeof(struct sockaddr_storage)));

			inet_ntop(AF_INET, &broker->sin_addr, addr_str, sizeof(addr_str));
			LOG_INF("Resolved: %s:%u", addr_str, htons(broker->sin_port));
			if(htons(broker->sin_port) == 8883)
			{
				break;
			}
		}
	} else {
		LOG_ERR("failed to resolve hostname err = %d (errno = %d)", ret, errno);
	}

	freeaddrinfo(ai);

	return ret;
}

int _mqtt_connect_thread_entry(void)
{
	setup_credentials();

	for (;;) {
		resolve_broker_addr(&broker_addr_in);

		mqtt_client_loop();

#if defined(CONFIG_MBEDTLS_MEMORY_DEBUG)
		size_t cur_used, cur_blocks, max_used, max_blocks;

		mbedtls_memory_buffer_alloc_cur_get(&cur_used, &cur_blocks);
		mbedtls_memory_buffer_alloc_max_get(&max_used, &max_blocks);
		LOG_INF("mbedTLS heap usage: MAX %u/%u (%u) CUR %u (%u)", max_used,
			CONFIG_MBEDTLS_HEAP_SIZE, max_blocks, cur_used, cur_blocks);
#endif

		k_sleep(K_SECONDS(1));
	}

	return 0;
}

K_THREAD_STACK_DEFINE(mqtts_thread_stack, 4096);
static struct k_thread m_thread;
static k_tid_t m_thread_id = NULL;

static int _example_mqtts_connect(const struct shell *sh, size_t argc, char *argv[])
{
	if(m_thread_id != NULL)
	{
		LOG_WRN("thread has been created.");
		return 0;
	}
    m_thread_id = k_thread_create(&m_thread,
										mqtts_thread_stack,
                     					4096,
                     					(k_thread_entry_t)_mqtt_connect_thread_entry,
                     					NULL, NULL, NULL,
                     					K_PRIO_PREEMPT( 8 ), 0, K_NO_WAIT );

	if( m_thread_id != NULL )
  	{
    	k_thread_name_set(m_thread_id, "mqtt");
  	}
  	else
  	{
		LOG_ERR("mqtt thread create failed.");
  	}

	return 0;
}

static int _example_mqtts_publish(const struct shell *sh, size_t argc, char *argv[])
{
	int ret = publish();
	LOG_INF("publish one message to server, ret:%d", ret);

	return 0;
}

static int _example_mqtts_disconnect(const struct shell *sh, size_t argc, char *argv[])
{
	if(m_thread_id == NULL)
	{
		LOG_WRN("thread has been about.");
		return 0;
	}

	LOG_INF("mqtt thread entry:%p", m_thread_id);
	if(m_thread_id != NULL)
	{
		int ret = k_thread_join(&m_thread, K_NO_WAIT);

		if(ret)
		{
			k_thread_abort(m_thread_id);
			LOG_INF( "abort thread mqtt" );
		}

		mqtt_disconnect(&client_ctx, NULL);
		close(client_ctx.transport.tcp.sock);

		m_thread_id = NULL;
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(mqtt_commands,
	SHELL_CMD(connect, NULL,
		"mqtts connect",
		_example_mqtts_connect),
	SHELL_CMD(disconnect, NULL,
		"mqtts disconnect",
		_example_mqtts_disconnect),
	SHELL_CMD(publish, NULL,
		"mqtts publish a message",
		_example_mqtts_publish),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(mqtt, &mqtt_commands,
		   "example for zephyr mqtt", NULL);

