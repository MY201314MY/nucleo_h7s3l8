#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/http/client.h>
#include <zephyr/shell/shell.h>
#include <zephyr/net/sntp.h>
#include <time.h>
#include <netdb.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(net_sntp_client_sample, LOG_LEVEL_DBG);

#define SNTP_PORT "123"

uint8_t SNTP_HOST[128] = "ntp.aliyun.com";

#define MAX_RECV_BUF_LEN 2048

int example_sntp_request(const struct shell *sh, size_t argc, char *argv[])
{
    static struct addrinfo hints;
	struct addrinfo *res;
	int ret = getaddrinfo(SNTP_HOST, SNTP_PORT, &hints, &res);
    if(ret<0)
    {
        LOG_ERR("getaddrinfo error ret : %d", ret);
        return -1;
    }
    LOG_INF("addrinfo @%p: ai_family=%d, ai_socktype=%d, ai_protocol=%d, sa_family=%d, sin_port=%x\n",
        res, res->ai_family, res->ai_socktype, res->ai_protocol, res->ai_addr->sa_family,
        ((struct sockaddr_in *)res->ai_addr)->sin_port);

    struct sntp_ctx ctx;
    struct sntp_time sntp_time;
    ret = sntp_init(&ctx, res->ai_addr, res->ai_addrlen);
    if (ret < 0) {
		LOG_ERR("failed to init sntp IPv4 ctx: %d", ret);
		goto end;
	}

    LOG_INF("sending sntp IPv4 request...");
	ret = sntp_query(&ctx, 4 * MSEC_PER_SEC, &sntp_time);
	if (ret < 0) {
		LOG_ERR("sntp IPv4 request failed: %d", ret);
		goto end;
	}

	LOG_INF("status: %d", ret);
	LOG_INF("time since epoch: high word: %u, low word: %u",
	    (uint32_t)(sntp_time.seconds >> 32), (uint32_t)sntp_time.seconds);
    
    {
        time_t timestamp = (time_t) sntp_time.seconds + 8*3600; // UTC+8

        struct tm *p = gmtime(&timestamp);

        LOG_INF("%04d/%02d/%02d %02d:%02d:%02d", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,
                        (p->tm_hour), p->tm_min, p->tm_sec);
    }

end:
    freeaddrinfo(res);
	sntp_close(&ctx);

    return 0;
}
