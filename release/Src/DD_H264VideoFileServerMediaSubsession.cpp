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
// "liveMedia"
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 video file.
// Implementation

#include "DD_H264VideoFileServerMediaSubsession.hh"
#include "H264VideoRTPSink.hh"
#include "H264CameraSource.hh"
#include "H264VideoStreamDiscreteFramer.hh"


# define __DBGFUNS
# ifdef __DBGFUNS
# define DBGFUNS(fmt,args...) fprintf(stdout,  fmt,  ##args)
# else
# define DBGFUNS(fmt,args...)
# endif

DD_H264VideoFileServerMediaSubsession*
DD_H264VideoFileServerMediaSubsession::createNew(UsageEnvironment & env, FramedSource * source) {
 OutPacketBuffer::maxSize = 521366;
  return new DD_H264VideoFileServerMediaSubsession(env, source);
}

DD_H264VideoFileServerMediaSubsession
::DD_H264VideoFileServerMediaSubsession(UsageEnvironment & env, FramedSource * source)
  : OnDemandServerMediaSubsession(env, true)
{

     m_pSource = source;  
     //m_pSDPLine = 0;
}

DD_H264VideoFileServerMediaSubsession::~DD_H264VideoFileServerMediaSubsession() 
{
  //DBGFUNS("__DD_H264VideoFileServerMediaSubsession::destructor__\n");
  //  if (m_pSDPLine)  
  //   {  
  //       free(m_pSDPLine);  
  //       m_pSDPLine=NULL;
  //   }  

}

FramedSource* DD_H264VideoFileServerMediaSubsession::createNewStreamSource(
  unsigned /*clientSessionId*/, unsigned& estBitrate) 
  
  {

  //DBGFUNS("__DD_H264VideoFileServerMediaSubsession::createNewStreamSource__\n");
  estBitrate = 1024000; // kbps, estimate

  //视频真正实现类
  return H264VideoStreamDiscreteFramer::createNew(envir(), H264CameraSource::createNew(envir()));
}

RTPSink* DD_H264VideoFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
 //DBGFUNS("__DD_H264VideoFileServerMediaSubsession::createNewRTPSink__\n");
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
