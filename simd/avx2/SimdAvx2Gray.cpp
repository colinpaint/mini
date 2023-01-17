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
#include "SimdStore.h"
#include "SimdMemory.h"
#include "SimdConversion.h"

namespace Simd::Avx2 {
  // GrayToBgr
  //{{{
  template <bool align> SIMD_INLINE void GrayToBgr (uint8_t * bgr, __m256i gray)
  {
      Store<align>((__m256i*)bgr + 0, GrayToBgr<0>(gray));
      Store<align>((__m256i*)bgr + 1, GrayToBgr<1>(gray));
      Store<align>((__m256i*)bgr + 2, GrayToBgr<2>(gray));
  }
  //}}}
  //{{{
  template <bool align> void GrayToBgr (const uint8_t * gray, size_t width, size_t height, size_t grayStride, uint8_t *bgr, size_t bgrStride)
  {
      assert(width >= A);
      if (align)
          assert(Aligned(bgr) && Aligned(bgrStride) && Aligned(gray) && Aligned(grayStride));

      size_t alignedWidth = AlignLo(width, A);
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < alignedWidth; col += A)
          {
              __m256i _gray = Load<align>((__m256i*)(gray + col));
              GrayToBgr<align>(bgr + 3 * col, _gray);
          }
          if (alignedWidth != width)
          {
              __m256i _gray = Load<false>((__m256i*)(gray + width - A));
              GrayToBgr<false>(bgr + 3 * (width - A), _gray);
          }
          gray += grayStride;
          bgr += bgrStride;
      }
  }
  //}}}
  //{{{
  void GrayToBgr (const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgr, size_t bgrStride)
  {
      if (Aligned(bgr) && Aligned(gray) && Aligned(bgrStride) && Aligned(grayStride))
          GrayToBgr<true>(gray, width, height, grayStride, bgr, bgrStride);
      else
          GrayToBgr<false>(gray, width, height, grayStride, bgr, bgrStride);
  }
  //}}}

  // Int16ToGray
  //{{{
  template <bool align> SIMD_INLINE void Int16ToGray (const int16_t * src, uint8_t * dst)
  {
      __m256i lo = Load<align>((__m256i*)src + 0);
      __m256i hi = Load<align>((__m256i*)src + 1);
      Store<align>((__m256i*)dst, PackI16ToU8(lo, hi));
  }
  //}}}
  //{{{
  template <bool align> void Int16ToGray (const int16_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
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
  void Int16ToGray (const uint8_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      if (Aligned(src) && Aligned(srcStride) && Aligned(dst) && Aligned(dstStride))
          Int16ToGray<true>((const int16_t *)src, width, height, srcStride / sizeof(int16_t), dst, dstStride);
      else
          Int16ToGray<false>((const int16_t *)src, width, height, srcStride / sizeof(int16_t), dst, dstStride);
  }
  //}}}

  // GrayToBgra
  //{{{
  template <bool align> SIMD_INLINE void GrayToBgra (uint8_t * bgra, __m256i gray, __m256i alpha)
  {
      __m256i bgLo = _mm256_unpacklo_epi8(gray, gray);
      __m256i bgHi = _mm256_unpackhi_epi8(gray, gray);
      __m256i raLo = _mm256_unpacklo_epi8(gray, alpha);
      __m256i raHi = _mm256_unpackhi_epi8(gray, alpha);

      Store<align>((__m256i*)bgra + 0, _mm256_unpacklo_epi16(bgLo, raLo));
      Store<align>((__m256i*)bgra + 1, _mm256_unpackhi_epi16(bgLo, raLo));
      Store<align>((__m256i*)bgra + 2, _mm256_unpacklo_epi16(bgHi, raHi));
      Store<align>((__m256i*)bgra + 3, _mm256_unpackhi_epi16(bgHi, raHi));
  }
  //}}}
  //{{{
  template <bool align> void GrayToBgra (const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
  {
      assert(width >= A);
      if (align)
          assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(gray) && Aligned(grayStride));

      __m256i _alpha = _mm256_set1_epi8(alpha);
      __m256i permuteOffsets = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
      size_t alignedWidth = AlignLo(width, A);
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < alignedWidth; col += A)
          {
              __m256i _gray = _mm256_permutevar8x32_epi32(Load<align>((__m256i*)(gray + col)), permuteOffsets);
              GrayToBgra<align>(bgra + 4 * col, _gray, _alpha);
          }
          if (alignedWidth != width)
          {
              __m256i _gray = _mm256_permutevar8x32_epi32(Load<false>((__m256i*)(gray + width - A)), permuteOffsets);
              GrayToBgra<false>(bgra + 4 * (width - A), _gray, _alpha);
          }
          gray += grayStride;
          bgra += bgraStride;
      }
  }
  //}}}
  //{{{
  void GrayToBgra (const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
  {
      if (Aligned(bgra) && Aligned(gray) && Aligned(bgraStride) && Aligned(grayStride))
          GrayToBgra<true>(gray, width, height, grayStride, bgra, bgraStride, alpha);
      else
          GrayToBgra<false>(gray, width, height, grayStride, bgra, bgraStride, alpha);
  }
  //}}}
  }
