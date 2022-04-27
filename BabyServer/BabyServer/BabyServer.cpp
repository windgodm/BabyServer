#include <iostream>
#include "csapp_netp_win.h"
#include "BabyHttp.h"

using std::cout;
using std::endl;

char pszClientHostname[MAXLINE], pszClientPort[MAXLINE];
char buffer[MAXLINE];
BabyHttp babyHttp;

void displayRecv(SOCKET connfd) {
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buffer, MAXLINE);
    printf("[recv] [start]%s[end]\n", buffer);
    //babyHttp.connfd = connfd;
    //babyHttp.ProcessRequest(buffer);
}

void ProcessRecv(SOCKET connfd) {
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buffer, MAXLINE);
    babyHttp.connfd = connfd;
    babyHttp.ProcessRequest(buffer);
}

int main(int argc, char **argv)
{
    WSADATA wsaData;
    int nResult = -1;
    char *pszPort;
    char pszDefaultPort[] = "4000";
    SOCKET listenfd = 0, connfd;
    size_t clientlen;
    SOCKADDR_STORAGE_LH clientAddr;

    // init WSA

    nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (nResult != 0) {
        printf("usage: %s <port>\n", argv[0]);
        printf("WSAStartup failed: %d\n", nResult);
        return 1;
    }

    // open server

    if (argc != 2) {
        cout << "Using default port(4000)" << endl;
        pszPort = pszDefaultPort;
    }
    else {
        pszPort = argv[1];
    }
    printf("[listen] port = %s\n", pszPort);

    // socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // bind
    SOCKADDR_IN local_addr = { 0, };
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(4000);
    if (bind(listenfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1)
        return 0;
    // listen
    listen(listenfd, LISTENQ);

    //listenfd = Open_listenfd(pszPort);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (SOCKADDR*)&clientAddr, (int*)&clientlen);
        GetNameInfoA((SOCKADDR*)&clientAddr, clientlen, pszClientHostname, MAXLINE,
            pszClientPort, MAXLINE, 0);
        printf("[Connect] (%s : %s)\n", pszClientHostname, pszClientPort);
        //displayRecv(connfd);
        ProcessRecv(connfd);
        closesocket(connfd);
    }

    // release

    WSACleanup();
}
