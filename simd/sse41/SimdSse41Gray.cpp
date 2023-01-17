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
#include "SimdStore.h"
#include "SimdMemory.h"

namespace Simd::Sse41 {
  //{{{
  //{{{
  template <bool align> SIMD_INLINE void GrayToBgra(uint8_t * bgra, __m128i gray, __m128i alpha)
  {
      __m128i bgLo = _mm_unpacklo_epi8(gray, gray);
      __m128i bgHi = _mm_unpackhi_epi8(gray, gray);
      __m128i raLo = _mm_unpacklo_epi8(gray, alpha);
      __m128i raHi = _mm_unpackhi_epi8(gray, alpha);

      Store<align>((__m128i*)bgra + 0, _mm_unpacklo_epi16(bgLo, raLo));
      Store<align>((__m128i*)bgra + 1, _mm_unpackhi_epi16(bgLo, raLo));
      Store<align>((__m128i*)bgra + 2, _mm_unpacklo_epi16(bgHi, raHi));
      Store<align>((__m128i*)bgra + 3, _mm_unpackhi_epi16(bgHi, raHi));
  }
  //}}}
  //{{{
  template <bool align> void GrayToBgra(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
  {
      assert(width >= A);
      if (align)
          assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(gray) && Aligned(grayStride));

      __m128i _alpha = _mm_set1_epi8(alpha);
      size_t alignedWidth = AlignLo(width, A);
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < alignedWidth; col += A)
          {
              __m128i _gray = Load<align>((__m128i*)(gray + col));
              GrayToBgra<align>(bgra + 4 * col, _gray, _alpha);
          }
          if (alignedWidth != width)
          {
              __m128i _gray = Load<false>((__m128i*)(gray + width - A));
              GrayToBgra<false>(bgra + 4 * (width - A), _gray, _alpha);
          }
          gray += grayStride;
          bgra += bgraStride;
      }
  }
  //}}}
  //{{{
  void GrayToBgra(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
  {
      if (Aligned(bgra) && Aligned(gray) && Aligned(bgraStride) && Aligned(grayStride))
          GrayToBgra<true>(gray, width, height, grayStride, bgra, bgraStride, alpha);
      else
          GrayToBgra<false>(gray, width, height, grayStride, bgra, bgraStride, alpha);
  }
  //}}}

  //{{{
  template <bool align> SIMD_INLINE void GrayToBgr(uint8_t * bgr, __m128i gray)
  {
      Store<align>((__m128i*)bgr + 0, _mm_shuffle_epi8(gray, K8_SHUFFLE_GRAY_TO_BGR0));
      Store<align>((__m128i*)bgr + 1, _mm_shuffle_epi8(gray, K8_SHUFFLE_GRAY_TO_BGR1));
      Store<align>((__m128i*)bgr + 2, _mm_shuffle_epi8(gray, K8_SHUFFLE_GRAY_TO_BGR2));
  }
  //}}}
  //{{{
  template <bool align> void GrayToBgr(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgr, size_t bgrStride)
  {
      assert(width >= A);
      if (align)
          assert(Aligned(bgr) && Aligned(bgrStride) && Aligned(gray) && Aligned(grayStride));

      size_t alignedWidth = AlignLo(width, A);
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < alignedWidth; col += A)
          {
              __m128i _gray = Load<align>((__m128i*)(gray + col));
              GrayToBgr<align>(bgr + 3 * col, _gray);
          }
          if (alignedWidth != width)
          {
              __m128i _gray = Load<false>((__m128i*)(gray + width - A));
              GrayToBgr<false>(bgr + 3 * (width - A), _gray);
          }
          gray += grayStride;
          bgr += bgrStride;
      }
  }
  //}}}
  //{{{
  void GrayToBgr(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgr, size_t bgrStride)
  {
      if (Aligned(bgr) && Aligned(gray) && Aligned(bgrStride) && Aligned(grayStride))
          GrayToBgr<true>(gray, width, height, grayStride, bgr, bgrStride);
      else
          GrayToBgr<false>(gray, width, height, grayStride, bgr, bgrStride);
  }
  //}}}

  //{{{
  template <bool align> SIMD_INLINE void Int16ToGray(const int16_t * src, uint8_t * dst)
  {
      __m128i lo = Load<align>((__m128i*)src + 0);
      __m128i hi = Load<align>((__m128i*)src + 1);
      Store<align>((__m128i*)dst, _mm_packus_epi16(lo, hi));
  }
  //}}}
  //{{{
  template <bool align> void Int16ToGray(const int16_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      assert(width >= A);
      if (align)
          assert(Aligned(src) && Aligned(srcStride, HA) && Aligned(dst) && Aligned(dstStride));

      size_t alignedWidth = AlignLo(width, A);
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < alignedWidth; col += A)
              Int16ToGray<align>(src + col, dst + col);
          if (alignedWidth != width)
              Int16ToGray<false>(src + width - A, dst + width - A);
          src += srcStride;
          dst += dstStride;
      }
  }
  //}}}
  //{{{
  void Int16ToGray(const uint8_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      if (Aligned(src) && Aligned(srcStride) && Aligned(dst) && Aligned(dstStride))
          Int16ToGray<true>((const int16_t *)src, width, height, srcStride / sizeof(int16_t), dst, dstStride);
      else
          Int16ToGray<false>((const int16_t *)src, width, height, srcStride / sizeof(int16_t), dst, dstStride);
  }
  //}}}
  }
  //}}}
