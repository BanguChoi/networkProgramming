#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void *send_msg(void *arg);	// 전송 스레드 기능
void *recv_msg(void *arg);	// 송신 스레드 기능
void error_handling(char *msg);

char name[NAME_SIZE] = "[DEFAULT]";	// 송신한 사람 이름 저장
char msg[BUF_SIZE];			// 전송할 메시지 내용 저장

int main(int argc, char *argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=4){
		printf("Usage : %s <IP> <port> <name> \n", argv[0]);
		exit(1);
	}

	sprintf(name, "[%s]", argv[3]);
	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

	// 송신 스레드 생성 (send_msg 함수 연결)
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	// 수신 스레드 생성 (recv_msg 함수 연결)
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	// Blocking 함수, 송신 스레드 종료 시 까지 함수 반환 X
	pthread_join(snd_thread, &thread_return);
	// Blocking 함수, 수신 스레드 종료 시 까지 함수 반환 X
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	return 0;
}

void *send_msg(void *arg){	// 송신 스레드 메인
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];	// 송신자 이름과 메시지 내용 저장
	while(1){
		fgets(msg, BUF_SIZE, stdin);
		if(!strcmp(msg, "q\n")||!strcmp(msg,"Q\n")){
			close(sock);
			exit(0);
		}
		sprintf(name_msg, "%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}

void *recv_msg(void *arg){	// 수신 스레드 메인
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];	// 송신자 이름과 메시지 내용 저장
	int str_len;
	while(1){
		str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len == -1)
			return (void*)-1;	// 형변환해서 반환
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
	}
	return NULL;
}

void error_handling(char *msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
