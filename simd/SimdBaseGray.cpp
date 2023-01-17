//{{{
/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2017 Yermalayeu Ihar.
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
#include "SimdDefs.h"
#include "SimdMath.h"

namespace Simd::Base {
  //{{{
  SIMD_INLINE void GrayToBgr(const uint8_t & gray, uint8_t * bgr)
  {
      bgr[0] = gray;
      bgr[1] = gray;
      bgr[2] = gray;
  }
  //}}}
  //{{{
  void GrayToBgr(const uint8_t * gray, size_t width, size_t height, size_t grayStride, uint8_t * bgr, size_t bgrStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0, offset = 0; col < width; ++col, offset += 3)
              GrayToBgr(gray[col], bgr + offset);
          gray += grayStride;
          bgr += bgrStride;
      }
  }
  //}}}

  //{{{
  SIMD_INLINE uint32_t GrayToBgra(uint32_t gray, uint32_t alpha)
  {
  #ifdef SIMD_BIG_ENDIAN
      return alpha | (gray << 8) | (gray << 16) | (gray << 24);
  #else
      return gray | (gray << 8) | (gray << 16) | (alpha << 24);
  #endif
  }
  //}}}
  //{{{
  void GrayToBgra(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
  {
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < width; ++col)
              ((uint32_t*)bgra)[col] = GrayToBgra(gray[col], alpha);
          gray += grayStride;
          bgra += bgraStride;
      }
  }
  //}}}

  //{{{
  static void Int16ToGray(const int16_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < width; ++col)
              dst[col] = RestrictRange(src[col]);
          src += srcStride;
          dst += dstStride;
      }
  }
  //}}}
  //{{{
  void Int16ToGray(const uint8_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      Int16ToGray((const int16_t *)src, width, height, srcStride / sizeof(int16_t), dst, dstStride);
  }
  //}}}
  }
