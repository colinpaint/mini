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
#include "SimdDefs.h"
#include "SimdMemory.h"
#include "SimdAlphaBlending.h"

namespace Simd {
  namespace Base {
    // copy
    //{{{
    void Copy (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t pixelSize,
               uint8_t* dst, size_t dstStride) {

      size_t rowSize = width * pixelSize;
      for (size_t row = 0; row < height; ++row) {
        memcpy(dst, src, rowSize);
        src += srcStride;
        dst += dstStride;
        }
      }
    //}}}
    //{{{
    void CopyFrame (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t pixelSize,
                    size_t frameLeft, size_t frameTop, size_t frameRight, size_t frameBottom,
                    uint8_t* dst, size_t dstStride) {

      if ((frameTop > frameBottom) || (frameBottom > height) ||
          (frameLeft > frameRight) || (frameRight > width))
        return;

      if (frameTop > 0) {
        size_t srcOffset = 0;
        size_t dstOffset = 0;
        size_t size = width * pixelSize;
        for (size_t row = 0; row < frameTop; ++row) {
          memcpy (dst + dstOffset, src + srcOffset, size);
          srcOffset += srcStride;
          dstOffset += dstStride;
          }
        }

      if (frameBottom < height) {
        size_t srcOffset = frameBottom * srcStride;
        size_t dstOffset = frameBottom * dstStride;
        size_t size = width * pixelSize;
        for (size_t row = frameBottom; row < height; ++row) {
          memcpy (dst + dstOffset, src + srcOffset, size);
          srcOffset += srcStride;
          dstOffset += dstStride;
          }
        }

      if (frameLeft > 0) {
        size_t srcOffset = frameTop * srcStride;
        size_t dstOffset = frameTop * dstStride;
        size_t size = frameLeft * pixelSize;
        for (size_t row = frameTop; row < frameBottom; ++row) {
          memcpy (dst + dstOffset, src + srcOffset, size);
          srcOffset += srcStride;
          dstOffset += dstStride;
          }
        }

      if (frameRight < width) {
        size_t srcOffset = frameTop * srcStride + frameRight*pixelSize;
        size_t dstOffset = frameTop * dstStride + frameRight*pixelSize;
        size_t size = (width - frameRight) * pixelSize;
        for (size_t row = frameTop; row < frameBottom; ++row) {
          memcpy (dst + dstOffset, src + srcOffset, size);
          srcOffset += srcStride;
          dstOffset += dstStride;
          }
        }
      }
    //}}}

    // fill
    //{{{
    void Fill (uint8_t* dst, size_t stride, size_t width, size_t height, size_t pixelSize, uint8_t value) {

      size_t rowSize = width*pixelSize;
      for (size_t row = 0; row < height; ++row) {
        memset (dst, value, rowSize);
        dst += stride;
        }
      }
    //}}}
    //{{{
    void FillFrame (uint8_t* dst, size_t stride, size_t width, size_t height, size_t pixelSize,
                    size_t frameLeft, size_t frameTop, size_t frameRight, size_t frameBottom, uint8_t value) {

      if (frameTop) {
        size_t offset = 0;
        size_t size = width*pixelSize;
        for (size_t row = 0; row < frameTop; ++row) {
          memset(dst + offset, value, size);
          offset += stride;
          }
        }

      if (height - frameBottom) {
        size_t offset = frameBottom*stride;
        size_t size = width*pixelSize;
        for (size_t row = frameBottom; row < height; ++row) {
          memset(dst + offset, value, size);
          offset += stride;
          }
        }

      if (frameLeft) {
        size_t offset = frameTop*stride;
        size_t size = frameLeft*pixelSize;
        for (size_t row = frameTop; row < frameBottom; ++row) {
          memset(dst + offset, value, size);
          offset += stride;
          }
        }

      if (width - frameRight) {
        size_t offset = frameTop*stride + frameRight*pixelSize;
        size_t size = (width - frameRight)*pixelSize;
        for (size_t row = frameTop; row < frameBottom; ++row) {
          memset(dst + offset, value, size);
          offset += stride;
          }
        }
      }
    //}}}

