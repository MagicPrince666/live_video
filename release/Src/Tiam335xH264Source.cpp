/*
 * V4L2.cpp
 *
 *  Created on: 2016年11月1日
 *      Author: Stone
 */

#include "Tiam335xH264Source.hh"
#include "FetchData.hh"
//#include "cbuf.h"

#define __DBGFUNS
#ifdef __DBGFUNS
#define DBGFUNS(fmt, args...) printf(fmt, ##args)
#else
#define DBGFUNS(fmt, args...)
#endif
#if 0
#endif

FILE *rec_file = NULL;

bool emptyBufferFlag = true;

// extern cbuf_t g_cbuf;
Tiam335xH264Source::Tiam335xH264Source(UsageEnvironment &env) : FramedSource(env), m_pToken(0)
{
    m_can_get_nextframe = true;
    m_is_queue_empty    = false;
    bVideoFirst         = true;
    m_started           = false;

    printf("Tiam335xH264Source::Tiam335xH264Source \n");
    gettimeofday(&sPresentationTime, NULL);

    //启动获取视频数据线程
    FetchData::startCap();
    emptyBufferFlag = true;
    FetchData::setSource(this);
    m_eventTriggerId = envir().taskScheduler().createEventTrigger(Tiam335xH264Source::updateDataNotify);
}

Tiam335xH264Source::~Tiam335xH264Source()
{
    if (rec_file != NULL) {
        fclose(rec_file);
    }
    rec_file = NULL;
    FetchData::stopCap();

    envir().taskScheduler().deleteEventTrigger(m_eventTriggerId);
}

int timeval_substract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);

    result->tv_sec  = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return diff < 0;
}

void timeval_add(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int total = (t2->tv_usec + 1000000 * t2->tv_sec) + (t1->tv_usec + 1000000 * t1->tv_sec);

    result->tv_sec  = total / 1000000;
    result->tv_usec = total % 1000000;
}

struct timeval Tiam335xH264Source::sPresentationTime;
struct timeval Tiam335xH264Source::sdiff;
bool Tiam335xH264Source::sbTimeUpdate = false;
void Tiam335xH264Source::updateTime(struct timeval &p)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    sPresentationTime.tv_sec  = p.tv_sec;
    sPresentationTime.tv_usec = p.tv_usec;

    int i = timeval_substract(&sdiff, &now, &sPresentationTime);
    printf("DIFF:%d \n", sdiff);

    sbTimeUpdate = true;
}

void Tiam335xH264Source::doUpdateStart()
{
    envir().taskScheduler().triggerEvent(m_eventTriggerId, this);
}

void Tiam335xH264Source::doUpdateDataNotify()
{
    // nextTask() = envir().taskScheduler().scheduleDelayedTask(0,(TaskFunc*)FramedSource::afterGetting,this);
    afterGetting(this);
}

void Tiam335xH264Source::GetFrameData()
{
    unsigned len = FetchData::getData(fTo, fMaxSize, fFrameSize, fNumTruncatedBytes);

    gettimeofday(&fPresentationTime, NULL);
    afterGetting(this);

    if (!m_can_get_nextframe) {
        envir().taskScheduler().unscheduleDelayedTask(nextTask());
        m_is_queue_empty = true;
    }
}

void Tiam335xH264Source::doGetNextFrame()
{
    if (!m_started) {
        m_started = true;
    }

    GetFrameData();
}

void Tiam335xH264Source::doStopGettingFrames()
{
    //启动获取视频数据线程
    m_can_get_nextframe = false;

    while (!m_is_queue_empty && m_started) {
        usleep(10000);
    }
}

//网络包尺寸，注意尺寸不能太小，否则会崩溃
unsigned int Tiam335xH264Source::maxFrameSize() const
{
    printf("Tiam335xH264Source::maxFrameSize \n");
    return 150000;
}
