#define __LIBRARY__
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<linux/sem.h>

#define M 530           /*打出数字总数*/
#define N 5             /*消费者进程数*/
#define BUFSIZE 10      /*缓冲区大小*/

_syscall2(sem_t*,sem_open,const char *,name,unsigned int,value); 
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char *,name);

int main()
{
    sem_t *empty, *full, *mutex;    /*3个信号量*/
    int fd;     /*共享缓冲区文件描述符*/
    int i,j,k,child;
    int data;
    pid_t pid;
    int buf_out = 0; /*从缓冲区读取位置*/
    int buf_in = 0;  /*写入缓冲区位置*/

    if ((mutex = sem_open("mnutex",1)) == NULL) {
        perror("sem_open() error!\n");
        return -1;
    }

    if ((empty = sem_open("empty",10)) == NULL) {
        perror("sem_open() error!\n");
        return -1;
    }

    if ((full = sem_open("full",0))== NULL) {
        perror("sem_open() error!\n");
        return -1;
    }

    fd = open("buffer.txt", O_CREAT|O_TRUNC|O_RDWR,0666);
    lseek(fd, BUFSIZE*sizeof(int),SEEK_SET);
    write(fd,(char*)&buf_out,sizeof(int));

    /*生产者进程*/
    if ((pid=fork())==0)
    {
        printf("I'm producer. pid= %d\n",getpid());
        for ( i = 0; i < M; i++)
        {
            sem_wait(empty);
            sem_wait(mutex);

            lseek(fd,buf_in*sizeof(int), SEEK_SET);
            write(fd,(char*)&i,sizeof(int));
            buf_in = (buf_in + 1) % BUFSIZE;

            sem_post(mutex);
            sem_post(full);
        }
        printf("producer end.\n");
        fflush(stdout);
        return 0;
    }
    else if(pid < 0)
    {
        perror("Fail to fork!\n");
        return -1;
    }

    for(j = 0 ; j < N ; j++)
    {
        if ((pid=fork()) == 0)
        {
            for (k = 0; k < M/N; k++)
            {
                sem_wait(full);
                sem_wait(mutex);

                lseek(fd,BUFSIZE*sizeof(int),SEEK_SET);
                read(fd,(char*)&buf_out,sizeof(int));

                lseek(fd,buf_out*sizeof(int),SEEK_SET);
                read(fd,(char *)&data,sizeof(int));

                buf_out = (buf_out + 1) % BUFSIZE;
                lseek(fd, BUFSIZE*sizeof(int),SEEK_SET);
                write(fd,(char*)&buf_out,sizeof(int));

                sem_post(mutex);
                sem_post(empty);

                printf("%d:  %d\n",getpid(),data);
                fflush(stdout);
            }
            printf("child-%d: pid = %d end.\n",j , getpid());
            return 0;
        }
        else if(pid < 0)
        {
            perror("Fail to fork!\n");
            return -1;
        }
    }

    child = N + 1;
    while(child--)
        wait(NULL);
    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");

    close(fd);
    return 0;
}