/*
 * TIAM335X_H264_H_.h
 *
 *  Created on: 2016年11月1日
 *      Author: Stone
 */
#ifndef TIAM335X_H264_H_
#define TIAM335X_H264_H_

#include "FramedSource.hh"
#include <iostream>
#include <sys/time.h>

#define FRAME_PER_SEC 30
class H264CameraSource : public FramedSource
{

public:
    static H264CameraSource *createNew(UsageEnvironment &env)
    {
        return new H264CameraSource(env);
    }

    H264CameraSource(UsageEnvironment &env);
    virtual ~H264CameraSource();

    static void updateTime(struct timeval &p);

    void doUpdateStart();
    static void updateDataNotify(void *d) { ((H264CameraSource *)d)->doUpdateDataNotify(); };
    void doUpdateDataNotify();

protected: // redefined virtual functions
    virtual void doGetNextFrame();
    //   static void getNextFrame(void * ptr);
    virtual void doStopGettingFrames();

    virtual unsigned int maxFrameSize() const;
    void GetFrameData();

    void *m_pToken;

    static struct timeval sPresentationTime;
    static struct timeval sdiff;

    static bool sbTimeUpdate;

    EventTriggerId m_eventTriggerId;

    bool bVideoFirst;

    bool m_can_get_nextframe;
    bool m_is_queue_empty;
    bool m_started;
};

#endif /* TIAM335X_H264_H_ */
