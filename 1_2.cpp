/*基于clone()的系统调用，实现两个生产者和两个消费者
*/
#include "unistd.h"
#include "sched.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "semaphore.h"
#include "string.h"

int producer(void *args);
int consumer(void *args);

pthread_mutex_t mutex;
sem_t product;
sem_t warehouse;
char buffer[8][4];
int buf_out = 0; /*消费者取出数据的索引*/
int buf_in = 0; /*生产者放数据的索引*/
int bp = 0;

int main(int argc, char** argv)
{
	pthread_mutex_init(&mutex, NULL);
	sem_init(&product, 0, 0);
	sem_init(&warehouse, 0, 8);
	int clone_flag, arg, retval;
	char* stack;
	clone_flag = CLONE_VM|CLONE_SIGHAND|CLONE_FS|CLONE_FILES;
	for(int i=0; i<2; ++i){
		/*创建4个线程*/
		arg = i;
		stack = (char*)malloc(4096);
		retval = clone(producer, &(stack[4095]), clone_flag, (void*)&arg);
		stack = (char*)malloc(4096);
		retval = clone(consumer, &(stack[4095]), clone_flag, (void*)&arg);
		sleep(1);
	}
	exit(1);
}

int producer(void* args)
{
	int id = *((int*)args);
	for(int i=0; i<10; ++i){
		sleep(i+1);
		sem_wait(&warehouse);
		pthread_mutex_lock(&mutex);
		/*if(id == 0)
			strcpy(buffer[buf_in], "aaa\0");
		else
			strcpy(buffer[buf_in], "bbb\0");
		printf("producer%d produce %s in %d\n", id,buffer[buf_in],buf_in);
		buf_in = (buf_in+1)%8;*/
		if(id == 0)
			strcpy(buffer[bp],"aaa\0");
		else
			strcpy(buffer[bp],"bbb\0");
		bp++;
        printf("producer%d produce %s in %d\n", id,buffer[bp-1],bp-1);

		pthread_mutex_unlock(&mutex);
		sem_post(&product);
	}
	printf("producer%d is over!\n", id);
}

int consumer(void* args)
{
	int id = *((int*)args);
	for(int i=0; i<10; ++i){
		sleep(10-i);
		sem_wait(&product);
		pthread_mutex_lock(&mutex);
		/*printf("consumer%d get %s in %d\n", id,buffer[buf_out],buf_out);
		buf_out = (buf_out+1)%8;*/
		bp--;
		printf("consumer%d get %s in %d\n", id,buffer[bp],bp);

		pthread_mutex_unlock(&mutex);
		sem_post(&warehouse);
	}
	printf("consumer%d is over!\n", id);
}
