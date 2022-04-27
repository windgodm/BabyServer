/*
 * csapp.h - prototypes and definitions for the CS:APP3e book
 */
/* $begin csapp.h */
#ifndef __CSAPP_H__
#define __CSAPP_H__

#include<stdio.h>
#include"winsock2.h"
#include"WS2tcpip.h"
#pragma comment(lib, "ws2_32.lib")

//
//  Flags used in "hints" argument to getaddrinfo()
//      - AI_ADDRCONFIG is supported starting with Vista
//      - default is AI_ADDRCONFIG ON whether the flag is set or not
//        because the performance penalty in not having ADDRCONFIG in
//        the multi-protocol stack environment is severe;
//        this defaulting may be disabled by specifying the AI_ALL flag,
//        in that case AI_ADDRCONFIG must be EXPLICITLY specified to
//        enable ADDRCONFIG behavior
//

#define AI_PASSIVE                  0x00000001  // Socket address will be used in bind() call
#define AI_CANONNAME                0x00000002  // Return canonical name in first ai_canonname
#define AI_NUMERICHOST              0x00000004  // Nodename must be a numeric address string
#define AI_NUMERICSERV              0x00000008  // Servicename must be a numeric port number
#define AI_DNS_ONLY                 0x00000010  // Restrict queries to unicast DNS only (no LLMNR, netbios, etc.)

#define AI_ALL                      0x00000100  // Query both IP6 and IP4 with AI_V4MAPPED
#define AI_ADDRCONFIG               0x00000400  // Resolution only if global address configured
#define AI_V4MAPPED                 0x00000800  // On v6 failure, query v4 and convert to V4MAPPED format

#define AI_NON_AUTHORITATIVE        0x00004000  // LUP_NON_AUTHORITATIVE
#define AI_SECURE                   0x00008000  // LUP_SECURE
#define AI_RETURN_PREFERRED_NAMES   0x00010000  // LUP_RETURN_PREFERRED_NAMES

#define AI_FQDN                     0x00020000  // Return the FQDN in ai_canonname
#define AI_FILESERVER               0x00040000  // Resolving fileserver name resolution
#define AI_DISABLE_IDN_ENCODING     0x00080000  // Disable Internationalized Domain Names handling
#define AI_EXTENDED                 0x80000000      // Indicates this is extended ADDRINFOEX(2/..) struct
#define AI_RESOLUTION_HANDLE        0x40000000  // Request resolution handle

/* Persistent state for the robust I/O (Rio) package */
/* $begin rio_t */
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */
    int rio_cnt;               /* Unread bytes in internal buf */
    char *rio_bufptr;          /* Next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;
/* $end rio_t */

/* Misc constants */
#define	MAXLINE	 8192  /* Max text line length */
#define MAXBUF   8192  /* Max I/O buffer size */
#define LISTENQ  1024  /* Second argument to listen() */

/* Our own error-handling functions */
void windows_error(char *msg);

/* Rio (Robust I/O) package */
size_t rio_readn(int fd, void *usrbuf, size_t n);
size_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd); 
size_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);
size_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* Wrappers for Rio package */
size_t Rio_readn(int fd, void *usrbuf, size_t n);
void Rio_writen(int fd, void *usrbuf, size_t n);
void Rio_readinitb(rio_t *rp, int fd); 
size_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n);
size_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* Reentrant protocol-independent client/server helpers */
SOCKET open_clientfd(char *hostname, char *port);
SOCKET open_listenfd(char *port);

/* Wrappers for reentrant protocol-independent client/server helpers */
SOCKET Open_clientfd(char *hostname, char *port);
SOCKET Open_listenfd(char *port);

#endif /* __CSAPP_H__ */
/* $end csapp.h */
