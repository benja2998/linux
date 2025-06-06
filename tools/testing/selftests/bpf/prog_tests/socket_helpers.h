/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __SOCKET_HELPERS__
#define __SOCKET_HELPERS__

#include <sys/un.h>
#include <linux/vm_sockets.h>

/* include/linux/net.h */
#define SOCK_TYPE_MASK 0xf

#define IO_TIMEOUT_SEC 30
#define MAX_STRERR_LEN 256

/* workaround for older vm_sockets.h */
#ifndef VMADDR_CID_LOCAL
#define VMADDR_CID_LOCAL 1
#endif

/* include/linux/cleanup.h */
#define __get_and_null(p, nullvalue)                                           \
	({                                                                     \
		__auto_type __ptr = &(p);                                      \
		__auto_type __val = *__ptr;                                    \
		*__ptr = nullvalue;                                            \
		__val;                                                         \
	})

#define take_fd(fd) __get_and_null(fd, -EBADF)

/* Wrappers that fail the test on error and report it. */

#define _FAIL(errnum, fmt...)                                                  \
	({                                                                     \
		error_at_line(0, (errnum), __func__, __LINE__, fmt);           \
		CHECK_FAIL(true);                                              \
	})
#define FAIL(fmt...) _FAIL(0, fmt)
#define FAIL_ERRNO(fmt...) _FAIL(errno, fmt)
#define FAIL_LIBBPF(err, msg)                                                  \
	({                                                                     \
		char __buf[MAX_STRERR_LEN];                                    \
		libbpf_strerror((err), __buf, sizeof(__buf));                  \
		FAIL("%s: %s", (msg), __buf);                                  \
	})


#define xaccept_nonblock(fd, addr, len)                                        \
	({                                                                     \
		int __ret =                                                    \
			accept_timeout((fd), (addr), (len), IO_TIMEOUT_SEC);   \
		if (__ret == -1)                                               \
			FAIL_ERRNO("accept");                                  \
		__ret;                                                         \
	})

#define xbind(fd, addr, len)                                                   \
	({                                                                     \
		int __ret = bind((fd), (addr), (len));                         \
		if (__ret == -1)                                               \
			FAIL_ERRNO("bind");                                    \
		__ret;                                                         \
	})

#define xclose(fd)                                                             \
	({                                                                     \
		int __ret = close((fd));                                       \
		if (__ret == -1)                                               \
			FAIL_ERRNO("close");                                   \
		__ret;                                                         \
	})

#define xconnect(fd, addr, len)                                                \
	({                                                                     \
		int __ret = connect((fd), (addr), (len));                      \
		if (__ret == -1)                                               \
			FAIL_ERRNO("connect");                                 \
		__ret;                                                         \
	})

#define xgetsockname(fd, addr, len)                                            \
	({                                                                     \
		int __ret = getsockname((fd), (addr), (len));                  \
		if (__ret == -1)                                               \
			FAIL_ERRNO("getsockname");                             \
		__ret;                                                         \
	})

