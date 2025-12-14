#include <netdb.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/http/client.h>
#include <zephyr/shell/shell.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/dns_resolve.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <string.h>

#include <zephyr/posix/netinet/in.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/posix/poll.h>

#include "certs.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(net_https_client, LOG_LEVEL_DBG);

#define CA_CERTIFICATE_TAG 1

#define HTTPS_PORT "443"

uint8_t HTTPS_HOST[128] = "www.example.com";

#define MAX_RECV_BUF_LEN 2048

static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN];


static int response_cb(struct http_response *rsp,
			enum http_final_call final_data,
			void *user_data)
{
	if (final_data == HTTP_DATA_MORE) {
		LOG_INF("Partial data received (%zd bytes)", rsp->data_len);
        LOG_HEXDUMP_DBG(rsp->recv_buf, rsp->data_len, "http recv");
		//printf("%.*s", (int)rsp->data_len, rsp->recv_buf);
	} else if (final_data == HTTP_DATA_FINAL) {
		LOG_INF("All the data received (%zd bytes)", rsp->data_len);
        LOG_HEXDUMP_DBG(rsp->recv_buf, rsp->data_len, "http recv");
		//printf("%.*s", (int)rsp->data_len, rsp->recv_buf);
	}

	LOG_INF("Response to %s", (const char *)user_data);
	LOG_INF("Response status %s", rsp->http_status);

    return 0;
}

/* It's useless for host : www.example.com. */
static const char *head[] = {
	"Range: bytes=0-511\r\n",
	NULL
};

int example_https_request(const struct shell *sh, size_t argc, char *argv[])
{
	int sock4 = -1;
	int32_t timeout = 30 * MSEC_PER_SEC;

    static struct addrinfo hints = {
		.ai_flags = AI_NUMERICSERV,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	char peer_addr[INET6_ADDRSTRLEN];

	LOG_INF("Looking up %s\n", HTTPS_HOST);
	int ret = getaddrinfo(HTTPS_HOST, HTTPS_PORT, &hints, &res);
    if(ret<0)
    {
        LOG_ERR("getaddrinfo error ret : %d", ret);
        return -1;
    }
    LOG_INF("addrinfo @%p : ai_family=%d, ai_socktype=%d, ai_protocol=%d, sa_family=%d, sin_port=%x\n",
        res, res->ai_family, res->ai_socktype, res->ai_protocol, res->ai_addr->sa_family,
        ((struct sockaddr_in *)res->ai_addr)->sin_port);

	inet_ntop(res->ai_family, &((struct sockaddr_in *)(res->ai_addr))->sin_addr, peer_addr,
		  INET6_ADDRSTRLEN);
	LOG_INF("Resolved %s (%s)\n", peer_addr, net_family2str(res->ai_family));

	tls_credential_add(CA_CERTIFICATE_TAG,
					 TLS_CREDENTIAL_CA_CERTIFICATE,
					 ca_certificate,
					 sizeof(ca_certificate));

	sec_tag_t sec_tag_list[] = {
		CA_CERTIFICATE_TAG,
	};


    sock4 = socket(res->ai_family, SOCK_STREAM, IPPROTO_TLS_1_2);

	if (sock4 < 0) {
		LOG_ERR("Cannot create HTTP connection.");
		return -ECONNABORTED;
	}

	enum {
		NONE = 0,
		OPTIONAL = 1,
		REQUIRED = 2,
	};

	int verify = REQUIRED;

	ret = setsockopt(sock4, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));
	if (ret) {
		LOG_ERR("Failed to setup peer verification, err %d\n", errno);
		return -errno;
	}

	ret = setsockopt(sock4, SOL_TLS, TLS_SEC_TAG_LIST,
					 sec_tag_list, sizeof(sec_tag_list));
	if (ret < 0) {
		LOG_ERR("Failed to set secure option (%d)", -errno);
		ret = -errno;
	}

	ret = setsockopt(sock4, SOL_TLS, TLS_HOSTNAME, HTTPS_HOST, strlen(HTTPS_HOST));

	if (ret < 0) {
		LOG_ERR("Failed to set %s : (%d)", HTTPS_HOST, -errno);
		ret = -errno;
	}

    ret = connect(sock4, res->ai_addr, res->ai_addrlen);
	if (ret < 0) {
		LOG_ERR("can't connect to remote host, ret : %d", -errno);
		close(sock4);
		return -ETIMEDOUT;
	}

	if (sock4 >= 0 && IS_ENABLED(CONFIG_NET_IPV4)) {
		struct http_request req;

		memset(&req, 0, sizeof(req));

		req.method = HTTP_GET;
		req.url = "/";
		req.header_fields = head;
		req.host = HTTPS_HOST;
		req.protocol = "HTTP/1.1";
		req.response = response_cb;
		req.recv_buf = recv_buf_ipv4;
		req.recv_buf_len = sizeof(recv_buf_ipv4);

		ret = http_client_req(sock4, &req, timeout, "IPv4 GET");
		LOG_INF("ret (socket id) : %d", ret);

		close(sock4);
	}
	
	freeaddrinfo(res);

	LOG_INF("--- END ---");

    return 0;
}
