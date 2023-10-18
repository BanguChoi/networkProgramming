#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

int clnt_cnt = 0;		// 현재 접속한 clnt 숫자
int clnt_socks[MAX_CLNT];	// 공유 데이터
pthread_mutex_t mutx;		// mutex의 참조값 저장을 위해 전역변수 선언, 스레드에서 사용됨

int main(int argc, char * argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	
	if(argc != 2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");
		
	while(1){
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		// 새로운 연결 형성때마다 clnt_cnt, clnt_socks 배열에 해당 정보 등록
		clnt_socks[clnt_cnt++] = clnt_sock;	//임계영역 처리해주는 공유영역
		pthread_mutex_unlock(&mutx);
		
		// 추가된 clnt에게 서비스 제공하기위한 thread 생성
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		// 종료된 thread가 메모리에서 완전히 소멸
		pthread_detach(t_id);
		printf("Connected client IP : %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg){
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE];
	
	while((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
		send_msg(msg, str_len);
	
	// 종료된 clnt socket을 list에서 제거
	// 다른 thread socket 추가/삭제 방지
	pthread_mutex_lock(&mutx);
	for(i = 0; i<clnt_cnt; i++){	// remove disconnected client
		if(clnt_sock == clnt_socks[i]){
			while(i++<clnt_cnt-1)
				clnt_socks[i] = clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);

	close(clnt_sock);
	return NULL;
}
// 연결된 모든 clnt에게 메시지 전송
void send_msg(char * msg, int len){	// send to all
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0;i<clnt_cnt;i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

void error_handling(char * msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
