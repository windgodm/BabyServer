/* 
 * csapp.c - Functions for the CS:APP3e book
 *
 * Updated 10/2016 reb:
 *   - Fixed bug in sio_ltoa that didn't cover negative numbers
 *
 * Updated 2/2016 droh:
 *   - Updated open_clientfd and open_listenfd to fail more gracefully
 *
 * Updated 8/2014 droh: 
 *   - New versions of open_clientfd and open_listenfd are reentrant and
 *     protocol independent.
 *
 *   - Added protocol-independent inet_ntop and inet_pton functions. The
 *     inet_ntoa and inet_aton functions are obsolete.
 *
 * Updated 7/2014 droh:
 *   - Aded reentrant sio (signal-safe I/O) routines
 * 
 * Updated 4/2013 droh: 
 *   - rio_readlineb: fixed edge case bug
 *   - rio_readnb: removed redundant EINTR check
 */

#include "csapp_netp_win.h"

/************************** 
 * Debug
 **************************/

void displayAddrinfo(struct addrinfo* p) {
    printf("addrinfo\n");
    // family
    printf(" family = ");
    switch(p->ai_family){
        case 2:  printf("AF_INET 2   // internetwork: UDP, TCP, etc.\n"); break;
        case 23: printf("AF_INET6 23 // Internetwork Version 6\n"); break;
        default: printf("p->ai_family\n");
    }
    // sockettype
    printf(" ai_socktype = %d\n", p->ai_socktype);
    // protocol
    printf(" ai_protocol = %d\n", p->ai_protocol);
    // ip
    char str[INET6_ADDRSTRLEN];
    struct in_addr* ai_addr = (struct in_addr*)&p->ai_addr->sa_data[2];
    //unsigned char*  = (char*)p->ai_addr + 4;
    if(p->ai_family != 23) {
        //printf(" ai_addr = %u.%u.%u.%u\n", ai_addr[0], ai_addr[1], ai_addr[2], ai_addr[3]);
        inet_ntop(AF_INET, ai_addr, str, INET_ADDRSTRLEN);
    } else {
        //printf(" ai_addr = ");
        //for(int i = 0; i < 16; i++)
        //    printf("%u.", ai_addr[i]);
        //printf("\n");
        inet_ntop(AF_INET6, ai_addr, str, INET6_ADDRSTRLEN);
    }
    printf(" ai_addr = %s\n", str);
}

/************************** 
 * Error-handling functions
 **************************/

void windows_error(char *msg) /* Windows-style error */
{
    char buffer[64];
    strerror_s(buffer, sizeof(buffer), errno);
    fprintf(stderr, "%s: strerror(%d)=%s\n", msg, errno, buffer);
    exit(0);
}

/****************************************
 * The Rio package - Robust I/O functions
 ****************************************/

/*
 * rio_readn - Robustly recv n bytes (unbuffered)
 */
size_t rio_readn(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    size_t nread;
    char *bufp = (char*)usrbuf;

    while (nleft > 0) {
	if ((nread = recv(fd, bufp, (int)nleft, 0)) < 0) {
	    if (errno == EINTR) /* Interrupted by sig handler return */
		nread = 0;      /* and call recv() again */
	    else
		return -1;      /* errno set by recv() */ 
	} 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* Return >= 0 */
}

/*
 * rio_writen - Robustly send n bytes (unbuffered)
 */
size_t rio_writen(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    size_t nwritten;
    char *bufp = (char* )usrbuf;

    while (nleft > 0) {
        if ((nwritten = send(fd, bufp, (int)nleft, 0)) <= 0) {
            if (errno == EINTR)  /* Interrupted by sig handler return */
                nwritten = 0;    /* and call send() again */
            else
                return -1;       /* errno set by send() */
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}


/* 
 * rio_read - This is a wrapper for the recv() function that
 *    transfers min(n, rio_cnt) bytes from an internal buffer to a user
 *    buffer, where n is the number of bytes requested by the user and
 *    rio_cnt is the number of unread bytes in the internal buffer. On
 *    entry, rio_read() refills the internal buffer via a call to
 *    fread() if the internal buffer is empty.
 */
static size_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    size_t cnt;

    while (rp->rio_cnt <= 0) {  /* Refill if buf is empty */
	rp->rio_cnt = recv(rp->rio_fd, rp->rio_buf, 
			   sizeof(rp->rio_buf), 0);
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR) /* Interrupted by sig handler return */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else 
	    rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += (int)cnt;
    rp->rio_cnt -= (int)cnt;
    return cnt;
}

/*
 * rio_readinitb - Associate a descriptor with a read buffer and reset buffer
 */
void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}

/*
 * rio_readnb - Robustly read n bytes (buffered)
 */
size_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    size_t nread;
    char *bufp = (char*)usrbuf;
    
    while (nleft > 0) {
	if ((nread = rio_read(rp, bufp, nleft)) < 0) 
            return -1;          /* errno set by fread() */ 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

/* 
 * rio_readlineb - Robustly read a text line (buffered)
 */
size_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    size_t n, rc;
    char c, *bufp = (char*)usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = rio_read(rp, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n') {
                n++;
     		break;
            }
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;	  /* Error */
    }
    *bufp = 0;
    return n-1;
}

