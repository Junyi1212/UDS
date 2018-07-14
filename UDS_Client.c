//�� ���ǿͻ��˵�connectģ�飬������socket��̲�ͬ���ǣ�
//UNIX Domain Socket�ͻ���һ��Ҫ��ʽ����bind��������������ϵͳ�Զ�����ĵ�ַ��
//�ͻ���bindһ���Լ�ָ����socket�ļ����ĺô��ǣ�
//���ļ������԰� ���ͻ��˵�pid�Ա���������ֲ�ͬ�Ŀͻ��ˡ�
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