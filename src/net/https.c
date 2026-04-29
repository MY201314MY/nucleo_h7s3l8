#include <netdb.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/http/client.h>
#include <zephyr/shell/shell.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/device.h>
#include <zephyr/net/net_if.h>
#include <zephyr/posix/netinet/in.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/poll.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_crt.h>

#include "certs.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(_net_https_client, LOG_LEVEL_DBG);

#define CA_CERTIFICATE_TAG 2

#define HTTPS_PORT "443"
typedef struct {
    uint8_t host[128];
    uint8_t path[128];
} http_host_t;

http_host_t http_host[] = 
{
    {
        .host = "ecc256.badssl.com",
        .path = "/"
    },
    {
        .host = "api.bilibili.com",
        .path = "/x/relation/stat?vmid=128505057"
    },
    {
        .host = "www.example.com",
        .path = "/"
    },
};

#define MAX_RECV_BUF_LEN 2048

static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN];


static int response_cb(struct http_response *rsp,
			enum http_final_call final_data,
			void *user_data)
{
	if (final_data == HTTP_DATA_MORE) {
	} else if (final_data == HTTP_DATA_FINAL) {
		if(rsp->body_frag_start != NULL)
        {
            int length = rsp->data_len - (rsp->body_frag_start - rsp->recv_buf);
            LOG_INF("http received body length : %d", length);
            LOG_HEXDUMP_DBG(rsp->body_frag_start, length, "http body recv");
        }
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
	int fd = -1;
	int32_t timeout = 30 * MSEC_PER_SEC;
    http_host_t *p = NULL;
    if(argc == 2)
    {
        int operation = atoi(argv[1]);
        
        LOG_DBG("operation %d selected", operation);

        if(operation>= ARRAY_SIZE(http_host))
        {
            LOG_ERR("unknown operation");
            return 0;
        }

        p = &http_host[operation];
    }
    else
    {
        p = &http_host[0];
    }

    static struct addrinfo hints = {
        .ai_family = AF_INET,
		.ai_flags = AI_NUMERICSERV,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res, *index;
	char addr[64];

    int ret = tls_credential_add(CA_CERTIFICATE_TAG,
                        TLS_CREDENTIAL_CA_CERTIFICATE,
                        ca_certificate,
                        sizeof(ca_certificate));
    if ((ret < 0) && (ret != -EEXIST))
    {
		LOG_ERR("Failed to add device certificate: %d", ret);
	}

	LOG_DBG("request host : %s", p->host);

	ret = getaddrinfo(p->host, HTTPS_PORT, &hints, &res);
    if(ret != 0)
    {
        LOG_ERR("getaddrinfo error ret : %d", ret);
        goto end;
    }

    for(index=res; index != NULL; index=index->ai_next)
	{
        if(index->ai_family != AF_INET)
        {
            LOG_INF("skip address(AF_INET6)...");
            continue;
        }
        inet_ntop(index->ai_family, &((struct sockaddr_in *)(index->ai_addr))->sin_addr, addr,
            INET_ADDRSTRLEN);
        LOG_INF("resolved %s (%s)", addr, net_family2str(index->ai_family));

        sec_tag_t sec_tag_list[] = {
            CA_CERTIFICATE_TAG,
        };


        fd = socket(index->ai_family, SOCK_STREAM, IPPROTO_TLS_1_2);

        if (fd < 0) {
            LOG_ERR("socket created failed, ret=%d.", -errno);
            continue;;
        }

        enum {
            NONE = 0,
            OPTIONAL = 1,
            REQUIRED = 2,
        };

        int verify = REQUIRED;

        ret = setsockopt(fd, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));
        if (ret) {
            LOG_ERR("failed to setup peer verification, err %d", errno);
            return -errno;
        }

        ret = setsockopt(fd, SOL_TLS, TLS_SEC_TAG_LIST,
                        sec_tag_list, sizeof(sec_tag_list));
        if (ret < 0) {
            LOG_ERR("failed to set secure option (%d), ret=%d", -errno, ret);
            goto end;
        }

        struct timeval tv;
        tv.tv_sec = 8;
        tv.tv_usec = 0;

        ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
                        &tv, sizeof(tv));
        if (ret < 0) {
            LOG_ERR("failed to set socket receive timeout(%d), ret=%d", -errno, ret);
            goto end;
        }

        ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
                        &tv, sizeof(tv));
        if (ret < 0) {
            LOG_ERR("failed to set socket transmit timeout(%d), ret=%d", -errno, ret);
            goto end;
        }

        ret = setsockopt(fd, SOL_TLS, TLS_HOSTNAME, p->host, strlen(p->host));

        if (ret < 0) {
            LOG_ERR("failed to set %s : (%d)", p->host, -errno);
            ret = -errno;
        }

        ret = connect(fd, index->ai_addr, index->ai_addrlen);
        if (ret < 0) {
            LOG_ERR("can't connect to remote host, ret : %d", -errno);
            close(fd);
            break;
        }

        if (fd >= 0 && IS_ENABLED(CONFIG_NET_IPV4)) {
            struct http_request req;

            memset(&req, 0, sizeof(req));

            req.method = HTTP_GET;
            req.url = p->path;
            req.header_fields = head;
            req.host = p->host;
            req.protocol = "HTTP/1.1";
            req.response = response_cb;
            req.recv_buf = recv_buf_ipv4;
            req.recv_buf_len = sizeof(recv_buf_ipv4);

            ret = http_client_req(fd, &req, timeout, "IPv4 GET");
            LOG_INF("ret (socket id) : %d", ret);
            break;
        }
    }
end:
	close(fd);
	freeaddrinfo(res);

	LOG_INF("--- END ---");

    return 0;
}