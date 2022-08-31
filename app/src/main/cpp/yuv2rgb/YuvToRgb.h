//
// Created by vladislav on 8/30/22.
//

#ifndef AR_SHOP_YUVTORGB_H
#define AR_SHOP_YUVTORGB_H

#include <cstdint>

#include "RenderScriptToolkit.h"
#include "TaskProcessor.h"
#include "Utils.h"

namespace renderscript {

  inline size_t roundUpTo16(size_t val) {
    return (val + 15u) & ~15u;
  }

  class YuvToRgbTask : public Task {
    uchar4 *mOut;
    size_t mCstep;
    size_t mStrideY;
    size_t mStrideU;
    size_t mStrideV;
    const uchar *mInY;
    const uchar *mInU;
    const uchar *mInV;

    void kernel(uchar4 *out, uint32_t xstart, uint32_t xend, uint32_t currentY);

    // Process a 2D tile of the overall work. threadIndex identifies which thread does the work.
    void processData(int threadIndex, size_t startX, size_t startY, size_t endX,
                     size_t endY) override;

  public:
    YuvToRgbTask(const uint8_t *input, uint8_t *output, size_t sizeX, size_t sizeY,
                 RenderScriptToolkit::YuvFormat format)
        : Task{sizeX, sizeY, 4, false, nullptr}, mOut{reinterpret_cast<uchar4 *>(output)} {
      switch (format) {
        case RenderScriptToolkit::YuvFormat::NV21:
          mCstep = 2;
          mStrideY = sizeX;
          mStrideU = mStrideY;
          mStrideV = mStrideY;
          mInY = reinterpret_cast<const uchar *>(input);
          mInV = reinterpret_cast<const uchar *>(input + mStrideY * sizeY);
          mInU = mInV + 1;
          break;
        case RenderScriptToolkit::YuvFormat::YV12:
          mCstep = 1;
          mStrideY = roundUpTo16(sizeX);
          mStrideU = roundUpTo16(mStrideY >> 1u);
          mStrideV = mStrideU;
          mInY = reinterpret_cast<const uchar *>(input);
          mInU = reinterpret_cast<const uchar *>(input + mStrideY * sizeY);
          mInV = mInU + mStrideV * sizeY / 2;
          break;
      }
    }
  };
}

#endif //AR_SHOP_YUVTORGB_H
