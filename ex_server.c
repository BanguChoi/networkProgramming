#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define OPSZ 4

void compare_answer(int opnum, int opnds[], int answer, int *ud_cnt);
void error_handling(char *message);

int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	char opinfo[BUF_SIZE];
	
	//static int result[3];
	int result[4];	// [1byte:answer][4byte:up_cnt][4byte:down_cnt][4byte:hit_cnt]
	static int ud_cnt[3];	// up,down,hit count
	
	int opnd_cnt, i, rand_num;
	int recv_cnt, recv_len, send_len;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size;
	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error!");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error!");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error!");
	clnt_addr_size = sizeof(clnt_addr);
	
	
	for(i=0;i<5;i++)
	{	
		opnd_cnt = 0;
		rand_num = rand() % 100;
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if(clnt_sock == -1)
			error_handling("accept() error!");
		printf("Client %d connected.. \n", i+1);
		printf("A random number generated : %d \n", rand_num);		

		memset(ud_cnt, 0, sizeof(ud_cnt));
		result[0] = rand_num;
		
		// Repeat until answer
		while(ud_cnt[2]==0)
		{
			read(clnt_sock, &opnd_cnt, 1);
			
			// Print client's answers
			read(clnt_sock, &opinfo[0], BUF_SIZE-1);
			printf("Client %d\'s answers : ", i+1);
			recv_len = 0;
			while(opnd_cnt*OPSZ > recv_len){
				printf("%d ", (int)opinfo[recv_len]);
				recv_len += OPSZ;
			}
			printf("\n");
		
			// Reset result value.
			memset(ud_cnt, 0, sizeof(ud_cnt));
			
			// Compare answer and Print result value.
			compare_answer(opnd_cnt, (int*)opinfo, rand_num, ud_cnt);
			printf("Reply : Up %d, Down %d, Hit %d \n", ud_cnt[0], ud_cnt[1], ud_cnt[2]);

			// Input result value in result[]. 
			// [4byte:answer][4byte:up_cnt][4byte:down_cnt][4byte:hit_cnt]
			send_len = 0;	// sending result length
			while(send_len<3){
				result[send_len+1] = ud_cnt[send_len];
				send_len++;
			}
			//write(clnt_sock, result, sizeof(result));
			write(clnt_sock, result, 4*OPSZ);
		}
		// Repeat until answer
		printf("Client%d left...\n", i+1);
		close(clnt_sock);
	}
	close(serv_sock);
	
	return 0;
}

void compare_answer(int opnum, int opnds[], int answer, int *ud_cnt)
{
	for(int i=0; i<opnum; i++){
		if(answer<opnds[i])
			ud_cnt[1]++;
		else if(answer>opnds[i])
			ud_cnt[0]++;
		else if(answer==opnds[i])
			ud_cnt[2]++;
	}
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