    //{{{
    SIMD_INLINE uint64_t Fill64 (uint8_t a, uint8_t b, uint8_t c) {

      #ifdef SIMD_BIG_ENDIAN
        return (uint64_t(a) << 56) | (uint64_t(b) << 48) | (uint64_t(c) << 40) | (uint64_t(a) << 32) |
               (uint64_t(b) << 24) | (uint64_t(c) << 16) | (uint64_t(a) << 8) | uint64_t(b);
      #else
        return uint64_t(a) | (uint64_t(b) << 8) | (uint64_t(c) << 16) | (uint64_t(a) << 24) |
               (uint64_t(b) << 32) | (uint64_t(c) << 40) | (uint64_t(a) << 48) | (uint64_t(b) << 56);
      #endif
    }
    //}}}
    //{{{
    SIMD_INLINE uint32_t Fill32 (uint8_t a, uint8_t b, uint8_t c) {

      #ifdef SIMD_BIG_ENDIAN
        return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(c) << 8) | uint32_t(a);
      #else
        return uint32_t(a) | (uint32_t(b) << 8) | (uint32_t(c) << 16) | (uint32_t(a) << 24);
      #endif
      }
    //}}}
    //{{{
    void FillBgr (uint8_t* dst, size_t stride, size_t width, size_t height, uint8_t blue, uint8_t green, uint8_t red) {

      size_t size = width* 3;
      size_t step = sizeof(size_t)* 3;
      size_t alignedSize = AlignLo(width, sizeof(size_t))* 3;
      size_t bgrs[3];

      #if defined(SIMD_X64_ENABLE) || defined(SIMD_PPC64_ENABLE) || defined(SIMD_ARM64_ENABLE)
        bgrs[0] = Fill64 (blue, green, red);
        bgrs[1] = Fill64 (red, blue, green);
        bgrs[2] = Fill64 (green, red, blue);
      #else
        bgrs[0] = Fill32 (blue, green, red);
        bgrs[1] = Fill32 (green, red, blue);
        bgrs[2] = Fill32 (red, blue, green);
      #endif

      for (size_t row = 0; row < height; ++row) {
        size_t offset = 0;
        for (; offset < alignedSize; offset += step) {
          ((size_t*)(dst + offset))[0] = bgrs[0];
          ((size_t*)(dst + offset))[1] = bgrs[1];
          ((size_t*)(dst + offset))[2] = bgrs[2];
          }
        for (; offset < size; offset += 3) {
          (dst + offset)[0] = blue;
          (dst + offset)[1] = green;
          (dst + offset)[2] = red;
          }
        dst += stride;
        }
      }
    //}}}

    #if defined(__GNUC__) && (defined(SIMD_X86_ENABLE) || defined(SIMD_X64_ENABLE))
      #pragma GCC push_options
      #pragma GCC optimize ("O2")
    #endif
    //{{{
    void FillBgra (uint8_t* dst, size_t stride, size_t width, size_t height, 
                   uint8_t blue, uint8_t green, uint8_t red, uint8_t alpha) {

      #ifdef SIMD_BIG_ENDIAN
        uint32_t bgra32 = uint32_t(alpha) | (uint32_t(red) << 8) | (uint32_t(green) << 16) | (uint32_t(blue) << 24);
      #else
        uint32_t bgra32 = uint32_t(blue) | (uint32_t(green) << 8) | (uint32_t(red) << 16) | (uint32_t(alpha) << 24);
      #endif

      #if defined(SIMD_X64_ENABLE) || defined(SIMD_PPC64_ENABLE) || defined(SIMD_ARM64_ENABLE)
        uint64_t bgra64 = uint64_t(bgra32) | (uint64_t(bgra32) << 32);
        size_t alignedWidth = AlignLo(width, 2);
        for (size_t row = 0; row < height; ++row) {
          for (size_t col = 0; col < alignedWidth; col += 2)
            *((uint64_t*)((uint32_t*)dst + col)) = bgra64;
          if (width != alignedWidth)
            ((uint32_t*)dst)[width - 1] = bgra32;
          dst += stride;
          }
      #else
        for (size_t row = 0; row < height; ++row) {
          for (size_t col = 0; col < width; ++col)
            ((uint32_t*)dst)[col] = bgra32;
          dst += stride;
        }
      #endif
      }
    //}}}
    //{{{
    void FillUv (uint8_t* dst, size_t stride, size_t width, size_t height, uint8_t u, uint8_t v) {

      #ifdef SIMD_BIG_ENDIAN
        uint16_t uv16 = uint32_t(v) | (uint32_t(u) << 8);
      #else
        uint16_t uv16 = uint32_t(u) | (uint32_t(v) << 8);
      #endif

      #if defined(SIMD_X64_ENABLE) || defined(SIMD_PPC64_ENABLE) || defined(SIMD_ARM64_ENABLE)
        uint64_t uv64 = uint64_t(uv16) | (uint64_t(uv16) << 16) | (uint64_t(uv16) << 32) | (uint64_t(uv16) << 48);
        size_t alignedWidth = AlignLo (width, 4);
        for (size_t row = 0; row < height; ++row) {
          size_t col = 0;
          for (; col < alignedWidth; col += 4)
            *((uint64_t*)((uint16_t*)dst + col)) = uv64;
          for (; col < width; col += 1)
            ((uint16_t*)dst)[col] = uv16;
          dst += stride;
          }
      #else
        uint32_t uv32 = uint32_t(uv16) | (uint32_t(uv16) << 16);
        size_t alignedWidth = AlignLo (width, 2);
        for (size_t row = 0; row < height; ++row) {
          size_t col = 0;
          for (; col < alignedWidth; col += 2)
            *((uint32_t*)((uint16_t*)dst + col)) = uv32;
          for (; col < width; col += 1)
            ((uint16_t*)dst)[col] = uv16;
          dst += stride;
          }
      #endif
    }
    //}}}
    #if defined(__GNUC__) && (defined(SIMD_X86_ENABLE) || defined(SIMD_X64_ENABLE))
      #pragma GCC pop_options
    #endif

