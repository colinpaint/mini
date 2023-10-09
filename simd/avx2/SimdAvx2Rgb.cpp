//{{{
/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2021 Yermalayeu Ihar.
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
#include "SimdConst.h"

namespace Simd {
  #ifdef SIMD_AVX2_ENABLE
    namespace Avx2 {
      //{{{  bgr to gray
      const __m256i K16_BLUE_RED = SIMD_MM256_SET2_EPI16(Base::BLUE_TO_GRAY_WEIGHT, Base::RED_TO_GRAY_WEIGHT);
      const __m256i K16_GREEN_ROUND = SIMD_MM256_SET2_EPI16(Base::GREEN_TO_GRAY_WEIGHT, Base::BGR_TO_GRAY_ROUND_TERM);
      //{{{
      SIMD_INLINE __m256i BgraToGray32v1(__m256i bgra)
      {
          const __m256i g0a0 = _mm256_and_si256(_mm256_srli_si256(bgra, 1), K16_00FF);
          const __m256i b0r0 = _mm256_and_si256(bgra, K16_00FF);
          const __m256i weightedSum = _mm256_add_epi32(_mm256_madd_epi16(g0a0, K16_GREEN_ROUND), _mm256_madd_epi16(b0r0, K16_BLUE_RED));
          return _mm256_srli_epi32(weightedSum, Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }
      //}}}
      //{{{
      SIMD_INLINE __m256i BgraToGrayv1(__m256i bgra[4])
      {
          const __m256i lo = PackI32ToI16(BgraToGray32v1(bgra[0]), BgraToGray32v1(bgra[1]));
          const __m256i hi = PackI32ToI16(BgraToGray32v1(bgra[2]), BgraToGray32v1(bgra[3]));
          return PackI16ToU8(lo, hi);
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE __m256i BgrToGray(const uint8_t * bgr)
      {
          __m256i bgra[4];
          bgra[0] = BgrToBgra<false>(Load<align>((__m256i*)(bgr + 0)), K32_01000000);
          bgra[1] = BgrToBgra<false>(Load<false>((__m256i*)(bgr + 24)), K32_01000000);
          bgra[2] = BgrToBgra<false>(Load<false>((__m256i*)(bgr + 48)), K32_01000000);
          bgra[3] = BgrToBgra<true>(Load<align>((__m256i*)(bgr + 64)), K32_01000000);
          return BgraToGrayv1(bgra);
      }
      //}}}
      //{{{
      template <bool align> void BgrToGray(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * gray, size_t grayStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(gray) && Aligned(grayStride) && Aligned(bgr) && Aligned(bgrStride));

          size_t alignedWidth = AlignLo(width, A);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  Store<align>((__m256i*)(gray + col), BgrToGray<align>(bgr + 3 * col));
              if (width != alignedWidth)
                  Store<false>((__m256i*)(gray + width - A), BgrToGray<false>(bgr + 3 * (width - A)));
              bgr += bgrStride;
              gray += grayStride;
          }
      }
      //}}}
      //{{{
      void BgrToGray(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * gray, size_t grayStride)
      {
          if (Aligned(gray) && Aligned(grayStride) && Aligned(bgr) && Aligned(bgrStride))
              BgrToGray<true>(bgr, width, height, bgrStride, gray, grayStride);
          else
              BgrToGray<false>(bgr, width, height, bgrStride, gray, grayStride);
      }

      //}}}
      //}}}
      //{{{  rgb to gray
      const __m256i K16_RED_BLUE = SIMD_MM256_SET2_EPI16(Base::RED_TO_GRAY_WEIGHT, Base::BLUE_TO_GRAY_WEIGHT);
      //{{{
      SIMD_INLINE __m256i RgbaToGray32(__m256i rgba)
      {
          const __m256i g0a0 = _mm256_and_si256(_mm256_srli_si256(rgba, 1), K16_00FF);
          const __m256i r0b0 = _mm256_and_si256(rgba, K16_00FF);
          const __m256i weightedSum = _mm256_add_epi32(_mm256_madd_epi16(g0a0, K16_GREEN_ROUND), _mm256_madd_epi16(r0b0, K16_RED_BLUE));
          return _mm256_srli_epi32(weightedSum, Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }
      //}}}
      //{{{
      SIMD_INLINE __m256i RgbaToGray(__m256i rgba[4])
      {
          const __m256i lo = PackI32ToI16(RgbaToGray32(rgba[0]), RgbaToGray32(rgba[1]));
          const __m256i hi = PackI32ToI16(RgbaToGray32(rgba[2]), RgbaToGray32(rgba[3]));
          return PackI16ToU8(lo, hi);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m256i RgbToGray(const uint8_t* rgb)
      {
          __m256i rgba[4];
          rgba[0] = BgrToBgra<false>(Load<align>((__m256i*)(rgb + 0)), K32_01000000);
          rgba[1] = BgrToBgra<false>(Load<false>((__m256i*)(rgb + 24)), K32_01000000);
          rgba[2] = BgrToBgra<false>(Load<false>((__m256i*)(rgb + 48)), K32_01000000);
          rgba[3] = BgrToBgra<true>(Load<align>((__m256i*)(rgb + 64)), K32_01000000);
          return RgbaToGray(rgba);
      }
      //}}}
      //{{{
      template <bool align> void RgbToGray(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* gray, size_t grayStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(gray) && Aligned(grayStride) && Aligned(rgb) && Aligned(rgbStride));

          size_t alignedWidth = AlignLo(width, A);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  Store<align>((__m256i*)(gray + col), RgbToGray<align>(rgb + 3 * col));
              if (width != alignedWidth)
                  Store<false>((__m256i*)(gray + width - A), RgbToGray<false>(rgb + 3 * (width - A)));
              rgb += rgbStride;
              gray += grayStride;
          }
      }

