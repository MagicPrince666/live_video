/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2014, Live Networks, Inc.  All rights reserved
// A test program that demonstrates how to stream - via unicast RTP
// - various kinds of file on demand, using a built-in RTSP server.
// main program

#include "CONSTANT.h"
#include "DD_H264VideoFileServerMediaSubsession.hh"
#include "FetchData.hh"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>

#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <thread>

UsageEnvironment *video_env;

//視頻採集
void video_thread_func(void *param)
{
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    video_env                = BasicUsageEnvironment::createNew(*scheduler);
    ServerMediaSession *s    = (ServerMediaSession *)param;

    s->addSubsession(DD_H264VideoFileServerMediaSubsession ::createNew(*video_env, NULL));

    video_env->taskScheduler().doEventLoop(); // does not return
}

UsageEnvironment *env;

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           char const *streamName, char const *inputFileName, const char *inputAudioFileName); // fwd
static void help();

int main(int argc, char **argv)
{

    // int init_cycle_buffer(void);
    // signal(SIGPIPE, _signal_handler);    // SIGPIPE，管道破裂。
    // signal(SIGSEGV, _signal_handler);    // SIGSEGV，非法内存访问
    // signal(SIGFPE, _signal_handler);       // SIGFPE，数学相关的异常，如被0除，浮点溢出，等等
    // signal(SIGABRT, _signal_handler);     // SIGABRT，由调用abort函数产生，进程非正常退出

    // RTSPServer videodev(video driver device)  audiodev(audio driver device) fps(>0) bitrates(unit:kbps >0) RTSPPORT(default 554)
    // eg: .RTSPServer /dev/video1 30  1024 554;
    //  you can input "RTSPServer" instead of "RTSPServer /dev/video1  /dev/dsp 30  1024 554"
    int fps      = 30;
    int bitrates = 2048;
    int port     = 8554;
    char *vd     = (char *)"/dev/video2";

    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        help();
        return 0;
    }

    if (argc == 1) {
        setVideoDevice(vd);
        setPort(port);
        setBitRate(bitrates);
        setFps(fps);
    } else {
        if (argc < 4) {
            help();
            return 0;
        }
        setVideoDevice(argv[1]);

        fps = atoi(argv[2]);
        if (fps <= 0) {
            help();
            return 0;
        }

        bitrates = atoi(argv[3]);
        if (bitrates <= 0) {
            help();
            return 0;
        }

        if (argc > 4) {
            int p = atoi(argv[4]);
            if (p > 0) {
                port = p;
            }
        }

        setPort(port);
        setBitRate(bitrates);
        setFps(fps);
    }

    // Begin by setting up our usage environment:
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    env                      = BasicUsageEnvironment::createNew(*scheduler);

    UserAuthenticationDatabase *authDB = NULL;
#ifdef ACCESS_CONTROL
    // To implement client access control to the RTSP server, do the following:
    authDB = new UserAuthenticationDatabase;
    authDB->addUserRecord("magic", "prince"); // replace these with real strings
                                              // Repeat the above with each <username>, <password> that you wish to allow
                                              // access to the server.
#endif

    // Create the RTSP server:
    RTSPServer *rtspServer = RTSPServer::createNew(*env, getPort(), authDB);
    if (rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    char const *streamName        = "live";
    char const *descriptionString = "Session streamed by \"DM RTSP\"";

    {

        ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                                descriptionString);

        std::thread video_thread(video_thread_func, sms);
        video_thread.detach();

        while (video_env == NULL) {
            sleep(1);
        }

        rtspServer->addServerMediaSession(sms);

        announceStream(rtspServer, sms, streamName, NULL, NULL);
    }

    // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
    // Try first with the default HTTP port (80), and then with the alternative HTTP
    // port numbers (8000 and 8080).

    if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
        *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
    } else {
        *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
    }

    env->taskScheduler().doEventLoop(); // does not return

    return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           char const *streamName, char const *inputFileName, const char *inputAudioFileName)
{

    char *url             = rtspServer->rtspURL(sms);
    UsageEnvironment &env = rtspServer->envir();
    env << "\n\"" << streamName << "\" stream, from the Video file \""
        << inputFileName << ", from the Audio file \"\n"
        << inputAudioFileName << ",\"\n";
    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}

static void help()
{
    printf("====================================\n");
    printf("CMD tips:\n");
    printf("  RTSPServer videodev(video driver device) fps(>0) bitrates(unit:kbps >0) RTSPPORT(default 554)\n");
    printf("  eg: ./RTSPServer /dev/video1 30 1024 554\n");
    printf("  you can input \"RTSPServer\" instead of \"RTSPServer /dev/video1 30 1024 554\" \n");
    printf("====================================\n");
    return;
}
