/*基于fork()系统调用，实现两个生产者和两个消费者的程序
*/
#include "sys/types.h"
#include "sys/file.h"
#include "sys/wait.h"
#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

using namespace std;

char r_buf[4]; /*读缓冲*/
char w_buf[4]; /*写缓冲*/
int pipe_fd[2]; /*pipe_fd[0]是读，pipe_fd[1]是写*/
pid_t pid1, pid2, pid3, pid4;

int producer(int id);
int consumer(int id);

int main(int argc, char **argv)
{
	//创建管道文件
	if(pipe(pipe_fd) < 0){
		printf("pipe create error \n");
		exit(-1);
	}
	else{
		printf("pipe is created successfully! \n");
		
		//创建子进程
		if((pid1 = fork()) == 0)
			producer(1);
		if((pid2 = fork()) == 0)
			producer(2);
		if((pid3 = fork()) == 0)
			consumer(1);
		if((pid4 = fork()) == 0)
			consumer(2);
	}
	
	close(pipe_fd[0]);
	close(pipe_fd[1]);

	int pid, status;
	for(int i=0; i<4; i++)
		pid = wait(&status);
	exit(0);
}

int producer(int id)
{
	printf("producer %d is running! \n", id);
	close(pipe_fd[0]);
	for(int i=1; i<10; i++){
   		sleep(10);
		if(id == 1)
			strcpy(w_buf, "aaa\0");
		else
			strcpy(w_buf, "bbb\0");
		if(write(pipe_fd[1], w_buf, 4) == -1)
			printf("write to pipe error! \n");
		else
			printf("producer %d write to pipe! \n", id);
	}
	close(pipe_fd[1]);
	printf("producer %d is over! \n", id);
	exit(id);
}

int consumer(int id)
{
	close(pipe_fd[1]);
	printf("consumer %d is running! \n", id);
	if(id == 1)
		strcpy(w_buf, "ccc\0");
	else
		strcpy(w_buf, "ddd\0");
	while(1){
  		sleep(1);
		strcpy(r_buf, "eee\0");
		if(read(pipe_fd[0], r_buf, 4) == 0)
			break;
		printf("consumer %d get %s, while the w_buf is %s\n", id, r_buf, w_buf);
	}
	close(pipe_fd[0]);
	printf("consumer %d is over! \n", id);
	exit(id);
}
