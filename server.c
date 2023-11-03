#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void read_childproc(int sig);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
				
	pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len, state;
	char buf[BUF_SIZE];
	
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	//signal handling
	
	act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD, &act, 0);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1){
		adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
		if(clnt_sock == -1)
			continue;
		else
			puts("[Client connected..]");
											
		pid = fork();
		if(pid == -1)
		{
			close(clnt_sock);
			continue;
		}
		/*
		if(pid == 0)
		{
			close(serv_sock);
			
			//preview
			//while((str_len=read(clnt_sock, buf, BUF_SIZE))!=0)
			//{
			//	write(clnt_sock, buf, str_len);
			//}
			//preview
			
			// read_routine(clnt_sock, buf);
			// write_routine(clnt_sock, buf);
			while(1)
			{
				write_routine(clnt_sock, buf);
				read_routine(clnt_sock, buf);
			}
			close(clnt_sock);
			puts("client disconnected...");
			return 0;
		}
		else
			close(clnt_sock);		
		*/
		while(1)
		{
			if(pid == 0)
			{
				write_routine(clnt_sock, buf);

			}
			else
			{
				read_routine(clnt_sock, buf);
			}

			close(clnt_sock);
			puts("client disconnected...");
			//return 0;
			break;
		}
	}
	close(serv_sock);
	return 0;
}

void read_childproc(int sig) {
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n", pid);
}

// read process
void read_routine(int sock, char *buf)
{
	while(1)
	{
		// preview
		// int msg_len;
		// int str_len = read(sock, &msg_len, sizeof(int));	// 메시지 길이 읽기
		// printf("%d\n",str_len);
		
		int str_len = read(sock, buf, BUF_SIZE);

		if(str_len == 0)
		{
			return;
		}
		
		// str_len = read(sock, buf, msg_len);	// 메시지 읽기
		//if(str_len == 0)
		//	return;

		buf[str_len] = 0;
		printf("Message from Client: %s", buf);
	}
}

// write process
void write_routine(int sock, char *buf)
{
	while(1)
	{	
		//fputs("-> ", stdout);
		fgets(buf, BUF_SIZE, stdin);
		
		/*
		if(!strcmp(buf,"quit\n"))
		{	
			shutdown(sock, SHUT_WR);
			return;
		}
		*/
		//printf("write_buf : %s \n", buf);
		write(sock, buf, strlen(buf));
	}
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}