//以 下是客户端的connect模块，与网络socket编程不同的是，
//UNIX Domain Socket客户端一般要显式调用bind函数，而不依赖系统自动分配的地址。
//客户端bind一个自己指定的socket文件名的好处是，
//该文件名可以包 含客户端的pid以便服务器区分不同的客户端。
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#define CLI_PATH    "client_"      /* +5 for pid = 14 chars */
#define SERVER_SOCKET_FILE "Server_File"
/*
* Create a client endpoint and connect to a server.
* Returns fd if all OK, <0 on error.
*/
int client_connect(const char *name)
{
	int                fd, len, err, rval;
	struct sockaddr_un un;
	/* create a UNIX domain stream socket */
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		return(-1);
	/* fill socket address structure with our address */
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%s%05d", CLI_PATH, getpid());
	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	unlink(un.sun_path);        /* in case it already exists */
	if (bind(fd, (struct sockaddr *)&un, len) < 0) 
	{
		rval = -2;
		goto errout;
	}
	/* fill socket address structure with server's address */
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	if (connect(fd, (struct sockaddr *)&un, len) < 0) 
	{
		rval = -4;
		goto errout;
	}
	return(fd);
errout:
	err = errno;
	close(fd);
	errno = err;
	return(rval);
}

int main(void)
{
    int client_fd = -1;
    char wBuffer[] = "Hello UDS, I'm Client!";
    char rBuffer[128] = {0};
    int wBufLen = strlen(wBuffer);
    client_fd = client_connect(SERVER_SOCKET_FILE);
    int byteRead = read(client_fd, (void *)rBuffer, 128);
    printf("Client:byteRead=%d, rBuffer=%s\n", byteRead, rBuffer);
    int bytesWrite = write(client_fd, (void *)wBuffer, wBufLen);
    printf("Client:bytesWrite=%d, wBufLen=%d\n", bytesWrite, wBufLen);
    return 0;
}