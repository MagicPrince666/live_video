#include "com.h"

// int fd;//文件描述符
// int epid; //epoll标识符
// struct epoll_event event;
// struct epoll_event events[6];//事件集合

int Serial::openSerial(char *cSerialName)
{
    int iFd;

    struct termios opt; 

    iFd = open(cSerialName, O_RDWR | O_NOCTTY |O_NONBLOCK);
	//iFd = open(cSerialName, O_RDWR | O_NOCTTY | O_NDELAY);//阻塞 |O_RSYNC
    if(iFd < 0) {
        perror(cSerialName);
        return -1;
    }

    tcgetattr(iFd, &opt);      

    cfsetispeed(&opt, B9600);
    cfsetospeed(&opt, B9600);

    
    /*
     * raw mode
     */
    opt.c_lflag   &=   ~(ECHO   |   ICANON   |   IEXTEN   |   ISIG);
    opt.c_iflag   &=   ~(BRKINT   |   ICRNL   |   INPCK   |   ISTRIP   |   IXON);
    opt.c_oflag   &=   ~(OPOST);
    opt.c_cflag   &=   ~(CSIZE   |   PARENB);
    opt.c_cflag   |=   CS8;

    /*
     * 'DATA_LEN' bytes can be read by serial
     */
    opt.c_cc[VMIN]   =   DATA_LEN;                                      
    opt.c_cc[VTIME]  =   150;

    if (tcsetattr(iFd,   TCSANOW,   &opt)<0) {
        return   -1;
    }


    return iFd;
}


int Serial::EpollInit(int cfd)
{
	epid = epoll_create(6);//可处理的最大句柄数6个
	
	event.events = EPOLLET | EPOLLIN;
	event.data.fd = cfd;
	if (epoll_ctl(epid, EPOLL_CTL_ADD, cfd, &event) != 0) {//将读事件添加到epoll的事件队列中
		printf("set epoll error!\n");
		return 0;
	}
	printf("set epoll ok!\n");
	
	return 1;
}

int Serial::ComRead(char * ReadBuff,const int ReadLen)
{
	//int read_len = 0;
	int len = 0;

	//下面开始epoll等待
	int i =0,witeNum= 0;
	while (1) 
	{
		witeNum = epoll_wait(epid, events, 1, 50);
		printf("witeNum0 = %d\n   ", witeNum);
		if( witeNum == 0)
				return 0;
		//printf("witeNum = %d\n", witeNum);
		
		for (i = 0; i < witeNum; i++) 
		{

			if ((events[i].events & EPOLLERR)
					|| (events[i].events & EPOLLHUP)
					|| (!(events[i].events & EPOLLIN))) 
			{
				printf("no data!\n");
				break;
			} 
			else if (events[i].events & EPOLLIN) 
			{//有数据进入  接受数据
				len = read(events[i].data.fd, ReadBuff, ReadLen);
				tcdrain(fd);
			    tcflush(fd,TCIOFLUSH);

				if(len != ReadLen) //如何保证每次都读到这些字节又不阻塞！ 
				{
					bzero(ReadBuff,15);
				}
				
				if( len == ReadLen)
				 return len;
		 
			}
		}
	}

	return len ;
}

