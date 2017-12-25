#ifndef _UART_H
#define _UART_H

#include <stdio.h>      
#include <stdlib.h>
#include <unistd.h>     /*Unix funtion*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*file control*/
#include <termios.h>    /*PPSIX end control */
#include <errno.h>      /*error*/
#include <getopt.h>
#include <string.h>
#include <stdlib.h> 

#define FALSE 1
#define TRUE 0

void set_speed(int fd, int speed);         //set bun
int set_Parity(int fd,int databits,int stopbits,int parity);   //
int acm_open(char *str);
//int acm_send(int fd, char *buff, int len);
//int acm_recv(int fd, char *buff, int len);
//int acm_close(int fd);

#endif
