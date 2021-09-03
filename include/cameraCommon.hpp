#pragma once
#include <fcntl.h>           // for O_RDWR
#include <unistd.h>          // for open(), creat()
#include <sys/ioctl.h>       // for ioctl()
#include <linux/videodev2.h> // for v4l2 struct
#include <sys/mman.h>        // for mmap
#include <cstdint>
#include <string>

extern "C"
{
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
    // #include "libavcodec/internal.h"
}

struct VideoBuffer
{
private:
    void *mBuf;
    uint32_t mLength;

public:
    VideoBuffer()
        : mBuf{}, mLength{0}
    {
    }

    VideoBuffer(void *buf, uint32_t length)
        : mBuf{buf}, mLength{length}
    {
    }

    inline void setBuf(void *buf) { mBuf = buf; }
    inline void setLength(uint32_t length) { mLength = length; }
    inline void *getBuf() const { return mBuf; }
    inline uint32_t getLength() const { return mLength; }
};

struct FrameInfo
{
private:
    int mFps;
    int mFmt;
    int mWidth;
    int mHeight;

public:
    FrameInfo()
        : mFps{0}, mFmt{-1}, mWidth{0}, mHeight{0} {}
    FrameInfo(int fps, int fmt, int w, int h)
        : mFps{fps}, mFmt{fmt}, mWidth{w}, mHeight{h} {}
    inline int getFps() const { return mFps; }
    inline int getFmt() const { return mFmt; }
    inline int getWidth() const { return mWidth; }
    inline int getHeight() const { return mHeight; }

    inline void setFps(const int fps) { mFps = fps; }
    inline void setFmt(const int fmt) { mFmt = fmt; }
    inline void setWidth(const int w) { mWidth = w; }
    inline void setHeight(const int h) { mHeight = h; }
};

class VideoFrameInterface
{
public:
    virtual void SendFrame(const VideoBuffer &frame) {}
    virtual void RecvFrame(std::string &frame) {}
};
