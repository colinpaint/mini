//{{{
/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2022 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
//}}}
#pragma once
#include "SimdDefs.h"

namespace Simd {
  #ifdef SIMD_SSE41_ENABLE
    namespace Sse41 {
      void AlphaBlending (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t channelCount,
          const uint8_t* alpha, size_t alphaStride, uint8_t* dst, size_t dstStride);
      void AlphaBlending2x (const uint8_t* src0, size_t src0Stride, const uint8_t* alpha0, size_t alpha0Stride,
          const uint8_t* src1, size_t src1Stride, const uint8_t* alpha1, size_t alpha1Stride,
          size_t width, size_t height, size_t channelCount, uint8_t* dst, size_t dstStride);
      void AlphaBlendingUniform (const uint8_t* src, size_t srcStride, size_t width, size_t height,
          size_t channelCount, uint8_t alpha, uint8_t* dst, size_t dstStride);
      void AlphaFilling (uint8_t* dst, size_t dstStride, size_t width, size_t height, const uint8_t* channel,
          size_t channelCount, const uint8_t* alpha, size_t alphaStride);
      void AlphaPremultiply (const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride, SimdBool argb);
      void AlphaUnpremultiply (const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride, SimdBool argb);

      void BayerToBgr (const uint8_t* bayer, size_t width, size_t height, size_t bayerStride, SimdPixelFormatType bayerFormat, uint8_t* bgr, size_t bgrStride);
      void BayerToBgra (const uint8_t* bayer, size_t width, size_t height, size_t bayerStride, SimdPixelFormatType bayerFormat, uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void BgraToBayer (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* bayer, size_t bayerStride, SimdPixelFormatType bayerFormat);

      void BgraToBgr (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* bgr, size_t bgrStride);
      void BgraToGray (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* gray, size_t grayStride);
      void BgraToRgb (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgb, size_t rgbStride);
      void BgraToRgba (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgba, size_t rgbaStride);
      void BgraToYuv420p (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride);
      void BgraToYuv422p (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride);
      void BgraToYuv444p (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride);
      void BgraToYuva420p (const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
          uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride, uint8_t* a, size_t aStride);
      void Bgr48pToBgra32 (const uint8_t* blue, size_t blueStride, size_t width, size_t height,
          const uint8_t* green, size_t greenStride, const uint8_t* red, size_t redStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void BgrToBayer (const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* bayer, size_t bayerStride, SimdPixelFormatType bayerFormat);
      void BgrToBgra (const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void BgrToGray (const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* gray, size_t grayStride);
      void BgrToRgb (const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* rgb, size_t rgbStride);
      void BgrToYuv420p (const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride);
      void BgrToYuv422p (const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride);
      void BgrToYuv444p (const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride);

      void DeinterleaveUv (const uint8_t* uv, size_t uvStride, size_t width, size_t height, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride);
      void DeinterleaveBgr (const uint8_t* bgr, size_t bgrStride, size_t width, size_t height, uint8_t* b, size_t bStride, uint8_t* g, size_t gStride, uint8_t* r, size_t rStride);
      void DeinterleaveBgra (const uint8_t* bgra, size_t bgraStride, size_t width, size_t height, uint8_t* b, size_t bStride, uint8_t* g, size_t gStride, uint8_t* r, size_t rStride, uint8_t* a, size_t aStride);

      void Fill32f (float* dst, size_t size, const float* value);
      void FillBgr (uint8_t* dst, size_t stride, size_t width, size_t height, uint8_t blue, uint8_t green, uint8_t red);
      void FillBgra (uint8_t* dst, size_t stride, size_t width, size_t height, uint8_t blue, uint8_t green, uint8_t red, uint8_t alpha);
      void FillPixel (uint8_t* dst, size_t stride, size_t width, size_t height, const uint8_t* pixel, size_t pixelSize);

      void CosineDistance32f (const float* a, const float* b, size_t size, float* distance);
      void Float32ToUint8 (const float* src, size_t size, const float* lower, const float* upper, uint8_t* dst);
      void Uint8ToFloat32 (const uint8_t* src, size_t size, const float* lower, const float* upper, float* dst);

      void GaussianBlur3x3 (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t channelCount, uint8_t* dst, size_t dstStride);

      void GrayToBgr (const uint8_t* gray, size_t width, size_t height, size_t grayStride, uint8_t* bgr, size_t bgrStride);
      void GrayToBgra (const uint8_t* gray, size_t width, size_t height, size_t grayStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void Int16ToGray (const uint8_t* src, size_t width, size_t height, size_t srcStride, uint8_t* dst, size_t dstStride);

      void InterleaveUv (const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
                         size_t width, size_t height, uint8_t* uv, size_t uvStride);
      void InterleaveBgr (const uint8_t* b, size_t bStride, const uint8_t* g, size_t gStride, const uint8_t* r,
                          size_t rStride, size_t width, size_t height, uint8_t* bgr, size_t bgrStride);
      void InterleaveBgra (const uint8_t* b, size_t bStride,
                           const uint8_t* g, size_t gStride,
                           const uint8_t* r, size_t rStride,
                           const uint8_t* a, size_t aStride,
                           size_t width, size_t height,
                           uint8_t* bgra, size_t bgraStride);

      void MeanFilter3x3 (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t channelCount,
                          uint8_t* dst, size_t dstStride);
      void MedianFilterRhomb3x3 (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                 size_t channelCount, uint8_t* dst, size_t dstStride);
      void MedianFilterRhomb5x5 (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                 size_t channelCount, uint8_t* dst, size_t dstStride);
      void MedianFilterSquare3x3 (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                  size_t channelCount, uint8_t* dst, size_t dstStride);
      void MedianFilterSquare5x5 (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                  size_t channelCount, uint8_t* dst, size_t dstStride);

      void RgbaToGray (const uint8_t* rgba, size_t width, size_t height, size_t rgbaStride,
                       uint8_t* gray, size_t grayStride);
      void RgbToBgra (const uint8_t* rgb, size_t width, size_t height, size_t rgbStride,
                      uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void RgbToGray (const uint8_t* rgb, size_t width, size_t height, size_t rgbStride,
                      uint8_t* gray, size_t grayStride);

      void ReduceColor2x2 (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                           uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride, size_t channelCount);
      void ReduceGray2x2 (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                          uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride);
      void ReduceGray3x3 (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                          uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride, int compensation);
      void ReduceGray4x4 (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                          uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride);
      void ReduceGray5x5 (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                          uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride, int compensation);

      void StretchGray2x2 (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                           uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride);

      void ResizeBilinear (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                           uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride,
                           size_t channelCount);

      void TransformImage (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                           size_t pixelSize, SimdTransformType transform,
                           uint8_t* dst, size_t dstStride);

      void Uyvy422ToBgr (const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height,
                         uint8_t* bgr, size_t bgrStride,
                         SimdYuvType yuvType);
      void Uyvy422ToYuv420p (const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height,
                             uint8_t* y, size_t yStride,
                             uint8_t* u, size_t uStride,
                             uint8_t* v, size_t vStride);

      void Yuva420pToBgra (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride);
      void Yuva444pToBgraV2 (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride, SimdYuvType yuvType);
      void Yuv420pToBgra (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void Yuv420pToBgraV2 (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType);
      void Yuv422pToBgra (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void Yuv444pToBgra (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha);
      void Yuv444pToBgraV2 (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType);
      void Yuv420pToBgr (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgr, size_t bgrStride);
      void Yuv422pToBgr (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgr, size_t bgrStride);
      void Yuv444pToBgr (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgr, size_t bgrStride);
      void Yuv420pToRgb (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride);
      void Yuv422pToRgb (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride);
      void Yuv444pToRgb (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride);
      void Yuv420pToHue (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* hue, size_t hueStride);
      void Yuv444pToHue (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* hue, size_t hueStride);
      void Yuv420pToUyvy422 (const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* uyvy, size_t uyvyStride);
    }
  #endif
  }
