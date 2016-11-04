#ifndef TCP_H
#define TCP_H

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#else
#include <windows.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>

enum {
    TCP_SERVER = 0,
    TCP_SSL_SERVER,
    TCP_CLIENT,
    TCP_SSL_CLIENT
};

typedef struct _tcp_channel {
    int s;
    struct sockaddr_in my_addr;
    int mode;
    SSL *ssl;
    SSL_CTX *ctx;
} tcp_channel;

#define tcp_fd(u) (u->s)

tcp_channel *tcp_open(int mode, const char *addr, int port);
tcp_channel *tcp_accept(tcp_channel *u);
int tcp_read(tcp_channel *u, char *buf, size_t len);
int tcp_write(tcp_channel *u, char *buf, size_t len);
int tcp_close(tcp_channel *u);

#endif