#define xgetsockopt(fd, level, name, val, len)                                 \
	({                                                                     \
		int __ret = getsockopt((fd), (level), (name), (val), (len));   \
		if (__ret == -1)                                               \
			FAIL_ERRNO("getsockopt(" #name ")");                   \
		__ret;                                                         \
	})

#define xlisten(fd, backlog)                                                   \
	({                                                                     \
		int __ret = listen((fd), (backlog));                           \
		if (__ret == -1)                                               \
			FAIL_ERRNO("listen");                                  \
		__ret;                                                         \
	})

#define xsetsockopt(fd, level, name, val, len)                                 \
	({                                                                     \
		int __ret = setsockopt((fd), (level), (name), (val), (len));   \
		if (__ret == -1)                                               \
			FAIL_ERRNO("setsockopt(" #name ")");                   \
		__ret;                                                         \
	})

#define xsend(fd, buf, len, flags)                                             \
	({                                                                     \
		ssize_t __ret = send((fd), (buf), (len), (flags));             \
		if (__ret == -1)                                               \
			FAIL_ERRNO("send");                                    \
		__ret;                                                         \
	})

#define xrecv_nonblock(fd, buf, len, flags)                                    \
	({                                                                     \
		ssize_t __ret = recv_timeout((fd), (buf), (len), (flags),      \
					     IO_TIMEOUT_SEC);                  \
		if (__ret == -1)                                               \
			FAIL_ERRNO("recv");                                    \
		__ret;                                                         \
	})

#define xsocket(family, sotype, flags)                                         \
	({                                                                     \
		int __ret = socket(family, sotype, flags);                     \
		if (__ret == -1)                                               \
			FAIL_ERRNO("socket");                                  \
		__ret;                                                         \
	})

static inline void close_fd(int *fd)
{
	if (*fd >= 0)
		xclose(*fd);
}

#define __close_fd __attribute__((cleanup(close_fd)))

static inline struct sockaddr *sockaddr(struct sockaddr_storage *ss)
{
	return (struct sockaddr *)ss;
}

static inline void init_addr_loopback4(struct sockaddr_storage *ss,
				       socklen_t *len)
{
	struct sockaddr_in *addr4 = memset(ss, 0, sizeof(*ss));

	addr4->sin_family = AF_INET;
	addr4->sin_port = 0;
	addr4->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	*len = sizeof(*addr4);
}

static inline void init_addr_loopback6(struct sockaddr_storage *ss,
				       socklen_t *len)
{
	struct sockaddr_in6 *addr6 = memset(ss, 0, sizeof(*ss));

	addr6->sin6_family = AF_INET6;
	addr6->sin6_port = 0;
	addr6->sin6_addr = in6addr_loopback;
	*len = sizeof(*addr6);
}

static inline void init_addr_loopback_unix(struct sockaddr_storage *ss,
					   socklen_t *len)
{
	struct sockaddr_un *addr = memset(ss, 0, sizeof(*ss));

	addr->sun_family = AF_UNIX;
	*len = sizeof(sa_family_t);
}

static inline void init_addr_loopback_vsock(struct sockaddr_storage *ss,
					    socklen_t *len)
{
	struct sockaddr_vm *addr = memset(ss, 0, sizeof(*ss));

	addr->svm_family = AF_VSOCK;
	addr->svm_port = VMADDR_PORT_ANY;
	addr->svm_cid = VMADDR_CID_LOCAL;
	*len = sizeof(*addr);
}

static inline void init_addr_loopback(int family, struct sockaddr_storage *ss,
				      socklen_t *len)
{
	switch (family) {
	case AF_INET:
		init_addr_loopback4(ss, len);
		return;
	case AF_INET6:
		init_addr_loopback6(ss, len);
		return;
	case AF_UNIX:
		init_addr_loopback_unix(ss, len);
		return;
	case AF_VSOCK:
		init_addr_loopback_vsock(ss, len);
		return;
	default:
		FAIL("unsupported address family %d", family);
	}
}

static inline int enable_reuseport(int s, int progfd)
{
	int err, one = 1;

	err = xsetsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
	if (err)
		return -1;
	err = xsetsockopt(s, SOL_SOCKET, SO_ATTACH_REUSEPORT_EBPF, &progfd,
			  sizeof(progfd));
	if (err)
		return -1;

	return 0;
}

static inline int socket_loopback_reuseport(int family, int sotype, int progfd)
{
	struct sockaddr_storage addr;
	socklen_t len = 0;
	int err, s;

	init_addr_loopback(family, &addr, &len);

	s = xsocket(family, sotype, 0);
	if (s == -1)
		return -1;

	if (progfd >= 0)
		enable_reuseport(s, progfd);

	err = xbind(s, sockaddr(&addr), len);
	if (err)
		goto close;

	if (sotype & SOCK_DGRAM)
		return s;

	err = xlisten(s, SOMAXCONN);
	if (err)
		goto close;

	return s;
close:
	xclose(s);
	return -1;
}

static inline int socket_loopback(int family, int sotype)
{
	return socket_loopback_reuseport(family, sotype, -1);
}

static inline int poll_connect(int fd, unsigned int timeout_sec)
{
	struct timeval timeout = { .tv_sec = timeout_sec };
	fd_set wfds;
	int r, eval;
	socklen_t esize = sizeof(eval);

	FD_ZERO(&wfds);
	FD_SET(fd, &wfds);

	r = select(fd + 1, NULL, &wfds, NULL, &timeout);
	if (r == 0)
		errno = ETIME;
	if (r != 1)
		return -1;

	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &eval, &esize) < 0)
		return -1;
	if (eval != 0) {
		errno = eval;
		return -1;
	}

	return 0;
}