      //}}}
      //{{{
      void RgbToGray(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* gray, size_t grayStride)
      {
          if (Aligned(gray) && Aligned(grayStride) && Aligned(rgb) && Aligned(rgbStride))
              RgbToGray<true>(rgb, width, height, rgbStride, gray, grayStride);
          else
              RgbToGray<false>(rgb, width, height, rgbStride, gray, grayStride);
      }
      //}}}
      //}}}
      //{{{  bgra to gray
      const __m256i K16_GREEN_0000 = SIMD_MM256_SET2_EPI16(Base::GREEN_TO_GRAY_WEIGHT, 0x0000);
      const __m256i K32_ROUND_TERM = SIMD_MM256_SET1_EPI32(Base::BGR_TO_GRAY_ROUND_TERM);
      //{{{
      SIMD_INLINE __m256i BgraToGray32v2(__m256i bgra)
      {
          const __m256i g0a0 = _mm256_and_si256(_mm256_srli_si256(bgra, 1), K16_00FF);
          const __m256i b0r0 = _mm256_and_si256(bgra, K16_00FF);
          const __m256i weightedSum = _mm256_add_epi32(_mm256_madd_epi16(g0a0, K16_GREEN_0000), _mm256_madd_epi16(b0r0, K16_BLUE_RED));
          return _mm256_srli_epi32(_mm256_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }
      //}}}
      //{{{
      SIMD_INLINE __m256i BgraToGrayv2(__m256i bgra[4])
      {
          const __m256i lo = PackI32ToI16(BgraToGray32v2(bgra[0]), BgraToGray32v2(bgra[1]));
          const __m256i hi = PackI32ToI16(BgraToGray32v2(bgra[2]), BgraToGray32v2(bgra[3]));
          return PackI16ToU8(lo, hi);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void Load(const uint8_t* p, __m256i a[4])
      {
          a[0] = Load<align>((__m256i*)p + 0);
          a[1] = Load<align>((__m256i*)p + 1);
          a[2] = Load<align>((__m256i*)p + 2);
          a[3] = Load<align>((__m256i*)p + 3);
      }
      //}}}
      //{{{
      template <bool align> void BgraToGray(const uint8_t *bgra, size_t width, size_t height, size_t bgraStride, uint8_t *gray, size_t grayStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(gray) && Aligned(grayStride));

          size_t alignedWidth = AlignLo(width, A);
          __m256i a[4];
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
              {
                  Load<align>(bgra + 4 * col, a);
                  Store<align>((__m256i*)(gray + col), BgraToGrayv2(a));
              }
              if (alignedWidth != width)
              {
                  Load<false>(bgra + 4 * (width - A), a);
                  Store<false>((__m256i*)(gray + width - A), BgraToGrayv2(a));
              }
              bgra += bgraStride;
              gray += grayStride;
          }
      }
      //}}}
      //{{{
      void BgraToGray(const uint8_t *bgra, size_t width, size_t height, size_t bgraStride, uint8_t *gray, size_t grayStride)
      {
          if (Aligned(bgra) && Aligned(gray) && Aligned(bgraStride) && Aligned(grayStride))
              BgraToGray<true>(bgra, width, height, bgraStride, gray, grayStride);
          else
              BgraToGray<false>(bgra, width, height, bgraStride, gray, grayStride);
      }
      //}}}
      //}}}
      //{{{  rgba to gray
      //{{{
      SIMD_INLINE __m256i RgbaToGray32v2(__m256i rgba)
      {
          const __m256i g0a0 = _mm256_and_si256(_mm256_srli_si256(rgba, 1), K16_00FF);
          const __m256i r0b0 = _mm256_and_si256(rgba, K16_00FF);
          const __m256i weightedSum = _mm256_add_epi32(_mm256_madd_epi16(g0a0, K16_GREEN_0000), _mm256_madd_epi16(r0b0, K16_RED_BLUE));
          return _mm256_srli_epi32(_mm256_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }
      //}}}
      //{{{
      SIMD_INLINE __m256i RgbaToGrayv2(__m256i rgba[4])
      {
          const __m256i lo = PackI32ToI16(RgbaToGray32v2(rgba[0]), RgbaToGray32v2(rgba[1]));
          const __m256i hi = PackI32ToI16(RgbaToGray32v2(rgba[2]), RgbaToGray32v2(rgba[3]));
          return PackI16ToU8(lo, hi);
      }
      //}}}
      //{{{
      template <bool align> void RgbaToGray(const uint8_t* rgba, size_t width, size_t height, size_t rgbaStride, uint8_t* gray, size_t grayStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(rgba) && Aligned(rgbaStride) && Aligned(gray) && Aligned(grayStride));

          size_t alignedWidth = AlignLo(width, A);
          __m256i a[4];
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
              {
                  Load<align>(rgba + 4 * col, a);
                  Store<align>((__m256i*)(gray + col), RgbaToGrayv2(a));
              }
              if (alignedWidth != width)
              {
                  Load<false>(rgba + 4 * (width - A), a);
                  Store<false>((__m256i*)(gray + width - A), RgbaToGrayv2(a));
              }
              rgba += rgbaStride;
              gray += grayStride;
          }
      }
      //}}}
      //{{{
      void RgbaToGray(const uint8_t* rgba, size_t width, size_t height, size_t rgbaStride, uint8_t* gray, size_t grayStride)
      {
          if (Aligned(rgba) && Aligned(gray) && Aligned(rgbaStride) && Aligned(grayStride))
              RgbaToGray<true>(rgba, width, height, rgbaStride, gray, grayStride);
          else
              RgbaToGray<false>(rgba, width, height, rgbaStride, gray, grayStride);
      }
      //}}}
      //}}}

