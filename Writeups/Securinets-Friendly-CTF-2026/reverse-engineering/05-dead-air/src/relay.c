#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "notice.h"
#include "payload.h"

#define RELAY_PORT "8080"

static volatile unsigned int TOKEN_SEED = 0x2b84f16du;
static volatile unsigned int HOST_SEED = 0x6d02be47u;
static volatile unsigned int CODE_SPAN = 64u;

static unsigned int digest(const char *k)
{
    unsigned int h = 0x811c9dc5u;
    while (*k) {
        h ^= (unsigned char)*k++;
        h *= 0x01000193u;
    }
    return h ? h : 0x9e3779b9u;
}

static void __attribute__((noinline)) unwrap(unsigned int x, const unsigned char *in,
                                             unsigned int n, char *out)
{
    for (unsigned int i = 0; i < n; i++) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        out[i] = (char)(in[i] ^ (x & 0xffu));
    }
    out[n] = '\0';
}

static unsigned int __attribute__((noinline)) code_key(void)
{
    const unsigned char *p = (const unsigned char *)(const void *)&unwrap;
    unsigned int h = 0x811c9dc5u;
    unsigned int n = CODE_SPAN;

    for (unsigned int i = 0; i < n; i++) {
        h ^= p[i];
        h *= 0x01000193u;
    }
    return h;
}

static int reach(int fd, const struct sockaddr *addr, socklen_t len)
{
    struct timeval tv;
    fd_set w;
    int flags;
    int err = 0;
    socklen_t elen = sizeof err;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        return connect(fd, addr, len);

    if (connect(fd, addr, len) != 0) {
        if (errno != EINPROGRESS)
            return -1;
        FD_ZERO(&w);
        FD_SET(fd, &w);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        if (select(fd + 1, NULL, &w, NULL, &tv) != 1)
            return -1;
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &elen) != 0 || err != 0)
            return -1;
    }

    fcntl(fd, F_SETFL, flags);
    return 0;
}

static int dial(const char *host)
{
    struct addrinfo hints;
    struct addrinfo *list = NULL;
    struct addrinfo *it;
    struct timeval tv;
    int fd = -1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, RELAY_PORT, &hints, &list) != 0)
        return -2;

    for (it = list; it != NULL; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0)
            continue;
        if (reach(fd, it->ai_addr, it->ai_addrlen) == 0)
            break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(list);

    if (fd < 0)
        return -3;

    tv.tv_sec = 30;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    return fd;
}

static int checkin(const char *host)
{
    char token[64];
    char request[512];
    char reply[4096];
    char note[192];
    ssize_t got;
    size_t used = 0;
    int fd;

    fd = dial(host);
    if (fd < 0)
        return -fd;

    unwrap(TOKEN_SEED ^ code_key(), TOKEN, TOKEN_LEN, token);

    snprintf(request, sizeof request,
             "GET /relay/checkin HTTP/1.1\r\n"
             "Host: %s:%s\r\n"
             "User-Agent: meridian-relay/4.2.1\r\n"
             "X-Relay-Expect: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             host, RELAY_PORT, token);

    if (write(fd, request, strlen(request)) < 0) {
        close(fd);
        return 4;
    }

    while (used + 1 < sizeof reply) {
        got = read(fd, reply + used, sizeof reply - used - 1);
        if (got <= 0)
            break;
        used += (size_t)got;
    }
    reply[used] = '\0';
    close(fd);

    if (used == 0)
        return 4;

    if (strstr(reply, token) == NULL)
        return 5;

    unwrap(digest(token), SEALED, SEALED_LEN, note);
    puts(note);
    return 0;
}

int main(void)
{
    char host[128];
    int rc;

#ifdef DESIGNER
    const char *override = getenv("RELAY_HOST");
    if (override != NULL && *override != '\0')
        snprintf(host, sizeof host, "%s", override);
    else
        unwrap(HOST_SEED ^ code_key(), HOSTNAME, HOSTNAME_LEN, host);
#else
    unwrap(HOST_SEED ^ code_key(), HOSTNAME, HOSTNAME_LEN, host);
#endif


    rc = checkin(host);
    switch (rc) {
    case 0:
        return 0;
    case 2:
        puts("the hostname does not resolve. find out what it is, and point it at your machine.");
        break;
    case 3:
        puts("I'm speaking, but no one is hearing me !!");
        break;
    case 4:
        puts("You heard me but you didn't reply !!");
        break;
    case 5:
        puts("That's not what I wanted to hear !");
        break;
    default:
        puts("check-in failed.");
        break;
    }
    return rc;
}
