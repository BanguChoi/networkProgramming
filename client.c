#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);

int main(int argc, char *argv[]){
	int sock;
	pid_t pid;
	char buf[BUF_SIZE+1];
	struct sockaddr_in serv_adr;
				
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
							
	sock=socket(PF_INET, SOCK_STREAM, 0); 
	if(sock == -1)
		error_handling("socket() error!");
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
														
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		printf("[Connected to Server...]\n");
	
	pid=fork();
	if(pid==0)
		write_routine(sock, buf);
	else 
		read_routine(sock, buf);

	close(sock);
	return 0;
}

void read_routine(int sock, char *buf)
{
	while(1)
	{
		int str_len = read(sock, buf, BUF_SIZE);
		if(str_len == 0)
		{
			return;
		}
	
		buf[str_len]= 0;
		printf("Message from server: %s", buf);
	}
}

void write_routine(int sock, char *buf)
{
	while(1){	
		//char msg[BUF_SIZE];
		
		// char opmsg[BUF_SIZE + 1];
		
		//fputs("-> ", stdout);
		
		//scanf("%s", msg);
		
		//fgets(msg, BUF_SIZE, stdin);
		fgets(buf, BUF_SIZE, stdin);

		if(!strcmp(buf, "quit\n"))
		{	
			shutdown(sock, SHUT_WR);
			return;
		}
		
		//opmsg[0] = (char)strlen(msg);
		//strcat(opmsg, msg);
		//strcpy(buf, opmsg);
		
		//int msg_len = strlen(msg);
		//sprintf(buf, "%d %s", msg_len, msg);
		
		//printf("buf : %s \n", buf);
		//write(sock, buf, strlen(buf) + 1);	// 문자열 끝의 NULL문자 포함
		
		//write(sock, buf, msg_len + 2);
		write(sock, buf, strlen(buf));
	}
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
