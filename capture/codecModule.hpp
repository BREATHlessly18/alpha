#pragma once

#include "base/task.hpp"
#include "base/taskModule.hpp"
#include "capture/callbackModule.hpp"
#include "capture/cameraCommon.hpp"
#include "capture/operateInterface.hpp"

namespace video_capture {
namespace capture {

class CodecModule : public OperateInterface, public base::Task {
 public:
  int Operate() override;
  int Start() override;
  int Stop() override;

 public:
  CodecModule();
  CodecModule(const FrameInfo &frameCap, std::weak_ptr<FrameQueue> que);
  ~CodecModule() override;
  void set(const FrameInfo &frameCap, std::weak_ptr<FrameQueue> que);

  void threadProcess() override;

 private:
  int initMedia();
  void freeMedia();
  int applyDecoder();
  int applyEncoder();
  int applyPaket(AVPacket **avpkt);
  int unrefPaket(AVPacket **avpkt);
  int applyFrame(AVFrame **avframe);
  int unrefFrame(AVFrame **avframe);
  int codecFrame(std::string &frame);
  int decode();
  int encode();
  int swsCode();
  int saveH264();
  int releaseFrameAndPaket();

 private:
  AVCodecID mDecID{AV_CODEC_ID_NONE}, mEncID{AV_CODEC_ID_NONE};
  AVCodec *mDec{}, *mEnc{};
  AVCodecContext *mDecCtx{}, *mEncCtx{};
  SwsContext *mSwsCtx{};
  AVFrame *mDecFrm{}, *mEncFrm{};
  AVPacket *mDecPkt{}, *mEncPkt{};
  FILE *mFp{};
  FrameInfo mCap{0, 0, 0, 0};
  std::unique_ptr<VideoFrameInterface> mpFrameRcver;
};

}  // namespace capture
}  // namespace video_capture
