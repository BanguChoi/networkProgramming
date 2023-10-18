#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define OPSZ 4
#define RLT_SIZE 4

void error_handling(char *message);

int main(int argc, char *argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	char opmsg[BUF_SIZE], retry; // [(1)opnd_cnt][(4)clnt_answer][(4)clnt_answer]...
	int result[4];
	int opnd_cnt, i, try_cnt = 0;
	// int str_len, recv_len, recv_cnt;

	if(argc!=3){
		printf("Usage : %s <IP><port>\n", argv[0]);
		exit(1);
	}

	

		
	
	while(1)
	{
		sock = socket(PF_INET, SOCK_STREAM, 0);
		if(sock == -1)
			error_handling("socket() error");

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
		serv_addr.sin_port = htons(atoi(argv[2]));
		
		if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
			error_handling("connect() error");
		
		printf("Welcome!\nThis is a number guessing game.\n");
		// Print IP:Port Number.
		printf("(%s:%d)\n\n",inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
		// Reset try_count and result value.
		try_cnt = 0;
		memset(result,0,sizeof(result));
		
		// Replay Range
		while(1)
		{
			// Ask answer count.
			fputs("# of your answers?> ",stdout);
			scanf("%d", &opnd_cnt);
		
			try_cnt += opnd_cnt;
			opmsg[0] = (char)opnd_cnt;
			// Input guessing answers until opnd_cnt.
			for(i = 0 ; i < opnd_cnt ; i++)
			{
				printf("Answer%d> ", i+1);
				scanf("%d", (int*)&opmsg[i*OPSZ+1]);
			}
		
			// Send opmsg to Server.
			write(sock, opmsg, opnd_cnt*OPSZ+1);
			
			memset(result,0,sizeof(result));
		
			// Receive result data from Server.
			// Receive answer.
			read(sock, &result, RLT_SIZE*4);	// result size = 16 (4*4)
			printf("\n[Reply from Server]\n");
			printf("Up: %d, Down: %d, Hit: %d \n", result[1], result[2], result[3]);
			if(result[3]!=0)
				break;
		}
		// Replay Range
		printf("You've got the right answer : %d \n", result[0]);
		printf("Your score is 100/%d = %d \n", try_cnt, (100/try_cnt));
		try_cnt = 0;
			
		printf("Try again? (y/n) ");
		scanf(" %c", &retry);
		if(retry == 'n')
			break;	
	}
	
	close(sock);
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
