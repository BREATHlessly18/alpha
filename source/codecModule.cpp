#include "codecModule.hpp"
#include <iostream>

// CodecModule::CodecTask::CodecTask(OperateInterface *pCapModule)
//     : mpCodecModule{pCapModule}
// {
// }

// CodecModule::CodecTask::~CodecTask()
// {
//     mpCodecModule = nullptr;
// }

// int CodecModule::CodecTask::process()
// {
//     return mpCodecModule->Operate();
// }

CodecModule::CodecModule()
    : OperateInterface(),
      work::Task(),
      mDecID{AV_CODEC_ID_MJPEG}, mEncID{AV_CODEC_ID_H264},
      mDec{}, mEnc{},
      mDecCtx{}, mEncCtx{},
      mSwsCtx{},
      mDecFrm{}, mEncFrm{},
      mDecPkt{}, mEncPkt{},
      mFp{nullptr},
      mCap{},
      mpFrameRcver{}
{
    mFp = ::fopen("output.h264", "wb+");
}

CodecModule::CodecModule(const FrameInfo &frameCap, std::shared_ptr<FrameQueue> &que)
    : OperateInterface(),
      work::Task(),
      mDecID{AV_CODEC_ID_MJPEG}, mEncID{AV_CODEC_ID_H264},
      mDec{}, mEnc{},
      mDecCtx{}, mEncCtx{},
      mSwsCtx{},
      mDecFrm{}, mEncFrm{},
      mDecPkt{}, mEncPkt{},
      mFp{nullptr},
      mCap{frameCap},
      mpFrameRcver{new VideoFrameProccess(que)}
{
    mFp = ::fopen("output.h264", "wb+");
}

CodecModule::~CodecModule()
{
}

void CodecModule::set(const FrameInfo &frameCap, std::shared_ptr<FrameQueue> &que)
{
    mCap = frameCap;
    mpFrameRcver.reset(new VideoFrameProccess(que));
}

int CodecModule::Start()
{
    if (work::Task::isRunning())
    {
        printf("the codec task has already started\n");
        return 0;
    }

    if (-1 == initMedia())
    {
        std::cerr << "initMedia fail\n";
        return -1;
    }

    if (false == work::Task::start().value())
    {
        printf("start video codec task fail\n");
        return -1;
    }

    return 0;
}

int CodecModule::Stop()
{
    if (false == work::Task::stop().value())
    {
        std::cerr << "stop codec task fail.\n";
        return -1;
    }

    freeMedia();

    std::cout << "stop codec task success.\n";
    return 0;
}

void CodecModule::threadProcess()
{
    // 接收对象从帧队列拿出一帧进行编解码
    std::string frame{};
    mpFrameRcver->RecvFrame(frame);
    if (codecFrame(frame) < 0)
    {
        std::cerr << "codecFrame fail.\n";
        return;
    }
}

int CodecModule::Operate()
{
    threadProcess();
    return 0;
}

int CodecModule::initMedia()
{
    av_register_all();
    avcodec_register_all();

    if (-1 == applyDecoder())
    {
        std::cerr << "applyDecoder fail.\n";
        freeMedia();
        return -1;
    }

    if (-1 == applyEncoder())
    {
        std::cerr << "applyEncoder fail.\n";
        freeMedia();
        return -1;
    }

    return 0;
}

void CodecModule::freeMedia()
{
    avcodec_close(mDecCtx);
    avcodec_close(mEncCtx);
    avcodec_free_context(&mDecCtx);
    avcodec_free_context(&mEncCtx);

    av_free_packet(mDecPkt);
    av_free_packet(mEncPkt);
    av_frame_free(&mDecFrm);
    av_frame_free(&mEncFrm);
}

