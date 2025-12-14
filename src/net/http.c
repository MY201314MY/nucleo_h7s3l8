#include <netdb.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>
#include <zephyr/shell/shell.h>
#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/device.h>
#include <zephyr/net/net_if.h>
#include <zephyr/posix/netinet/in.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/poll.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(net_https_client, LOG_LEVEL_DBG);

#define HTTP_PORT "80"

uint8_t HTTP_HOST[128] = "www.example.com";

#define MAX_RECV_BUF_LEN 2048

static uint8_t recv_buf[MAX_RECV_BUF_LEN];

static int response_cb(struct http_response *rsp,
			enum http_final_call final_data,
			void *user_data)
{
	if (final_data == HTTP_DATA_MORE) {
		LOG_INF("Partial data received (%zd bytes)", rsp->data_len);
        LOG_HEXDUMP_DBG(rsp->recv_buf, rsp->data_len, "http recv");
	} else if (final_data == HTTP_DATA_FINAL) {
		LOG_INF("All the data received (%zd bytes)", rsp->data_len);
        LOG_HEXDUMP_DBG(rsp->recv_buf, rsp->data_len, "http recv");
	}

	LOG_INF("Response to %s", (const char *)user_data);
	LOG_INF("Response status %s", rsp->http_status);

	return 0;
}

extern int example_http_request(const struct shell *sh, size_t argc, char *argv[])
{
	int fd = -1;
	int32_t timeout = 30 * MSEC_PER_SEC;

    if(argc==2)
	{
		strcpy(HTTP_HOST, argv[1]);
	}
	
	LOG_DBG("request host : %s", HTTP_HOST);
	

    static struct addrinfo hints = {
		.ai_flags = AI_NUMERICSERV,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	char peer_addr[INET_ADDRSTRLEN];

	printk("Looking up %s\n", HTTP_HOST);
	int ret = getaddrinfo(HTTP_HOST, HTTP_PORT, &hints, &res);
    if(ret<0)
    {
        LOG_ERR("getaddrinfo error ret : %d", ret);
        return -1;
    }
    LOG_INF("addrinfo @%p : ai_family=%d, ai_socktype=%d, ai_protocol=%d, sa_family=%d, sin_port=%x\n",
        res, res->ai_family, res->ai_socktype, res->ai_protocol, res->ai_addr->sa_family,
        ((struct sockaddr_in *)res->ai_addr)->sin_port);

	inet_ntop(res->ai_family, &((struct sockaddr_in *)(res->ai_addr))->sin_addr, peer_addr,
		  INET_ADDRSTRLEN);
	LOG_INF("Resolved %s (%s)\n", peer_addr, net_family2str(res->ai_family));

    fd = socket(res->ai_family, SOCK_STREAM, res->ai_protocol);

	if (fd < 0) {
		LOG_ERR("Cannot create HTTP connection(%d).", -errno);
		return -ECONNABORTED;
	}

    ret = connect(fd, res->ai_addr, res->ai_addrlen);
	if (ret < 0) {
		LOG_ERR("can't connect to remote host. ret : %d", -errno);
		return -ETIMEDOUT;
	}

	if (fd >= 0) {
		struct http_request req;

		memset(&req, 0, sizeof(req));

		req.method = HTTP_GET;
		req.url = "/";
		req.host = HTTP_HOST;
		req.protocol = "HTTP/1.1";
		req.response = response_cb;
		req.recv_buf = recv_buf;
		req.recv_buf_len = sizeof(recv_buf);

		ret = http_client_req(fd, &req, timeout, "HTTP GET");

		close(fd);
	}

	freeaddrinfo(res);

    return 0;
}