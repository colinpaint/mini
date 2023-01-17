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

namespace Simd { 
  #ifdef SIMD_AVX512BW_ENABLE
    namespace Avx512bw {

      template <bool align, bool mask> SIMD_INLINE void GrayToBgra(const uint8_t * gray, const __m512i & alpha, uint8_t * bgra, const __mmask64 tails[5])
      {
          __m512i gray0 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<align, mask>(gray + 0, tails[0])));
          __m512i bg0 = _mm512_unpacklo_epi8(gray0, gray0);
          __m512i bg1 = _mm512_unpackhi_epi8(gray0, gray0);
          __m512i ra0 = _mm512_unpacklo_epi8(gray0, alpha);
          __m512i ra1 = _mm512_unpackhi_epi8(gray0, alpha);
          Store<align, mask>(bgra + 0 * A, _mm512_unpacklo_epi16(bg0, ra0), tails[1]);
          Store<align, mask>(bgra + 1 * A, _mm512_unpackhi_epi16(bg0, ra0), tails[2]);
          Store<align, mask>(bgra + 2 * A, _mm512_unpacklo_epi16(bg1, ra1), tails[3]);
          Store<align, mask>(bgra + 3 * A, _mm512_unpackhi_epi16(bg1, ra1), tails[4]);
      }

      template <bool align> SIMD_INLINE void GrayToBgra2(const uint8_t * gray, const __m512i & alpha, uint8_t * bgra)
      {
          __m512i gray0 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, Load<align>(gray + 0));
          __m512i bg0 = _mm512_unpacklo_epi8(gray0, gray0);
          __m512i bg1 = _mm512_unpackhi_epi8(gray0, gray0);
          __m512i ra0 = _mm512_unpacklo_epi8(gray0, alpha);
          __m512i ra1 = _mm512_unpackhi_epi8(gray0, alpha);
          Store<align>(bgra + 0 * A, _mm512_unpacklo_epi16(bg0, ra0));
          Store<align>(bgra + 1 * A, _mm512_unpackhi_epi16(bg0, ra0));
          Store<align>(bgra + 2 * A, _mm512_unpacklo_epi16(bg1, ra1));
          Store<align>(bgra + 3 * A, _mm512_unpackhi_epi16(bg1, ra1));

          __m512i gray1 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, Load<align>(gray + A));
          __m512i bg2 = _mm512_unpacklo_epi8(gray1, gray1);
          __m512i bg3 = _mm512_unpackhi_epi8(gray1, gray1);
          __m512i ra2 = _mm512_unpacklo_epi8(gray1, alpha);
          __m512i ra3 = _mm512_unpackhi_epi8(gray1, alpha);
          Store<align>(bgra + 4 * A, _mm512_unpacklo_epi16(bg2, ra2));
          Store<align>(bgra + 5 * A, _mm512_unpackhi_epi16(bg2, ra2));
          Store<align>(bgra + 6 * A, _mm512_unpacklo_epi16(bg3, ra3));
          Store<align>(bgra + 7 * A, _mm512_unpackhi_epi16(bg3, ra3));
      }

      template <bool align> void GrayToBgra(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
      {
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(gray) && Aligned(grayStride));

          __m512i _alpha = _mm512_set1_epi8(alpha);
          size_t alignedWidth = AlignLo(width, A);
          __mmask64 tailMasks[5];
          tailMasks[0] = TailMask64(width - alignedWidth);
          for (size_t c = 0; c < 4; ++c)
              tailMasks[1 + c] = TailMask64((width - alignedWidth) * 4 - A*c);
          size_t fullAlignedWidth = AlignLo(width, DA);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < fullAlignedWidth; col += DA)
                  GrayToBgra2<align>(gray + col, _alpha, bgra + col * 4);
              for (; col < alignedWidth; col += A)
                  GrayToBgra<align, false>(gray + col, _alpha, bgra + col * 4, tailMasks);
              if (col < width)
                  GrayToBgra<align, true>(gray + col, _alpha, bgra + col * 4, tailMasks);
              gray += grayStride;
              bgra += bgraStride;
          }
      }

      void GrayToBgra(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(bgra) && Aligned(gray) && Aligned(bgraStride) && Aligned(grayStride))
              GrayToBgra<true>(gray, width, height, grayStride, bgra, bgraStride, alpha);
          else
              GrayToBgra<false>(gray, width, height, grayStride, bgra, bgraStride, alpha);
      }

      template <bool align, bool mask> SIMD_INLINE void Int16ToGray(const int16_t * src, uint8_t * dst, __mmask64 tail = -1)
      {
          __m512i src0 = Load<align, mask>(src + 00, __mmask32(tail >> 00));
          __m512i src1 = Load<align, mask>(src + HA, __mmask32(tail >> 32));
          Store<align, mask>(dst, _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_packus_epi16(src0, src1)), tail);
      }

      template <bool align> SIMD_INLINE void Int16ToGray2(const int16_t * src, uint8_t * dst)
      {
          Store<align>(dst + 0 * A, _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_packus_epi16(Load<align>(src + 0 * HA), Load<align>(src + 1 * HA))));
          Store<align>(dst + 1 * A, _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_packus_epi16(Load<align>(src + 2 * HA), Load<align>(src + 3 * HA))));
      }

      template <bool align> void Int16ToGray(const int16_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
      {
          if (align)
              assert(Aligned(src) && Aligned(srcStride, HA) && Aligned(dst) && Aligned(dstStride));

          size_t alignedWidth = AlignLo(width, A);
          size_t fullAlignedWidth = AlignLo(width, DA);
          __mmask64 tailMask = TailMask64(width - alignedWidth);
          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < fullAlignedWidth; col += DA)
                  Int16ToGray2<align>(src + col, dst + col);
              for (; col < alignedWidth; col += A)
                  Int16ToGray<align, false>(src + col, dst + col);
              if (col < width)
                  Int16ToGray<false, true>(src + col, dst + col, tailMask);
              src += srcStride;
              dst += dstStride;
          }
      }

      void Int16ToGray(const uint8_t * src, size_t width, size_t height, size_t srcStride, uint8_t * dst, size_t dstStride)
      {
          if (Aligned(src) && Aligned(srcStride) && Aligned(dst) && Aligned(dstStride))
              Int16ToGray<true>((const int16_t *)src, width, height, srcStride / sizeof(int16_t), dst, dstStride);
          else
              Int16ToGray<false>((const int16_t *)src, width, height, srcStride / sizeof(int16_t), dst, dstStride);
      }

      template <bool align, bool mask> SIMD_INLINE void GrayToBgr(const uint8_t * gray, uint8_t * bgr, const __mmask64 tails[4])
      {
          const __m512i gray0 = Load<align, mask>(gray + 0 * A, tails[0]);
          Store<align, mask>(bgr + 0 * A, GrayToBgr<0>(gray0), tails[1]);
          Store<align, mask>(bgr + 1 * A, GrayToBgr<1>(gray0), tails[2]);
          Store<align, mask>(bgr + 2 * A, GrayToBgr<2>(gray0), tails[3]);
      }

      template <bool align> SIMD_INLINE void GrayToBgr2(const uint8_t * gray, uint8_t * bgr)
      {
          const __m512i gray0 = Load<align>(gray + 0 * A);
          Store<align>(bgr + 0 * A, GrayToBgr<0>(gray0));
          Store<align>(bgr + 1 * A, GrayToBgr<1>(gray0));
          Store<align>(bgr + 2 * A, GrayToBgr<2>(gray0));
          const __m512i gray1 = Load<align>(gray + 1 * A);
          Store<align>(bgr + 3 * A, GrayToBgr<0>(gray1));
          Store<align>(bgr + 4 * A, GrayToBgr<1>(gray1));
          Store<align>(bgr + 5 * A, GrayToBgr<2>(gray1));
      }

      template <bool align> void GrayToBgr(const uint8_t * gray, size_t width, size_t height, size_t grayStride, uint8_t *bgr, size_t bgrStride)
      {
          if (align)
              assert(Aligned(bgr) && Aligned(bgrStride) && Aligned(gray) && Aligned(grayStride));

          size_t alignedWidth = AlignLo(width, A);
          __mmask64 tailMasks[4];
          tailMasks[0] = TailMask64(width - alignedWidth);
          for (size_t c = 0; c < 3; ++c)
              tailMasks[1 + c] = TailMask64((width - alignedWidth) * 3 - A*c);
          size_t fullAlignedWidth = AlignLo(width, DA);

          for (size_t row = 0; row < height; ++row)
          {
              size_t col = 0;
              for (; col < fullAlignedWidth; col += DA)
                  GrayToBgr2<align>(gray + col, bgr + col * 3);
              for (; col < alignedWidth; col += A)
                  GrayToBgr<align, false>(gray + col, bgr + col * 3, tailMasks);
              if (col < width)
                  GrayToBgr<align, true>(gray + col, bgr + col * 3, tailMasks);
              gray += grayStride;
              bgr += bgrStride;
          }
      }

      void GrayToBgr(const uint8_t *gray, size_t width, size_t height, size_t grayStride, uint8_t *bgr, size_t bgrStride)
      {
          if (Aligned(bgr) && Aligned(gray) && Aligned(bgrStride) && Aligned(grayStride))
              GrayToBgr<true>(gray, width, height, grayStride, bgr, bgrStride);
          else
              GrayToBgr<false>(gray, width, height, grayStride, bgr, bgrStride);
      }
    }
  #endif
  }
