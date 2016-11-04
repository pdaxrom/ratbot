#ifndef UDP_H
#define UDP_H

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#else
#include <windows.h>
#endif

enum {
    UDP_SERVER = 0,
    UDP_CLIENT
};

typedef struct _udp_forward {
    struct sockaddr_in addr;
    char 	*label;
    int 	used;
    uint32_t	total;
    struct _udp_forward *next;
} udp_forward;

typedef struct _udp_channel {
    struct sockaddr_in my_addr;
    struct sockaddr_in *inp_addr;
    struct sockaddr_in *out_addr;
    int s;
    int mode;
    udp_forward	*forward;
} udp_channel;

#define udp_fd(u) (u->s)

udp_channel *udp_open(int mode, char *addr, int port);
int udp_read(udp_channel *u, uint8_t *buf, size_t len);
int udp_write(udp_channel *u, uint8_t *buf, size_t len);
int udp_close(udp_channel *u);

int udp_read_src(udp_channel *u, uint8_t *buf, size_t len);
void udp_commit_dst(udp_channel *u);

int udp_forward_add(udp_channel *u, char *label);
int udp_forward_write(udp_channel *u, char *label, uint8_t *buf, size_t len);
void udp_forward_show(udp_channel *u);
void udp_forward_remove_inactive(udp_channel *u);

#endif
