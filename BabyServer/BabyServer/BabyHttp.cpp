#include "BabyHttp.h"

BabyHttp::BabyHttp() {
	connfd = 0;
	RequestGet = 0;
}

void BabyHttp::ProcessRequest(char* message) {
	enum HTTP_FUNC func = HTTP_FUNC::DEFAULT;

	// get
	if (message[0] == 'G' && message[1] == 'E' && message[2] == 'T') {
		printf("[GET]\n");
		func = HTTP_FUNC::GET;
	}

	if (func != HTTP_FUNC::GET) {
		DefaultRequestBad(connfd);
		return;
	}

	if (RequestGet) {
		RequestGet(connfd);
	}
	else {
		DefaultRequestGet(connfd);
	}
}

void BabyHttp::DefaultRequestBad(SOCKET connfd) const {
	char message[] = "\
HTTP/1.1 200 OK\r\n\
\r\n\
<html>\r\n\
Please use GET\r\n\
</html>\r\n\
";
	Rio_writen(connfd, message, sizeof(message));
}

void BabyHttp::DefaultRequestGet(SOCKET connfd) const {
	char message[] = "\
HTTP/1.1 200 OK\r\n\
\r\n\
<html>\r\n\
Hello world!!!\r\n\
</html>\r\n\
";
	
	Rio_writen(connfd, message, sizeof(message));
}