      //{{{  bgr to bgra
      //{{{
      template <bool align> SIMD_INLINE void BgrToBgra(const uint8_t * bgr, uint8_t * bgra, __m256i alpha)
      {
          Store<align>((__m256i*)bgra + 0, BgrToBgra<false>(Load<align>((__m256i*)(bgr + 0)), alpha));
          Store<align>((__m256i*)bgra + 1, BgrToBgra<false>(Load<false>((__m256i*)(bgr + 24)), alpha));
          Store<align>((__m256i*)bgra + 2, BgrToBgra<false>(Load<false>((__m256i*)(bgr + 48)), alpha));
          Store<align>((__m256i*)bgra + 3, BgrToBgra<true >(Load<align>((__m256i*)(bgr + 64)), alpha));
      }
      //}}}
      //{{{
      template <bool align> void BgrToBgra(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride));

          size_t alignedWidth = AlignLo(width, A);

          __m256i _alpha = _mm256_slli_si256(_mm256_set1_epi32(alpha), 3);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  BgrToBgra<align>(bgr + 3 * col, bgra + 4 * col, _alpha);
              if (width != alignedWidth)
                  BgrToBgra<false>(bgr + 3 * (width - A), bgra + 4 * (width - A), _alpha);
              bgr += bgrStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void BgrToBgra(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride))
              BgrToBgra<true>(bgr, width, height, bgrStride, bgra, bgraStride, alpha);
          else
              BgrToBgra<false>(bgr, width, height, bgrStride, bgra, bgraStride, alpha);
      }
      //}}}
      //}}}
      //{{{  bgr to rgb
      // to RGB
      //{{{
      const __m256i K8_SHFL_0S0 = SIMD_MM256_SETR_EPI8(0x2, 0x1, 0x0, 0x5, 0x4, 0x3, 0x8, 0x7, 0x6, 0xB, 0xA, 0x9, 0xE, 0xD, 0xC, -1,
          0x0, -1, 0x4, 0x3, 0x2, 0x7, 0x6, 0x5, 0xA, 0x9, 0x8, 0xD, 0xC, 0xB, -1, 0xF);
      //}}}
      //{{{
      const __m256i K8_SHFL_0P0 = SIMD_MM256_SETR_EPI8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x9,
          -1, 0x7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
      //}}}
      //{{{
      const __m256i K8_SHFL_0P1 = SIMD_MM256_SETR_EPI8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
          -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x8, -1);
      //}}}
      //{{{
      const __m256i K8_SHFL_1S1 = SIMD_MM256_SETR_EPI8(-1, 0x3, 0x2, 0x1, 0x6, 0x5, 0x4, 0x9, 0x8, 0x7, 0xC, 0xB, 0xA, 0xF, 0xE, 0xD,
          0x2, 0x1, 0x0, 0x5, 0x4, 0x3, 0x8, 0x7, 0x6, 0xB, 0xA, 0x9, 0xE, 0xD, 0xC, -1);
      //}}}
      //{{{
      const __m256i K8_SHFL_1P0 = SIMD_MM256_SETR_EPI8(0x6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
          -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
      //}}}
      //{{{
      const __m256i K8_SHFL_1P2 = SIMD_MM256_SETR_EPI8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
          -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x9);
      //}}}
      //{{{
      const __m256i K8_SHFL_2S2 = SIMD_MM256_SETR_EPI8(0x0, -1, 0x4, 0x3, 0x2, 0x7, 0x6, 0x5, 0xA, 0x9, 0x8, 0xD, 0xC, 0xB, -1, 0xF,
          -1, 0x3, 0x2, 0x1, 0x6, 0x5, 0x4, 0x9, 0x8, 0x7, 0xC, 0xB, 0xA, 0xF, 0xE, 0xD);
      //}}}
      //{{{
      const __m256i K8_SHFL_2P1 = SIMD_MM256_SETR_EPI8(-1, 0x7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
          -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
      //}}}
      //{{{
      const __m256i K8_SHFL_2P2 = SIMD_MM256_SETR_EPI8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x8, -1,
          0x6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgrToRgb(const uint8_t * src, uint8_t * dst)
      {
          __m256i s0 = Load<align>((__m256i*)src + 0);
          __m256i s1 = Load<align>((__m256i*)src + 1);
          __m256i s2 = Load<align>((__m256i*)src + 2);
          __m256i p0 = _mm256_permute4x64_epi64(s0, 0x1B);
          __m256i p1 = _mm256_permute4x64_epi64(s1, 0x1B);
          __m256i p2 = _mm256_permute4x64_epi64(s2, 0x1B);
          Store<align>((__m256i*)dst + 0, _mm256_or_si256(_mm256_or_si256(_mm256_shuffle_epi8(s0, K8_SHFL_0S0),
              _mm256_shuffle_epi8(p0, K8_SHFL_0P0)), _mm256_shuffle_epi8(p1, K8_SHFL_0P1)));
          Store<align>((__m256i*)dst + 1, _mm256_or_si256(_mm256_or_si256(_mm256_shuffle_epi8(s1, K8_SHFL_1S1),
              _mm256_shuffle_epi8(p0, K8_SHFL_1P0)), _mm256_shuffle_epi8(p2, K8_SHFL_1P2)));
          Store<align>((__m256i*)dst + 2, _mm256_or_si256(_mm256_or_si256(_mm256_shuffle_epi8(s2, K8_SHFL_2S2),
              _mm256_shuffle_epi8(p1, K8_SHFL_2P1)), _mm256_shuffle_epi8(p2, K8_SHFL_2P2)));
      }
      //}}}
      //{{{
      template <bool align> void BgrToRgb(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * rgb, size_t rgbStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgr) && Aligned(bgrStride) && Aligned(rgb) && Aligned(rgbStride));

          const size_t A3 = A * 3;
          size_t size = width * 3;
          size_t aligned = AlignLo(width, A) * 3;

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t i = 0; i < aligned; i += A3)
                  BgrToRgb<align>(bgr + i, rgb + i);
              if (aligned < size)
                  BgrToRgb<false>(bgr + size - A3, rgb + size - A3);
              bgr += bgrStride;
              rgb += rgbStride;
          }
      }
      //}}}
      //{{{
      void BgrToRgb(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * rgb, size_t rgbStride)
      {
          if (Aligned(bgr) && Aligned(bgrStride) && Aligned(rgb) && Aligned(rgbStride))
              BgrToRgb<true>(bgr, width, height, bgrStride, rgb, rgbStride);
          else
              BgrToRgb<false>(bgr, width, height, bgrStride, rgb, rgbStride);
      }
      //}}}
      //}}}
      //{{{  bgr to yuv420
      //{{{
      template <bool align> SIMD_INLINE void LoadBgr(const __m256i * p, __m256i & blue, __m256i & green, __m256i & red)
      {
          __m256i bgr[3];
          bgr[0] = Load<align>(p + 0);
          bgr[1] = Load<align>(p + 1);
          bgr[2] = Load<align>(p + 2);
          blue = BgrToBlue(bgr);
          green = BgrToGreen(bgr);
          red = BgrToRed(bgr);
      }
      //}}}

      #if defined(_MSC_VER) // Workaround for Visual Studio 2012 compiler bug in release mode:
      //{{{
      SIMD_INLINE __m256i Average16(const __m256i & s0, const __m256i & s1)
      {
          return _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(
              _mm256_hadd_epi16(_mm256_unpacklo_epi8(s0, K_ZERO), _mm256_unpackhi_epi8(s0, K_ZERO)),
              _mm256_hadd_epi16(_mm256_unpacklo_epi8(s1, K_ZERO), _mm256_unpackhi_epi8(s1, K_ZERO))), K16_0002), 2);
      }
      //}}}
      #else
      //{{{
      SIMD_INLINE __m256i Average16(const __m256i & s0, const __m256i & s1)
      {
          return _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(_mm256_maddubs_epi16(s0, K8_01), _mm256_maddubs_epi16(s1, K8_01)), K16_0002), 2);
      }
      //}}}
      #endif

      //{{{
      template <bool align> SIMD_INLINE void BgrToYuv420p(const uint8_t * bgr0, size_t bgrStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v)
      {
          const uint8_t * bgr1 = bgr0 + bgrStride;
          uint8_t * y1 = y0 + yStride;

          __m256i blue[2][2], green[2][2], red[2][2];

          LoadBgr<align>((__m256i*)bgr0 + 0, blue[0][0], green[0][0], red[0][0]);
          Store<align>((__m256i*)y0 + 0, BgrToY8(blue[0][0], green[0][0], red[0][0]));

          LoadBgr<align>((__m256i*)bgr0 + 3, blue[0][1], green[0][1], red[0][1]);
          Store<align>((__m256i*)y0 + 1, BgrToY8(blue[0][1], green[0][1], red[0][1]));

          LoadBgr<align>((__m256i*)bgr1 + 0, blue[1][0], green[1][0], red[1][0]);
          Store<align>((__m256i*)y1 + 0, BgrToY8(blue[1][0], green[1][0], red[1][0]));

          LoadBgr<align>((__m256i*)bgr1 + 3, blue[1][1], green[1][1], red[1][1]);
          Store<align>((__m256i*)y1 + 1, BgrToY8(blue[1][1], green[1][1], red[1][1]));

          blue[0][0] = Average16(blue[0][0], blue[1][0]);
          blue[0][1] = Average16(blue[0][1], blue[1][1]);
          green[0][0] = Average16(green[0][0], green[1][0]);
          green[0][1] = Average16(green[0][1], green[1][1]);
          red[0][0] = Average16(red[0][0], red[1][0]);
          red[0][1] = Average16(red[0][1], red[1][1]);

          Store<align>((__m256i*)u, PackI16ToU8(BgrToU16(blue[0][0], green[0][0], red[0][0]), BgrToU16(blue[0][1], green[0][1], red[0][1])));
          Store<align>((__m256i*)v, PackI16ToU8(BgrToV16(blue[0][0], green[0][0], red[0][0]), BgrToV16(blue[0][1], green[0][1], red[0][1])));
      }
      //}}}
      //{{{
      template <bool align> void BgrToYuv420p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
          }

          size_t alignedWidth = AlignLo(width, DA);
          const size_t A6 = A * 6;
          for (size_t row = 0; row < height; row += 2)
          {
              for (size_t colUV = 0, colY = 0, colBgr = 0; colY < alignedWidth; colY += DA, colUV += A, colBgr += A6)
                  BgrToYuv420p<align>(bgr + colBgr, bgrStride, y + colY, yStride, u + colUV, v + colUV);
              if (width != alignedWidth)
              {
                  size_t offset = width - DA;
                  BgrToYuv420p<false>(bgr + offset * 3, bgrStride, y + offset, yStride, u + offset / 2, v + offset / 2);
              }
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              bgr += 2 * bgrStride;
          }
      }
      //}}}
      //{{{
      void BgrToYuv420p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride))
              BgrToYuv420p<true>(bgr, width, height, bgrStride, y, yStride, u, uStride, v, vStride);
          else
              BgrToYuv420p<false>(bgr, width, height, bgrStride, y, yStride, u, uStride, v, vStride);
      }
      //}}}
      //}}}
      //{{{  bgr to yuv422p
      //{{{
      SIMD_INLINE void Average16v2(__m256i & a)
      {
      #ifdef SIMD_MADDUBS_ERROR
          a = _mm256_srli_epi16(_mm256_add_epi16(_mm256_hadd_epi16(_mm256_unpacklo_epi8(a, K_ZERO), _mm256_unpackhi_epi8(a, K_ZERO)), K16_0001), 1);
      #else
          a = _mm256_srli_epi16(_mm256_add_epi16(_mm256_maddubs_epi16(a, K8_01), K16_0001), 1);
      #endif
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE void BgrToYuv422p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v)
      {
          __m256i blue[2], green[2], red[2];

          LoadBgr<align>((__m256i*)bgr + 0, blue[0], green[0], red[0]);
          Store<align>((__m256i*)y + 0, BgrToY8(blue[0], green[0], red[0]));

          LoadBgr<align>((__m256i*)bgr + 3, blue[1], green[1], red[1]);
          Store<align>((__m256i*)y + 1, BgrToY8(blue[1], green[1], red[1]));

          Average16v2(blue[0]);
          Average16v2(blue[1]);
          Average16v2(green[0]);
          Average16v2(green[1]);
          Average16v2(red[0]);
          Average16v2(red[1]);

          Store<align>((__m256i*)u, PackI16ToU8(BgrToU16(blue[0], green[0], red[0]), BgrToU16(blue[1], green[1], red[1])));
          Store<align>((__m256i*)v, PackI16ToU8(BgrToV16(blue[0], green[0], red[0]), BgrToV16(blue[1], green[1], red[1])));
      }
      //}}}
      //{{{
      template <bool align> void BgrToYuv422p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          assert((width % 2 == 0) && (width >= DA));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
          }

          size_t alignedWidth = AlignLo(width, DA);
          const size_t A6 = A * 6;
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t colUV = 0, colY = 0, colBgr = 0; colY < alignedWidth; colY += DA, colUV += A, colBgr += A6)
                  BgrToYuv422p<align>(bgr + colBgr, y + colY, u + colUV, v + colUV);
              if (width != alignedWidth)
              {
                  size_t offset = width - DA;
                  BgrToYuv422p<false>(bgr + offset * 3, y + offset, u + offset / 2, v + offset / 2);
              }
              y += yStride;
              u += uStride;
              v += vStride;
              bgr += bgrStride;
          }
      }
      //}}}
      //{{{
      void BgrToYuv422p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride))
              BgrToYuv422p<true>(bgr, width, height, bgrStride, y, yStride, u, uStride, v, vStride);
          else
              BgrToYuv422p<false>(bgr, width, height, bgrStride, y, yStride, u, uStride, v, vStride);
      }
      //}}}
      //}}}
      //{{{  bgr to yuv444p
      //{{{
      template <bool align> SIMD_INLINE void BgrToYuv444p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v)
      {
          __m256i blue, green, red;
          LoadBgr<align>((__m256i*)bgr, blue, green, red);
          Store<align>((__m256i*)y, BgrToY8(blue, green, red));
          Store<align>((__m256i*)u, BgrToU8(blue, green, red));
          Store<align>((__m256i*)v, BgrToV8(blue, green, red));
      }
      //}}}
      //{{{
      template <bool align> void BgrToYuv444p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          assert(width >= A);
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
          }

          size_t alignedWidth = AlignLo(width, A);
          const size_t A3 = A * 3;
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0, colBgr = 0; col < alignedWidth; col += A, colBgr += A3)
                  BgrToYuv444p<align>(bgr + colBgr, y + col, u + col, v + col);
              if (width != alignedWidth)
              {
                  size_t col = width - A;
                  BgrToYuv444p<false>(bgr + col * 3, y + col, u + col, v + col);
              }
              y += yStride;
              u += uStride;
              v += vStride;
              bgr += bgrStride;
          }
      }
      //}}}
      //{{{
      void BgrToYuv444p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride))
              BgrToYuv444p<true>(bgr, width, height, bgrStride, y, yStride, u, uStride, v, vStride);
          else
              BgrToYuv444p<false>(bgr, width, height, bgrStride, y, yStride, u, uStride, v, vStride);
      }
      //}}}
      //}}}

      //{{{  bgra to bgr
      //{{{
      template <bool align> SIMD_INLINE __m256i BgraToBgr(const uint8_t* bgra)
      {
          __m256i _bgra = Load<align>((__m256i*)bgra);
          return _mm256_permutevar8x32_epi32(_mm256_shuffle_epi8(_bgra, K8_SHUFFLE_BGRA_TO_BGR), K32_PERMUTE_BGRA_TO_BGR);
      }
      //}}}
      //{{{
      template <bool align> void BgraToBgr(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * bgr, size_t bgrStride)
      {
          assert(width >= F);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride));

          size_t widthF = AlignLo(width, F) - F;

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < widthF; col += F)
                  Store<false>((__m256i*)(bgr + 3 * col), BgraToBgr<align>(bgra + 4 * col));
              Store24<false>(bgr + 3 * widthF, BgraToBgr<align>(bgra + 4 * widthF));
              if (widthF + F != width)
                  Store24<false>(bgr + 3 * (width - F), BgraToBgr<false>(bgra + 4 * (width - F)));
              bgra += bgraStride;
              bgr += bgrStride;
          }
      }
      //}}}
      //{{{
      void BgraToBgr(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * bgr, size_t bgrStride)
      {
          if (Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride))
              BgraToBgr<true>(bgra, width, height, bgraStride, bgr, bgrStride);
          else
              BgraToBgr<false>(bgra, width, height, bgraStride, bgr, bgrStride);
      }
      //}}}
      //}}}
      //{{{  bgra to rgb

      //{{{
      const __m256i K8_SHUFFLE_BGRA_TO_RGB = SIMD_MM256_SETR_EPI8(
          0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1,
          0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1);
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m256i BgraToRgb(const uint8_t* bgra)
      {
          __m256i _bgra = Load<align>((__m256i*)bgra);
          return _mm256_permutevar8x32_epi32(_mm256_shuffle_epi8(_bgra, K8_SHUFFLE_BGRA_TO_RGB), K32_PERMUTE_BGRA_TO_BGR);
      }
      //}}}
      //{{{
      template <bool align> void BgraToRgb(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgb, size_t rgbStride)
      {
          assert(width >= F);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride));

          size_t widthF = AlignLo(width, F) - F;

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < widthF; col += F)
                  Store<false>((__m256i*)(rgb + 3 * col), BgraToRgb<align>(bgra + 4 * col));
              Store24<false>(rgb + 3 * widthF, BgraToRgb<align>(bgra + 4 * widthF));
              if (widthF + F != width)
                  Store24<false>(rgb + 3 * (width - F), BgraToRgb<false>(bgra + 4 * (width - F)));
              bgra += bgraStride;
              rgb += rgbStride;
          }
      }
      //}}}
      //{{{
      void BgraToRgb(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgb, size_t rgbStride)
      {
          if (Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride))
              BgraToRgb<true>(bgra, width, height, bgraStride, rgb, rgbStride);
          else
              BgraToRgb<false>(bgra, width, height, bgraStride, rgb, rgbStride);
      }
      //}}}
      //}}}
      //{{{  bgra to rgba
      //{{{
      const __m256i K8_BGRA_TO_RGBA = SIMD_MM256_SETR_EPI8(
          0x2, 0x1, 0x0, 0x3, 0x6, 0x5, 0x4, 0x7, 0xA, 0x9, 0x8, 0xB, 0xE, 0xD, 0xC, 0xF,
          0x2, 0x1, 0x0, 0x3, 0x6, 0x5, 0x4, 0x7, 0xA, 0x9, 0x8, 0xB, 0xE, 0xD, 0xC, 0xF);
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToRgba(const uint8_t* bgra, uint8_t* rgba)
      {
          Store<align>((__m256i*)rgba, _mm256_shuffle_epi8(Load<align>((__m256i*)bgra), K8_BGRA_TO_RGBA));
      }
      //}}}
      //{{{
      template <bool align> void BgraToRgba(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgba, size_t rgbaStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(rgba) && Aligned(rgbaStride));

          size_t size = width * 4;
          size_t sizeA = AlignLo(size, A);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t i = 0; i < sizeA; i += A)
                  BgraToRgba<align>(bgra + i, rgba + i);
              if (size != sizeA)
                  BgraToRgba<false>(bgra + size - A, rgba + size - A);
              bgra += bgraStride;
              rgba += rgbaStride;
          }
      }
      //}}}

      //{{{
      void BgraToRgba(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgba, size_t rgbaStride)
      {
          if (Aligned(bgra) && Aligned(bgraStride) && Aligned(rgba) && Aligned(rgbaStride))
              BgraToRgba<true>(bgra, width, height, bgraStride, rgba, rgbaStride);
          else
              BgraToRgba<false>(bgra, width, height, bgraStride, rgba, rgbaStride);
      }
      //}}}

      //{{{
      //{{{
      template <bool align> SIMD_INLINE void LoadPreparedBgra16(const __m256i * bgra, __m256i & b16_r16, __m256i & g16_1)
      {
          __m256i _bgra = Load<align>(bgra);
          b16_r16 = _mm256_and_si256(_bgra, K16_00FF);
          g16_1 = _mm256_or_si256(_mm256_and_si256(_mm256_srli_si256(_bgra, 1), K32_000000FF), K32_00010000);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m256i LoadAndConvertY16(const __m256i * bgra, __m256i & b16_r16, __m256i & g16_1)
      {
          __m256i _b16_r16[2], _g16_1[2];
          LoadPreparedBgra16<align>(bgra + 0, _b16_r16[0], _g16_1[0]);
          LoadPreparedBgra16<align>(bgra + 1, _b16_r16[1], _g16_1[1]);
          b16_r16 = _mm256_permute4x64_epi64(_mm256_hadd_epi32(_b16_r16[0], _b16_r16[1]), 0xD8);
          g16_1 = _mm256_permute4x64_epi64(_mm256_hadd_epi32(_g16_1[0], _g16_1[1]), 0xD8);
          return SaturateI16ToU8(_mm256_add_epi16(K16_Y_ADJUST, PackI32ToI16(BgrToY32(_b16_r16[0], _g16_1[0]), BgrToY32(_b16_r16[1], _g16_1[1]))));
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m256i LoadAndConvertY8(const __m256i * bgra, __m256i b16_r16[2], __m256i g16_1[2])
      {
          return PackI16ToU8(LoadAndConvertY16<align>(bgra + 0, b16_r16[0], g16_1[0]), LoadAndConvertY16<align>(bgra + 2, b16_r16[1], g16_1[1]));
      }
      //}}}
      //{{{
      SIMD_INLINE void Average16v3(__m256i & a, const __m256i & b)
      {
          a = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(a, b), K16_0002), 2);
      }
      //}}}
      //{{{
      SIMD_INLINE __m256i ConvertU16(__m256i b16_r16[2], __m256i g16_1[2])
      {
          return SaturateI16ToU8(_mm256_add_epi16(K16_UV_ADJUST, PackI32ToI16(BgrToU32(b16_r16[0], g16_1[0]), BgrToU32(b16_r16[1], g16_1[1]))));
      }
      //}}}
      //{{{
      SIMD_INLINE __m256i ConvertV16(__m256i b16_r16[2], __m256i g16_1[2])
      {
          return SaturateI16ToU8(_mm256_add_epi16(K16_UV_ADJUST, PackI32ToI16(BgrToV32(b16_r16[0], g16_1[0]), BgrToV32(b16_r16[1], g16_1[1]))));
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE void BgraToYuv420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v)
      {
          const uint8_t * bgra1 = bgra0 + bgraStride;
          uint8_t * y1 = y0 + yStride;

          __m256i _b16_r16[2][2][2], _g16_1[2][2][2];
          Store<align>((__m256i*)y0 + 0, LoadAndConvertY8<align>((__m256i*)bgra0 + 0, _b16_r16[0][0], _g16_1[0][0]));
          Store<align>((__m256i*)y0 + 1, LoadAndConvertY8<align>((__m256i*)bgra0 + 4, _b16_r16[0][1], _g16_1[0][1]));
          Store<align>((__m256i*)y1 + 0, LoadAndConvertY8<align>((__m256i*)bgra1 + 0, _b16_r16[1][0], _g16_1[1][0]));
          Store<align>((__m256i*)y1 + 1, LoadAndConvertY8<align>((__m256i*)bgra1 + 4, _b16_r16[1][1], _g16_1[1][1]));

          Average16v3(_b16_r16[0][0][0], _b16_r16[1][0][0]);
          Average16v3(_b16_r16[0][0][1], _b16_r16[1][0][1]);
          Average16v3(_b16_r16[0][1][0], _b16_r16[1][1][0]);
          Average16v3(_b16_r16[0][1][1], _b16_r16[1][1][1]);

          Average16v3(_g16_1[0][0][0], _g16_1[1][0][0]);
          Average16v3(_g16_1[0][0][1], _g16_1[1][0][1]);
          Average16v3(_g16_1[0][1][0], _g16_1[1][1][0]);
          Average16v3(_g16_1[0][1][1], _g16_1[1][1][1]);

          Store<align>((__m256i*)u, PackI16ToU8(ConvertU16(_b16_r16[0][0], _g16_1[0][0]), ConvertU16(_b16_r16[0][1], _g16_1[0][1])));
          Store<align>((__m256i*)v, PackI16ToU8(ConvertV16(_b16_r16[0][0], _g16_1[0][0]), ConvertV16(_b16_r16[0][1], _g16_1[0][1])));
      }
      //}}}
      //{{{
      template <bool align> void BgraToYuv420p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          size_t alignedWidth = AlignLo(width, DA);
          const size_t A8 = A * 8;
          for (size_t row = 0; row < height; row += 2)
          {
              for (size_t colUV = 0, colY = 0, colBgra = 0; colY < alignedWidth; colY += DA, colUV += A, colBgra += A8)
                  BgraToYuv420p<align>(bgra + colBgra, bgraStride, y + colY, yStride, u + colUV, v + colUV);
              if (width != alignedWidth)
              {
                  size_t offset = width - DA;
                  BgraToYuv420p<false>(bgra + offset * 4, bgraStride, y + offset, yStride, u + offset / 2, v + offset / 2);
              }
              y += 2 * yStride;
              u += uStride;
              v += vStride;
              bgra += 2 * bgraStride;
          }
      }
      //}}}
      //{{{
      void BgraToYuv420p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              BgraToYuv420p<true>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
          else
              BgraToYuv420p<false>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
      }
      //}}}

      //{{{
      SIMD_INLINE void Average16v4(__m256i a[2][2])
      {
          a[0][0] = _mm256_srli_epi16(_mm256_add_epi16(a[0][0], K16_0001), 1);
          a[0][1] = _mm256_srli_epi16(_mm256_add_epi16(a[0][1], K16_0001), 1);
          a[1][0] = _mm256_srli_epi16(_mm256_add_epi16(a[1][0], K16_0001), 1);
          a[1][1] = _mm256_srli_epi16(_mm256_add_epi16(a[1][1], K16_0001), 1);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToYuv422p(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v)
      {
          __m256i _b16_r16[2][2], _g16_1[2][2];
          Store<align>((__m256i*)y + 0, LoadAndConvertY8<align>((__m256i*)bgra + 0, _b16_r16[0], _g16_1[0]));
          Store<align>((__m256i*)y + 1, LoadAndConvertY8<align>((__m256i*)bgra + 4, _b16_r16[1], _g16_1[1]));

          Average16v4(_b16_r16);
          Average16v4(_g16_1);

          Store<align>((__m256i*)u, PackI16ToU8(ConvertU16(_b16_r16[0], _g16_1[0]), ConvertU16(_b16_r16[1], _g16_1[1])));
          Store<align>((__m256i*)v, PackI16ToU8(ConvertV16(_b16_r16[0], _g16_1[0]), ConvertV16(_b16_r16[1], _g16_1[1])));
      }
      //}}}
      //{{{
      template <bool align> void BgraToYuv422p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          assert((width % 2 == 0) && (width >= DA));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          size_t alignedWidth = AlignLo(width, DA);
          const size_t A8 = A * 8;
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t colUV = 0, colY = 0, colBgra = 0; colY < alignedWidth; colY += DA, colUV += A, colBgra += A8)
                  BgraToYuv422p<align>(bgra + colBgra, y + colY, u + colUV, v + colUV);
              if (width != alignedWidth)
              {
                  size_t offset = width - DA;
                  BgraToYuv422p<false>(bgra + offset * 4, y + offset, u + offset / 2, v + offset / 2);
              }
              y += yStride;
              u += uStride;
              v += vStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void BgraToYuv422p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              BgraToYuv422p<true>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
          else
              BgraToYuv422p<false>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
      }
      //}}}
      //{{{
      SIMD_INLINE __m256i ConvertY16(__m256i b16_r16[2], __m256i g16_1[2])
      {
          return SaturateI16ToU8(_mm256_add_epi16(K16_Y_ADJUST, PackI32ToI16(BgrToY32(b16_r16[0], g16_1[0]), BgrToY32(b16_r16[1], g16_1[1]))));
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE void BgraToYuv444p(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v)
      {
          __m256i _b16_r16[2][2], _g16_1[2][2];
          LoadPreparedBgra16<align>((__m256i*)bgra + 0, _b16_r16[0][0], _g16_1[0][0]);
          LoadPreparedBgra16<align>((__m256i*)bgra + 1, _b16_r16[0][1], _g16_1[0][1]);
          LoadPreparedBgra16<align>((__m256i*)bgra + 2, _b16_r16[1][0], _g16_1[1][0]);
          LoadPreparedBgra16<align>((__m256i*)bgra + 3, _b16_r16[1][1], _g16_1[1][1]);

          Store<align>((__m256i*)y, PackI16ToU8(ConvertY16(_b16_r16[0], _g16_1[0]), ConvertY16(_b16_r16[1], _g16_1[1])));
          Store<align>((__m256i*)u, PackI16ToU8(ConvertU16(_b16_r16[0], _g16_1[0]), ConvertU16(_b16_r16[1], _g16_1[1])));
          Store<align>((__m256i*)v, PackI16ToU8(ConvertV16(_b16_r16[0], _g16_1[0]), ConvertV16(_b16_r16[1], _g16_1[1])));
      }
      //}}}
      //{{{
      template <bool align> void BgraToYuv444p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          assert(width >= A);
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          size_t alignedWidth = AlignLo(width, A);
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0, colBgra = 0; col < alignedWidth; col += A, colBgra += QA)
                  BgraToYuv444p<align>(bgra + colBgra, y + col, u + col, v + col);
              if (width != alignedWidth)
              {
                  size_t offset = width - A;
                  BgraToYuv444p<false>(bgra + offset * 4, y + offset, u + offset, v + offset);
              }
              y += yStride;
              u += uStride;
              v += vStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void BgraToYuv444p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              BgraToYuv444p<true>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
          else
              BgraToYuv444p<false>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE void LoadPreparedBgra16(const __m256i * bgra, __m256i & b16_r16, __m256i & g16_1, __m256i & a32)
      {
          __m256i _bgra = Load<align>(bgra);
          b16_r16 = _mm256_and_si256(_bgra, K16_00FF);
          g16_1 = _mm256_or_si256(_mm256_and_si256(_mm256_srli_si256(_bgra, 1), K32_000000FF), K32_00010000);
          a32 = _mm256_and_si256(_mm256_srli_si256(_bgra, 3), K32_000000FF);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void LoadAndConvertYA16(const __m256i * bgra, __m256i & b16_r16, __m256i & g16_1, __m256i & y16, __m256i & a16)
      {
          __m256i _b16_r16[2], _g16_1[2], a32[2];
          LoadPreparedBgra16<align>(bgra + 0, _b16_r16[0], _g16_1[0], a32[0]);
          LoadPreparedBgra16<align>(bgra + 1, _b16_r16[1], _g16_1[1], a32[1]);
          b16_r16 = _mm256_permute4x64_epi64(_mm256_hadd_epi32(_b16_r16[0], _b16_r16[1]), 0xD8);
          g16_1 = _mm256_permute4x64_epi64(_mm256_hadd_epi32(_g16_1[0], _g16_1[1]), 0xD8);
          y16 = SaturateI16ToU8(_mm256_add_epi16(K16_Y_ADJUST, PackI32ToI16(BgrToY32(_b16_r16[0], _g16_1[0]), BgrToY32(_b16_r16[1], _g16_1[1]))));
          a16 = PackI32ToI16(a32[0], a32[1]);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void LoadAndStoreYA(const __m256i * bgra, __m256i b16_r16[2], __m256i g16_1[2], __m256i * y, __m256i * a)
      {
          __m256i y16[2], a16[2];
          LoadAndConvertYA16<align>(bgra + 0, b16_r16[0], g16_1[0], y16[0], a16[0]);
          LoadAndConvertYA16<align>(bgra + 2, b16_r16[1], g16_1[1], y16[1], a16[1]);
          Store<align>(y, PackI16ToU8(y16[0], y16[1]));
          Store<align>(a, PackI16ToU8(a16[0], a16[1]));
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToYuva420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v, uint8_t * a0, size_t aStride)
      {
          const uint8_t * bgra1 = bgra0 + bgraStride;
          uint8_t * y1 = y0 + yStride;
          uint8_t * a1 = a0 + aStride;

          __m256i _b16_r16[2][2][2], _g16_1[2][2][2];
          LoadAndStoreYA<align>((__m256i*)bgra0 + 0, _b16_r16[0][0], _g16_1[0][0], (__m256i*)y0 + 0, (__m256i*)a0 + 0);
          LoadAndStoreYA<align>((__m256i*)bgra0 + 4, _b16_r16[0][1], _g16_1[0][1], (__m256i*)y0 + 1, (__m256i*)a0 + 1);
          LoadAndStoreYA<align>((__m256i*)bgra1 + 0, _b16_r16[1][0], _g16_1[1][0], (__m256i*)y1 + 0, (__m256i*)a1 + 0);
          LoadAndStoreYA<align>((__m256i*)bgra1 + 4, _b16_r16[1][1], _g16_1[1][1], (__m256i*)y1 + 1, (__m256i*)a1 + 1);

          Average16(_b16_r16[0][0][0], _b16_r16[1][0][0]);
          Average16(_b16_r16[0][0][1], _b16_r16[1][0][1]);
          Average16(_b16_r16[0][1][0], _b16_r16[1][1][0]);
          Average16(_b16_r16[0][1][1], _b16_r16[1][1][1]);

          Average16(_g16_1[0][0][0], _g16_1[1][0][0]);
          Average16(_g16_1[0][0][1], _g16_1[1][0][1]);
          Average16(_g16_1[0][1][0], _g16_1[1][1][0]);
          Average16(_g16_1[0][1][1], _g16_1[1][1][1]);

          Store<align>((__m256i*)u, PackI16ToU8(ConvertU16(_b16_r16[0][0], _g16_1[0][0]), ConvertU16(_b16_r16[0][1], _g16_1[0][1])));
          Store<align>((__m256i*)v, PackI16ToU8(ConvertV16(_b16_r16[0][0], _g16_1[0][0]), ConvertV16(_b16_r16[0][1], _g16_1[0][1])));
      }
      //}}}
      //{{{
      template <bool align> void BgraToYuva420p(const uint8_t * bgra, size_t bgraStride, size_t width, size_t height, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride, uint8_t * a, size_t aStride)
      {
          assert((width % 2 == 0) && (height % 2 == 0) && (width >= DA) && (height >= 2));
          if (align)
          {
              assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
              assert(Aligned(v) && Aligned(vStride) && Aligned(a) && Aligned(aStride) && Aligned(bgra) && Aligned(bgraStride));
          }

          size_t alignedWidth = AlignLo(width, DA);
          const size_t A8 = A * 8;
          for (size_t row = 0; row < height; row += 2)
          {
              for (size_t colUV = 0, colYA = 0, colBgra = 0; colYA < alignedWidth; colYA += DA, colUV += A, colBgra += A8)
                  BgraToYuva420p<align>(bgra + colBgra, bgraStride, y + colYA, yStride, u + colUV, v + colUV, a + colYA, aStride);
              if (width != alignedWidth)
              {
                  size_t offset = width - DA;
                  BgraToYuva420p<false>(bgra + offset * 4, bgraStride, y + offset, yStride, u + offset / 2, v + offset / 2, a + offset, aStride);
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
      void BgraToYuva420p(const uint8_t * bgra, size_t bgraStride, size_t width, size_t height, uint8_t * y, size_t yStride,
          uint8_t * u, size_t uStride, uint8_t * v, size_t vStride, uint8_t * a, size_t aStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) && Aligned(v) && Aligned(vStride)
              && Aligned(a) && Aligned(aStride) && Aligned(bgra) && Aligned(bgraStride))
              BgraToYuva420p<true>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride, a, aStride);
          else
              BgraToYuva420p<false>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride, a, aStride);
      }
      //}}}
      //}}}
      //}}}

      //{{{  rgb to bgra
      //{{{
      template <bool align> SIMD_INLINE void RgbToBgra(const uint8_t* rgb, uint8_t* bgra, __m256i alpha)
      {
          Store<align>((__m256i*)bgra + 0, RgbToBgra<false>(Load<align>((__m256i*)(rgb + 0)), alpha));
          Store<align>((__m256i*)bgra + 1, RgbToBgra<false>(Load<false>((__m256i*)(rgb + 24)), alpha));
          Store<align>((__m256i*)bgra + 2, RgbToBgra<false>(Load<false>((__m256i*)(rgb + 48)), alpha));
          Store<align>((__m256i*)bgra + 3, RgbToBgra<true >(Load<align>((__m256i*)(rgb + 64)), alpha));
      }
      //}}}
      //{{{
      template <bool align> void RgbToBgra(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride));

          size_t alignedWidth = AlignLo(width, A);

          __m256i _alpha = _mm256_slli_si256(_mm256_set1_epi32(alpha), 3);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  RgbToBgra<align>(rgb + 3 * col, bgra + 4 * col, _alpha);
              if (width != alignedWidth)
                  RgbToBgra<false>(rgb + 3 * (width - A), bgra + 4 * (width - A), _alpha);
              rgb += rgbStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void RgbToBgra(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride))
              RgbToBgra<true>(rgb, width, height, rgbStride, bgra, bgraStride, alpha);
          else
              RgbToBgra<false>(rgb, width, height, rgbStride, bgra, bgraStride, alpha);
      }
      //}}}
      //}}}
      //{{{  bgr48p to bgra32
      //{{{
      template <bool align> SIMD_INLINE void Bgr48pToBgra32(uint8_t * bgra,
          const uint8_t * blue, const uint8_t * green, const uint8_t * red, size_t offset, __m256i alpha)
      {
          __m256i _blue = _mm256_and_si256(LoadPermuted<align>((__m256i*)(blue + offset)), K16_00FF);
          __m256i _green = _mm256_and_si256(LoadPermuted<align>((__m256i*)(green + offset)), K16_00FF);
          __m256i _red = _mm256_and_si256(LoadPermuted<align>((__m256i*)(red + offset)), K16_00FF);

          __m256i bg = _mm256_or_si256(_blue, _mm256_slli_si256(_green, 1));
          __m256i ra = _mm256_or_si256(_red, alpha);

          Store<align>((__m256i*)bgra + 0, _mm256_unpacklo_epi16(bg, ra));
          Store<align>((__m256i*)bgra + 1, _mm256_unpackhi_epi16(bg, ra));
      }
      //}}}
      //{{{
      template <bool align> void Bgr48pToBgra32(const uint8_t * blue, size_t blueStride, size_t width, size_t height,
          const uint8_t * green, size_t greenStride, const uint8_t * red, size_t redStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          assert(width >= HA);
          if (align)
          {
              assert(Aligned(blue) && Aligned(blueStride));
              assert(Aligned(green) && Aligned(greenStride));
              assert(Aligned(red) && Aligned(redStride));
              assert(Aligned(bgra) && Aligned(bgraStride));
          }

          __m256i _alpha = _mm256_slli_si256(_mm256_set1_epi16(alpha), 1);
          size_t alignedWidth = AlignLo(width, HA);
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0, srcOffset = 0, dstOffset = 0; col < alignedWidth; col += HA, srcOffset += A, dstOffset += DA)
                  Bgr48pToBgra32<align>(bgra + dstOffset, blue, green, red, srcOffset, _alpha);
              if (width != alignedWidth)
                  Bgr48pToBgra32<false>(bgra + (width - HA) * 4, blue, green, red, (width - HA) * 2, _alpha);
              blue += blueStride;
              green += greenStride;
              red += redStride;
              bgra += bgraStride;
          }
      }
      //}}}
      //{{{
      void Bgr48pToBgra32(const uint8_t * blue, size_t blueStride, size_t width, size_t height,
          const uint8_t * green, size_t greenStride, const uint8_t * red, size_t redStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(blue) && Aligned(blueStride) && Aligned(green) && Aligned(greenStride) &&
              Aligned(red) && Aligned(redStride) && Aligned(bgra) && Aligned(bgraStride))
              Bgr48pToBgra32<true>(blue, blueStride, width, height, green, greenStride, red, redStride, bgra, bgraStride, alpha);
          else
              Bgr48pToBgra32<false>(blue, blueStride, width, height, green, greenStride, red, redStride, bgra, bgraStride, alpha);
      }
      //}}}
      //}}}
      }
  #endif
  }
