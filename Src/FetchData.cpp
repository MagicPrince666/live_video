#include "FetchData.hh"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "CONSTANT.h"
#include "cbuf.h"
#include "Tiam335xH264Source.hh"
#include "v4l2uvc.h"
#include "h264_xu_ctrls.h"
//#include "Queue.h"

//#define SOFT_H264

# define __DBGFUNS
# ifdef __DBGFUNS
#   define DBGFUNS(fmt,args...) printf(fmt,  ##args)
# else
#   define DBGFUNS(fmt,args...)
# endif

struct H264Format *gH264fmt = NULL;

int Dbg_Param = 0x1f;

#define CLEAR(x) memset (&(x), 0, sizeof (x))
int errnoexit(const char *s)
{
	printf("%s error %d, %s", s, errno, strerror (errno));
	return -1;
}
int xioctl(int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
};

#ifndef SOFT_H264

//#define V4L2_PIX_FMT_H264 v4l2_fourcc('H','2','6','4') /* H264 */





struct v4l2_buffer buf0;
struct buffer {
	void *         start;
	size_t         length;
};

static char            dev_name[16];
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;

struct vdIn *vd;

struct tm *tdate;
time_t curdate;


int open_device(int i)
{
	printf("-------------- open_device-------------------\n");

	struct stat st;
	
	strcpy(dev_name, getVideoDevice());

	if (-1 == stat (dev_name, &st))
	{
		printf("Cannot identify '%s': %d, %s\n", dev_name, errno, strerror (errno));
		for(i ; i < 5 ; i++)
		{
			sprintf(dev_name,"/dev/video%d",i);
			printf("try %s\n",dev_name);
			if (-1 != stat (dev_name, &st)) 
			{
				setVideoDevice(dev_name);
				break;
			}
		}
		if(i == 5)
		{
			printf("Check your camera\n");
			return -1;
		}
	}

	if (!S_ISCHR (st.st_mode))
	{
		printf("%s is no device", dev_name);
		return -1;
	}
	vd = (struct vdIn *) calloc(1, sizeof(struct vdIn));
	printf("Use %s\n",dev_name);
	vd->fd = open(dev_name, O_RDWR);
	printf("vd->fd 11 : %d", vd->fd);

	if (0 >= vd->fd)
	{
		printf("Cannot open '%s': %d, %s", dev_name, errno, strerror (errno));
		return -1;
	}
	return 0;
}

int init_device(int width, int height,int format)
{
	printf("--------------- init_device------------------\n");

	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl (vd->fd, VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			printf("%s is no V4L2 device", dev_name);
			return -1;
		}
		else
		{
			return errnoexit ("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		printf("%s is no video capture device", dev_name);
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("%s does not support streaming i/o", dev_name);
		return -1;
	}

	CLEAR (cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl (vd->fd, VIDIOC_CROPCAP, &cropcap))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; 

		if (-1 == xioctl (vd->fd, VIDIOC_S_CROP, &crop))
		{
			switch (errno)
			{
				case EINVAL:
					break;
				default:
					break;
			}
		}
	}

	CLEAR (fmt);
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = format;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	if (-1 == xioctl (vd->fd, VIDIOC_S_FMT, &fmt))
		return errnoexit ("VIDIOC_S_FMT");

	min = fmt.fmt.pix.width * 2;

	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	struct v4l2_streamparm parm;
	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(vd->fd, VIDIOC_G_PARM, &parm);
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = getFps();
	//parm.parm.capture.timeperframe.denominator = 30; 
	ioctl(vd->fd, VIDIOC_S_PARM, &parm);


	int init_mmap(void);
	return init_mmap ();

}

int SUPPORTED_BUFFER_NUMBER = 4;

