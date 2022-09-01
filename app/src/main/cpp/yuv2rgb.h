//
// Created by vladislav on 9/1/22.
//

#ifndef AR_SHOP_YUV2RGB_H
#define AR_SHOP_YUV2RGB_H

#include <cstdint>

void ConvertYUV420ToARGB8888(const uint8_t* const yData,
                             const uint8_t* const uData,
                             const uint8_t* const vData, uint32_t* output,
                             const int width, const int height,
                             const int y_row_stride, const int uv_row_stride,
                             const int uv_pixel_stride);

#endif //AR_SHOP_YUV2RGB_H
