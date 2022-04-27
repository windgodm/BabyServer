#pragma once
#include "csapp_netp_win.h"

typedef void (*ProcRequestFunc)(SOCKET connfd);

enum HTTP_FUNC
{
	DEFAULT,
	GET
};

class BabyHttp
{
public:
	SOCKET connfd;
	ProcRequestFunc RequestGet;
public:
	BabyHttp();
	void ProcessRequest(char* message);
	void DefaultRequestBad(SOCKET connfd) const;
	void DefaultRequestGet(SOCKET connfd) const;
};