static inline int poll_read(int fd, unsigned int timeout_sec)
{
	struct timeval timeout = { .tv_sec = timeout_sec };
	fd_set rfds;
	int r;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	r = select(fd + 1, &rfds, NULL, NULL, &timeout);
	if (r == 0)
		errno = ETIME;

	return r == 1 ? 0 : -1;
}

static inline int accept_timeout(int fd, struct sockaddr *addr, socklen_t *len,
				 unsigned int timeout_sec)
{
	if (poll_read(fd, timeout_sec))
		return -1;

	return accept(fd, addr, len);
}

static inline int recv_timeout(int fd, void *buf, size_t len, int flags,
			       unsigned int timeout_sec)
{
	if (poll_read(fd, timeout_sec))
		return -1;

	return recv(fd, buf, len, flags);
}


static inline int create_pair(int family, int sotype, int *p0, int *p1)
{
	__close_fd int s, c = -1, p = -1;
	struct sockaddr_storage addr;
	socklen_t len;
	int err;

	s = socket_loopback(family, sotype);
	if (s < 0)
		return s;

	c = xsocket(family, sotype, 0);
	if (c < 0)
		return c;

	init_addr_loopback(family, &addr, &len);
	err = xbind(c, sockaddr(&addr), len);
	if (err)
		return err;

	len = sizeof(addr);
	err = xgetsockname(s, sockaddr(&addr), &len);
	if (err)
		return err;

	err = connect(c, sockaddr(&addr), len);
	if (err) {
		if (errno != EINPROGRESS) {
			FAIL_ERRNO("connect");
			return err;
		}

		err = poll_connect(c, IO_TIMEOUT_SEC);
		if (err) {
			FAIL_ERRNO("poll_connect");
			return err;
		}
	}

	switch (sotype & SOCK_TYPE_MASK) {
	case SOCK_DGRAM:
		err = xgetsockname(c, sockaddr(&addr), &len);
		if (err)
			return err;

		err = xconnect(s, sockaddr(&addr), len);
		if (err)
			return err;

		*p0 = take_fd(s);
		break;
	case SOCK_STREAM:
	case SOCK_SEQPACKET:
		p = xaccept_nonblock(s, NULL, NULL);
		if (p < 0)
			return p;

		*p0 = take_fd(p);
		break;
	default:
		FAIL("Unsupported socket type %#x", sotype);
		return -EOPNOTSUPP;
	}

	*p1 = take_fd(c);
	return 0;
}

static inline int create_socket_pairs(int family, int sotype, int *c0, int *c1,
				      int *p0, int *p1)
{
	int err;

	err = create_pair(family, sotype, c0, p0);
	if (err)
		return err;

	err = create_pair(family, sotype, c1, p1);
	if (err) {
		close(*c0);
		close(*p0);
	}

	return err;
}

static inline const char *socket_kind_to_str(int sock_fd)
{
	socklen_t opt_len;
	int domain, type;

	opt_len = sizeof(domain);
	if (getsockopt(sock_fd, SOL_SOCKET, SO_DOMAIN, &domain, &opt_len))
		FAIL_ERRNO("getsockopt(SO_DOMAIN)");

	opt_len = sizeof(type);
	if (getsockopt(sock_fd, SOL_SOCKET, SO_TYPE, &type, &opt_len))
		FAIL_ERRNO("getsockopt(SO_TYPE)");

	switch (domain) {
	case AF_INET:
		switch (type) {
		case SOCK_STREAM:
			return "tcp4";
		case SOCK_DGRAM:
			return "udp4";
		}
		break;
	case AF_INET6:
		switch (type) {
		case SOCK_STREAM:
			return "tcp6";
		case SOCK_DGRAM:
			return "udp6";
		}
		break;
	case AF_UNIX:
		switch (type) {
		case SOCK_STREAM:
			return "u_str";
		case SOCK_DGRAM:
			return "u_dgr";
		case SOCK_SEQPACKET:
			return "u_seq";
		}
		break;
	case AF_VSOCK:
		switch (type) {
		case SOCK_STREAM:
			return "v_str";
		case SOCK_DGRAM:
			return "v_dgr";
		case SOCK_SEQPACKET:
			return "v_seq";
		}
		break;
	}

	return "???";
}

#endif // __SOCKET_HELPERS__