int CodecModule::applyDecoder()
{
    mDec = avcodec_find_decoder(mDecID);
    if (nullptr == mDec)
    {
        std::cerr << "avcodec_find_decoder fail\n";
        return -1;
    }

    if (mDecCtx != nullptr)
    {
        avcodec_free_context(&mDecCtx);
    }

    mDecCtx = avcodec_alloc_context3(mDec);
    if (nullptr == mDecCtx)
    {
        std::cerr << "avcodec_find_decoder fail\n";
        return -1;
    }

    mDecCtx->width = mCap.getWidth();
    mDecCtx->height = mCap.getHeight();
    mDecCtx->pix_fmt = AVPixelFormat(mCap.getFmt());

    if (0 > avcodec_open2(mDecCtx, mDec, NULL))
    {
        std::cerr << "avcodec_open2 for dec fail\n";
        return -1;
    }

    return 0;
}

int CodecModule::applyEncoder()
{
    mEnc = avcodec_find_encoder(mEncID);
    if (nullptr == mEnc)
    {
        std::cerr << "encoder not found\n";
        return -1;
    }

    if (mEncCtx != nullptr)
    {
        avcodec_free_context(&mEncCtx);
    }

    mEncCtx = avcodec_alloc_context3(mEnc);
    if (nullptr == mEncCtx)
    {
        std::cerr << "Could not allocate video codec context\n";
        return -1;
    }

    mEncCtx->width = mCap.getWidth();
    mEncCtx->height = mCap.getHeight();
    mEncCtx->bit_rate = 0.96 * 1000000;
    mEncCtx->framerate = (AVRational){mCap.getFps(), 1};
    mEncCtx->time_base = (AVRational){1, mCap.getFps()};
    mEncCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    mEncCtx->gop_size = 10;
    mEncCtx->flags |= AV_CODEC_FLAG2_LOCAL_HEADER;

    if (mEnc->id == AV_CODEC_ID_H264)
    {
        // 防止编码延迟修改参数
        av_opt_set(mEncCtx->priv_data, "preset", "superfast", 0);
        av_opt_set(mEncCtx->priv_data, "tune", "zerolatency", 0);
    }

    if (avcodec_open2(mEncCtx, mEnc, NULL) < 0)
    {
        std::cerr << "avcodec_open2 enc Error" << std::endl;
        return -1;
    }

    return 0;
}

int CodecModule::applyPaket(AVPacket **avpkt)
{
    if (*avpkt != nullptr)
    {
        av_free_packet(*avpkt);
    }

    *avpkt = av_packet_alloc();
    if (nullptr == *avpkt)
    {
        std::cerr << "av_packet_alloc fail\n";
        return -1;
    }

    return 0;
}

int CodecModule::unrefPaket(AVPacket **avpkt)
{
    if (nullptr == *avpkt)
    {
        std::cerr << "avpkt is null.\n";
        return -1;
    }

    av_packet_unref(*avpkt);
    return 0;
}

int CodecModule::applyFrame(AVFrame **avframe)
{
    if (*avframe != nullptr)
    {
        av_frame_free(avframe);
    }

    *avframe = av_frame_alloc();
    if (nullptr == *avframe)
    {
        std::cerr << "av_frame_alloc fail\n";
        return -1;
    }

    return 0;
}

int CodecModule::unrefFrame(AVFrame **avframe)
{
    if (*avframe != nullptr)
    {
        std::cerr << "avframe is null.\n";
        return -1;
    }

    av_frame_unref(*avframe);
    return 0;
}

