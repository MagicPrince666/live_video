#include "uart.h"
#include <fcntl.h>      /*file control*/
#include <signal.h>
#include <pthread.h> 
#include <iostream>
#include <unistd.h>

#define FIFO_NAME "/tmp/my_fifo"
#define BUF_SIZE 128

using namespace std;

int run = 1;
int fd_uart1 = -1,fd_uart2 = -1;

static void sigint_handler(int sig)
{
    run = 0;
    cout << "--- quit the loop! ---" << endl;
}

void * uart_send (void *arg) 
{
    while(run)
    {
        //write(fd_uart1, "hello!,I'm uart1\n",17);	
        //write(fd_uart2, "hello!,I'm uart2\n",17);	
        sleep(1);
    }
    pthread_exit(NULL);
}

void * uart1_rev (void *arg) 
{
    char buff[128] = {0};
    int count = 0; 
    int pipe_fd;
    int res;

    unlink(FIFO_NAME);//删除原来的
    printf("unlink %s\n",FIFO_NAME);

    if (access(FIFO_NAME, F_OK) == -1) {
        res = mkfifo(FIFO_NAME, 0766);
        if (res != 0) {
            printf("cant\'t creat pipe file %s\n", FIFO_NAME);
            exit(1);
        }
    }
    printf("PID %d open pipe O_WRONLY\n", getpid());
    pipe_fd = open(FIFO_NAME, O_WRONLY);

    while(run)
    {
        count = read(fd_uart1, buff, 128);
        if(count)
        {        
            cout << "uart1 received " << count << " buf" << endl;
            cout << "buf:" << buff << endl;

            if (pipe_fd != -1) { //将收到数据写入管道
                res = write(pipe_fd, buff, strlen(buff));
                printf("buffer num %d \n", res);   
            } 

            memset(buff,0,128);
        }
        sleep(1);
    }
    
    if(pipe_fd > 0)
        close(pipe_fd);
    pthread_exit(NULL);
}

void * uart2_rev (void *arg) 
{
    char buff[128] = {0};
    int count = 0;
    while(run)
    {
        count = read(fd_uart2, buff, 128);
        if(count)
        {       
            cout << "uart2 received " << count << " buf" << endl;
            cout << "buf:" << buff << endl;
            memset(buff,0,128);
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

int main(int argc,char **argv)
{
    pthread_t pthread_id[3];//线程ID

    //signal(SIGINT, sigint_handler);//信号处理

    fd_uart1 = acm_open((char *)"/dev/ttyS1");
    if(fd_uart1 >0)
    cout << "open ttyS1 suceess" << endl;
    else
    {
        cout << "open ttyS2 error" << endl;
        exit(0);
    }

    fd_uart2 = acm_open((char *)"/dev/ttyS2");
    if(fd_uart2 >0)
    cout << "open ttyS2 suceess" <<endl;
    else
    {
        cout << "open ttyS2 error" << endl;
        exit(0);
    }

    if (pthread_create(&pthread_id[0], NULL, uart_send, NULL))
        cout << "Create uart_send error!" << endl;
    if (pthread_create(&pthread_id[1], NULL, uart1_rev, NULL))
        cout << "Create uart1_rev error!" << endl;    
    if (pthread_create(&pthread_id[2], NULL, uart2_rev, NULL))
        cout << "Create uart2_rev error!" << endl;
    
    if(pthread_id[0] !=0) {                   
        pthread_join(pthread_id[0],NULL);
        cout << "uart_send "<< pthread_id[0]<< " exit!"  << endl;
    }
    if(pthread_id[1] !=0) {                   
        pthread_join(pthread_id[1],NULL);
        cout << "uart1_rev " << pthread_id[1] << " exit!"  << endl;
    }
    if(pthread_id[2] !=0) {                   
        pthread_join(pthread_id[2],NULL);
        cout << "uart2_rev " << pthread_id[2] << " exit!" <<endl;
    }

    if(fd_uart1)close(fd_uart1);
    if(fd_uart2)close(fd_uart2);

    return 0;
}