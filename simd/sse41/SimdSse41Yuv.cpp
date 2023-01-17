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
#include "SimdInterleave.h"
#include "SimdDeinterleave.h"
#include "SimdYuvToBgr.h"

namespace Simd::Sse41 {
  // to RGB
  //{{{
  template <bool align> SIMD_INLINE void YuvToBgr(__m128i y, __m128i u, __m128i v, __m128i * bgr)
  {
      __m128i blue = YuvToBlue(y, u);
      __m128i green = YuvToGreen(y, u, v);
      __m128i red = YuvToRed(y, v);
      Store<align>(bgr + 0, InterleaveBgr<0>(blue, green, red));
      Store<align>(bgr + 1, InterleaveBgr<1>(blue, green, red));
      Store<align>(bgr + 2, InterleaveBgr<2>(blue, green, red));
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv444pToBgr(const uint8_t * y, const uint8_t * u, const uint8_t * v, uint8_t * bgr)
  {
      YuvToBgr<align>(Load<align>((__m128i*)y), Load<align>((__m128i*)u), Load<align>((__m128i*)v), (__m128i*)bgr);
  }
  //}}}
  //{{{
  template <bool align> void Yuv444pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
      size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
  {
      assert(width >= A);
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
      }

      size_t bodyWidth = AlignLo(width, A);
      size_t tail = width - bodyWidth;
      size_t A3 = A * 3;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colYuv = 0, colBgr = 0; colYuv < bodyWidth; colYuv += A, colBgr += A3)
          {
              Yuv444pToBgr<align>(y + colYuv, u + colYuv, v + colYuv, bgr + colBgr);
          }
          if (tail)
          {
              size_t col = width - A;
              Yuv444pToBgr<false>(y + col, u + col, v + col, bgr + 3 * col);
          }
          y += yStride;
          u += uStride;
          v += vStride;
          bgr += bgrStride;
      }
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv422pToBgr(const uint8_t * y, const __m128i & u, const __m128i & v, uint8_t * bgr)
  {
      YuvToBgr<align>(Load<align>((__m128i*)y + 0), _mm_unpacklo_epi8(u, u), _mm_unpacklo_epi8(v, v), (__m128i*)bgr + 0);
      YuvToBgr<align>(Load<align>((__m128i*)y + 1), _mm_unpackhi_epi8(u, u), _mm_unpackhi_epi8(v, v), (__m128i*)bgr + 3);
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv422pToBgr(const uint8_t * y, const uint8_t * u, const uint8_t * v, uint8_t * bgr)
  {
      Yuv422pToBgr<align>(y, Load<align>((__m128i*)u), Load<align>((__m128i*)v), bgr);
  }
  //}}}
  //{{{
  template <bool align> void Yuv422pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
      size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
  {
      assert((width % 2 == 0) && (width >= DA));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
      }

      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      size_t A6 = A * 6;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colUV = 0, colY = 0, colBgr = 0; colY < bodyWidth; colY += DA, colUV += A, colBgr += A6)
              Yuv422pToBgr<align>(y + colY, u + colUV, v + colUV, bgr + colBgr);
          if (tail)
          {
              size_t offset = width - DA;
              Yuv422pToBgr<false>(y + offset, u + offset / 2, v + offset / 2, bgr + 3 * offset);
          }
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
  template <bool align> void Yuv420pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
      size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
      }

      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      size_t A6 = A * 6;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colBgr = 0; colY < bodyWidth; colY += DA, colUV += A, colBgr += A6)
          {
              __m128i u_ = Load<align>((__m128i*)(u + colUV));
              __m128i v_ = Load<align>((__m128i*)(v + colUV));
              Yuv422pToBgr<align>(y + colY, u_, v_, bgr + colBgr);
              Yuv422pToBgr<align>(y + colY + yStride, u_, v_, bgr + colBgr + bgrStride);
          }
          if (tail)
          {
              size_t offset = width - DA;
              __m128i u_ = Load<false>((__m128i*)(u + offset / 2));
              __m128i v_ = Load<false>((__m128i*)(v + offset / 2));
              Yuv422pToBgr<false>(y + offset, u_, v_, bgr + 3 * offset);
              Yuv422pToBgr<false>(y + offset + yStride, u_, v_, bgr + 3 * offset + bgrStride);
          }
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
  template <bool align> SIMD_INLINE void YuvToRgb(__m128i y, __m128i u, __m128i v, __m128i* rgb)
  {
      __m128i blue = YuvToBlue(y, u);
      __m128i green = YuvToGreen(y, u, v);
      __m128i red = YuvToRed(y, v);
      Store<align>(rgb + 0, InterleaveBgr<0>(red, green, blue));
      Store<align>(rgb + 1, InterleaveBgr<1>(red, green, blue));
      Store<align>(rgb + 2, InterleaveBgr<2>(red, green, blue));
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv444pToRgb(const uint8_t* y, const uint8_t* u, const uint8_t* v, uint8_t* rgb)
  {
      YuvToRgb<align>(Load<align>((__m128i*)y), Load<align>((__m128i*)u), Load<align>((__m128i*)v), (__m128i*)rgb);
  }
  //}}}
  //{{{
  template <bool align> void Yuv444pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
  {
      assert(width >= A);
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride));
      }

      size_t bodyWidth = AlignLo(width, A);
      size_t tail = width - bodyWidth;
      size_t A3 = A * 3;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colYuv = 0, colRgb = 0; colYuv < bodyWidth; colYuv += A, colRgb += A3)
          {
              Yuv444pToRgb<align>(y + colYuv, u + colYuv, v + colYuv, rgb + colRgb);
          }
          if (tail)
          {
              size_t col = width - A;
              Yuv444pToRgb<false>(y + col, u + col, v + col, rgb + 3 * col);
          }
          y += yStride;
          u += uStride;
          v += vStride;
          rgb += rgbStride;
      }
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv422pToRgb(const uint8_t* y, const __m128i& u, const __m128i& v, uint8_t* rgb)
  {
      YuvToRgb<align>(Load<align>((__m128i*)y + 0), _mm_unpacklo_epi8(u, u), _mm_unpacklo_epi8(v, v), (__m128i*)rgb + 0);
      YuvToRgb<align>(Load<align>((__m128i*)y + 1), _mm_unpackhi_epi8(u, u), _mm_unpackhi_epi8(v, v), (__m128i*)rgb + 3);
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv422pToRgb(const uint8_t* y, const uint8_t* u, const uint8_t* v, uint8_t* rgb)
  {
      Yuv422pToRgb<align>(y, Load<align>((__m128i*)u), Load<align>((__m128i*)v), rgb);
  }
  //}}}
  //{{{
  template <bool align> void Yuv422pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
  {
      assert((width % 2 == 0) && (width >= DA));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride));
      }

      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      size_t A6 = A * 6;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colUV = 0, colY = 0, colRgb = 0; colY < bodyWidth; colY += DA, colUV += A, colRgb += A6)
              Yuv422pToRgb<align>(y + colY, u + colUV, v + colUV, rgb + colRgb);
          if (tail)
          {
              size_t offset = width - DA;
              Yuv422pToRgb<false>(y + offset, u + offset / 2, v + offset / 2, rgb + 3 * offset);
          }
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
  template <bool align> void Yuv420pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(rgb) && Aligned(rgbStride));
      }

      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      size_t A6 = A * 6;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colRgb = 0; colY < bodyWidth; colY += DA, colUV += A, colRgb += A6)
          {
              __m128i u_ = Load<align>((__m128i*)(u + colUV));
              __m128i v_ = Load<align>((__m128i*)(v + colUV));
              Yuv422pToRgb<align>(y + colY, u_, v_, rgb + colRgb);
              Yuv422pToRgb<align>(y + colY + yStride, u_, v_, rgb + colRgb + rgbStride);
          }
          if (tail)
          {
              size_t offset = width - DA;
              __m128i u_ = Load<false>((__m128i*)(u + offset / 2));
              __m128i v_ = Load<false>((__m128i*)(v + offset / 2));
              Yuv422pToRgb<false>(y + offset, u_, v_, rgb + 3 * offset);
              Yuv422pToRgb<false>(y + offset + yStride, u_, v_, rgb + 3 * offset + rgbStride);
          }
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

  // to BGRA
  //{{{
  template <class T> SIMD_INLINE void YuvaToBgra16(__m128i y16, __m128i u16, __m128i v16, const __m128i& a16, __m128i* bgra)
  {
      const __m128i b16 = YuvToBlue16<T>(y16, u16);
      const __m128i g16 = YuvToGreen16<T>(y16, u16, v16);
      const __m128i r16 = YuvToRed16<T>(y16, v16);
      const __m128i bg8 = _mm_or_si128(b16, _mm_slli_si128(g16, 1));
      const __m128i ra8 = _mm_or_si128(r16, _mm_slli_si128(a16, 1));
      _mm_storeu_si128(bgra + 0, _mm_unpacklo_epi16(bg8, ra8));
      _mm_storeu_si128(bgra + 1, _mm_unpackhi_epi16(bg8, ra8));
  }
  //}}}
  //{{{
  template <class T> SIMD_INLINE void Yuva444pToBgraV2(const uint8_t* y, const uint8_t* u, const uint8_t* v, const uint8_t* a, uint8_t* bgra)
  {
      __m128i _y = _mm_loadu_si128((__m128i*)y);
      __m128i _u = _mm_loadu_si128((__m128i*)u);
      __m128i _v = _mm_loadu_si128((__m128i*)v);
      __m128i _a = _mm_loadu_si128((__m128i*)a);
      YuvaToBgra16<T>(UnpackY<T, 0>(_y), UnpackUV<T, 0>(_u), UnpackUV<T, 0>(_v), UnpackU8<0>(_a), (__m128i*)bgra + 0);
      YuvaToBgra16<T>(UnpackY<T, 1>(_y), UnpackUV<T, 1>(_u), UnpackUV<T, 1>(_v), UnpackU8<1>(_a), (__m128i*)bgra + 2);
  }
  //}}}
  //{{{
  template <class T> void Yuva444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride,
      const uint8_t* v, size_t vStride, const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride)
  {
      assert((width >= A));

      size_t widthA = AlignLo(width, A);
      size_t tail = width - widthA;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < widthA; col += A)
              Yuva444pToBgraV2<T>(y + col, u + col, v + col, a + col, bgra + col * 4);
          if (tail)
          {
              size_t col = width - A;
              Yuva444pToBgraV2<T>(y + col, u + col, v + col, a + col, bgra + col * 4);
          }
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
  template <bool align> SIMD_INLINE void AdjustedYuv16ToBgra(__m128i y16, __m128i u16, __m128i v16,
      const __m128i& a_0, __m128i* bgra)
  {
      const __m128i b16 = AdjustedYuvToBlue16(y16, u16);
      const __m128i g16 = AdjustedYuvToGreen16(y16, u16, v16);
      const __m128i r16 = AdjustedYuvToRed16(y16, v16);
      const __m128i bg8 = _mm_or_si128(b16, _mm_slli_si128(g16, 1));
      const __m128i ra8 = _mm_or_si128(r16, a_0);
      Store<align>(bgra + 0, _mm_unpacklo_epi16(bg8, ra8));
      Store<align>(bgra + 1, _mm_unpackhi_epi16(bg8, ra8));
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv16ToBgra(__m128i y16, __m128i u16, __m128i v16,
      const __m128i& a_0, __m128i* bgra)
  {
      AdjustedYuv16ToBgra<align>(AdjustY16(y16), AdjustUV16(u16), AdjustUV16(v16), a_0, bgra);
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuva8ToBgra(__m128i y8, __m128i u8, __m128i v8, const __m128i& a8, __m128i* bgra)
  {
      Yuv16ToBgra<align>(_mm_unpacklo_epi8(y8, K_ZERO), _mm_unpacklo_epi8(u8, K_ZERO),
          _mm_unpacklo_epi8(v8, K_ZERO), _mm_unpacklo_epi8(K_ZERO, a8), bgra + 0);
      Yuv16ToBgra<align>(_mm_unpackhi_epi8(y8, K_ZERO), _mm_unpackhi_epi8(u8, K_ZERO),
          _mm_unpackhi_epi8(v8, K_ZERO), _mm_unpackhi_epi8(K_ZERO, a8), bgra + 2);
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuva422pToBgra(const uint8_t* y, const __m128i& u, const __m128i& v,
      const uint8_t* a, uint8_t* bgra)
  {
      Yuva8ToBgra<align>(Load<align>((__m128i*)y + 0), _mm_unpacklo_epi8(u, u), _mm_unpacklo_epi8(v, v), Load<align>((__m128i*)a + 0), (__m128i*)bgra + 0);
      Yuva8ToBgra<align>(Load<align>((__m128i*)y + 1), _mm_unpackhi_epi8(u, u), _mm_unpackhi_epi8(v, v), Load<align>((__m128i*)a + 1), (__m128i*)bgra + 4);
  }
  //}}}
  //{{{
  template <bool align> void Yuva420pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride));
          assert(Aligned(a) && Aligned(aStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colBgra = 0; colY < bodyWidth; colY += DA, colUV += A, colBgra += OA)
          {
              __m128i u_ = Load<align>((__m128i*)(u + colUV));
              __m128i v_ = Load<align>((__m128i*)(v + colUV));
              Yuva422pToBgra<align>(y + colY, u_, v_, a + colY, bgra + colBgra);
              Yuva422pToBgra<align>(y + colY + yStride, u_, v_, a + colY + aStride, bgra + colBgra + bgraStride);
          }
          if (tail)
          {
              size_t offset = width - DA;
              __m128i u_ = Load<false>((__m128i*)(u + offset / 2));
              __m128i v_ = Load<false>((__m128i*)(v + offset / 2));
              Yuva422pToBgra<false>(y + offset, u_, v_, a + offset, bgra + 4 * offset);
              Yuva422pToBgra<false>(y + offset + yStride, u_, v_, a + offset + aStride, bgra + 4 * offset + bgraStride);
          }
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          a += 2 * aStride;
          bgra += 2 * bgraStride;
      }
  }
  //}}}
  //{{{
  void Yuva420pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride)
  {
      if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride)
          && Aligned(a) && Aligned(aStride) && Aligned(bgra) && Aligned(bgraStride))
          Yuva420pToBgra<true>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride);
      else
          Yuva420pToBgra<false>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride);
  }
  //}}}

  //{{{
  template <bool align> SIMD_INLINE void Yuv8ToBgra(__m128i y8, __m128i u8, __m128i v8, const __m128i& a_0, __m128i* bgra)
  {
      Yuv16ToBgra<align>(_mm_unpacklo_epi8(y8, K_ZERO), _mm_unpacklo_epi8(u8, K_ZERO),
          _mm_unpacklo_epi8(v8, K_ZERO), a_0, bgra + 0);
      Yuv16ToBgra<align>(_mm_unpackhi_epi8(y8, K_ZERO), _mm_unpackhi_epi8(u8, K_ZERO),
          _mm_unpackhi_epi8(v8, K_ZERO), a_0, bgra + 2);
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv444pToBgra(const uint8_t* y, const uint8_t* u,
      const uint8_t* v, const __m128i& a_0, uint8_t* bgra)
  {
      Yuv8ToBgra<align>(Load<align>((__m128i*)y), Load<align>((__m128i*)u), Load<align>((__m128i*)v), a_0, (__m128i*)bgra);
  }
  //}}}
  //{{{
  template <bool align> void Yuv444pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      assert(width >= A);
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      __m128i a_0 = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
      size_t bodyWidth = AlignLo(width, A);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colYuv = 0, colBgra = 0; colYuv < bodyWidth; colYuv += A, colBgra += QA)
          {
              Yuv444pToBgra<align>(y + colYuv, u + colYuv, v + colYuv, a_0, bgra + colBgra);
          }
          if (tail)
          {
              size_t col = width - A;
              Yuv444pToBgra<false>(y + col, u + col, v + col, a_0, bgra + 4 * col);
          }
          y += yStride;
          u += uStride;
          v += vStride;
          bgra += bgraStride;
      }
  }
  //}}}
  //{{{
  void Yuv444pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
          && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
          Yuv444pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
      else
          Yuv444pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
  }
  //}}}

  //{{{
  template <bool align> SIMD_INLINE void Yuv422pToBgra(const uint8_t* y, const __m128i& u, const __m128i& v,
      const __m128i& a_0, uint8_t* bgra)
  {
      Yuv8ToBgra<align>(Load<align>((__m128i*)y + 0), _mm_unpacklo_epi8(u, u), _mm_unpacklo_epi8(v, v), a_0, (__m128i*)bgra + 0);
      Yuv8ToBgra<align>(Load<align>((__m128i*)y + 1), _mm_unpackhi_epi8(u, u), _mm_unpackhi_epi8(v, v), a_0, (__m128i*)bgra + 4);
  }
  //}}}
  //{{{
  template <bool align> void Yuv420pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      __m128i a_0 = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colBgra = 0; colY < bodyWidth; colY += DA, colUV += A, colBgra += OA)
          {
              __m128i u_ = Load<align>((__m128i*)(u + colUV));
              __m128i v_ = Load<align>((__m128i*)(v + colUV));
              Yuv422pToBgra<align>(y + colY, u_, v_, a_0, bgra + colBgra);
              Yuv422pToBgra<align>(y + colY + yStride, u_, v_, a_0, bgra + colBgra + bgraStride);
          }
          if (tail)
          {
              size_t offset = width - DA;
              __m128i u_ = Load<false>((__m128i*)(u + offset / 2));
              __m128i v_ = Load<false>((__m128i*)(v + offset / 2));
              Yuv422pToBgra<false>(y + offset, u_, v_, a_0, bgra + 4 * offset);
              Yuv422pToBgra<false>(y + offset + yStride, u_, v_, a_0, bgra + 4 * offset + bgraStride);
          }
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          bgra += 2 * bgraStride;
      }
  }
  //}}}
  //{{{
  void Yuv420pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
          && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
          Yuv420pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
      else
          Yuv420pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
  }
  //}}}

  //{{{
  template <bool align> SIMD_INLINE void Yuv422pToBgra(const uint8_t* y, const uint8_t* u, const uint8_t* v, const __m128i& a_0, uint8_t* bgra)
  {
      Yuv422pToBgra<align>(y, Load<align>((__m128i*)u), Load<align>((__m128i*)v), a_0, bgra);
  }
  //}}}
  //{{{
  template <bool align> void Yuv422pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      assert((width % 2 == 0) && (width >= DA));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      __m128i a_0 = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colUV = 0, colY = 0, colBgra = 0; colY < bodyWidth; colY += DA, colUV += A, colBgra += OA)
              Yuv422pToBgra<align>(y + colY, u + colUV, v + colUV, a_0, bgra + colBgra);
          if (tail)
          {
              size_t offset = width - DA;
              Yuv422pToBgra<false>(y + offset, u + offset / 2, v + offset / 2, a_0, bgra + 4 * offset);
          }
          y += yStride;
          u += uStride;
          v += vStride;
          bgra += bgraStride;
      }
  }
  //}}}
  //{{{
  void Yuv422pToBgra(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
          && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
          Yuv422pToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
      else
          Yuv422pToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
  }
  //}}}

  //{{{
  template <bool align, class T> SIMD_INLINE void YuvToBgra16(__m128i y16, __m128i u16, __m128i v16, const __m128i& a_0, __m128i* bgra)
  {
      const __m128i b16 = YuvToBlue16<T>(y16, u16);
      const __m128i g16 = YuvToGreen16<T>(y16, u16, v16);
      const __m128i r16 = YuvToRed16<T>(y16, v16);
      const __m128i bg8 = _mm_or_si128(b16, _mm_slli_si128(g16, 1));
      const __m128i ra8 = _mm_or_si128(r16, a_0);
      Store<align>(bgra + 0, _mm_unpacklo_epi16(bg8, ra8));
      Store<align>(bgra + 1, _mm_unpackhi_epi16(bg8, ra8));
  }
  //}}}
  //{{{
  template <bool align, class T> SIMD_INLINE void YuvToBgra(__m128i y8, __m128i u8, __m128i v8, const __m128i& a_0, __m128i* bgra)
  {
      YuvToBgra16<align, T>(UnpackY<T, 0>(y8), UnpackUV<T, 0>(u8), UnpackUV<T, 0>(v8), a_0, bgra + 0);
      YuvToBgra16<align, T>(UnpackY<T, 1>(y8), UnpackUV<T, 1>(u8), UnpackUV<T, 1>(v8), a_0, bgra + 2);
  }
  //}}}
  //{{{
  template <bool align, class T> SIMD_INLINE void Yuv444pToBgraV2(const uint8_t* y, const uint8_t* u, const uint8_t* v, const __m128i& a_0, uint8_t* bgra)
  {
      YuvToBgra<align, T>(Load<align>((__m128i*)y), Load<align>((__m128i*)u), Load<align>((__m128i*)v), a_0, (__m128i*)bgra);
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

      __m128i a_0 = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
      size_t bodyWidth = AlignLo(width, A);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colYuv = 0, colBgra = 0; colYuv < bodyWidth; colYuv += A, colBgra += QA)
          {
              Yuv444pToBgraV2<align, T>(y + colYuv, u + colYuv, v + colYuv, a_0, bgra + colBgra);
          }
          if (tail)
          {
              size_t col = width - A;
              Yuv444pToBgraV2<false, T>(y + col, u + col, v + col, a_0, bgra + 4 * col);
          }
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
  template <bool align, class T> SIMD_INLINE void Yuv422pToBgraV2(const uint8_t* y, const __m128i& u, const __m128i& v,
      const __m128i& a_0, uint8_t* bgra)
  {
      YuvToBgra<align, T>(Load<align>((__m128i*)y + 0), _mm_unpacklo_epi8(u, u), _mm_unpacklo_epi8(v, v), a_0, (__m128i*)bgra + 0);
      YuvToBgra<align, T>(Load<align>((__m128i*)y + 1), _mm_unpackhi_epi8(u, u), _mm_unpackhi_epi8(v, v), a_0, (__m128i*)bgra + 4);
  }
  //}}}
  //{{{
  template <bool align, class T> void Yuv420pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
      size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      __m128i a_0 = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colBgra = 0; colY < bodyWidth; colY += DA, colUV += A, colBgra += OA)
          {
              __m128i u_ = Load<align>((__m128i*)(u + colUV));
              __m128i v_ = Load<align>((__m128i*)(v + colUV));
              Yuv422pToBgraV2<align, T>(y + colY, u_, v_, a_0, bgra + colBgra);
              Yuv422pToBgraV2<align, T>(y + colY + yStride, u_, v_, a_0, bgra + colBgra + bgraStride);
          }
          if (tail)
          {
              size_t offset = width - DA;
              __m128i u_ = Load<false>((__m128i*)(u + offset / 2));
              __m128i v_ = Load<false>((__m128i*)(v + offset / 2));
              Yuv422pToBgraV2<false, T>(y + offset, u_, v_, a_0, bgra + 4 * offset);
              Yuv422pToBgraV2<false, T>(y + offset + yStride, u_, v_, a_0, bgra + 4 * offset + bgraStride);
          }
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

  // toHue
  //{{{
  SIMD_INLINE __m128i MulDiv32(__m128i dividend, __m128i divisor, const __m128 & KF_255_DIV_6)
  {
      return _mm_cvttps_epi32(_mm_div_ps(_mm_mul_ps(KF_255_DIV_6, _mm_cvtepi32_ps(dividend)), _mm_cvtepi32_ps(divisor)));
  }
  //}}}
  //{{{
  SIMD_INLINE __m128i MulDiv16(__m128i dividend, __m128i divisor, const __m128 & KF_255_DIV_6)
  {
      const __m128i quotientLo = MulDiv32(_mm_unpacklo_epi16(dividend, K_ZERO), _mm_unpacklo_epi16(divisor, K_ZERO), KF_255_DIV_6);
      const __m128i quotientHi = MulDiv32(_mm_unpackhi_epi16(dividend, K_ZERO), _mm_unpackhi_epi16(divisor, K_ZERO), KF_255_DIV_6);
      return _mm_packs_epi32(quotientLo, quotientHi);
  }
  //}}}
  //{{{
  SIMD_INLINE __m128i AdjustedYuvToHue16(__m128i y, __m128i u, __m128i v, const __m128 & KF_255_DIV_6)
  {
      const __m128i red = AdjustedYuvToRed16(y, v);
      const __m128i green = AdjustedYuvToGreen16(y, u, v);
      const __m128i blue = AdjustedYuvToBlue16(y, u);
      const __m128i max = MaxI16(red, green, blue);
      const __m128i range = _mm_subs_epi16(max, MinI16(red, green, blue));

      const __m128i redMaxMask = _mm_cmpeq_epi16(red, max);
      const __m128i greenMaxMask = _mm_andnot_si128(redMaxMask, _mm_cmpeq_epi16(green, max));
      const __m128i blueMaxMask = _mm_andnot_si128(redMaxMask, _mm_andnot_si128(greenMaxMask, K_INV_ZERO));

      const __m128i redMaxCase = _mm_and_si128(redMaxMask,
          _mm_add_epi16(_mm_sub_epi16(green, blue), _mm_mullo_epi16(range, K16_0006)));
      const __m128i greenMaxCase = _mm_and_si128(greenMaxMask,
          _mm_add_epi16(_mm_sub_epi16(blue, red), _mm_mullo_epi16(range, K16_0002)));
      const __m128i blueMaxCase = _mm_and_si128(blueMaxMask,
          _mm_add_epi16(_mm_sub_epi16(red, green), _mm_mullo_epi16(range, K16_0004)));

      const __m128i dividend = _mm_or_si128(_mm_or_si128(redMaxCase, greenMaxCase), blueMaxCase);

      return _mm_andnot_si128(_mm_cmpeq_epi16(range, K_ZERO), _mm_and_si128(MulDiv16(dividend, range, KF_255_DIV_6), K16_00FF));
  }
  //}}}
  //{{{
  SIMD_INLINE __m128i YuvToHue16(__m128i y, __m128i u, __m128i v, const __m128 & KF_255_DIV_6)
  {
      return AdjustedYuvToHue16(AdjustY16(y), AdjustUV16(u), AdjustUV16(v), KF_255_DIV_6);
  }
  //}}}
  //{{{
  SIMD_INLINE __m128i YuvToHue8(__m128i y, __m128i u, __m128i v, const __m128 & KF_255_DIV_6)
  {
      return _mm_packus_epi16(
          YuvToHue16(_mm_unpacklo_epi8(y, K_ZERO), _mm_unpacklo_epi8(u, K_ZERO), _mm_unpacklo_epi8(v, K_ZERO), KF_255_DIV_6),
          YuvToHue16(_mm_unpackhi_epi8(y, K_ZERO), _mm_unpackhi_epi8(u, K_ZERO), _mm_unpackhi_epi8(v, K_ZERO), KF_255_DIV_6));
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void Yuv420pToHue(const uint8_t * y, __m128i u, __m128i v, uint8_t * hue, const __m128 & KF_255_DIV_6)
  {
      Store<align>((__m128i*)(hue), YuvToHue8(Load<align>((__m128i*)(y)), _mm_unpacklo_epi8(u, u), _mm_unpacklo_epi8(v, v), KF_255_DIV_6));
      Store<align>((__m128i*)(hue + A), YuvToHue8(Load<align>((__m128i*)(y + A)), _mm_unpackhi_epi8(u, u), _mm_unpackhi_epi8(v, v), KF_255_DIV_6));
  }
  //}}}
  //{{{
  template <bool align> void Yuv420pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
      size_t width, size_t height, uint8_t * hue, size_t hueStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(hue) && Aligned(hueStride));
      }

      const __m128 KF_255_DIV_6 = _mm_set_ps1(Base::KF_255_DIV_6);

      size_t bodyWidth = AlignLo(width, DA);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, col_hue = 0; colY < bodyWidth; colY += DA, colUV += A, col_hue += DA)
          {
              __m128i u_ = Load<align>((__m128i*)(u + colUV));
              __m128i v_ = Load<align>((__m128i*)(v + colUV));
              Yuv420pToHue<align>(y + colY, u_, v_, hue + col_hue, KF_255_DIV_6);
              Yuv420pToHue<align>(y + yStride + colY, u_, v_, hue + hueStride + col_hue, KF_255_DIV_6);
          }
          if (tail)
          {
              size_t offset = width - DA;
              __m128i u_ = Load<false>((__m128i*)(u + offset / 2));
              __m128i v_ = Load<false>((__m128i*)(v + offset / 2));
              Yuv420pToHue<false>(y + offset, u_, v_, hue + offset, KF_255_DIV_6);
              Yuv420pToHue<false>(y + yStride + offset, u_, v_, hue + hueStride + offset, KF_255_DIV_6);
          }
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          hue += 2 * hueStride;
      }
  }
  //}}}
  //{{{
  void Yuv420pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
      size_t width, size_t height, uint8_t * hue, size_t hueStride)
  {
      if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride) && Aligned(hue) && Aligned(hueStride))
          Yuv420pToHue<true>(y, yStride, u, uStride, v, vStride, width, height, hue, hueStride);
      else
          Yuv420pToHue<false>(y, yStride, u, uStride, v, vStride, width, height, hue, hueStride);
  }
  //}}}

  // to Uyvy422
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

      const __m128 KF_255_DIV_6 = _mm_set_ps1(Base::KF_255_DIV_6);

      size_t bodyWidth = AlignLo(width, A);
      size_t tail = width - bodyWidth;
      for (size_t row = 0; row < height; row += 1)
      {
          for (size_t col = 0; col < bodyWidth; col += A)
          {
              Store<align>((__m128i*)(hue + col), YuvToHue8(Load<align>((__m128i*)(y + col)),
                  Load<align>((__m128i*)(u + col)), Load<align>((__m128i*)(v + col)), KF_255_DIV_6));
          }
          if (tail)
          {
              size_t offset = width - A;
              Store<false>((__m128i*)(hue + offset), YuvToHue8(Load<false>((__m128i*)(y + offset)),
                  Load<false>((__m128i*)(u + offset)), Load<false>((__m128i*)(v + offset)), KF_255_DIV_6));
          }
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
  template<bool align> SIMD_INLINE void Yuv420pToUyvy422(const uint8_t* y0, size_t yStride,
      const uint8_t* u, const uint8_t* v, uint8_t* uyvy0, size_t uyvyStride)
  {
      __m128i u0 = Load<align>((__m128i*)u);
      __m128i v0 = Load<align>((__m128i*)v);
      __m128i uv0 = _mm_unpacklo_epi8(u0, v0);
      __m128i uv1 = _mm_unpackhi_epi8(u0, v0);

      __m128i y00 = Load<align>((__m128i*)y0 + 0);
      __m128i y01 = Load<align>((__m128i*)y0 + 1);
      Store<align>((__m128i*)uyvy0 + 0, _mm_unpacklo_epi8(uv0, y00));
      Store<align>((__m128i*)uyvy0 + 1, _mm_unpackhi_epi8(uv0, y00));
      Store<align>((__m128i*)uyvy0 + 2, _mm_unpacklo_epi8(uv1, y01));
      Store<align>((__m128i*)uyvy0 + 3, _mm_unpackhi_epi8(uv1, y01));

      const uint8_t* y1 = y0 + yStride;
      __m128i y10 = Load<align>((__m128i*)y1 + 0);
      __m128i y11 = Load<align>((__m128i*)y1 + 1);
      uint8_t* uyvy1 = uyvy0 + uyvyStride;
      Store<align>((__m128i*)uyvy1 + 0, _mm_unpacklo_epi8(uv0, y10));
      Store<align>((__m128i*)uyvy1 + 1, _mm_unpackhi_epi8(uv0, y10));
      Store<align>((__m128i*)uyvy1 + 2, _mm_unpacklo_epi8(uv1, y11));
      Store<align>((__m128i*)uyvy1 + 3, _mm_unpackhi_epi8(uv1, y11));
  }
  //}}}
  //{{{
  template<bool align> void Yuv420pToUyvy422(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride,
      const uint8_t* v, size_t vStride, size_t width, size_t height, uint8_t* uyvy, size_t uyvyStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && width >= 2 * A);
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(uyvy) && Aligned(uyvyStride));
      }

      size_t width2A = AlignLo(width, 2 * A);
      size_t tailY = width - 2 * A;
      size_t tailUV = width / 2 - A;
      size_t tailUyvy = width * 2 - 4 * A;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colY = 0, colUV = 0, colUyvy = 0; colY < width2A; colY += 2 * A, colUV += 1 * A, colUyvy += 4 * A)
              Yuv420pToUyvy422<align>(y + colY, yStride, u + colUV, v + colUV, uyvy + colUyvy, uyvyStride);
          if (width2A != width)
              Yuv420pToUyvy422<false>(y + tailY, yStride, u + tailUV, v + tailUV, uyvy + tailUyvy, uyvyStride);
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          uyvy += 2 * uyvyStride;
      }
  }
  //}}}
  //{{{
  void Yuv420pToUyvy422(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride,
      const uint8_t* v, size_t vStride, size_t width, size_t height, uint8_t* uyvy, size_t uyvyStride)
  {
      if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
          && Aligned(v) && Aligned(vStride) && Aligned(uyvy) && Aligned(uyvyStride))
          Yuv420pToUyvy422<true>(y, yStride, u, uStride, v, vStride, width, height, uyvy, uyvyStride);
      else
          Yuv420pToUyvy422<false>(y, yStride, u, uStride, v, vStride, width, height, uyvy, uyvyStride);
  }
  //}}}

  // Uyvy422ToBgr
  //{{{
  template <class T> SIMD_INLINE __m128i UnpackY(__m128i uyvy)
  {
      static const __m128i Y_SHUFFLE = SIMD_MM_SETR_EPI8(0x1, -1, 0x3, -1, 0x5, -1, 0x7, -1, 0x9, -1, 0xB, -1, 0xD, -1, 0xF, -1);
      static const __m128i Y_LO = SIMD_MM_SET1_EPI16(T::Y_LO);
      return _mm_subs_epi16(_mm_shuffle_epi8(uyvy, Y_SHUFFLE), Y_LO);
  }
  //}}}
  //{{{
  template <class T> SIMD_INLINE __m128i UnpackU(__m128i uyvy)
  {
      static const __m128i U_SHUFFLE = SIMD_MM_SETR_EPI8(0x0, -1, 0x0, -1, 0x4, -1, 0x4, -1, 0x8, -1, 0x8, -1, 0xC, -1, 0xC, -1);
      static const __m128i U_Z = SIMD_MM_SET1_EPI16(T::UV_Z);
      return _mm_subs_epi16(_mm_shuffle_epi8(uyvy, U_SHUFFLE), U_Z);
  }
  //}}}
  //{{{
  template <class T> SIMD_INLINE __m128i UnpackV(__m128i uyvy)
  {
      static const __m128i V_SHUFFLE = SIMD_MM_SETR_EPI8(0x2, -1, 0x2, -1, 0x6, -1, 0x6, -1, 0xA, -1, 0xA, -1, 0xE, -1, 0xE, -1);
      static const __m128i V_Z= SIMD_MM_SET1_EPI16(T::UV_Z);
      return _mm_subs_epi16(_mm_shuffle_epi8(uyvy, V_SHUFFLE), V_Z);
  }
  //}}}
  //{{{
  template <bool align, class T> SIMD_INLINE void Uyvy422ToBgr(const uint8_t* uyvy, uint8_t* bgr)
  {
      __m128i uyvy0 = Load<align>((__m128i*)uyvy + 0);
      __m128i y0 = UnpackY<T>(uyvy0);
      __m128i u0 = UnpackU<T>(uyvy0);
      __m128i v0 = UnpackV<T>(uyvy0);
      __m128i blue0 = YuvToBlue16<T>(y0, u0);
      __m128i green0 = YuvToGreen16<T>(y0, u0, v0);
      __m128i red0 = YuvToRed16<T>(y0, v0);

      __m128i uyvy1 = Load<align>((__m128i*)uyvy + 1);
      __m128i y1 = UnpackY<T>(uyvy1);
      __m128i u1 = UnpackU<T>(uyvy1);
      __m128i v1 = UnpackV<T>(uyvy1);
      __m128i blue1 = YuvToBlue16<T>(y1, u1);
      __m128i green1 = YuvToGreen16<T>(y1, u1, v1);
      __m128i red1 = YuvToRed16<T>(y1, v1);

      __m128i blue = _mm_packus_epi16(blue0, blue1);
      __m128i green = _mm_packus_epi16(green0, green1);
      __m128i red = _mm_packus_epi16(red0, red1);
      Store<align>((__m128i*)bgr + 0, InterleaveBgr<0>(blue, green, red));
      Store<align>((__m128i*)bgr + 1, InterleaveBgr<1>(blue, green, red));
      Store<align>((__m128i*)bgr + 2, InterleaveBgr<2>(blue, green, red));
  }
  //}}}
  //{{{
  template <bool align, class T> void Uyvy422ToBgr(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* bgr, size_t bgrStride)
  {
      assert((width % 2 == 0) && (width >= A));
      if (align)
          assert(Aligned(uyvy) && Aligned(uyvyStride) && Aligned(bgr) && Aligned(bgrStride));

      size_t sizeS = width * 2, sizeD = width * 3;
      size_t sizeS2A = AlignLo(sizeS, 2 * A);
      size_t tailS = sizeS - 2 * A;
      size_t tailD = sizeD - 3 * A;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colS = 0, colD = 0; colS < sizeS2A; colS += 2 * A, colD += 3 * A)
              Uyvy422ToBgr<align, T>(uyvy + colS, bgr + colD);
          if (sizeS2A != sizeS)
              Uyvy422ToBgr<false, T>(uyvy + tailS, bgr + tailD);
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

  // Uyvy422ToYuv420p
  //{{{
  template<bool align> SIMD_INLINE void Uyvy422ToYuv420p(const uint8_t* uyvy0, size_t uyvyStride, uint8_t* y0, size_t yStride, uint8_t* u, uint8_t* v)
  {
      __m128i uyvy00 = Load<align>((__m128i*)uyvy0 + 0);
      __m128i uyvy01 = Load<align>((__m128i*)uyvy0 + 1);
      __m128i uyvy02 = Load<align>((__m128i*)uyvy0 + 2);
      __m128i uyvy03 = Load<align>((__m128i*)uyvy0 + 3);

      Store<align>((__m128i*)y0 + 0, Deinterleave8<1>(uyvy00, uyvy01));
      Store<align>((__m128i*)y0 + 1, Deinterleave8<1>(uyvy02, uyvy03));

      const uint8_t* uyvy1 = uyvy0 + uyvyStride;
      __m128i uyvy10 = Load<align>((__m128i*)uyvy1 + 0);
      __m128i uyvy11 = Load<align>((__m128i*)uyvy1 + 1);
      __m128i uyvy12 = Load<align>((__m128i*)uyvy1 + 2);
      __m128i uyvy13 = Load<align>((__m128i*)uyvy1 + 3);

      uint8_t* y1 = y0 + yStride;
      Store<align>((__m128i*)y1 + 0, Deinterleave8<1>(uyvy10, uyvy11));
      Store<align>((__m128i*)y1 + 1, Deinterleave8<1>(uyvy12, uyvy13));

      __m128i uv0 = _mm_avg_epu8(Deinterleave8<0>(uyvy00, uyvy01), Deinterleave8<0>(uyvy10, uyvy11));
      __m128i uv1 = _mm_avg_epu8(Deinterleave8<0>(uyvy02, uyvy03), Deinterleave8<0>(uyvy12, uyvy13));

      Store<align>((__m128i*)u, Deinterleave8<0>(uv0, uv1));
      Store<align>((__m128i*)v, Deinterleave8<1>(uv0, uv1));
  }
  //}}}
  //{{{
  template<bool align> void Uyvy422ToYuv420p(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && width >= 2 * A);
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(uyvy) && Aligned(uyvyStride));
      }

      size_t width2A = AlignLo(width, 2 * A);
      size_t tailUyvy = width * 2 - 4 * A;
      size_t tailY = width - 2 * A;
      size_t tailUV = width / 2 - A;
      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUyvy = 0, colY = 0, colUV = 0; colY < width2A; colUyvy += 4 * A, colY += 2 * A, colUV += 1 * A)
              Uyvy422ToYuv420p<align>(uyvy + colUyvy, uyvyStride, y + colY, yStride, u + colUV, v + colUV);
          if (width2A != width)
              Uyvy422ToYuv420p<false>(uyvy + tailUyvy, uyvyStride, y + tailY, yStride, u + tailUV, v + tailUV);
          uyvy += 2 * uyvyStride;
          y += 2 * yStride;
          u += uStride;
          v += vStride;
      }
  }
  //}}}
  //{{{
  void Uyvy422ToYuv420p(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
  {
      if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
          && Aligned(v) && Aligned(vStride) && Aligned(uyvy) && Aligned(uyvyStride))
          Uyvy422ToYuv420p<true>(uyvy, uyvyStride, width, height, y, yStride, u, uStride, v, vStride);
      else
          Uyvy422ToYuv420p<false>(uyvy, uyvyStride, width, height, y, yStride, u, uStride, v, vStride);
  }
  //}}}
  }
