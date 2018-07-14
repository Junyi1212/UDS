#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/stat.h>

#define QLEN 10
#define SERVER_SOCKET_FILE "Server_File"

//以下是服务器的listen模块，与网络socket编程类似，在bind之后要listen，
//表示通过bind的地址（也就是socket文件）提供服务。
/*
* Create a server endpoint of a connection.
* Returns fd if all OK, <0 on error.
*/
int server_listen(const char *name)
{
	int                 fd, len, err, rval;
	struct sockaddr_un  un;
	/* create a UNIX domain stream socket */
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		return(-1);
	unlink(name);   /* in case it already exists */
	/* fill in socket address structure */
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
	/* bind the name to the descriptor */
	if (bind(fd, (struct sockaddr *)&un, len) < 0) 
	{
		rval = -2;
		goto errout;
	}
	if (listen(fd, QLEN) < 0) 
	{ /* tell kernel we're a server */
		rval = -3;
		goto errout;
	}
	return(fd);
errout:
	err = errno;
	close(fd);
	errno = err;
	return(rval);
}

//以 下是服务器的accept模块，通过accept得到客户端地址也应该是一个socket文件，
//如果不是socket文件就返回错误码，如果是 socket文件，
//在建立连接后这个文件就没有用了，调用unlink把它删掉，通过传出参数uidptr返回客户端程序的user id。
int server_accept(int listenfd, uid_t *uidptr)
{
	int                 clifd, len, err, rval;
	time_t              staletime;
	struct sockaddr_un  un;
	struct stat         statbuf;
	len = sizeof(un);
	if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0)
		return(-1);     /* often errno=EINTR, if signal caught */
	/* obtain the client's uid from its calling address */
	len -= offsetof(struct sockaddr_un, sun_path); /* len of pathname */
	un.sun_path[len] = 0;           /* null terminate */
	if (stat(un.sun_path, &statbuf) < 0)
	{
		rval = -2;
		goto errout;
	}
	if (S_ISSOCK(statbuf.st_mode) == 0)
	{
		rval = -3;      /* not a socket */
		goto errout;
	}
	if (uidptr != NULL)
		*uidptr = statbuf.st_uid;   /* return uid of caller */
	unlink(un.sun_path);        /* we're done with pathname now */
	return(clifd);
errout:
	err = errno;
	close(clifd);
	errno = err;
	return(rval);
}

int main(void)
{
    int server_fd = -1;
    int client_fd = -1;
    char wBuffer[] = "Hello UDS, I'm Server!";
    char rBuffer[128] = {0};
    int wBufLen = strlen(wBuffer);
    server_fd = server_listen(SERVER_SOCKET_FILE);
    client_fd = server_accept(server_fd, NULL);
    int bytesWrite = write(client_fd, (void *)wBuffer, wBufLen);
    printf("Server:bytesWrite=%d, wBufLen=%d\n", bytesWrite, wBufLen);
    int byteRead = read(client_fd, (void *)rBuffer, 128);
    printf("Server:byteRead=%d, rBuffer=%s\n", byteRead, rBuffer);
    return 0;
}