int CodecModule::codecFrame(std::string &frame)
{
    if (frame.empty())
    {
        std::cerr << "frame.empty.\n";
        return -1;
    }

    if (nullptr == mDecCtx || nullptr == mEncCtx)
    {
        std::cerr << "did not initialize the media.\n";
        return -1;
    }

    int ret{-1};
    ret = applyPaket(&mDecPkt);
    if (-1 == ret)
    {
        std::cerr << "applyPaket fail.\n";
        return -1;
    }
    mDecPkt->size = (int)frame.size();
    mDecPkt->data = (uint8_t *)frame.data();

    ret = applyFrame(&mDecFrm);
    if (-1 == ret)
    {
        std::cerr << "applyFrame for dec fail.\n";
        return -1;
    }

    ret = decode();
    if (0 > ret)
    {
        std::cerr << "decode fail. ret:" << ret << "\n";
        return -1;
    }

    ret = applyFrame(&mEncFrm);
    if (-1 == ret)
    {
        std::cerr << "applyFrame for enc fail.\n";
        return -1;
    }
    mEncFrm->width = mEncCtx->width;
    mEncFrm->height = mEncCtx->height;
    mEncFrm->format = mEncCtx->pix_fmt;

    if (av_frame_get_buffer(mEncFrm, 32) < 0)
    {
        std::cerr << "Could not allocate the video frame data\n";
        return -1;
    }

    if (av_frame_is_writable(mEncFrm) <= 0)
    {
        std::cout << "mEncFrm need make writable\n";
        if (av_frame_make_writable(mEncFrm) < 0)
        {
            std::cerr << "av_frame_make_writable fail\n";
            return -1;
        }
    }

    ret = swsCode();
    if (0 > ret)
    {
        std::cerr << "swsCode fail. ret:" << ret << "\n";
        return -1;
    }
    //unrefFrame(&mDecFrm);

    ret = encode();
    if (0 > ret)
    {
        std::cerr << "encode fail. ret:" << ret << "\n";
        return -1;
    }

    releaseFrameAndPaket();
    return 0;
}

int CodecModule::decode()
{
    if (nullptr == mDecCtx || nullptr == mDecPkt || nullptr == mDecFrm)
    {
        std::cerr << "parameters check fail\n";
        return -1;
    }

    int ret{0};
    ret = avcodec_send_packet(mDecCtx, mDecPkt);
    if (ret < 0)
    {
        std::cerr << "avcodec_send_packet\n";
        return false;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(mDecCtx, mDecFrm);
        if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret)
        {
            std::cout << "wait for more AVPacket. ret:" << ret << "\n";
            return ret;
        }

        if (ret < 0)
        {
            std::cerr << "avcodec_receive_frame fail, stop decode! ret:" << ret << "\n";
            return ret;
        }

        unrefPaket(&mDecPkt);
        break;
    }

    return 0;
}

int CodecModule::swsCode()
{
    if (nullptr == mSwsCtx)
    {
        sws_freeContext(mSwsCtx);
    }

    mSwsCtx = sws_getContext(mDecFrm->width, mDecFrm->height, AVPixelFormat(mDecFrm->format),
                             mEncCtx->width, mEncCtx->height, mEncCtx->pix_fmt,
                             SWS_BILINEAR, NULL, NULL, NULL);

    if (nullptr == mSwsCtx)
    {
        std::cerr << "sws_getContext fail.\n";
        return -1;
    }

    sws_scale(mSwsCtx, mDecFrm->data, mDecFrm->linesize, 0, mDecFrm->height,
              mEncFrm->data, mEncFrm->linesize);

    sws_freeContext(mSwsCtx);

    return 0;
}

int CodecModule::encode()
{
    int ret = 0;
    avcodec_send_frame(mEncCtx, mEncFrm);
    AVPacket *packet = av_packet_alloc();

    while (true)
    {
        ret = avcodec_receive_packet(mEncCtx, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            std::cerr << "wait for more AVFrame\n";
            return false;
        }
        else if (ret < 0)
        {
            return -1;
        }

        if (mFp != nullptr)
        {
            fwrite(packet->data, 1, packet->size, mFp);
        }

        av_packet_unref(packet);
        break;
    }

    return 0;
}

int CodecModule::releaseFrameAndPaket()
{
    av_free_packet(mDecPkt);
    av_free_packet(mEncPkt);
    av_frame_free(&mDecFrm);
    av_frame_free(&mEncFrm);

    return 0;
}