    //{{{
    void FillPixel (uint8_t* dst, size_t stride, size_t width, size_t height, const uint8_t* pixel, size_t pixelSize) {

      switch (pixelSize) {
        case 1:
          Fill (dst, stride, width, height, 1, pixel[0]);
          break;

        case 2:
          FillUv (dst, stride, width, height, pixel[0], pixel[1]);
          break;

        case 3:
          FillBgr (dst, stride, width, height, pixel[0], pixel[1], pixel[2]);
          break;

        case 4:
          FillBgra (dst, stride, width, height, pixel[0], pixel[1], pixel[2], pixel[3]);
          break;

        default:
          assert(0);
        }
      }
    //}}}
    //{{{
    void Fill32f (float* dst, size_t size, const float* value) {

      if (value == 0 || value[0] == 0)
        memset(dst, 0, size * sizeof(float));
      else {
        float v = value[0];
        for (; size; --size)
          *dst++ = v;
        }
      }
    //}}}

    // alphaBlending
    //{{{
    SIMD_INLINE int AlphaBlending (int src, int dst, int alpha) {
      return DivideBy255 (src*alpha + dst*(0xFF - alpha));
      }
    //}}}

    template <size_t channelCount> void AlphaBlending(const uint8_t* src, int alpha, uint8_t* dst);
    //{{{
    template<> SIMD_INLINE void AlphaBlending<1> (const uint8_t* src, int alpha, uint8_t* dst) {
      dst[0] = AlphaBlending(src[0], dst[0], alpha);
      }
    //}}}
    //{{{
    template<> SIMD_INLINE void AlphaBlending<2> (const uint8_t* src, int alpha, uint8_t* dst) {
      dst[0] = AlphaBlending(src[0], dst[0], alpha);
      dst[1] = AlphaBlending(src[1], dst[1], alpha);
      }
    //}}}
    //{{{
    template<> SIMD_INLINE void AlphaBlending<3> (const uint8_t* src, int alpha, uint8_t* dst) {
      dst[0] = AlphaBlending(src[0], dst[0], alpha);
      dst[1] = AlphaBlending(src[1], dst[1], alpha);
      dst[2] = AlphaBlending(src[2], dst[2], alpha);
      }
    //}}}
    //{{{
    template<> SIMD_INLINE void AlphaBlending<4> (const uint8_t* src, int alpha, uint8_t* dst) {
      dst[0] = AlphaBlending(src[0], dst[0], alpha);
      dst[1] = AlphaBlending(src[1], dst[1], alpha);
      dst[2] = AlphaBlending(src[2], dst[2], alpha);
      dst[3] = AlphaBlending(src[3], dst[3], alpha);
      }
    //}}}
    //{{{
    template<size_t channelCount> void AlphaBlending (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                                      const uint8_t* alpha, size_t alphaStride, 
                                                      uint8_t* dst, size_t dstStride) {

      for (size_t row = 0; row < height; ++row) {
        for (size_t col = 0, offset = 0; col < width; ++col, offset += channelCount)
          AlphaBlending<channelCount>(src + offset, alpha[col], dst + offset);
        src += srcStride;
        alpha += alphaStride;
        dst += dstStride;
        }
      }
    //}}}
    //{{{
    void AlphaBlending (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t channelCount,
                        const uint8_t* alpha, size_t alphaStride, uint8_t* dst, size_t dstStride) {

      assert (channelCount >= 1 && channelCount <= 4);

      switch (channelCount) {
        case 1: AlphaBlending<1> (src, srcStride, width, height, alpha, alphaStride, dst, dstStride); break;
        case 2: AlphaBlending<2> (src, srcStride, width, height, alpha, alphaStride, dst, dstStride); break;
        case 3: AlphaBlending<3> (src, srcStride, width, height, alpha, alphaStride, dst, dstStride); break;
        case 4: AlphaBlending<4> (src, srcStride, width, height, alpha, alphaStride, dst, dstStride); break;
        }
      }
    //}}}