/**********************************
 * Wrappers for robust I/O routines
 **********************************/

size_t Rio_readn(int fd, void *ptr, size_t nbytes) 
{
    size_t n;
  
    if ((n = rio_readn(fd, ptr, nbytes)) < 0)
	windows_error((char*)"Rio_readn error");
    return n;
}

void Rio_writen(int fd, void *usrbuf, size_t n) 
{
    if (rio_writen(fd, usrbuf, n) != n)
	windows_error((char*)"Rio_writen error");
}

void Rio_readinitb(rio_t *rp, int fd)
{
    rio_readinitb(rp, fd);
} 

size_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    size_t rc;

    if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
	windows_error((char*)"Rio_readnb error");
    return rc;
}

size_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    size_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
	windows_error((char*)"Rio_readlineb error");
    return rc;
}

/******************************** 
 * Client/server helper functions
 ********************************/
/*
 * open_clientfd - Open connection to server at <hostname, port> and
 *     return a socket descriptor ready for reading and writing. This
 *     function is reentrant and protocol-independent.
 *
 *     On error, returns: 
 *       -2 for GetAddrInfo error
 *       -1 with errno set for other errors.
 */
SOCKET open_clientfd(char *hostname, char *port) {
    SOCKET clientfd;
    int rc;
    struct addrinfo hints, *listp, *p;
    char buffer[64];

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* ipv4 */
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    //hints.ai_flags |= AI_NUMERICHOST;
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections */
    if ((rc = GetAddrInfoA(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "GetAddrInfo failed (%s:%s) gai_strerror(%d)=%s\n", hostname, port, rc, gai_strerrorA(rc));
        return -2;
    }
  
    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, (int)p->ai_addrlen) != -1) {
            break; /* Success */
        }
        if (closesocket(clientfd) < 0) { /* Connect failed, try another */  //line:netp:openclientfd:closesocketfd
            strerror_s(buffer, sizeof(buffer), errno);
            fprintf(stderr, "open_clientfd: closesocket failed: %s\n", buffer);
            return -1;
        } 
    } 

    /* Clean up */
    FreeAddrInfoA(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}

/*  
 * open_listenfd - Open and return a listening socket on port. This
 *     function is reentrant and protocol-independent.
 *
 *     On error, returns: 
 *       -2 for GetAddrInfo error
 *       -1 with errno set for other errors.
 */
SOCKET open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    SOCKET listenfd;
    int rc, optval = 1;
    char buffer[64];

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* ipv4 */
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    if ((rc = GetAddrInfoA(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "GetAddrInfo failed (port %s) gai_strerror(%d)=%s\n", port, rc, gai_strerrorA(rc));
        return -2;
    }

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        displayAddrinfo(p);

        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;  /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,    //line:netp:csapp:setsockopt
            (char*)&optval , sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, (int)p->ai_addrlen) == 0)
            break; /* Success */
        if (closesocket(listenfd) < 0) { /* Bind failed, try the next */
            strerror_s(buffer, sizeof(buffer), errno);
            fprintf(stderr, "open_listenfd closesocket failed: %s\n", buffer);
            return -1;
        }
    }


    /* Clean up */
    FreeAddrInfoA(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0) {
        closesocket(listenfd);
	return -1;
    }
    return listenfd;
}

/****************************************************
 * Wrappers for reentrant protocol-independent helpers
 ****************************************************/

SOCKET Open_clientfd(char *hostname, char *port) 
{
    SOCKET rc;

    if ((rc = open_clientfd(hostname, port)) < 0)
	    windows_error((char*)"Open_clientfd error");
    return rc;
}

SOCKET Open_listenfd(char *port)
{
    SOCKET rc;

    if ((rc = open_listenfd(port)) < 0)
	    windows_error((char*)"Open_listenfd error");
    return rc;
}
