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
LOG_MODULE_REGISTER(_net_http_client, LOG_LEVEL_DBG);

#define HTTP_PORT "80"

uint8_t HTTP_HOST[128] = "www.example.com";

#define MAX_RECV_BUF_LEN 2048

static uint8_t recv_buf[MAX_RECV_BUF_LEN];

static int response_cb(struct http_response *rsp,
			enum http_final_call final_data,
			void *user_data)
{
	if (final_data == HTTP_DATA_MORE) {
		LOG_INF("partial data received (%zd bytes)", rsp->data_len);
        LOG_HEXDUMP_DBG(rsp->recv_buf, rsp->data_len, "http recv");
	}
	else if (final_data == HTTP_DATA_FINAL) {
		LOG_INF("all the data received (%zd bytes)", rsp->data_len);
        LOG_HEXDUMP_DBG(rsp->recv_buf, rsp->data_len, "http recv");
	}

	LOG_INF("response status %s", rsp->http_status);

	return 0;
}

int example_http_request(const struct shell *sh, size_t argc, char *argv[])
{
	int fd = -1;
	int32_t timeout = 30 * MSEC_PER_SEC;

    if(argc==2)
	{
		strcpy(HTTP_HOST, argv[1]);
	}
	
	LOG_DBG("request host : %s", HTTP_HOST);
	

    static struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_protocol = IPPROTO_TCP,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res, *index;
	char addr[64];

	LOG_INF("looking up %s", HTTP_HOST);
	int ret = getaddrinfo(HTTP_HOST, HTTP_PORT, &hints, &res);
    if(ret != 0)
    {
        LOG_ERR("getaddrinfo error ret : %d", ret);
        goto end;
    }

	for(index=res;index != NULL; index=index->ai_next)
	{
		LOG_INF("addrinfo @%p : ai_family=%d, ai_socktype=%d, ai_protocol=%d, sa_family=%d, sin_port=%x",
        index, index->ai_family, index->ai_socktype, index->ai_protocol, index->ai_addr->sa_family,
        ((struct sockaddr_in *)index->ai_addr)->sin_port);

		inet_ntop(index->ai_family, &net_sin(index->ai_addr)->sin_addr, addr,
			sizeof(addr));
		LOG_INF("resolved %s (%s)", addr, net_family2str(index->ai_family));

    	fd = socket(index->ai_family, index->ai_socktype, index->ai_protocol);

		if (fd < 0) {
			LOG_ERR("cannot create HTTP connection(%d).", -errno);
			continue;
		}

		struct timeval tv;
		tv.tv_sec = 8;
		tv.tv_usec = 0;

		ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
						&tv, sizeof(tv));
		if (ret < 0) {
			LOG_ERR("failed to set socket receive timeout(%d), ret=%d", -errno, ret);
			close(fd);
			continue;
		}

		ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
						&tv, sizeof(tv));
		if (ret < 0) {
			LOG_ERR("failed to set socket transmit timeout(%d), ret=%d", -errno, ret);
			close(fd);
			continue;
		}

		ret = connect(fd, index->ai_addr, index->ai_addrlen);
		if (ret < 0) {
			LOG_ERR("can't connect to remote host. ret : %d", -errno);
			close(fd);
			continue;
		}

	
		struct http_request req;

		memset(&req, 0, sizeof(req));

		req.method = HTTP_GET;
		req.url = "/";
		req.host = HTTP_HOST;
		req.protocol = "HTTP/1.1";
		req.response = response_cb;
		req.recv_buf = recv_buf;
		req.recv_buf_len = sizeof(recv_buf);

		ret = http_client_req(fd, &req, timeout, NULL);

		shutdown(fd, SHUT_RDWR);
		break;
	}
end:
	close(fd);
	freeaddrinfo(res);

    return 0;
}