    template <size_t channelCount> void AlphaBlending2x (const uint8_t* src0, int alpha0, const uint8_t* src1, int alpha1, uint8_t* dst);
    //{{{
    template<> SIMD_INLINE void AlphaBlending2x<1> (const uint8_t* src0, int alpha0, const 
                                                    uint8_t* src1, int alpha1, uint8_t* dst) {
      dst[0] = AlphaBlending(src1[0], AlphaBlending(src0[0], dst[0], alpha0), alpha1);
      }
    //}}}
    //{{{
    template<> SIMD_INLINE void AlphaBlending2x<2> (const uint8_t* src0, int alpha0, 
                                                    const uint8_t* src1, int alpha1, uint8_t* dst) {
      dst[0] = AlphaBlending(src1[0], AlphaBlending(src0[0], dst[0], alpha0), alpha1);
      dst[1] = AlphaBlending(src1[1], AlphaBlending(src0[1], dst[1], alpha0), alpha1);
      }
    //}}}
    //{{{
    template<> SIMD_INLINE void AlphaBlending2x<3> (const uint8_t* src0, int alpha0, 
                                                    const uint8_t* src1, int alpha1, uint8_t* dst) {
      dst[0] = AlphaBlending(src1[0], AlphaBlending(src0[0], dst[0], alpha0), alpha1);
      dst[1] = AlphaBlending(src1[1], AlphaBlending(src0[1], dst[1], alpha0), alpha1);
      dst[2] = AlphaBlending(src1[2], AlphaBlending(src0[2], dst[2], alpha0), alpha1);
      }
    //}}}
    //{{{
    template<> SIMD_INLINE void AlphaBlending2x<4> (const uint8_t* src0, int alpha0, 
                                                    const uint8_t* src1, int alpha1, uint8_t* dst) {
      dst[0] = AlphaBlending(src1[0], AlphaBlending(src0[0], dst[0], alpha0), alpha1);
      dst[1] = AlphaBlending(src1[1], AlphaBlending(src0[1], dst[1], alpha0), alpha1);
      dst[2] = AlphaBlending(src1[2], AlphaBlending(src0[2], dst[2], alpha0), alpha1);
      dst[3] = AlphaBlending(src1[3], AlphaBlending(src0[3], dst[3], alpha0), alpha1);
      }
    //}}}
    //{{{
    template<size_t channelCount> void AlphaBlending2x (const uint8_t* src0, size_t src0Stride, 
                                                        const uint8_t* alpha0, size_t alpha0Stride,
                                                        const uint8_t* src1, size_t src1Stride, 
                                                        const uint8_t* alpha1, size_t alpha1Stride, 
                                                        size_t width, size_t height, uint8_t* dst, size_t dstStride) {

      for (size_t row = 0; row < height; ++row) {
        for (size_t col = 0, offset = 0; col < width; ++col, offset += channelCount)
          AlphaBlending2x<channelCount>(src0 + offset, alpha0[col], src1 + offset, alpha1[col], dst + offset);
        src0 += src0Stride;
        alpha0 += alpha0Stride;
        src1 += src1Stride;
        alpha1 += alpha1Stride;
        dst += dstStride;
        }
      }
    //}}}
    //{{{
    void AlphaBlending2x (const uint8_t* src0, size_t src0Stride, const uint8_t* alpha0, size_t alpha0Stride,
                          const uint8_t* src1, size_t src1Stride, const uint8_t* alpha1, size_t alpha1Stride,
                          size_t width, size_t height, size_t channelCount, uint8_t* dst, size_t dstStride) {

      assert(channelCount >= 1 && channelCount <= 4);

      switch (channelCount) {
        case 1: AlphaBlending2x<1> (src0, src0Stride, alpha0, alpha0Stride, src1, src1Stride, alpha1, alpha1Stride, width, height, dst, dstStride); break;
        case 2: AlphaBlending2x<2> (src0, src0Stride, alpha0, alpha0Stride, src1, src1Stride, alpha1, alpha1Stride, width, height, dst, dstStride); break;
        case 3: AlphaBlending2x<3> (src0, src0Stride, alpha0, alpha0Stride, src1, src1Stride, alpha1, alpha1Stride, width, height, dst, dstStride); break;
        case 4: AlphaBlending2x<4> (src0, src0Stride, alpha0, alpha0Stride, src1, src1Stride, alpha1, alpha1Stride, width, height, dst, dstStride); break;
        }
      }
    //}}}

