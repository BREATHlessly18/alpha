#include "captureModule.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>

#define lbdFunc(x)                                             \
    do                                                         \
    {                                                          \
        if (x == -1)                                           \
            std::cerr << "here!!!! " << __LINE__ << std::endl; \
    } while (0)

CaptureModule::CaptureModule()
    : OperateInterface(),
      work::Task(),
      mDeviceId{-1},
      mCapability{}, mpFrameSnder{}, mVideoFramePool{}
{
}

CaptureModule::CaptureModule(const FrameInfo &frameCap, std::shared_ptr<FrameQueue> &que)
    : OperateInterface(),
      work::Task(),
      mDeviceId{-1},
      mCapability{frameCap}, mpFrameSnder{new VideoFrameProccess(que)}, mVideoFramePool{}
{
}

CaptureModule::~CaptureModule()
{
}

void CaptureModule::set(const FrameInfo &frameCap, std::shared_ptr<FrameQueue> &que)
{
    mCapability = frameCap;
    mpFrameSnder.reset(new VideoFrameProccess(que));
}

int CaptureModule::Start()
{
    if (work::Task::isRunning())
    {
        printf("the capture task has already started\n");
        return 0;
    }

    if (initV4l2() == -1)
    {
        printf("initV4l2 fail\n");
        return -1;
    }

    if (applyMemory() == -1)
    {
        printf("applyMemory fail\n");
        return -1;
    }

    if (false == work::Task::start().value())
    {
        printf("start video capture task fail\n");
        return -1;
    }

    v4l2_buf_type type{};
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(mDeviceId, VIDIOC_STREAMON, &type))
    {
        printf("turn on stream fail\n");
        return -1;
    }
    return 0;
}

int CaptureModule::Stop()
{
    if (false == work::Task::stop().value())
    {
        std::cerr << "stop capture task fail.\n";
        return -1;
    }

    if (-1 == releaseMemory())
    {
        std::cerr << "releaseMemory fail.\n";
        return -1;
    }

    close(mDeviceId);
    mDeviceId = -1;

    std::cout << "stop capture task success.\n";
    return 0;
}

int CaptureModule::initV4l2()
{
    const char deviceName[]{"/dev/video0"};
    mDeviceId = open(deviceName, O_RDWR | O_NONBLOCK, 0);
    if (-1 == mDeviceId)
    {
        printf("open %s with O_RDWR | O_NONBLOCK fail\n", deviceName);
        return -1;
    }

    v4l2_fmtdesc fmt{};
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    v4l2_format video_fmt{};
    video_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    video_fmt.fmt.pix.sizeimage = 0;
    video_fmt.fmt.pix.width = mCapability.getWidth();
    video_fmt.fmt.pix.height = mCapability.getHeight();
    video_fmt.fmt.pix.pixelformat = mCapability.getFmt();
    if (-1 == ioctl(mDeviceId, VIDIOC_S_FMT, &video_fmt))
    {
        std::cerr << "VIDIOC_S_FMT fail, erron = " << errno << std::endl;
        return -1;
    }
    v4l2_streamparm streamparms{};
    streamparms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(mDeviceId, VIDIOC_G_PARM, &streamparms))
    {
        std::cerr << "VIDIOC_G_PARM fail, errno = " << errno << std::endl;
        return -1;
    }
    else
    {
        memset(&streamparms, 0, sizeof(streamparms));
        streamparms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        streamparms.parm.capture.timeperframe.numerator = 1;
        streamparms.parm.capture.timeperframe.denominator = 30;
        if (ioctl(mDeviceId, VIDIOC_S_PARM, &streamparms) < 0)
        {
            std::cerr << "VIDIOC_S_PARM fail, errno =" << errno << std::endl;
            return -1;
        }
    }

    return 0;
}

int CaptureModule::applyMemory()
{
    if (-1 == mDeviceId)
    {
        std::cerr << __func__ << ", parameters check fail\n";
        return -1;
    }

    v4l2_requestbuffers rbuffer{};
    rbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rbuffer.memory = V4L2_MEMORY_MMAP;
    rbuffer.count = DEFAULT_BUFFER_COUNT;

    if (ioctl(mDeviceId, VIDIOC_REQBUFS, &rbuffer) < 0)
    {
        std::cerr << "VIDIOC_REQBUFS fail. errno = " << errno << std::endl;
        return -1;
    }

    // 申请buffer  入队等待
    mVideoFramePool.clear();
    for (uint8_t i{0}; i < rbuffer.count; ++i)
    {
        v4l2_buffer buffer{};
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;

        if (ioctl(mDeviceId, VIDIOC_QUERYBUF, &buffer) < 0)
        {
            std::cerr << "VIDIOC_QUERYBUF fail. errno =" << errno << std::endl;
            return -1;
        }

        VideoBuffer element(
            mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, mDeviceId, buffer.m.offset),
            buffer.length);

        if (MAP_FAILED == element.getBuf())
        {
            continue;
        }

        if (ioctl(mDeviceId, VIDIOC_QBUF, &buffer) < 0)
        {
            std::cerr << "VIDIOC_QBUF fail. errno =" << errno << std::endl;
            return -1;
        }
        mVideoFramePool.push_back(element);
    }
    std::cerr << "fd:" << mDeviceId << std::endl;
    return 0;
}

int CaptureModule::releaseMemory()
{
    if (-1 == mDeviceId)
    {
        std::cerr << __func__ << ", parameters check fail\n";
        return -1;
    }

    // 释放buffer
    for (uint8_t i{0}; i < mVideoFramePool.size(); ++i)
    {
        if (-1 == munmap(mVideoFramePool[i].getBuf(), mVideoFramePool[i].getLength()))
        {
            std::cerr << "munmap buffer[" << i << "] fail\n";
        }
    }
    mVideoFramePool.clear();
    return 0;
}

void CaptureModule::threadProcess()
{
    int ret{-1};
    fd_set rSet{};
    timeval timeout{};

    FD_ZERO(&rSet);
    FD_SET(mDeviceId, &rSet);
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    ret = select(mDeviceId + 1, &rSet, NULL, NULL, &timeout);
    if (ret < 0 && errno != EINTR)
    {
        return;
    }
    else if (0 == ret)
    {
        std::cerr << "time out\n";
        return;
    }
    else if (!FD_ISSET(mDeviceId, &rSet))
    {
        return;
    }

    // 从输出队列中取一帧
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(mDeviceId, VIDIOC_DQBUF, &buf))
    {
        if (errno != EINTR)
        {
            std::cerr << "could not sync on a buffer on device\n"
                      << strerror(errno)
                      << "\n";
            return;
        }
    }

    if (buf.index >= DEFAULT_BUFFER_COUNT)
    {
        std::cerr << "buf.index invalid\n";
        return;
    }
    void *buffer = (uint8_t *)mVideoFramePool[buf.index].getBuf();
    uint32_t length = mVideoFramePool[buf.index].getLength();

    // 将这一帧交给发送对象，发送对象会塞给帧队列单例
    mpFrameSnder->SendFrame(VideoBuffer(buffer, length));

    // 回队
    if (-1 == ioctl(mDeviceId, VIDIOC_QBUF, &buf))
    {
        std::cerr << "failed to enqueue capture buffer\n";
        return;
    }
}

int CaptureModule::Operate()
{
    threadProcess();
    return 0;
}
