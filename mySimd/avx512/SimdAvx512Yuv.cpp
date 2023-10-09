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
#include "SimdMemory.h"
#include "SimdStore.h"
#include "SimdConversion.h"
#include "SimdYuvToBgr.h"
#include "SimdInterleave.h"

namespace Simd {
  #ifdef SIMD_AVX512BW_ENABLE
    namespace Avx512bw {
      //{{{
      template <bool align, bool mask> SIMD_INLINE void YuvToBgr(const __m512i & y, const __m512i & u, const __m512i & v, uint8_t * bgr, const __mmask64 * tails)
      {
          __m512i blue = YuvToBlue(y, u);
          __m512i green = YuvToGreen(y, u, v);
          __m512i red = YuvToRed(y, v);
          Store<align, mask>(bgr + 0 * A, InterleaveBgr<0>(blue, green, red), tails[0]);
          Store<align, mask>(bgr + 1 * A, InterleaveBgr<1>(blue, green, red), tails[1]);
          Store<align, mask>(bgr + 2 * A, InterleaveBgr<2>(blue, green, red), tails[2]);
      }
      //}}}
      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv420pToBgr(const uint8_t * y0, const uint8_t * y1, const uint8_t * u, const uint8_t * v, uint8_t * bgr0, uint8_t * bgr1, const __mmask64 * tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i u0 = UnpackU8<0>(_u, _u);
          __m512i u1 = UnpackU8<1>(_u, _u);
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          __m512i v0 = UnpackU8<0>(_v, _v);
          __m512i v1 = UnpackU8<1>(_v, _v);
          YuvToBgr<align, mask>(Load<align, mask>(y0 + 0, tails[1]), u0, v0, bgr0 + 0 * A, tails + 3);
          YuvToBgr<align, mask>(Load<align, mask>(y0 + A, tails[2]), u1, v1, bgr0 + 3 * A, tails + 6);
          YuvToBgr<align, mask>(Load<align, mask>(y1 + 0, tails[1]), u0, v0, bgr1 + 0 * A, tails + 3);
          YuvToBgr<align, mask>(Load<align, mask>(y1 + A, tails[2]), u1, v1, bgr1 + 3 * A, tails + 6);
      }
      //}}}
      //{{{
      template <bool align> void Yuv420pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
          }

          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[9];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 6; ++i)
              tailMasks[3 + i] = TailMask64(tail * 6 - A * i);
          for (size_t row = 0; row < height; row += 2)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv420pToBgr<align, false>(y + col * 2, y + yStride + col * 2, u + col, v + col, bgr + col * 6, bgr + bgrStride + col * 6, tailMasks);
              if (col < width)
                  Yuv420pToBgr<align, true>(y + col * 2, y + yStride + col * 2, u + col, v + col, bgr + col * 6, bgr + bgrStride + col * 6, tailMasks);
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              bgr += 2 * bgrStride;
          }
      }
      //}}}
      //{{{
      void Yuv420pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride))
              Yuv420pToBgr<true>(y, yStride, u, uStride, v, vStride, width, height, bgr, bgrStride);
          else
              Yuv420pToBgr<false>(y, yStride, u, uStride, v, vStride, width, height, bgr, bgrStride);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv422pToBgr(const uint8_t * y, const uint8_t * u, const uint8_t * v, uint8_t * bgr, const __mmask64 * tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          YuvToBgr<align, mask>(Load<align, mask>(y + 0, tails[1]), _mm512_unpacklo_epi8(_u, _u), _mm512_unpacklo_epi8(_v, _v), bgr + 0 * A, tails + 3);
          YuvToBgr<align, mask>(Load<align, mask>(y + A, tails[2]), _mm512_unpackhi_epi8(_u, _u), _mm512_unpackhi_epi8(_v, _v), bgr + 3 * A, tails + 6);
      }
      //}}}
      //{{{
      template <bool align> void Yuv422pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
      {
          assert((width % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
          }

          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[9];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 6; ++i)
              tailMasks[3 + i] = TailMask64(tail * 6 - A * i);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv422pToBgr<align, false>(y + col * 2, u + col, v + col, bgr + col * 6, tailMasks);
              if (col < width)
                  Yuv422pToBgr<align, true>(y + col * 2, u + col, v + col, bgr + col * 6, tailMasks);
              y += yStride;
              u += uStride;
              v += vStride;
              bgr += bgrStride;
          }
      }
      //}}}
      //{{{
      void Yuv422pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride))
              Yuv422pToBgr<true>(y, yStride, u, uStride, v, vStride, width, height, bgr, bgrStride);
          else
              Yuv422pToBgr<false>(y, yStride, u, uStride, v, vStride, width, height, bgr, bgrStride);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv444pToBgr(const uint8_t * y, const uint8_t * u, const uint8_t * v, uint8_t * bgr, const __mmask64 * tails)
      {
          YuvToBgr<align, mask>(Load<align, mask>(y, tails[0]), Load<align, mask>(u, tails[0]), Load<align, mask>(v, tails[0]), bgr, tails + 1);
      }

      template <bool align> void Yuv444pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
      {
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
          }

          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[4];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 3; ++i)
              tailMasks[1 + i] = TailMask64(tail * 3 - A * i);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv444pToBgr<align, false>(y + col, u + col, v + col, bgr + col * 3, tailMasks);
              if (col < width)
                  Yuv444pToBgr<align, true>(y + col, u + col, v + col, bgr + col * 3, tailMasks);
              y += yStride;
              u += uStride;
              v += vStride;
              bgr += bgrStride;
          }
      }
      //}}}
      //{{{
      void Yuv444pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride))
              Yuv444pToBgr<true>(y, yStride, u, uStride, v, vStride, width, height, bgr, bgrStride);
          else
              Yuv444pToBgr<false>(y, yStride, u, uStride, v, vStride, width, height, bgr, bgrStride);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void YuvToRgb(const __m512i& y, const __m512i& u, const __m512i& v, uint8_t* rgb, const __mmask64* tails)
      {
          __m512i blue = YuvToBlue(y, u);
          __m512i green = YuvToGreen(y, u, v);
          __m512i red = YuvToRed(y, v);
          Store<align, mask>(rgb + 0 * A, InterleaveBgr<0>(red, green, blue), tails[0]);
          Store<align, mask>(rgb + 1 * A, InterleaveBgr<1>(red, green, blue), tails[1]);
          Store<align, mask>(rgb + 2 * A, InterleaveBgr<2>(red, green, blue), tails[2]);
      }
      //}}}
      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv420pToRgb(const uint8_t* y0, const uint8_t* y1, const uint8_t* u, const uint8_t* v, uint8_t* rgb0, uint8_t* rgb1, const __mmask64* tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i u0 = UnpackU8<0>(_u, _u);
          __m512i u1 = UnpackU8<1>(_u, _u);
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          __m512i v0 = UnpackU8<0>(_v, _v);
          __m512i v1 = UnpackU8<1>(_v, _v);
          YuvToRgb<align, mask>(Load<align, mask>(y0 + 0, tails[1]), u0, v0, rgb0 + 0 * A, tails + 3);
          YuvToRgb<align, mask>(Load<align, mask>(y0 + A, tails[2]), u1, v1, rgb0 + 3 * A, tails + 6);
          YuvToRgb<align, mask>(Load<align, mask>(y1 + 0, tails[1]), u0, v0, rgb1 + 0 * A, tails + 3);
          YuvToRgb<align, mask>(Load<align, mask>(y1 + A, tails[2]), u1, v1, rgb1 + 3 * A, tails + 6);
      }
      //}}}
      //{{{
      template <bool align> void Yuv420pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride));
          }

          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[9];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 6; ++i)
              tailMasks[3 + i] = TailMask64(tail * 6 - A * i);
          for (size_t row = 0; row < height; row += 2)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv420pToRgb<align, false>(y + col * 2, y + yStride + col * 2, u + col, v + col, rgb + col * 6, rgb + rgbStride + col * 6, tailMasks);
              if (col < width)
                  Yuv420pToRgb<align, true>(y + col * 2, y + yStride + col * 2, u + col, v + col, rgb + col * 6, rgb + rgbStride + col * 6, tailMasks);
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              rgb += 2 * rgbStride;
          }
      }
      //}}}
      //{{{
      void Yuv420pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride))
              Yuv420pToRgb<true>(y, yStride, u, uStride, v, vStride, width, height, rgb, rgbStride);
          else
              Yuv420pToRgb<false>(y, yStride, u, uStride, v, vStride, width, height, rgb, rgbStride);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv422pToRgb(const uint8_t* y, const uint8_t* u, const uint8_t* v, uint8_t* rgb, const __mmask64* tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          YuvToRgb<align, mask>(Load<align, mask>(y + 0, tails[1]), _mm512_unpacklo_epi8(_u, _u), _mm512_unpacklo_epi8(_v, _v), rgb + 0 * A, tails + 3);
          YuvToRgb<align, mask>(Load<align, mask>(y + A, tails[2]), _mm512_unpackhi_epi8(_u, _u), _mm512_unpackhi_epi8(_v, _v), rgb + 3 * A, tails + 6);
      }

      template <bool align> void Yuv422pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
      {
          assert((width % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride));
          }

          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[9];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 6; ++i)
              tailMasks[3 + i] = TailMask64(tail * 6 - A * i);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv422pToRgb<align, false>(y + col * 2, u + col, v + col, rgb + col * 6, tailMasks);
              if (col < width)
                  Yuv422pToRgb<align, true>(y + col * 2, u + col, v + col, rgb + col * 6, tailMasks);
              y += yStride;
              u += uStride;
              v += vStride;
              rgb += rgbStride;
          }
      }
      //}}}
      //{{{
      void Yuv422pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride))
              Yuv422pToRgb<true>(y, yStride, u, uStride, v, vStride, width, height, rgb, rgbStride);
          else
              Yuv422pToRgb<false>(y, yStride, u, uStride, v, vStride, width, height, rgb, rgbStride);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv444pToRgb(const uint8_t* y, const uint8_t* u, const uint8_t* v, uint8_t* rgb, const __mmask64* tails)
      {
          YuvToRgb<align, mask>(Load<align, mask>(y, tails[0]), Load<align, mask>(u, tails[0]), Load<align, mask>(v, tails[0]), rgb, tails + 1);
      }
      //}}}
      //{{{
      template <bool align> void Yuv444pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
      {
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride));
          }

          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[4];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 3; ++i)
              tailMasks[1 + i] = TailMask64(tail * 3 - A * i);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv444pToRgb<align, false>(y + col, u + col, v + col, rgb + col * 3, tailMasks);
              if (col < width)
                  Yuv444pToRgb<align, true>(y + col, u + col, v + col, rgb + col * 3, tailMasks);
              y += yStride;
              u += uStride;
              v += vStride;
              rgb += rgbStride;
          }
      }
      //}}}
      //{{{
      void Yuv444pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride))
              Yuv444pToRgb<true>(y, yStride, u, uStride, v, vStride, width, height, rgb, rgbStride);
          else
              Yuv444pToRgb<false>(y, yStride, u, uStride, v, vStride, width, height, rgb, rgbStride);
      }
      //}}}

      //{{{
      SIMD_INLINE void Uyvy422ToYuv420p(const uint8_t* uyvy0, size_t uyvyStride, uint8_t* y0, size_t yStride,
          uint8_t* u, uint8_t* v, __mmask32 uyvyMask0, __mmask32 uyvyMask1, __mmask32 yuvMask)
      {
          static const __m512i SHFL = SIMD_MM512_SETR_EPI8(
              0x0, 0x4, 0x8, 0xC, 0x2, 0x6, 0xA, 0xE, 0x1, 0x3, 0x5, 0x7, 0x9, 0xB, 0xD, 0xF,
              0x0, 0x4, 0x8, 0xC, 0x2, 0x6, 0xA, 0xE, 0x1, 0x3, 0x5, 0x7, 0x9, 0xB, 0xD, 0xF,
              0x0, 0x4, 0x8, 0xC, 0x2, 0x6, 0xA, 0xE, 0x1, 0x3, 0x5, 0x7, 0x9, 0xB, 0xD, 0xF,
              0x0, 0x4, 0x8, 0xC, 0x2, 0x6, 0xA, 0xE, 0x1, 0x3, 0x5, 0x7, 0x9, 0xB, 0xD, 0xF);
          static const __m512i PRMYY = SIMD_MM512_SETR_EPI32(0x02, 0x03, 0x06, 0x07, 0x0A, 0x0B, 0x0E, 0x0F, 0x12, 0x13, 0x16, 0x17, 0x1A, 0x1B, 0x1E, 0x1F);
          static const __m512i PRMUV = SIMD_MM512_SETR_EPI32(0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x01, 0x05, 0x09, 0x0D, 0x11, 0x15, 0x19, 0x1D);

          __m512i uyvy00 = _mm512_shuffle_epi8(_mm512_maskz_loadu_epi16(uyvyMask0, uyvy0 + 0 * 64), SHFL);
          __m512i uyvy01 = _mm512_shuffle_epi8(_mm512_maskz_loadu_epi16(uyvyMask1, uyvy0 + 1 * 64), SHFL);

          const uint8_t* uyvy1 = uyvy0 + uyvyStride;
          __m512i uyvy10 = _mm512_shuffle_epi8(_mm512_maskz_loadu_epi16(uyvyMask0, uyvy1 + 0 * 64), SHFL);
          __m512i uyvy11 = _mm512_shuffle_epi8(_mm512_maskz_loadu_epi16(uyvyMask1, uyvy1 + 1 * 64), SHFL);

          uint8_t* y1 = y0 + yStride;
          _mm512_mask_storeu_epi16(y0, yuvMask, _mm512_permutex2var_epi32(uyvy00, PRMYY, uyvy01));
          _mm512_mask_storeu_epi16(y1, yuvMask, _mm512_permutex2var_epi32(uyvy10, PRMYY, uyvy11));

          __m512i uv = _mm512_avg_epu8(_mm512_permutex2var_epi32(uyvy00, PRMUV, uyvy01), _mm512_permutex2var_epi32(uyvy10, PRMUV, uyvy11));
          _mm256_mask_storeu_epi8(u, yuvMask, _mm512_extracti64x4_epi64(uv, 0));
          _mm256_mask_storeu_epi8(v, yuvMask, _mm512_extracti64x4_epi64(uv, 1));
      }
      //}}}
      //{{{
      void Uyvy422ToYuv420p(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0) && width >= 2 * A);

          assert((width % 2 == 0) && (height % 2 == 0) && width >= 2 * A);

          size_t size = width / 2;
          size_t size32 = AlignLo(size, 32);
          size_t tail = size - size32;
          __mmask32 yuvMask = TailMask32(tail);
          __mmask32 uyvyMask0 = TailMask32(tail * 2 - 32 * 0);
          __mmask32 uyvyMask1 = TailMask32(tail * 2 - 32 * 1);

          for (size_t row = 0; row < height; row += 2)
          {
              size_t colUyvy = 0, colY = 0, colUV = 0;
              for (; colUV < size32; colY += 64, colUV += 32, colUyvy += 128)
                  Uyvy422ToYuv420p(uyvy + colUyvy, uyvyStride, y + colY, yStride, u + colUV, v + colUV, __mmask32(-1), __mmask32(-1), __mmask32(-1));
              if (tail)
                  Uyvy422ToYuv420p(uyvy + colUyvy, uyvyStride, y + colY, yStride, u + colUV, v + colUV, uyvyMask0, uyvyMask1, yuvMask);
              uyvy += 2 * uyvyStride;
              y += 2 * yStride;
              u += uStride;
              v += vStride;
          }
      }
      //}}}

      template <bool align, bool mask, class T>
      #if defined(_MSC_VER) && _MSC_VER >= 1900
      inline
      #else
      SIMD_INLINE
      #endif
      //{{{
      void Yuv420pToBgraV2(const uint8_t* y0, const uint8_t* y1, const uint8_t* u, const uint8_t* v,
          const __m512i& a, uint8_t* bgra0, uint8_t* bgra1, const __mmask64* tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i u0 = UnpackU8<0>(_u, _u);
          __m512i u1 = UnpackU8<1>(_u, _u);
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          __m512i v0 = UnpackU8<0>(_v, _v);
          __m512i v1 = UnpackU8<1>(_v, _v);
          YuvToBgra<align, mask, T>(Load<align, mask>(y0 + 0, tails[1]), u0, v0, a, bgra0 + 00, tails + 3);
          YuvToBgra<align, mask, T>(Load<align, mask>(y0 + A, tails[2]), u1, v1, a, bgra0 + QA, tails + 7);
          YuvToBgra<align, mask, T>(Load<align, mask>(y1 + 0, tails[1]), u0, v0, a, bgra1 + 00, tails + 3);
          YuvToBgra<align, mask, T>(Load<align, mask>(y1 + A, tails[2]), u1, v1, a, bgra1 + QA, tails + 7);
      }
      //}}}

      //{{{
      template <bool align, class T> void Yuv420pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
      {
          assert((width % 2 == 0) && (height % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          __m512i a = _mm512_set1_epi8(alpha);
          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[11];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 8; ++i)
              tailMasks[3 + i] = TailMask64(tail * 8 - A * i);
          for (size_t row = 0; row < height; row += 2)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv420pToBgraV2<align, false, T>(y + col * 2, y + yStride + col * 2, u + col, v + col, a, bgra + col * 8, bgra + bgraStride + col * 8, tailMasks);
              if (col < width)
                  Yuv420pToBgraV2<align, true, T>(y + col * 2, y + yStride + col * 2, u + col, v + col, a, bgra + col * 8, bgra + bgraStride + col * 8, tailMasks);
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              bgra += 2 * bgraStride;
          }
      }
      //}}}
      //{{{
      template <bool align> void Yuv420pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType)
      {
          switch (yuvType)
          {
          case SimdYuvBt601: Yuv420pToBgraV2<align, Base::Bt601>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          case SimdYuvBt709: Yuv420pToBgraV2<align, Base::Bt709>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          case SimdYuvBt2020: Yuv420pToBgraV2<align, Base::Bt2020>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          case SimdYuvTrect871: Yuv420pToBgraV2<align, Base::Trect871>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          default:
              assert(0);
          }
      }
      //}}}
      //{{{
      void Yuv420pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              Yuv420pToBgraV2<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha, yuvType);
          else
              Yuv420pToBgraV2<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha, yuvType);
      }
      //}}}

      //{{{
      SIMD_INLINE void Yuv420pToUyvy422(const uint8_t* y0, size_t yStride, const uint8_t* u, const uint8_t* v,
          uint8_t* uyvy0, size_t uyvyStride, __mmask32 yuvMask, __mmask32 uyvyMask0, __mmask32 uyvyMask1)
      {
          static const __m512i PRM0 = SIMD_MM512_SETR_EPI32(0x00, 0x08, 0x10, 0x11, 0x01, 0x09, 0x12, 0x13, 0x02, 0x0A, 0x14, 0x15, 0x03, 0x0B, 0x16, 0x17);
          static const __m512i PRM1 = SIMD_MM512_SETR_EPI32(0x04, 0x0C, 0x18, 0x19, 0x05, 0x0D, 0x1A, 0x1B, 0x06, 0x0E, 0x1C, 0x1D, 0x07, 0x0F, 0x1E, 0x1F);
          static const __m512i SHFL = SIMD_MM512_SETR_EPI8(
              0x0, 0x8, 0x4, 0x9, 0x1, 0xA, 0x5, 0xB, 0x2, 0xC, 0x6, 0xD, 0x3, 0xE, 0x7, 0xF,
              0x0, 0x8, 0x4, 0x9, 0x1, 0xA, 0x5, 0xB, 0x2, 0xC, 0x6, 0xD, 0x3, 0xE, 0x7, 0xF,
              0x0, 0x8, 0x4, 0x9, 0x1, 0xA, 0x5, 0xB, 0x2, 0xC, 0x6, 0xD, 0x3, 0xE, 0x7, 0xF,
              0x0, 0x8, 0x4, 0x9, 0x1, 0xA, 0x5, 0xB, 0x2, 0xC, 0x6, 0xD, 0x3, 0xE, 0x7, 0xF);
          __m512i uv = Load(u, v, yuvMask);
          __m512i _y0 = _mm512_maskz_loadu_epi16(yuvMask, y0);
          _mm512_mask_storeu_epi16(uyvy0 + 0 * 64, uyvyMask0, _mm512_shuffle_epi8(_mm512_permutex2var_epi32(uv, PRM0, _y0), SHFL));
          _mm512_mask_storeu_epi16(uyvy0 + 1 * 64, uyvyMask1, _mm512_shuffle_epi8(_mm512_permutex2var_epi32(uv, PRM1, _y0), SHFL));
          __m512i _y1 = _mm512_maskz_loadu_epi16(yuvMask, y0 + yStride);
          uint8_t* uyvy1 = uyvy0 + uyvyStride;
          _mm512_mask_storeu_epi16(uyvy1 + 0 * 64, uyvyMask0, _mm512_shuffle_epi8(_mm512_permutex2var_epi32(uv, PRM0, _y1), SHFL));
          _mm512_mask_storeu_epi16(uyvy1 + 1 * 64, uyvyMask1, _mm512_shuffle_epi8(_mm512_permutex2var_epi32(uv, PRM1, _y1), SHFL));
      }
      //}}}
      //{{{
      void Yuv420pToUyvy422(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride,
          const uint8_t* v, size_t vStride, size_t width, size_t height, uint8_t* uyvy, size_t uyvyStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0) && width >= 2 * A);

          size_t size = width / 2;
          size_t size32 = AlignLo(size, 32);
          size_t tail = size - size32;
          __mmask32 yuvMask = TailMask32(tail);
          __mmask32 uyvyMask0 = TailMask32(tail * 2 - 32 * 0);
          __mmask32 uyvyMask1 = TailMask32(tail * 2 - 32 * 1);

          for (size_t row = 0; row < height; row += 2)
          {
              size_t colY = 0, colUV = 0, colUyvy = 0;
              for (; colUV < size32; colY += 64, colUV += 32, colUyvy += 128)
                  Yuv420pToUyvy422(y + colY, yStride, u + colUV, v + colUV, uyvy + colUyvy, uyvyStride, __mmask32(-1), __mmask32(-1), __mmask32(-1));
              if (tail)
                  Yuv420pToUyvy422(y + colY, yStride, u + colUV, v + colUV, uyvy + colUyvy, uyvyStride, yuvMask, uyvyMask0, uyvyMask1);
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              uyvy += 2 * uyvyStride;
          }
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void YuvToBgra(const __m512i & y, const __m512i & u, const __m512i & v, const __m512i & a, uint8_t * bgra, const __mmask64 * tails)
      {
          __m512i b = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, YuvToBlue(y, u));
          __m512i g = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, YuvToGreen(y, u, v));
          __m512i r = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, YuvToRed(y, v));
          __m512i bg0 = UnpackU8<0>(b, g);
          __m512i bg1 = UnpackU8<1>(b, g);
          __m512i ra0 = UnpackU8<0>(r, a);
          __m512i ra1 = UnpackU8<1>(r, a);
          Store<align, mask>(bgra + 0 * A, UnpackU16<0>(bg0, ra0), tails[0]);
          Store<align, mask>(bgra + 1 * A, UnpackU16<1>(bg0, ra0), tails[1]);
          Store<align, mask>(bgra + 2 * A, UnpackU16<0>(bg1, ra1), tails[2]);
          Store<align, mask>(bgra + 3 * A, UnpackU16<1>(bg1, ra1), tails[3]);
      }
      //}}}
      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuva420pToBgra(const uint8_t * y0, const uint8_t * y1, const uint8_t * u, const uint8_t * v,
          const uint8_t * a0, const uint8_t * a1, uint8_t * bgra0, uint8_t * bgra1, const __mmask64 * tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i u0 = UnpackU8<0>(_u, _u);
          __m512i u1 = UnpackU8<1>(_u, _u);
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          __m512i v0 = UnpackU8<0>(_v, _v);
          __m512i v1 = UnpackU8<1>(_v, _v);
          YuvToBgra<align, mask>(Load<align, mask>(y0 + 0, tails[1]), u0, v0, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<align, mask>(a0 + 0, tails[1]))), bgra0 + 00, tails + 3);
          YuvToBgra<align, mask>(Load<align, mask>(y0 + A, tails[2]), u1, v1, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<align, mask>(a0 + A, tails[2]))), bgra0 + QA, tails + 7);
          YuvToBgra<align, mask>(Load<align, mask>(y1 + 0, tails[1]), u0, v0, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<align, mask>(a1 + 0, tails[1]))), bgra1 + 00, tails + 3);
          YuvToBgra<align, mask>(Load<align, mask>(y1 + A, tails[2]), u1, v1, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<align, mask>(a1 + A, tails[2]))), bgra1 + QA, tails + 7);
      }
      //}}}
      //{{{
      template <bool align> void Yuva420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          const uint8_t * a, size_t aStride, size_t width, size_t height, uint8_t * bgra, size_t bgraStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride));
              assert(Aligned(a) && Aligned(aStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[11];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 8; ++i)
              tailMasks[3 + i] = TailMask64(tail * 8 - A * i);
          for (size_t row = 0; row < height; row += 2)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuva420pToBgra<align, false>(y + col * 2, y + yStride + col * 2, u + col, v + col, a + col * 2, a + aStride + col * 2,
                      bgra + col * 8, bgra + bgraStride + col * 8, tailMasks);
              if (col < width)
                  Yuva420pToBgra<align, true>(y + col * 2, y + yStride + col * 2, u + col, v + col, a + col * 2, a + aStride + col * 2,
                      bgra + col * 8, bgra + bgraStride + col * 8, tailMasks);
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              a += 2 * aStride;
              bgra += 2 * bgraStride;
          }
      }
      //}}}
      //{{{
      void Yuva420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          const uint8_t * a, size_t aStride, size_t width, size_t height, uint8_t * bgra, size_t bgraStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride)
              && Aligned(a) && Aligned(aStride) && Aligned(bgra) && Aligned(bgraStride))
              Yuva420pToBgra<true>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride);
          else
              Yuva420pToBgra<false>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride);
      }
      //}}}

      //{{{
      template <class T> SIMD_INLINE void YuvaToBgra16(__m512i y16, __m512i u16, __m512i v16, const __m512i& a16, __m512i* bgra, __mmask64 tail)
      {
          const __m512i b16 = YuvToBlue16<T>(y16, u16);
          const __m512i g16 = YuvToGreen16<T>(y16, u16, v16);
          const __m512i r16 = YuvToRed16<T>(y16, v16);
          const __m512i bg8 = _mm512_or_si512(b16, _mm512_slli_epi16(g16, 8));
          const __m512i ra8 = _mm512_or_si512(r16, _mm512_slli_epi16(a16, 8));
          __m512i bgra0 = _mm512_unpacklo_epi16(bg8, ra8);
          __m512i bgra1 = _mm512_unpackhi_epi16(bg8, ra8);
          _mm512_mask_storeu_epi32(bgra + 0, __mmask16(tail >> 0 * 16), bgra0);
          _mm512_mask_storeu_epi32(bgra + 1, __mmask16(tail >> 1 * 16), bgra1);
      }
      //}}}
      //{{{
      template <class T> SIMD_INLINE void Yuva444pToBgraV2(const uint8_t* y, const uint8_t* u, const uint8_t* v, const uint8_t* a, uint8_t* bgra, __mmask64 tail = __mmask64(-1))
      {
          __m512i _y = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, _mm512_maskz_loadu_epi8(tail, y));
          __m512i _u = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, _mm512_maskz_loadu_epi8(tail, u));
          __m512i _v = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, _mm512_maskz_loadu_epi8(tail, v));
          __m512i _a = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, _mm512_maskz_loadu_epi8(tail, a));
          YuvaToBgra16<T>(UnpackY<T, 0>(_y), UnpackUV<T, 0>(_u), UnpackUV<T, 0>(_v), UnpackU8<0>(_a), (__m512i*)bgra + 0, tail >> 0 * 16);
          YuvaToBgra16<T>(UnpackY<T, 1>(_y), UnpackUV<T, 1>(_u), UnpackUV<T, 1>(_v), UnpackU8<1>(_a), (__m512i*)bgra + 2, tail >> 2 * 16);
      }
      //}}}
      //{{{
      template <class T> void Yuva444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride,
          const uint8_t* v, size_t vStride, const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride)
      {
          size_t widthA = AlignLo(width, A);
          __mmask64 tail = TailMask64(width - widthA);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < widthA; col += A)
                  Yuva444pToBgraV2<T>(y + col, u + col, v + col, a + col, bgra + col * 4);
              if (tail)
                  Yuva444pToBgraV2<T>(y + col, u + col, v + col, a + col, bgra + col * 4, tail);
              y += yStride;
              u += uStride;
              v += vStride;
              a += aStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void Yuva444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride, SimdYuvType yuvType)
      {
          switch (yuvType)
          {
          case SimdYuvBt601: Yuva444pToBgraV2<Base::Bt601>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
          case SimdYuvBt709: Yuva444pToBgraV2<Base::Bt709>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
          case SimdYuvBt2020: Yuva444pToBgraV2<Base::Bt2020>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
          case SimdYuvTrect871: Yuva444pToBgraV2<Base::Trect871>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
          default:
              assert(0);
          }
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv420pToBgra(const uint8_t * y0, const uint8_t * y1, const uint8_t * u, const uint8_t * v,
          const __m512i & a, uint8_t * bgra0, uint8_t * bgra1, const __mmask64 * tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i u0 = UnpackU8<0>(_u, _u);
          __m512i u1 = UnpackU8<1>(_u, _u);
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          __m512i v0 = UnpackU8<0>(_v, _v);
          __m512i v1 = UnpackU8<1>(_v, _v);
          YuvToBgra<align, mask>(Load<align, mask>(y0 + 0, tails[1]), u0, v0, a, bgra0 + 00, tails + 3);
          YuvToBgra<align, mask>(Load<align, mask>(y0 + A, tails[2]), u1, v1, a, bgra0 + QA, tails + 7);
          YuvToBgra<align, mask>(Load<align, mask>(y1 + 0, tails[1]), u0, v0, a, bgra1 + 00, tails + 3);
          YuvToBgra<align, mask>(Load<align, mask>(y1 + A, tails[2]), u1, v1, a, bgra1 + QA, tails + 7);
      }
      //}}}
      //{{{
      template <bool align> void Yuv420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          assert((width % 2 == 0) && (height % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          __m512i a = _mm512_set1_epi8(alpha);
          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[11];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 8; ++i)
              tailMasks[3 + i] = TailMask64(tail * 8 - A * i);
          for (size_t row = 0; row < height; row += 2)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv420pToBgra<align, false>(y + col * 2, y + yStride + col * 2, u + col, v + col, a, bgra + col * 8, bgra + bgraStride + col * 8, tailMasks);
              if (col < width)
                  Yuv420pToBgra<align, true>(y + col * 2, y + yStride + col * 2, u + col, v + col, a, bgra + col * 8, bgra + bgraStride + col * 8, tailMasks);
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              bgra += 2 * bgraStride;
          }
      }
      //}}}
      //{{{
      void Yuv420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              Yuv420pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
          else
              Yuv420pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv422pToBgra(const uint8_t * y, const uint8_t * u, const uint8_t * v, const __m512i & a, uint8_t * bgra, const __mmask64 * tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          YuvToBgra<align, mask>(Load<align, mask>(y + 0, tails[1]), UnpackU8<0>(_u, _u), UnpackU8<0>(_v, _v), a, bgra + 00, tails + 3);
          YuvToBgra<align, mask>(Load<align, mask>(y + A, tails[2]), UnpackU8<1>(_u, _u), UnpackU8<1>(_v, _v), a, bgra + QA, tails + 7);
      }
      //}}}
      //{{{
      template <bool align> void Yuv422pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          assert((width % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          __m512i a = _mm512_set1_epi8(alpha);
          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[11];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t i = 0; i < 8; ++i)
              tailMasks[3 + i] = TailMask64(tail * 8 - A * i);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv422pToBgra<align, false>(y + col * 2, u + col, v + col, a, bgra + col * 8, tailMasks);
              if (col < width)
                  Yuv422pToBgra<align, true>(y + col * 2, u + col, v + col, a, bgra + col * 8, tailMasks);
              y += yStride;
              u += uStride;
              v += vStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void Yuv422pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              Yuv422pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
          else
              Yuv422pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv444pToBgra(const uint8_t * y, const uint8_t * u, const uint8_t * v, const __m512i & a, uint8_t * bgra, const __mmask64 * tails)
      {
          YuvToBgra<align, mask>(Load<align, mask>(y, tails[0]), Load<align, mask>(u, tails[0]), Load<align, mask>(v, tails[0]), a, bgra, tails + 1);
      }
      //}}}
      //{{{
      template <bool align> void Yuv444pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          __m512i a = _mm512_set1_epi8(alpha);
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[5];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 4; ++i)
              tailMasks[1 + i] = TailMask64(tail * 4 - A * i);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv444pToBgra<align, false>(y + col, u + col, v + col, a, bgra + col * 4, tailMasks);
              if (col < width)
                  Yuv444pToBgra<align, true>(y + col, u + col, v + col, a, bgra + col * 4, tailMasks);
              y += yStride;
              u += uStride;
              v += vStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void Yuv444pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              Yuv444pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
          else
              Yuv444pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
      }
      //}}}

      //{{{
      template <bool align, bool mask, class T> SIMD_INLINE void Yuv444pToBgraV2(const uint8_t* y,
          const uint8_t* u, const uint8_t* v, const __m512i& a, uint8_t* bgra, const __mmask64* tails)
      {
          YuvToBgra<align, mask, T>(Load<align, mask>(y, tails[0]), Load<align, mask>(u, tails[0]), Load<align, mask>(v, tails[0]), a, bgra, tails + 1);
      }
      //}}}
      //{{{
      template <bool align, class T> void Yuv444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
      {
          assert(width >= A);
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          __m512i a = _mm512_set1_epi8(alpha);
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[5];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 4; ++i)
              tailMasks[1 + i] = TailMask64(tail * 4 - A * i);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv444pToBgraV2<align, false, T>(y + col, u + col, v + col, a, bgra + col * 4, tailMasks);
              if (col < width)
                  Yuv444pToBgraV2<align, true, T>(y + col, u + col, v + col, a, bgra + col * 4, tailMasks);
              y += yStride;
              u += uStride;
              v += vStride;
              bgra += bgraStride;
          }

      }
      //}}}
      //{{{
      template <bool align> void Yuv444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType)
      {
          switch (yuvType)
          {
          case SimdYuvBt601: Yuv444pToBgraV2<align, Base::Bt601>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          case SimdYuvBt709: Yuv444pToBgraV2<align, Base::Bt709>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          case SimdYuvBt2020: Yuv444pToBgraV2<align, Base::Bt2020>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          case SimdYuvTrect871: Yuv444pToBgraV2<align, Base::Trect871>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
          default:
              assert(0);
          }
      }
      //}}}
      //{{{
      void Yuv444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
          size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              Yuv444pToBgraV2<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha, yuvType);
          else
              Yuv444pToBgraV2<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha, yuvType);
      }
      //}}}

      //{{{

      SIMD_INLINE __m512i MulDiv32(const __m512i & dividend, const __m512i & divisor, const __m512 & KF_255_DIV_6)
      {
          return _mm512_cvttps_epi32(_mm512_div_ps(_mm512_mul_ps(KF_255_DIV_6, _mm512_cvtepi32_ps(dividend)), _mm512_cvtepi32_ps(divisor)));
      }
      //}}}
      //{{{
      SIMD_INLINE __m512i MulDiv16(const __m512i & dividend, const __m512i & divisor, const __m512 & KF_255_DIV_6)
      {
          const __m512i quotientLo = MulDiv32(_mm512_unpacklo_epi16(dividend, K_ZERO), _mm512_unpacklo_epi16(divisor, K_ZERO), KF_255_DIV_6);
          const __m512i quotientHi = MulDiv32(_mm512_unpackhi_epi16(dividend, K_ZERO), _mm512_unpackhi_epi16(divisor, K_ZERO), KF_255_DIV_6);
          return _mm512_packs_epi32(quotientLo, quotientHi);
      }
      //}}}
      //{{{
      SIMD_INLINE __m512i AdjustedYuvToHue16(const __m512i & y, const __m512i & u, const __m512i & v, const __m512 & KF_255_DIV_6)
      {
          const __m512i red = AdjustedYuvToRed16(y, v);
          const __m512i green = AdjustedYuvToGreen16(y, u, v);
          const __m512i blue = AdjustedYuvToBlue16(y, u);
          const __m512i max = MaxI16(red, green, blue);
          const __m512i range = _mm512_subs_epi16(max, MinI16(red, green, blue));

          const __mmask32 redMaxMask = _mm512_cmpeq_epi16_mask(red, max);
          const __mmask32 greenMaxMask = (~redMaxMask)&_mm512_cmpeq_epi16_mask(green, max);
          const __mmask32 blueMaxMask = ~(redMaxMask | greenMaxMask);

          __m512i dividend = _mm512_maskz_add_epi16(redMaxMask, _mm512_sub_epi16(green, blue), _mm512_mullo_epi16(range, K16_0006));
          dividend = _mm512_mask_add_epi16(dividend, greenMaxMask, _mm512_sub_epi16(blue, red), _mm512_mullo_epi16(range, K16_0002));
          dividend = _mm512_mask_add_epi16(dividend, blueMaxMask, _mm512_sub_epi16(red, green), _mm512_mullo_epi16(range, K16_0004));

          return _mm512_and_si512(MulDiv16(dividend, range, KF_255_DIV_6), _mm512_maskz_set1_epi16(_mm512_cmpneq_epi16_mask(range, K_ZERO), 0xFF));
      }
      //}}}
      //{{{
      template <bool align, bool mask> SIMD_INLINE void YuvToHue(const __m512i & y, const __m512i & u, const __m512i & v, const __m512 & KF_255_DIV_6, uint8_t * hue, __mmask64 tail)
      {
          __m512i lo = AdjustedYuvToHue16(AdjustY16(UnpackU8<0>(y)), AdjustUV16(UnpackU8<0>(u)), AdjustUV16(UnpackU8<0>(v)), KF_255_DIV_6);
          __m512i hi = AdjustedYuvToHue16(AdjustY16(UnpackU8<1>(y)), AdjustUV16(UnpackU8<1>(u)), AdjustUV16(UnpackU8<1>(v)), KF_255_DIV_6);
          Store<align, mask>(hue, _mm512_packus_epi16(lo, hi), tail);
      }
      //}}}
      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv420pToHue(const uint8_t * y0, const uint8_t * y1, const uint8_t * u, const uint8_t * v,
          const __m512 & KF_255_DIV_6, uint8_t * hue0, uint8_t * hue1, const __mmask64 * tails)
      {
          __m512i _u = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(u, tails[0])));
          __m512i u0 = UnpackU8<0>(_u, _u);
          __m512i u1 = UnpackU8<1>(_u, _u);
          __m512i _v = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<align, mask>(v, tails[0])));
          __m512i v0 = UnpackU8<0>(_v, _v);
          __m512i v1 = UnpackU8<1>(_v, _v);
          YuvToHue<align, mask>(Load<align, mask>(y0 + 0, tails[1]), u0, v0, KF_255_DIV_6, hue0 + 0, tails[1]);
          YuvToHue<align, mask>(Load<align, mask>(y0 + A, tails[2]), u1, v1, KF_255_DIV_6, hue0 + A, tails[2]);
          YuvToHue<align, mask>(Load<align, mask>(y1 + 0, tails[1]), u0, v0, KF_255_DIV_6, hue1 + 0, tails[1]);
          YuvToHue<align, mask>(Load<align, mask>(y1 + A, tails[2]), u1, v1, KF_255_DIV_6, hue1 + A, tails[2]);
      }
      //}}}
      //{{{
      template <bool align> void Yuv420pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * hue, size_t hueStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(hue) && Aligned(hueStride));
          }

          const __m512 KF_255_DIV_6 = _mm512_set1_ps(Base::KF_255_DIV_6);

          width /= 2;
          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMasks[3];
          tailMasks[0] = TailMask64(tail);
          for (size_t i = 0; i < 2; ++i)
              tailMasks[1 + i] = TailMask64(tail * 2 - A * i);
          for (size_t row = 0; row < height; row += 2)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv420pToHue<align, false>(y + col * 2, y + yStride + col * 2, u + col, v + col, KF_255_DIV_6, hue + col * 2, hue + hueStride + col * 2, tailMasks);
              if (col < width)
                  Yuv420pToHue<align, true>(y + col * 2, y + yStride + col * 2, u + col, v + col, KF_255_DIV_6, hue + col * 2, hue + hueStride + col * 2, tailMasks);
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              hue += 2 * hueStride;
          }
      }
      //}}}
      //{{{
      void Yuv420pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride, size_t width, size_t height, uint8_t * hue, size_t hueStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride) && Aligned(hue) && Aligned(hueStride))
              Yuv420pToHue<true>(y, yStride, u, uStride, v, vStride, width, height, hue, hueStride);
          else
              Yuv420pToHue<false>(y, yStride, u, uStride, v, vStride, width, height, hue, hueStride);
      }
      //}}}

      //{{{
      template <bool align, bool mask> SIMD_INLINE void Yuv444pToHue(const uint8_t * y, const uint8_t * u, const uint8_t * v, const __m512 & KF_255_DIV_6, uint8_t * hue, __mmask64 tail = -1)
      {
          YuvToHue<align, mask>(Load<align, mask>(y, tail), Load<align, mask>(u, tail), Load<align, mask>(v, tail), KF_255_DIV_6, hue, tail);
      }
      //}}}
      //{{{
      template <bool align> void Yuv444pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * hue, size_t hueStride)
      {
          assert(width >= A);
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(hue) && Aligned(hueStride));
          }

          const __m512 KF_255_DIV_6 = _mm512_set1_ps(Base::KF_255_DIV_6);

          size_t alignedWidth = AlignLo(width, A);
          size_t tail = width - alignedWidth;
          __mmask64 tailMask = TailMask64(tail);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < alignedWidth; col += A)
                  Yuv444pToHue<align, false>(y + col, u + col, v + col, KF_255_DIV_6, hue + col);
              if (col < width)
                  Yuv444pToHue<align, true>(y + col, u + col, v + col, KF_255_DIV_6, hue + col, tailMask);
              y += yStride;
              u += uStride;
              v += vStride;
              hue += hueStride;
          }
      }
      //}}}
      //{{{
      void Yuv444pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
          size_t width, size_t height, uint8_t * hue, size_t hueStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride) && Aligned(hue) && Aligned(hueStride))
              Yuv444pToHue<true>(y, yStride, u, uStride, v, vStride, width, height, hue, hueStride);
          else
              Yuv444pToHue<false>(y, yStride, u, uStride, v, vStride, width, height, hue, hueStride);
      }
      //}}}

      //{{{
      template <class T> SIMD_INLINE __m512i UnpackY(__m512i uyvy)
      {
          static const __m512i Y_SHUFFLE = SIMD_MM512_SETR_EPI8(
              0x1, -1, 0x3, -1, 0x5, -1, 0x7, -1, 0x9, -1, 0xB, -1, 0xD, -1, 0xF, -1,
              0x1, -1, 0x3, -1, 0x5, -1, 0x7, -1, 0x9, -1, 0xB, -1, 0xD, -1, 0xF, -1,
              0x1, -1, 0x3, -1, 0x5, -1, 0x7, -1, 0x9, -1, 0xB, -1, 0xD, -1, 0xF, -1,
              0x1, -1, 0x3, -1, 0x5, -1, 0x7, -1, 0x9, -1, 0xB, -1, 0xD, -1, 0xF, -1);
          static const __m512i Y_LO = SIMD_MM512_SET1_EPI16(T::Y_LO);
          return _mm512_subs_epi16(_mm512_shuffle_epi8(uyvy, Y_SHUFFLE), Y_LO);
      }
      //}}}
      //{{{
      template <class T> SIMD_INLINE __m512i UnpackU(__m512i uyvy)
      {
          static const __m512i U_SHUFFLE = SIMD_MM512_SETR_EPI8(
              0x0, -1, 0x0, -1, 0x4, -1, 0x4, -1, 0x8, -1, 0x8, -1, 0xC, -1, 0xC, -1,
              0x0, -1, 0x0, -1, 0x4, -1, 0x4, -1, 0x8, -1, 0x8, -1, 0xC, -1, 0xC, -1,
              0x0, -1, 0x0, -1, 0x4, -1, 0x4, -1, 0x8, -1, 0x8, -1, 0xC, -1, 0xC, -1,
              0x0, -1, 0x0, -1, 0x4, -1, 0x4, -1, 0x8, -1, 0x8, -1, 0xC, -1, 0xC, -1);
          static const __m512i U_Z = SIMD_MM512_SET1_EPI16(T::UV_Z);
          return _mm512_subs_epi16(_mm512_shuffle_epi8(uyvy, U_SHUFFLE), U_Z);
      }
      //}}}
      //{{{
      template <class T> SIMD_INLINE __m512i UnpackV(__m512i uyvy)
      {
          static const __m512i V_SHUFFLE = SIMD_MM512_SETR_EPI8(
              0x2, -1, 0x2, -1, 0x6, -1, 0x6, -1, 0xA, -1, 0xA, -1, 0xE, -1, 0xE, -1,
              0x2, -1, 0x2, -1, 0x6, -1, 0x6, -1, 0xA, -1, 0xA, -1, 0xE, -1, 0xE, -1,
              0x2, -1, 0x2, -1, 0x6, -1, 0x6, -1, 0xA, -1, 0xA, -1, 0xE, -1, 0xE, -1,
              0x2, -1, 0x2, -1, 0x6, -1, 0x6, -1, 0xA, -1, 0xA, -1, 0xE, -1, 0xE, -1);
          static const __m512i V_Z= SIMD_MM512_SET1_EPI16(T::UV_Z);
          return _mm512_subs_epi16(_mm512_shuffle_epi8(uyvy, V_SHUFFLE), V_Z);
      }
      //}}}
      //{{{
      template <bool align, bool mask, class T> SIMD_INLINE void Uyvy422ToBgr(const uint8_t* uyvy, uint8_t* bgr, __mmask64 tails[5])
      {
          __m512i uyvy0 = Load<align, mask>(uyvy + 0 * A, tails[0]);
          __m512i y0 = UnpackY<T>(uyvy0);
          __m512i u0 = UnpackU<T>(uyvy0);
          __m512i v0 = UnpackV<T>(uyvy0);
          __m512i b0 = YuvToBlue16<T>(y0, u0);
          __m512i g0 = YuvToGreen16<T>(y0, u0, v0);
          __m512i r0 = YuvToRed16<T>(y0, v0);

          __m512i uyvy1 = Load<align, mask>(uyvy + 1 * A, tails[1]);
          __m512i y1 = UnpackY<T>(uyvy1);
          __m512i u1 = UnpackU<T>(uyvy1);
          __m512i v1 = UnpackV<T>(uyvy1);
          __m512i b1 = YuvToBlue16<T>(y1, u1);
          __m512i g1 = YuvToGreen16<T>(y1, u1, v1);
          __m512i r1 = YuvToRed16<T>(y1, v1);

          __m512i b = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_packus_epi16(b0, b1));
          __m512i g = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_packus_epi16(g0, g1));
          __m512i r = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_packus_epi16(r0, r1));
          Store<align, mask>(bgr + 0 * A, InterleaveBgr<0>(b, g, r), tails[2]);
          Store<align, mask>(bgr + 1 * A, InterleaveBgr<1>(b, g, r), tails[3]);
          Store<align, mask>(bgr + 2 * A, InterleaveBgr<2>(b, g, r), tails[4]);
      }
      //}}}
      //{{{
      template <bool align, class T> void Uyvy422ToBgr(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* bgr, size_t bgrStride)
      {
          assert((width % 2 == 0) && (width >= 2 * A));
          if (align)
              assert(Aligned(uyvy) && Aligned(uyvyStride) && Aligned(bgr) && Aligned(bgrStride));

          size_t widthA = AlignLo(width, A);
          size_t sizeS = width * 2, sizeD = width * 3;
          size_t sizeSA = widthA * 2, sizeDA = widthA * 3;
          __mmask64 tails[5];
          if (widthA < width)
          {
              tails[0] = TailMask64(sizeS - sizeSA - A * 0);
              tails[1] = TailMask64(sizeS - sizeSA - A * 1);
              tails[2] = TailMask64(sizeD - sizeDA - A * 0);
              tails[3] = TailMask64(sizeD - sizeDA - A * 1);
              tails[4] = TailMask64(sizeD - sizeDA - A * 2);
          }
          for (size_t row = 0; row < height; ++row)
          {
              size_t colS = 0, colD = 0;
              for (; colS < sizeSA; colS += 2 * A, colD += 3 * A)
                  Uyvy422ToBgr<align, false, T>(uyvy + colS, bgr + colD, tails);
              if(widthA < width)
                  Uyvy422ToBgr<align, true, T>(uyvy + colS, bgr + colD, tails);
              uyvy += uyvyStride;
              bgr += bgrStride;
          }
      }
      //}}}
      //{{{
      template<bool align> void Uyvy422ToBgr(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* bgr, size_t bgrStride, SimdYuvType yuvType)
      {
          switch (yuvType)
          {
          case SimdYuvBt601: Uyvy422ToBgr<align, Base::Bt601>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
          case SimdYuvBt709: Uyvy422ToBgr<align, Base::Bt709>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
          case SimdYuvBt2020: Uyvy422ToBgr<align, Base::Bt2020>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
          case SimdYuvTrect871: Uyvy422ToBgr<align, Base::Trect871>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
          default:
              assert(0);
          }
      }
      //}}}
      //{{{
      void Uyvy422ToBgr(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* bgr, size_t bgrStride, SimdYuvType yuvType)
      {
          if (Aligned(uyvy) && Aligned(uyvyStride) && Aligned(bgr) && Aligned(bgrStride))
              Uyvy422ToBgr<true>(uyvy, uyvyStride, width, height, bgr, bgrStride, yuvType);
          else
              Uyvy422ToBgr<false>(uyvy, uyvyStride, width, height, bgr, bgrStride, yuvType);
      }
      //}}}
      }
  #endif
  }