    //{{{
    template<size_t channelCount> void AlphaBlendingUniform (const uint8_t* src, size_t srcStride, 
                                                             size_t width, size_t height, uint8_t alpha, 
                                                             uint8_t* dst, size_t dstStride) {

      for (size_t row = 0; row < height; ++row) {
        for (size_t col = 0, offset = 0; col < width; ++col, offset += channelCount)
          AlphaBlending<channelCount>(src + offset, alpha, dst + offset);
        src += srcStride;
        dst += dstStride;
        }
      }
    //}}}
    //{{{
    void AlphaBlendingUniform (const uint8_t* src, size_t srcStride, size_t width, size_t height, 
                               size_t channelCount, uint8_t alpha, uint8_t* dst, size_t dstStride) {

      assert(channelCount >= 1 && channelCount <= 4);

      switch (channelCount) {
        case 1: AlphaBlendingUniform<1> (src, srcStride, width, height, alpha, dst, dstStride); break;
        case 2: AlphaBlendingUniform<2> (src, srcStride, width, height, alpha, dst, dstStride); break;
        case 3: AlphaBlendingUniform<3> (src, srcStride, width, height, alpha, dst, dstStride); break;
        case 4: AlphaBlendingUniform<4> (src, srcStride, width, height, alpha, dst, dstStride); break;
        }
    }
    //}}}

    //{{{
    template<size_t channelCount> void AlphaFilling (uint8_t* dst, size_t dstStride, 
                                                     size_t width, size_t height, const uint8_t* channel, 
                                                     const uint8_t* alpha, size_t alphaStride) {
      for (size_t row = 0; row < height; ++row) {
        for (size_t col = 0, offset = 0; col < width; ++col, offset += channelCount)
          AlphaBlending<channelCount>(channel, alpha[col], dst + offset);
        alpha += alphaStride;
        dst += dstStride;
        }
      }
    //}}}
    //{{{
    void AlphaFilling (uint8_t* dst, size_t dstStride, size_t width, size_t height, 
                       const uint8_t* channel, size_t channelCount, const uint8_t* alpha, size_t alphaStride) {

      assert(channelCount >= 1 && channelCount <= 4);

      switch (channelCount) {
        case 1: AlphaFilling<1> (dst, dstStride, width, height, channel, alpha, alphaStride); break;
        case 2: AlphaFilling<2> (dst, dstStride, width, height, channel, alpha, alphaStride); break;
        case 3: AlphaFilling<3> (dst, dstStride, width, height, channel, alpha, alphaStride); break;
        case 4: AlphaFilling<4> (dst, dstStride, width, height, channel, alpha, alphaStride); break;
        }
      }
    //}}}

    //{{{
    template<bool argb> void AlphaPremultiply (const uint8_t* src, size_t srcStride, 
                                               size_t width, size_t height, 
                                               uint8_t* dst, size_t dstStride) {

      for (size_t row = 0; row < height; ++row) {
        for (size_t col = 0, end = width*4; col < end; col += 4)
          AlphaPremultiply<argb>(src + col, dst + col);
        src += srcStride;
        dst += dstStride;
        }
      }
    //}}}
    //{{{
    void AlphaPremultiply (const uint8_t* src, size_t srcStride, size_t width, size_t height, 
                           uint8_t* dst, size_t dstStride, SimdBool argb) {
      if (argb)
        AlphaPremultiply<true> (src, srcStride, width, height, dst, dstStride);
      else
        AlphaPremultiply<false> (src, srcStride, width, height, dst, dstStride);
      }
    //}}}

    //{{{
    template<bool argb> void AlphaUnpremultiply (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                                 uint8_t* dst, size_t dstStride) {

      for (size_t row = 0; row < height; ++row) {
        for (size_t col = 0, end = width * 4; col < end; col += 4)
          AlphaUnpremultiply<argb>(src + col, dst + col);
        src += srcStride;
        dst += dstStride;
        }
      }
    //}}}
    //{{{
    void AlphaUnpremultiply (const uint8_t* src, size_t srcStride, size_t width, size_t height, 
                             uint8_t* dst, size_t dstStride, SimdBool argb) {
      if (argb)
        AlphaUnpremultiply<true> (src, srcStride, width, height, dst, dstStride);
      else
        AlphaUnpremultiply<false> (src, srcStride, width, height, dst, dstStride);
      }
    //}}}
    }
  }