int init_mmap(void)
{

	printf("---------------init_mmap------------------\n");
		perror("-init_mmap ");

	struct v4l2_requestbuffers req;

	CLEAR (req);
	req.count               = SUPPORTED_BUFFER_NUMBER;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (vd->fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			printf("%s does not support memory mapping", dev_name);
			return -1;
		}
		else
		{
			return errnoexit ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2)
	{
		printf("Insufficient buffer memory on %s", dev_name);
		return -1;
 	}

	buffers = (buffer*) calloc (req.count, sizeof (*buffers));
	SUPPORTED_BUFFER_NUMBER = req.count;
		printf("SUPPORTED_BUFFER_NUMBER %d\n",SUPPORTED_BUFFER_NUMBER);


	if (!buffers)
	{
		printf("Out of memory");
		return -1;
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
	{
		struct v4l2_buffer buf;
		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl (vd->fd, VIDIOC_QUERYBUF, &buf))
			return errnoexit ("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
		mmap (NULL ,
			buf.length,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			vd->fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
			return errnoexit ("mmap");

	}
	
	return 0;
}

int start_previewing(void)
{

	printf("--------------start_previewing-------------------\n");
	perror("-start_previewing ");

	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i)
	{
		struct v4l2_buffer buf;
		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if (-1 == xioctl (vd->fd, VIDIOC_QBUF, &buf))
			return errnoexit ("VIDIOC_QBUF");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (vd->fd, VIDIOC_STREAMON, &type))
		return errnoexit ("VIDIOC_STREAMON");

	return 0;
}

int cameraInit()
{
	printf("-cameraInit 1\n");
	
	int width = 1280; 
	int height = 720;
		
	int format = V4L2_PIX_FMT_H264;
	printf("-cameraInit 2\n");
	
	int ret;
	ret = open_device(1);

	printf("-cameraInit 3 ret:%d\n", ret);

	if(ret != -1)
	{
		printf("------open_device--success-- !\n ");
		ret = init_device(width,height,format);
	}
	if(ret != -1)
	{
		printf("------init_device---success------- !\n ");
		ret = start_previewing();
	}

	if(ret != -1)
	{
		printf("---start_previewing------success------- !\n ");
	}

	if(ret != -1)
	{
		time(&curdate);  
		tdate = localtime (&curdate);
		printf("XU_OSD_Set_RTC_ curdate tm_year:%d \n",tdate->tm_year);
		XU_OSD_Set_CarcamCtrl(vd->fd, (unsigned char)0, (unsigned char)0, (unsigned char)0);
		if(XU_OSD_Set_RTC(vd->fd, tdate->tm_year + 1900, tdate->tm_mon + 1,tdate->tm_mday, tdate->tm_hour, tdate->tm_min, tdate->tm_sec) <0)
			printf("XU_OSD_Set_RTC_fd = %d Failed\n",fd);
		if(XU_OSD_Set_Enable(vd->fd, 1, 1) <0)
			printf(" XU_OSD_Set_Enable_fd = %d Failed\n",fd);
	}

	if(ret != -1)
	{
		ret = XU_Init_Ctrl(vd->fd);
		if(ret<0)
		{
				printf("XU_H264_Set_BitRate Failed\n");	
			
		} else
		{
				double m_BitRate = 0.0;
				
				if(XU_H264_Set_BitRate(vd->fd, getBitRate()) < 0 )
				{
					printf("XU_H264_Set_BitRate Failed\n");
				}

				XU_H264_Get_BitRate(vd->fd, &m_BitRate);
				if(m_BitRate < 0 )
				{
					printf("XU_H264_Get_BitRate Failed\n");
				}
				printf("------------XU_H264_Set_BitRate ok------m_BitRate:%f----\n", m_BitRate);			
		}
	}
	return ret;
}

FILE* rec_fp1 = NULL;

struct v4l2_buffer __buf;

void cameraUninit(void)
{
  printf("TSS cameraUninit 110219-1\n");

  if(!buffers) return;//已经释放，直接返回

  printf("TSS cameraUninit 110219-2\n");
  
	for (n_buffers = 0; n_buffers < SUPPORTED_BUFFER_NUMBER; ++n_buffers)
	{
  		printf("TSS cameraUninit 110219-3\n");
		
		if(buffers[n_buffers].start!=NULL)
		{   
  			printf("TSS cameraUninit 110219-4\n");
			
			if(-1==munmap(buffers[n_buffers].start,buffers[n_buffers].length))
			{
				perror("Fail to \"munmap\"\n");
			}
		}else
		{   
			DBGFUNS("__buffers[%d].start=0__\n",n_buffers);
		}   
	}

	// 释放申请的存储空间
	if(buffers)
	{
  		printf("TSS cameraUninit 110219-5\n");
		free(buffers);
		buffers=NULL;
	}

	if(vd)
	{
		perror("TSS  Uinit 6");
		
		if(vd->fd > 0)
		{
			int r = close(vd->fd);
			printf("Uinit 1-2  close(vd->fd) r:%d\n", r);
			
			vd->fd = -1;
		}
		printf("3 Uinit 2\n");
		
		int r1 = close_v4l2(vd);

		printf("Uinit 2-1  close_v4l2(vd); r1:%d \n",r1);

		vd = NULL;
	}
	printf("Uinit END 3\n");
	
};

// Queue<Mediadata*> sWorkDataQueue;
const int QUEUE_LEN_MAX = 4;
const int QUEUE_SIMPLE_UNIT_SIZE = 100000;
void Init()
{
	void Uinit();
   	Uinit();

  	cameraInit();
#if 0
	if(rec_fp1 == NULL)
	{
		char rec_filename[] = "RecordH264.264";	
		rec_fp1 = fopen(rec_filename, "wb");
	}
#endif
}



int Uinit()
{	
	perror("TSS  Uinit 1");
	cameraUninit();
	perror("TSS  Uinit 2");
	
	perror("TSS  Uinit 3");
	
	if(rec_fp1)
	{
		fclose(rec_fp1);
		rec_fp1 = NULL;
	}

	perror("TSS  Uinit 5");
	
	return 0;
};

void* FetchData::s_source = NULL;
bool s_quit = true; 

bool emptyBuffer = false;
char t_buf[204800];
void *go(void *arg)
{
	printf("GOOOOOOOOOOOOOOOOOooo s_quit:%d\n", s_quit);
    while(!s_quit)
	{
		printf("FetchDatavvd go 1 ");
		
		if(vd != NULL && vd->fd != NULL)
		{
			printf("FetchDatavvd go 2 ");
			
			break;
		}else
		{
			printf("FetchDatavvd go 2 ");
			
			Init();
			sleep(1);
		}
	}

   while(!s_quit)
   {
		if(vd == NULL || vd->fd == NULL)
		{
			printf("test FCCC 4 \n");
			
			return NULL;
		}
	  
    	CLEAR (__buf);
		__buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		__buf.memory = V4L2_MEMORY_MMAP;
		
		int ret = ioctl(vd->fd, VIDIOC_DQBUF, &__buf);
		
		if (ret < 0) 
		{   
			printf("go() FetchData Unable to dequeue buffer ret:%d!\n", ret);
			cameraUninit();
			if(-1 == cameraInit())
				exit(0);
			else
			sleep(1);
            continue;
		}
		
		int l = cbuf_enqueue(&FetchData::data, buffers[__buf.index].start, __buf.bytesused);

		//fwrite(buffers[__buf.index].start, __buf.bytesused, 1, rec_fp1);

		ret = ioctl(vd->fd, VIDIOC_QBUF, &__buf);

		if (ret < 0) 
		{
			printf("Unable to requeue buffer\n");
		}
   }
}

int FetchData::bit_rate_setting(int rate)
{
	printf("-----start seting bit rate-----\n");
	int ret = -1;
	
	setBitRate(rate/1000);
	printf("write to the setting\n");
	if(!s_quit)//未有客户端接入
	{
		if(vd->fd > 0)//未初始化不能访问
			ret = XU_Init_Ctrl(vd->fd);
		if(ret<0)
		{
			printf("XU_H264_Set_BitRate Failed\n");	
		} else
		{
			double m_BitRate = 0.0;
			
			if(XU_H264_Set_BitRate(vd->fd, rate) < 0 )
				printf("XU_H264_Set_BitRate Failed\n");

			XU_H264_Get_BitRate(vd->fd, &m_BitRate);
			if(m_BitRate < 0 )
				printf("XU_H264_Get_BitRate Failed\n");

			printf("----m_BitRate:%f----\n", m_BitRate);			
		}
	}
	else
	{
		printf("camera no init\n");
		return 1;
	}
	return ret;
}

int FetchData::getData(void* fTo, unsigned fMaxSize, unsigned& fFrameSize, unsigned& fNumTruncatedBytes)
{
	
	if(!s_b_running)
	{
		printf("FetchData::getData s_b_running = false  \n");
		return 0;
	}
	
	if(vd == NULL || vd->fd == NULL)
	{
		printf("test FCCC 4 \n");	
		return NULL;
	}
	
	CLEAR (__buf);
	__buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	__buf.memory = V4L2_MEMORY_MMAP;
	
	int ret = ioctl(vd->fd, VIDIOC_DQBUF, &__buf);
	
	if (ret < 0) 
	{
		printf("FetchData Unable to dequeue buffer ret:%d!\n", ret);
		cameraUninit();
		if(-1 == cameraInit())
			exit(0);
		return 0;
	}
	
	unsigned len = __buf.bytesused;

	//拷贝视频到live555缓存
	if(len < fMaxSize)
	{            
		memcpy(fTo, buffers[__buf.index].start, len);
		fFrameSize = len;
		fNumTruncatedBytes = 0;
	}        
	else        
	{           
		memcpy(fTo, buffers[__buf.index].start, fMaxSize);
		fNumTruncatedBytes = len - fMaxSize; 
		fFrameSize = fMaxSize;
	}

	ret = ioctl(vd->fd, VIDIOC_QBUF, &__buf);

	if (ret < 0) 
	{
		printf("Unable to requeue buffer\n");
	}

	return len;
}

#else

#include "video_capture.h"
#include "h264encoder.h"

#define MAX 10
#define KEY "012345"
#define DelayTime 33*1000//(50us*1000=0.05s 20f/s)

#define DEVICE "/dev/video0"
#define SET_WIDTH 640
#define SET_HEIGHT 480

struct cam_data Buff[2];

pthread_t thread[2];
int flag[2],point=0;
FILE * f_d;
//unsigned char *h264_buf;
int h264_length=0;
int framelength=0;
extern Encoder en;
struct camera *cam;


int cameraInit()
{
	printf("-cameraInit 1\n");

cam = (struct camera *) malloc(sizeof(struct camera));
if (!cam) {
	printf("malloc camera failure!\n");
	exit(1);
}
cam->device_name = (char *)DEVICE;
cam->buffers = NULL;
cam->width = SET_WIDTH;
cam->height = SET_HEIGHT;

framelength=sizeof(unsigned char)*cam->width * cam->height * 2;

v4l2_init(cam);
compress_begin(&en, cam->width, cam->height);//初始化编码器

return 0;

}


struct v4l2_buffer __buf;

void cameraUninit(void)
{
  	printf("TSS cameraUninit 110219-1\n");
	v4l2_close(cam);	
};

// Queue<Mediadata*> sWorkDataQueue;
const int QUEUE_LEN_MAX = 4;
const int QUEUE_SIMPLE_UNIT_SIZE = 100000;
void Init()
{
	void Uinit();
   	Uinit();
  	cameraInit();
}


int Uinit()
{	
	cameraUninit();
	
	return 0;
};

void* FetchData::s_source = NULL;
bool s_quit = true; 

bool emptyBuffer = false;
char t_buf[204800];
void *go(void *arg)
{
	printf("GOOOOOOOOOOOOOOOOOooo s_quit:%d\n", s_quit);
    while(!s_quit)
	{
//my code
		printf("FetchDatavvd go 1 \n");

		if(cam != NULL && cam->fd != 0)
		{
			printf("FetchDatavvd go 2 \n");
			
			break;
		}else
		{
			printf("FetchDatavvd go 3 \n");
			
			Init();
			sleep(1);
		}
	}

   while(!s_quit)
   {
		int i=-1;
		if(buffOneFrame(&Buff[i] , cam))//采集一帧数据
		{
			flag[i]=1;//缓冲区i已满
			Buff[i].rpos=0;
			i=!i;	//切换到另一个缓冲区		
			Buff[i].wpos=0;
			flag[i]=0;//缓冲区i为空
		}	
		/*H.264压缩视频*/
		if(flag[1]==0 && flag[0]==0 || flag[i]==-1) ;
		if(flag[0]==1) i=0;
		if(flag[1]==1) i=1;
		if(i)   printf("-------------video_Encode_Thread Buff 1\n");
		if(!i)   printf("-------------video_Encode_Thread Buff 0\n");	
		encode_frame(Buff[i].cam_mbuf + Buff[i].rpos,0);
		Buff[i].rpos+=framelength;
		if(Buff[i].rpos>=BUF_SIZE) { Buff[i].rpos=0;Buff[!i].rpos=0;flag[i]=-1;}

		int len = cbuf_enqueue(&FetchData::data,h264_buf, h264_length);
   }
}

int FetchData::bit_rate_setting(int rate)
{
	printf("-----start seting bit rate-----\n");
	int ret = -1;
	
	setBitRate(rate/1000);
	printf("write to the setting\n");
	if(!s_quit)//未有客户端接入
	{
		//mycode
	}
	else
	{
		printf("camera no init\n");
		return 1;
	}
	return ret;
}

int FetchData::getData(void* fTo, unsigned fMaxSize, unsigned& fFrameSize, unsigned& fNumTruncatedBytes)
{
	
	if(!s_b_running)
	{
		printf("FetchData::getData s_b_running = false  \n");
		return 0;
	}
//my code
	if(cam == NULL || cam->fd == 0)
	{
		printf("test FCCC 4 \n");	
		return -1;
	}
	
	int i=-1;
	if(buffOneFrame(&Buff[i] , cam))//采集一帧数据
	{
		flag[i]=1;//缓冲区i已满
		Buff[i].rpos=0;
		i=!i;	//切换到另一个缓冲区		
		Buff[i].wpos=0;
		flag[i]=0;//缓冲区i为空
	}	
	/*H.264压缩视频*/
	if(flag[1]==0 && flag[0]==0 || flag[i]==-1) return 1;
	if(flag[0]==1) i=0;
	if(flag[1]==1) i=1;
	if(i)   printf("-------------video_Encode_Thread Buff 1\n");
	if(!i)   printf("-------------video_Encode_Thread Buff 0\n");	
	encode_frame(Buff[i].cam_mbuf + Buff[i].rpos,0);
	Buff[i].rpos+=framelength;
	if(Buff[i].rpos>=BUF_SIZE) { Buff[i].rpos=0;Buff[!i].rpos=0;flag[i]=-1;}
	
	int len = cbuf_enqueue(&FetchData::data,h264_buf, h264_length);
	//拷贝视频到live555缓存
	if(len < fMaxSize)
	{            
		memcpy(fTo, h264_buf, len);
		fFrameSize = len;
		fNumTruncatedBytes = 0;
	}        
	else        
	{           
		memcpy(fTo, h264_buf, fMaxSize);
		fNumTruncatedBytes = len - fMaxSize; 
		fFrameSize = fMaxSize;
	}
	return len;
}

#endif

bool FetchData::s_b_running=false;
cbuf_t FetchData::data; 
pthread_t s_thread;
FetchData::FetchData()
{
	
}
FetchData::~FetchData()
{
	
}

void FetchData::EmptyBuffer()
{
	emptyBuffer = true;
}

void FetchData::startCap()
{
	s_b_running = true;
	data.CUBFEACHDATALEN = 200000;
	cbuf_init(&data);

	printf("void FetchData::startCap() 1\n");
	if(!s_quit)
	{
		return;
	}
	s_quit = false;
	printf("void FetchData::startCap() 3\n");
    Init();
	printf("void FetchData::startCap() 4\n");
	printf("pthread_create ok \n");  
}

void FetchData::stopCap()
{
	s_b_running = false;
	printf("FetchData stopCap 1\n");  
    s_quit = true;
	cbuf_destroy(&data);
    Uinit();
}