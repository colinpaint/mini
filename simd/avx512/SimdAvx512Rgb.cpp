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
#include "SimdConversion.h"

namespace Simd::Avx512bw {
  //{{{  BgraToYuv420p
  //{{{
  template <bool align, bool mask> SIMD_INLINE void LoadPreparedBgra16(const uint8_t * bgra, __m512i & b16_r16, __m512i & g16_1, const __mmask64 * ms)
  {
      __m512i _bgra = Load<align, mask>(bgra, ms[0]);
      b16_r16 = _mm512_and_si512(_bgra, K16_00FF);
      g16_1 = _mm512_or_si512(_mm512_shuffle_epi8(_bgra, K8_SUFFLE_BGRA_TO_G000), K32_00010000);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE __m512i LoadAndConvertBgraToY16(const uint8_t * bgra, __m512i & b16_r16, __m512i & g16_1, const __mmask64 * ms)
  {
      __m512i _b16_r16[2], _g16_1[2];
      LoadPreparedBgra16<align, mask>(bgra + 0, _b16_r16[0], _g16_1[0], ms + 0);
      LoadPreparedBgra16<align, mask>(bgra + A, _b16_r16[1], _g16_1[1], ms + 1);
      b16_r16 = Hadd32(_b16_r16[0], _b16_r16[1]);
      g16_1 = Hadd32(_g16_1[0], _g16_1[1]);
      return Saturate16iTo8u(_mm512_add_epi16(K16_Y_ADJUST, _mm512_packs_epi32(BgrToY32(_b16_r16[0], _g16_1[0]), BgrToY32(_b16_r16[1], _g16_1[1]))));
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE __m512i LoadAndConvertBgraToY8(const uint8_t * bgra, __m512i b16_r16[2], __m512i g16_1[2], const __mmask64 * ms)
  {
      __m512i lo = LoadAndConvertBgraToY16<align, mask>(bgra + 0 * A, b16_r16[0], g16_1[0], ms + 0);
      __m512i hi = LoadAndConvertBgraToY16<align, mask>(bgra + 2 * A, b16_r16[1], g16_1[1], ms + 2);
      return Permuted2Pack16iTo8u(lo, hi);
  }
  //}}}
  //{{{
  SIMD_INLINE void Average16(__m512i & a, const __m512i & b)
  {
      a = _mm512_srli_epi16(_mm512_add_epi16(_mm512_add_epi16(a, b), K16_0002), 2);
  }
  //}}}
  //{{{
  SIMD_INLINE __m512i ConvertU16(__m512i b16_r16[2], __m512i g16_1[2])
  {
      return Saturate16iTo8u(_mm512_add_epi16(K16_UV_ADJUST, _mm512_packs_epi32(BgrToU32(b16_r16[0], g16_1[0]), BgrToU32(b16_r16[1], g16_1[1]))));
  }
  //}}}
  //{{{
  SIMD_INLINE __m512i ConvertV16(__m512i b16_r16[2], __m512i g16_1[2])
  {
      return Saturate16iTo8u(_mm512_add_epi16(K16_UV_ADJUST, _mm512_packs_epi32(BgrToV32(b16_r16[0], g16_1[0]), BgrToV32(b16_r16[1], g16_1[1]))));
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToYuv420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v, const __mmask64 * ms)
  {
      const uint8_t * bgra1 = bgra0 + bgraStride;
      uint8_t * y1 = y0 + yStride;

      __m512i _b16_r16[2][2][2], _g16_1[2][2][2];
      Store<align, mask>(y0 + 0, LoadAndConvertBgraToY8<align, mask>(bgra0 + 0 * A, _b16_r16[0][0], _g16_1[0][0], ms + 0), ms[8]);
      Store<align, mask>(y0 + A, LoadAndConvertBgraToY8<align, mask>(bgra0 + 4 * A, _b16_r16[0][1], _g16_1[0][1], ms + 4), ms[9]);
      Store<align, mask>(y1 + 0, LoadAndConvertBgraToY8<align, mask>(bgra1 + 0 * A, _b16_r16[1][0], _g16_1[1][0], ms + 0), ms[8]);
      Store<align, mask>(y1 + A, LoadAndConvertBgraToY8<align, mask>(bgra1 + 4 * A, _b16_r16[1][1], _g16_1[1][1], ms + 4), ms[9]);

      Average16(_b16_r16[0][0][0], _b16_r16[1][0][0]);
      Average16(_b16_r16[0][0][1], _b16_r16[1][0][1]);
      Average16(_b16_r16[0][1][0], _b16_r16[1][1][0]);
      Average16(_b16_r16[0][1][1], _b16_r16[1][1][1]);

      Average16(_g16_1[0][0][0], _g16_1[1][0][0]);
      Average16(_g16_1[0][0][1], _g16_1[1][0][1]);
      Average16(_g16_1[0][1][0], _g16_1[1][1][0]);
      Average16(_g16_1[0][1][1], _g16_1[1][1][1]);

      Store<align, mask>(u, Permuted2Pack16iTo8u(ConvertU16(_b16_r16[0][0], _g16_1[0][0]), ConvertU16(_b16_r16[0][1], _g16_1[0][1])), ms[10]);
      Store<align, mask>(v, Permuted2Pack16iTo8u(ConvertV16(_b16_r16[0][0], _g16_1[0][0]), ConvertV16(_b16_r16[0][1], _g16_1[0][1])), ms[10]);
  }
  //}}}
  //{{{
  template <bool align> void BgraToYuv420p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      width /= 2;
      size_t alignedWidth = AlignLo(width, A);
      size_t tail = width - alignedWidth;
      __mmask64 tailMasks[11];
      for (size_t i = 0; i < 8; ++i)
          tailMasks[i] = TailMask64(tail * 8 - A*i);
      for (size_t i = 0; i < 2; ++i)
          tailMasks[8 + i] = TailMask64(tail * 2 - A*i);
      tailMasks[10] = TailMask64(tail);
      for (size_t row = 0; row < height; row += 2)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgraToYuv420p<align, false>(bgra + col * 8, bgraStride, y + col * 2, yStride, u + col, v + col, tailMasks);
          if (col < width)
              BgraToYuv420p<align, true>(bgra + col * 8, bgraStride, y + col * 2, yStride, u + col, v + col, tailMasks);
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
  //}}}
  //{{{  BgraToYuv422p
  //{{{
  SIMD_INLINE void Average16(__m512i a[2][2])
  {
      a[0][0] = _mm512_srli_epi16(_mm512_add_epi16(a[0][0], K16_0001), 1);
      a[0][1] = _mm512_srli_epi16(_mm512_add_epi16(a[0][1], K16_0001), 1);
      a[1][0] = _mm512_srli_epi16(_mm512_add_epi16(a[1][0], K16_0001), 1);
      a[1][1] = _mm512_srli_epi16(_mm512_add_epi16(a[1][1], K16_0001), 1);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToYuv422p(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v, const __mmask64 * ms)
  {
      __m512i _b16_r16[2][2], _g16_1[2][2];
      Store<align, mask>(y + 0, LoadAndConvertBgraToY8<align, mask>(bgra + 0 * A, _b16_r16[0], _g16_1[0], ms + 0), ms[8]);
      Store<align, mask>(y + A, LoadAndConvertBgraToY8<align, mask>(bgra + 4 * A, _b16_r16[1], _g16_1[1], ms + 4), ms[9]);

      Average16(_b16_r16);
      Average16(_g16_1);

      Store<align, mask>(u, Permuted2Pack16iTo8u(ConvertU16(_b16_r16[0], _g16_1[0]), ConvertU16(_b16_r16[1], _g16_1[1])), ms[10]);
      Store<align, mask>(v, Permuted2Pack16iTo8u(ConvertV16(_b16_r16[0], _g16_1[0]), ConvertV16(_b16_r16[1], _g16_1[1])), ms[10]);
  }
  //}}}
  //{{{
  template <bool align> void BgraToYuv422p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert(width % 2 == 0);
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      width /= 2;
      size_t alignedWidth = AlignLo(width, A);
      size_t tail = width - alignedWidth;
      __mmask64 tailMasks[11];
      for (size_t i = 0; i < 8; ++i)
          tailMasks[i] = TailMask64(tail * 8 - A*i);
      for (size_t i = 0; i < 2; ++i)
          tailMasks[8 + i] = TailMask64(tail * 2 - A*i);
      tailMasks[10] = TailMask64(tail);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgraToYuv422p<align, false>(bgra + col * 8, y + col * 2, u + col, v + col, tailMasks);
          if (col < width)
              BgraToYuv422p<align, true>(bgra + col * 8, y + col * 2, u + col, v + col, tailMasks);
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
  //}}}
  //{{{  BgraToYuv444p
  //{{{
  SIMD_INLINE __m512i ConvertY16(__m512i b16_r16[2], __m512i g16_1[2])
  {
      return Saturate16iTo8u(_mm512_add_epi16(K16_Y_ADJUST, _mm512_packs_epi32(BgrToY32(b16_r16[0], g16_1[0]), BgrToY32(b16_r16[1], g16_1[1]))));
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToYuv444p(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v, const __mmask64 * ms)
  {
      __m512i _b16_r16[2][2], _g16_1[2][2];
      LoadPreparedBgra16<align, mask>(bgra + 0 * A, _b16_r16[0][0], _g16_1[0][0], ms + 0);
      LoadPreparedBgra16<align, mask>(bgra + 1 * A, _b16_r16[0][1], _g16_1[0][1], ms + 1);
      LoadPreparedBgra16<align, mask>(bgra + 2 * A, _b16_r16[1][0], _g16_1[1][0], ms + 2);
      LoadPreparedBgra16<align, mask>(bgra + 3 * A, _b16_r16[1][1], _g16_1[1][1], ms + 3);

      Store<align, mask>(y, Permuted2Pack16iTo8u(ConvertY16(_b16_r16[0], _g16_1[0]), ConvertY16(_b16_r16[1], _g16_1[1])), ms[4]);
      Store<align, mask>(u, Permuted2Pack16iTo8u(ConvertU16(_b16_r16[0], _g16_1[0]), ConvertU16(_b16_r16[1], _g16_1[1])), ms[4]);
      Store<align, mask>(v, Permuted2Pack16iTo8u(ConvertV16(_b16_r16[0], _g16_1[0]), ConvertV16(_b16_r16[1], _g16_1[1])), ms[4]);
  }
  //}}}
  //{{{
  template <bool align> void BgraToYuv444p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
      }

      size_t alignedWidth = AlignLo(width, A);
      size_t tail = width - alignedWidth;
      __mmask64 tailMasks[5];
      for (size_t i = 0; i < 4; ++i)
          tailMasks[i] = TailMask64(tail * 4 - A*i);
      tailMasks[4] = TailMask64(tail);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgraToYuv444p<align, false>(bgra + col * 4, y + col, u + col, v + col, tailMasks);
          if (col < width)
              BgraToYuv444p<align, true>(bgra + col * 4, y + col, u + col, v + col, tailMasks);
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
  //}}}
  //{{{  BgraToYuva420p
  //{{{
  template <bool align, bool mask> SIMD_INLINE void LoadPreparedBgra16(const uint8_t * bgra, __m512i & b16_r16, __m512i & g16_1, __m512i & a32, const __mmask64 * tails)
  {
      __m512i _bgra = Load<align, mask>(bgra, tails[0]);
      b16_r16 = _mm512_and_si512(_bgra, K16_00FF);
      g16_1 = _mm512_or_si512(_mm512_shuffle_epi8(_bgra, K8_SUFFLE_BGRA_TO_G000), K32_00010000);
      a32 = _mm512_shuffle_epi8(_bgra, K8_SUFFLE_BGRA_TO_A000);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void LoadAndConvertYA16(const uint8_t * bgra, __m512i & b16_r16, __m512i & g16_1, __m512i & y16, __m512i & a16, const __mmask64 * tails)
  {
      __m512i _b16_r16[2], _g16_1[2], a32[2];
      LoadPreparedBgra16<align, mask>(bgra + 0, _b16_r16[0], _g16_1[0], a32[0], tails + 0);
      LoadPreparedBgra16<align, mask>(bgra + A, _b16_r16[1], _g16_1[1], a32[1], tails + 1);
      b16_r16 = Hadd32(_b16_r16[0], _b16_r16[1]);
      g16_1 = Hadd32(_g16_1[0], _g16_1[1]);
      y16 = Saturate16iTo8u(_mm512_add_epi16(K16_Y_ADJUST, _mm512_packs_epi32(BgrToY32(_b16_r16[0], _g16_1[0]), BgrToY32(_b16_r16[1], _g16_1[1]))));
      a16 = _mm512_packs_epi32(a32[0], a32[1]);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void LoadAndStoreYA(const uint8_t * bgra, __m512i b16_r16[2], __m512i g16_1[2], uint8_t * y, uint8_t * a, const __mmask64 * tails)
  {
      __m512i y16[2], a16[2];
      LoadAndConvertYA16<align, mask>(bgra + 0 * A, b16_r16[0], g16_1[0], y16[0], a16[0], tails + 0);
      LoadAndConvertYA16<align, mask>(bgra + 2 * A, b16_r16[1], g16_1[1], y16[1], a16[1], tails + 2);
      Store<align, mask>(y, Permuted2Pack16iTo8u(y16[0], y16[1]), tails[4]);
      Store<align, mask>(a, Permuted2Pack16iTo8u(a16[0], a16[1]), tails[4]);
  }

  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToYuva420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v, uint8_t * a0, size_t aStride, const __mmask64 * tails)
  {
      const uint8_t * bgra1 = bgra0 + bgraStride;
      uint8_t * y1 = y0 + yStride;
      uint8_t * a1 = a0 + aStride;

      __m512i _b16_r16[2][2][2], _g16_1[2][2][2];
      LoadAndStoreYA<align, mask>(bgra0 + 0 * A, _b16_r16[0][0], _g16_1[0][0], y0 + 0, a0 + 0, tails + 0);
      LoadAndStoreYA<align, mask>(bgra0 + 4 * A, _b16_r16[0][1], _g16_1[0][1], y0 + A, a0 + A, tails + 5);
      LoadAndStoreYA<align, mask>(bgra1 + 0 * A, _b16_r16[1][0], _g16_1[1][0], y1 + 0, a1 + 0, tails + 0);
      LoadAndStoreYA<align, mask>(bgra1 + 4 * A, _b16_r16[1][1], _g16_1[1][1], y1 + A, a1 + A, tails + 5);

      Average16(_b16_r16[0][0][0], _b16_r16[1][0][0]);
      Average16(_b16_r16[0][0][1], _b16_r16[1][0][1]);
      Average16(_b16_r16[0][1][0], _b16_r16[1][1][0]);
      Average16(_b16_r16[0][1][1], _b16_r16[1][1][1]);

      Average16(_g16_1[0][0][0], _g16_1[1][0][0]);
      Average16(_g16_1[0][0][1], _g16_1[1][0][1]);
      Average16(_g16_1[0][1][0], _g16_1[1][1][0]);
      Average16(_g16_1[0][1][1], _g16_1[1][1][1]);

      Store<align, mask>(u, Permuted2Pack16iTo8u(ConvertU16(_b16_r16[0][0], _g16_1[0][0]), ConvertU16(_b16_r16[0][1], _g16_1[0][1])), tails[10]);
      Store<align, mask>(v, Permuted2Pack16iTo8u(ConvertV16(_b16_r16[0][0], _g16_1[0][0]), ConvertV16(_b16_r16[0][1], _g16_1[0][1])), tails[10]);
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

      width /= 2;
      size_t alignedWidth = AlignLo(width, A);
      size_t tail = width - alignedWidth;
      __mmask64 tails[11];
      for (size_t i = 0; i < 4; ++i)
      {
          tails[i + 0] = TailMask64(tail * 8 - A * (i + 0));
          tails[i + 5] = TailMask64(tail * 8 - A * (i + 4));
      }
      tails[4] = TailMask64(tail * 2 - A * 0);
      tails[9] = TailMask64(tail * 2 - A * 1);
      tails[10] = TailMask64(tail);
      for (size_t row = 0; row < height; row += 2)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgraToYuva420p<align, false>(bgra + col*8, bgraStride, y + col*2, yStride, u + col, v + col, a + col*2, aStride, tails);
          if (col < width)
              BgraToYuva420p<align, true>(bgra + col*8, bgraStride, y + col*2, yStride, u + col, v + col, a + col*2, aStride, tails);
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
  //{{{  BgraToGray32
  const __m512i K16_BLUE_RED = SIMD_MM512_SET2_EPI16(Base::BLUE_TO_GRAY_WEIGHT, Base::RED_TO_GRAY_WEIGHT);
  const __m512i K16_GREEN_0000 = SIMD_MM512_SET2_EPI16(Base::GREEN_TO_GRAY_WEIGHT, 0x0000);
  const __m512i K32_ROUND_TERM = SIMD_MM512_SET1_EPI32(Base::BGR_TO_GRAY_ROUND_TERM);
  //{{{
  SIMD_INLINE __m512i BgraToGray32(__m512i bgra)
  {
      const __m512i g0a0 = _mm512_shuffle_epi8(bgra, K8_SUFFLE_BGRA_TO_G0A0);
      const __m512i b0r0 = _mm512_and_si512(bgra, K16_00FF);
      const __m512i weightedSum = _mm512_add_epi32(_mm512_madd_epi16(g0a0, K16_GREEN_0000), _mm512_madd_epi16(b0r0, K16_BLUE_RED));
      return _mm512_srli_epi32(_mm512_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
  }
  //}}}
  //}}}
  //{{{  BgraToGray
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToGray(const uint8_t * bgra, uint8_t * gray, __mmask64 ms[5])
  {
      __m512i gray0 = BgraToGray32(Load<align, mask>(bgra + 0 * A, ms[0]));
      __m512i gray1 = BgraToGray32(Load<align, mask>(bgra + 1 * A, ms[1]));
      __m512i gray2 = BgraToGray32(Load<align, mask>(bgra + 2 * A, ms[2]));
      __m512i gray3 = BgraToGray32(Load<align, mask>(bgra + 3 * A, ms[3]));
      __m512i gray01 = _mm512_packs_epi32(gray0, gray1);
      __m512i gray23 = _mm512_packs_epi32(gray2, gray3);
      __m512i gray0123 = _mm512_packus_epi16(gray01, gray23);
      Store<align, mask>(gray, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, gray0123), ms[4]);
  }
  //}}}
  //{{{
  template <bool align> void BgraToGray(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * gray, size_t grayStride)
  {
      if (align)
          assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(gray) && Aligned(grayStride));

      size_t alignedWidth = AlignLo(width, A);
      __mmask64 tailMasks[5];
      for (size_t c = 0; c < 4; ++c)
          tailMasks[c] = TailMask64((width - alignedWidth) * 4 - A * c);
      tailMasks[4] = TailMask64(width - alignedWidth);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgraToGray<align, false>(bgra + col * 4, gray + col, tailMasks);
          if (col < width)
              BgraToGray<align, true>(bgra + col * 4, gray + col, tailMasks);
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

  //{{{  BgraToBgr
  //{{{
  const __m512i K8_SUFFLE_BGRA_TO_BGR = SIMD_MM512_SETR_EPI8(
      0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1,
      0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1,
      0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1,
      0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1);
  //}}}
  const __m512i K32_PERMUTE_BGRA_TO_BGR = SIMD_MM512_SETR_EPI32(0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1);
  const __m512i K32_PERMUTE_BGRA_TO_BGR_0 = SIMD_MM512_SETR_EPI32(0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0A, 0x0C, 0x0D, 0x0E, 0x10, 0x11, 0x12, 0x14);
  const __m512i K32_PERMUTE_BGRA_TO_BGR_1 = SIMD_MM512_SETR_EPI32(0x05, 0x06, 0x08, 0x09, 0x0A, 0x0C, 0x0D, 0x0E, 0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x18, 0x19);
  const __m512i K32_PERMUTE_BGRA_TO_BGR_2 = SIMD_MM512_SETR_EPI32(0x0A, 0x0C, 0x0D, 0x0E, 0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x18, 0x19, 0x1A, 0x1C, 0x1D, 0x1E);
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToBgr(const uint8_t * bgra, uint8_t * bgr, __mmask64 bgraMask = -1, __mmask64 bgrMask = 0x0000ffffffffffff)
  {
      __m512i _bgra = Load<align, mask>(bgra, bgraMask);
      __m512i _bgr = _mm512_permutexvar_epi32(K32_PERMUTE_BGRA_TO_BGR, _mm512_shuffle_epi8(_bgra, K8_SUFFLE_BGRA_TO_BGR));
      Store<false, true>(bgr, _bgr, bgrMask);
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void BgraToBgr(const uint8_t * bgra, uint8_t * bgr)
  {
      __m512i bgr0 = _mm512_shuffle_epi8(Load<align>(bgra + 0 * A), K8_SUFFLE_BGRA_TO_BGR);
      __m512i bgr1 = _mm512_shuffle_epi8(Load<align>(bgra + 1 * A), K8_SUFFLE_BGRA_TO_BGR);
      __m512i bgr2 = _mm512_shuffle_epi8(Load<align>(bgra + 2 * A), K8_SUFFLE_BGRA_TO_BGR);
      __m512i bgr3 = _mm512_shuffle_epi8(Load<align>(bgra + 3 * A), K8_SUFFLE_BGRA_TO_BGR);
      Store<align>(bgr + 0 * A, _mm512_permutex2var_epi32(bgr0, K32_PERMUTE_BGRA_TO_BGR_0, bgr1));
      Store<align>(bgr + 1 * A, _mm512_permutex2var_epi32(bgr1, K32_PERMUTE_BGRA_TO_BGR_1, bgr2));
      Store<align>(bgr + 2 * A, _mm512_permutex2var_epi32(bgr2, K32_PERMUTE_BGRA_TO_BGR_2, bgr3));
  }
  //}}}
  //{{{
  template <bool align> void BgraToBgr(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * bgr, size_t bgrStride)
  {
      if (align)
          assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride));

      size_t fullAlignedWidth = AlignLo(width, A);
      size_t alignedWidth = AlignLo(width, F);
      __mmask64 bgraTailMask = TailMask64((width - alignedWidth) * 4);
      __mmask64 bgrTailMask = TailMask64((width - alignedWidth) * 3);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < fullAlignedWidth; col += A)
              BgraToBgr<align>(bgra + 4 * col, bgr + 3 * col);
          for (; col < alignedWidth; col += F)
              BgraToBgr<align, false>(bgra + 4 * col, bgr + 3 * col);
          if (col < width)
              BgraToBgr<align, true>(bgra + 4 * col, bgr + 3 * col, bgraTailMask, bgrTailMask);
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
  //{{{  BgraToRgb
  //{{{
  const __m512i K8_SUFFLE_BGRA_TO_RGB = SIMD_MM512_SETR_EPI8(
      0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1,
      0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1,
      0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1,
      0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1);
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToRgb(const uint8_t* bgra, uint8_t* rgb, __mmask64 bgraMask = -1, __mmask64 rgbMask = 0x0000ffffffffffff)
  {
      __m512i _bgra = Load<align, mask>(bgra, bgraMask);
      __m512i _rgb = _mm512_permutexvar_epi32(K32_PERMUTE_BGRA_TO_BGR, _mm512_shuffle_epi8(_bgra, K8_SUFFLE_BGRA_TO_RGB));
      Store<false, true>(rgb, _rgb, rgbMask);
  }
  //}}}
  //{{{
  template <bool align> SIMD_INLINE void BgraToRgb(const uint8_t* bgra, uint8_t* rgb)
  {
      __m512i rgb0 = _mm512_shuffle_epi8(Load<align>(bgra + 0 * A), K8_SUFFLE_BGRA_TO_RGB);
      __m512i rgb1 = _mm512_shuffle_epi8(Load<align>(bgra + 1 * A), K8_SUFFLE_BGRA_TO_RGB);
      __m512i rgb2 = _mm512_shuffle_epi8(Load<align>(bgra + 2 * A), K8_SUFFLE_BGRA_TO_RGB);
      __m512i rgb3 = _mm512_shuffle_epi8(Load<align>(bgra + 3 * A), K8_SUFFLE_BGRA_TO_RGB);
      Store<align>(rgb + 0 * A, _mm512_permutex2var_epi32(rgb0, K32_PERMUTE_BGRA_TO_BGR_0, rgb1));
      Store<align>(rgb + 1 * A, _mm512_permutex2var_epi32(rgb1, K32_PERMUTE_BGRA_TO_BGR_1, rgb2));
      Store<align>(rgb + 2 * A, _mm512_permutex2var_epi32(rgb2, K32_PERMUTE_BGRA_TO_BGR_2, rgb3));
  }
  //}}}
  //{{{
  template <bool align> void BgraToRgb(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgb, size_t rgbStride)
  {
      if (align)
          assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride));

      size_t fullAlignedWidth = AlignLo(width, A);
      size_t alignedWidth = AlignLo(width, F);
      __mmask64 bgraTailMask = TailMask64((width - alignedWidth) * 4);
      __mmask64 rgbTailMask = TailMask64((width - alignedWidth) * 3);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < fullAlignedWidth; col += A)
              BgraToRgb<align>(bgra + 4 * col, rgb + 3 * col);
          for (; col < alignedWidth; col += F)
              BgraToRgb<align, false>(bgra + 4 * col, rgb + 3 * col);
          if (col < width)
              BgraToRgb<align, true>(bgra + 4 * col, rgb + 3 * col, bgraTailMask, rgbTailMask);
          bgra += bgraStride;
          rgb += rgbStride;
      }
  }
  //}}}
  //{{{
  void BgraToRgb(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* bgr, size_t bgrStride)
  {
      if (Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride))
          BgraToRgb<true>(bgra, width, height, bgraStride, bgr, bgrStride);
      else
          BgraToRgb<false>(bgra, width, height, bgraStride, bgr, bgrStride);
  }
  //}}}
  //}}}
  //{{{  BgraToRgba
  //{{{
  const __m512i K8_BGRA_TO_RGBA = SIMD_MM512_SETR_EPI8(
      0x2, 0x1, 0x0, 0x3, 0x6, 0x5, 0x4, 0x7, 0xA, 0x9, 0x8, 0xB, 0xE, 0xD, 0xC, 0xF,
      0x2, 0x1, 0x0, 0x3, 0x6, 0x5, 0x4, 0x7, 0xA, 0x9, 0x8, 0xB, 0xE, 0xD, 0xC, 0xF,
      0x2, 0x1, 0x0, 0x3, 0x6, 0x5, 0x4, 0x7, 0xA, 0x9, 0x8, 0xB, 0xE, 0xD, 0xC, 0xF,
      0x2, 0x1, 0x0, 0x3, 0x6, 0x5, 0x4, 0x7, 0xA, 0x9, 0x8, 0xB, 0xE, 0xD, 0xC, 0xF);
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgraToRgba(const uint8_t* bgra, uint8_t* rgba, __mmask64 tail = -1)
  {
      Store<align, mask>(rgba, _mm512_shuffle_epi8((Load<align, mask>(bgra, tail)), K8_BGRA_TO_RGBA), tail);
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
      __mmask64 tail = TailMask64(size - sizeA);

      for (size_t row = 0; row < height; ++row)
      {
          size_t i = 0;
          for (; i < sizeA; i += A)
              BgraToRgba<align, false>(bgra + i, rgba + i);
          if (i < size)
              BgraToRgba<align, true>(bgra + i, rgba + i, tail);
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
  //}}}

  //{{{  RgbaToGray
  const __m512i K16_RED_BLUE = SIMD_MM512_SET2_EPI16(Base::RED_TO_GRAY_WEIGHT, Base::BLUE_TO_GRAY_WEIGHT);
  //{{{
  SIMD_INLINE __m512i RgbaToGray32(__m512i rgba)
  {
      const __m512i g0a0 = _mm512_shuffle_epi8(rgba, K8_SUFFLE_BGRA_TO_G0A0);
      const __m512i r0b0 = _mm512_and_si512(rgba, K16_00FF);
      const __m512i weightedSum = _mm512_add_epi32(_mm512_madd_epi16(g0a0, K16_GREEN_0000), _mm512_madd_epi16(r0b0, K16_RED_BLUE));
      return _mm512_srli_epi32(_mm512_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void RgbaToGray(const uint8_t* rgba, uint8_t* gray, __mmask64 ms[5])
  {
      __m512i gray0 = RgbaToGray32(Load<align, mask>(rgba + 0 * A, ms[0]));
      __m512i gray1 = RgbaToGray32(Load<align, mask>(rgba + 1 * A, ms[1]));
      __m512i gray2 = RgbaToGray32(Load<align, mask>(rgba + 2 * A, ms[2]));
      __m512i gray3 = RgbaToGray32(Load<align, mask>(rgba + 3 * A, ms[3]));
      __m512i gray01 = _mm512_packs_epi32(gray0, gray1);
      __m512i gray23 = _mm512_packs_epi32(gray2, gray3);
      __m512i gray0123 = _mm512_packus_epi16(gray01, gray23);
      Store<align, mask>(gray, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, gray0123), ms[4]);
  }
  //}}}
  //{{{
  template <bool align> void RgbaToGray(const uint8_t* rgba, size_t width, size_t height, size_t rgbaStride, uint8_t* gray, size_t grayStride)
  {
      if (align)
          assert(Aligned(rgba) && Aligned(rgbaStride) && Aligned(gray) && Aligned(grayStride));

      size_t alignedWidth = AlignLo(width, A);
      __mmask64 tailMasks[5];
      for (size_t c = 0; c < 4; ++c)
          tailMasks[c] = TailMask64((width - alignedWidth) * 4 - A * c);
      tailMasks[4] = TailMask64(width - alignedWidth);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              RgbaToGray<align, false>(rgba + col * 4, gray + col, tailMasks);
          if (col < width)
              RgbaToGray<align, true>(rgba + col * 4, gray + col, tailMasks);
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

  //{{{  BgrToYuv420p
  //{{{
  template <bool align, bool mask> SIMD_INLINE void LoadPreparedBgr16(const uint8_t * bgr, __m512i & b16_r16, __m512i & g16_1, const __mmask64 * ms)
  {
      __m512i _bgr = Load<align, mask>(bgr, ms[0]);
      __m512i bgr1 = _mm512_permutex2var_epi32(_bgr, K32_PERMUTE_BGR_TO_BGRA, K8_01);
      b16_r16 = _mm512_shuffle_epi8(bgr1, K8_SUFFLE_BGR_TO_B0R0);
      g16_1 = _mm512_shuffle_epi8(bgr1, K8_SUFFLE_BGR_TO_G010);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE __m512i LoadAndConvertBgrToY16(const uint8_t * bgr, __m512i & b16_r16, __m512i & g16_1, const __mmask64 * ms)
  {
      __m512i _b16_r16[2], _g16_1[2];
      LoadPreparedBgr16<align, mask>(bgr + 00, _b16_r16[0], _g16_1[0], ms + 0);
      LoadPreparedBgr16<false, mask>(bgr + 48, _b16_r16[1], _g16_1[1], ms + 1);
      b16_r16 = Hadd32(_b16_r16[0], _b16_r16[1]);
      g16_1 = Hadd32(_g16_1[0], _g16_1[1]);
      return Saturate16iTo8u(_mm512_add_epi16(K16_Y_ADJUST, _mm512_packs_epi32(BgrToY32(_b16_r16[0], _g16_1[0]), BgrToY32(_b16_r16[1], _g16_1[1]))));
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE __m512i LoadAndConvertBgrToY8(const uint8_t * bgr, __m512i b16_r16[2], __m512i g16_1[2], const __mmask64 * ms)
  {
      __m512i lo = LoadAndConvertBgrToY16<align, mask>(bgr + 00, b16_r16[0], g16_1[0], ms + 0);
      __m512i hi = LoadAndConvertBgrToY16<false, mask>(bgr + 96, b16_r16[1], g16_1[1], ms + 2);
      return Permuted2Pack16iTo8u(lo, hi);
  }
  //}}}
  //{{{
  SIMD_INLINE void Average16v2(__m512i & a, const __m512i & b)
  {
      a = _mm512_srli_epi16(_mm512_add_epi16(_mm512_add_epi16(a, b), K16_0002), 2);
  }
  //}}}
  //{{{
  SIMD_INLINE __m512i ConvertU16v2(__m512i b16_r16[2], __m512i g16_1[2])
  {
      return Saturate16iTo8u(_mm512_add_epi16(K16_UV_ADJUST, _mm512_packs_epi32(BgrToU32(b16_r16[0], g16_1[0]), BgrToU32(b16_r16[1], g16_1[1]))));
  }
  //}}}
  //{{{
  SIMD_INLINE __m512i ConvertV16v2(__m512i b16_r16[2], __m512i g16_1[2])
  {
      return Saturate16iTo8u(_mm512_add_epi16(K16_UV_ADJUST, _mm512_packs_epi32(BgrToV32(b16_r16[0], g16_1[0]), BgrToV32(b16_r16[1], g16_1[1]))));
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgrToYuv420p(const uint8_t * bgr0, size_t bgrStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v, const __mmask64 * ms)
  {
      const uint8_t * bgr1 = bgr0 + bgrStride;
      uint8_t * y1 = y0 + yStride;

      __m512i _b16_r16[2][2][2], _g16_1[2][2][2];
      Store<align, mask>(y0 + 0, LoadAndConvertBgrToY8<align, mask>(bgr0 + 0 * A, _b16_r16[0][0], _g16_1[0][0], ms + 0), ms[8]);
      Store<align, mask>(y0 + A, LoadAndConvertBgrToY8<align, mask>(bgr0 + 3 * A, _b16_r16[0][1], _g16_1[0][1], ms + 4), ms[9]);
      Store<align, mask>(y1 + 0, LoadAndConvertBgrToY8<align, mask>(bgr1 + 0 * A, _b16_r16[1][0], _g16_1[1][0], ms + 0), ms[8]);
      Store<align, mask>(y1 + A, LoadAndConvertBgrToY8<align, mask>(bgr1 + 3 * A, _b16_r16[1][1], _g16_1[1][1], ms + 4), ms[9]);

      Average16v2(_b16_r16[0][0][0], _b16_r16[1][0][0]);
      Average16v2(_b16_r16[0][0][1], _b16_r16[1][0][1]);
      Average16v2(_b16_r16[0][1][0], _b16_r16[1][1][0]);
      Average16v2(_b16_r16[0][1][1], _b16_r16[1][1][1]);

      Average16v2(_g16_1[0][0][0], _g16_1[1][0][0]);
      Average16v2(_g16_1[0][0][1], _g16_1[1][0][1]);
      Average16v2(_g16_1[0][1][0], _g16_1[1][1][0]);
      Average16v2(_g16_1[0][1][1], _g16_1[1][1][1]);

      Store<align, mask>(u, Permuted2Pack16iTo8u(ConvertU16v2(_b16_r16[0][0], _g16_1[0][0]), ConvertU16v2(_b16_r16[0][1], _g16_1[0][1])), ms[10]);
      Store<align, mask>(v, Permuted2Pack16iTo8u(ConvertV16v2(_b16_r16[0][0], _g16_1[0][0]), ConvertV16v2(_b16_r16[0][1], _g16_1[0][1])), ms[10]);
  }
  //}}}
  //{{{
  template <bool align> void BgrToYuv420p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0));
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
      }

      width /= 2;
      size_t alignedWidth = AlignLo(width - 1, A);
      size_t tail = width - alignedWidth;
      __mmask64 tailMasks[11];
      for (size_t i = 0; i < 8; ++i)
          tailMasks[i] = TailMask64(tail * 6 - 48 * i) & 0x0000FFFFFFFFFFFF;
      for (size_t i = 0; i < 2; ++i)
          tailMasks[8 + i] = TailMask64(tail * 2 - A*i);
      tailMasks[10] = TailMask64(tail);
      for (size_t row = 0; row < height; row += 2)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgrToYuv420p<align, false>(bgr + col * 6, bgrStride, y + col * 2, yStride, u + col, v + col, tailMasks);
          if (col < width)
              BgrToYuv420p<align, true>(bgr + col * 6, bgrStride, y + col * 2, yStride, u + col, v + col, tailMasks);
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
  //{{{  BgrToYuv422p
  //{{{
  SIMD_INLINE void Average16v3(__m512i a[2][2])
  {
      a[0][0] = _mm512_srli_epi16(_mm512_add_epi16(a[0][0], K16_0001), 1);
      a[0][1] = _mm512_srli_epi16(_mm512_add_epi16(a[0][1], K16_0001), 1);
      a[1][0] = _mm512_srli_epi16(_mm512_add_epi16(a[1][0], K16_0001), 1);
      a[1][1] = _mm512_srli_epi16(_mm512_add_epi16(a[1][1], K16_0001), 1);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgrToYuv422p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v, const __mmask64 * ms)
  {
      __m512i _b16_r16[2][2], _g16_1[2][2];
      Store<align, mask>(y + 0, LoadAndConvertBgrToY8<align, mask>(bgr + 0 * A, _b16_r16[0], _g16_1[0], ms + 0), ms[8]);
      Store<align, mask>(y + A, LoadAndConvertBgrToY8<align, mask>(bgr + 3 * A, _b16_r16[1], _g16_1[1], ms + 4), ms[9]);

      Average16v3(_b16_r16);
      Average16v3(_g16_1);

      Store<align, mask>(u, Permuted2Pack16iTo8u(ConvertU16v2(_b16_r16[0], _g16_1[0]), ConvertU16v2(_b16_r16[1], _g16_1[1])), ms[10]);
      Store<align, mask>(v, Permuted2Pack16iTo8u(ConvertV16v2(_b16_r16[0], _g16_1[0]), ConvertV16v2(_b16_r16[1], _g16_1[1])), ms[10]);
  }
  //}}}
  //{{{
  template <bool align> void BgrToYuv422p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert(width % 2 == 0);
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
      }

      width /= 2;
      size_t alignedWidth = AlignLo(width - 1, A);
      size_t tail = width - alignedWidth;
      __mmask64 tailMasks[11];
      for (size_t i = 0; i < 8; ++i)
          tailMasks[i] = TailMask64(tail * 6 - 48 * i) & 0x0000FFFFFFFFFFFF;
      for (size_t i = 0; i < 2; ++i)
          tailMasks[8 + i] = TailMask64(tail * 2 - A*i);
      tailMasks[10] = TailMask64(tail);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgrToYuv422p<align, false>(bgr + col * 6, y + col * 2, u + col, v + col, tailMasks);
          if (col < width)
              BgrToYuv422p<align, true>(bgr + col * 6, y + col * 2, u + col, v + col, tailMasks);
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
  //{{{  BgrToYuv444p
  //{{{
  SIMD_INLINE __m512i ConvertY16v2(__m512i b16_r16[2], __m512i g16_1[2])
  {
      return Saturate16iTo8u(_mm512_add_epi16(K16_Y_ADJUST, _mm512_packs_epi32(BgrToY32(b16_r16[0], g16_1[0]), BgrToY32(b16_r16[1], g16_1[1]))));
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgrToYuv444p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v, const __mmask64 * ms)
  {
      __m512i _b16_r16[2][2], _g16_1[2][2];
      LoadPreparedBgr16<align, mask>(bgr + 0x00, _b16_r16[0][0], _g16_1[0][0], ms + 0);
      LoadPreparedBgr16<false, mask>(bgr + 0x30, _b16_r16[0][1], _g16_1[0][1], ms + 1);
      LoadPreparedBgr16<false, mask>(bgr + 0x60, _b16_r16[1][0], _g16_1[1][0], ms + 2);
      LoadPreparedBgr16<false, mask>(bgr + 0x90, _b16_r16[1][1], _g16_1[1][1], ms + 3);

      Store<align, mask>(y, Permuted2Pack16iTo8u(ConvertY16v2(_b16_r16[0], _g16_1[0]), ConvertY16v2(_b16_r16[1], _g16_1[1])), ms[4]);
      Store<align, mask>(u, Permuted2Pack16iTo8u(ConvertU16v2(_b16_r16[0], _g16_1[0]), ConvertU16v2(_b16_r16[1], _g16_1[1])), ms[4]);
      Store<align, mask>(v, Permuted2Pack16iTo8u(ConvertV16v2(_b16_r16[0], _g16_1[0]), ConvertV16v2(_b16_r16[1], _g16_1[1])), ms[4]);
  }
  //}}}
  //{{{
  template <bool align> void BgrToYuv444p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      if (align)
      {
          assert(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride));
          assert(Aligned(v) && Aligned(vStride) && Aligned(bgr) && Aligned(bgrStride));
      }

      size_t alignedWidth = AlignLo(width - 1, A);
      size_t tail = width - alignedWidth;
      __mmask64 tailMasks[5];
      for (size_t i = 0; i < 4; ++i)
          tailMasks[i] = TailMask64(tail * 3 - 48 * i) & 0x0000FFFFFFFFFFFF;
      tailMasks[4] = TailMask64(tail);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgrToYuv444p<align, false>(bgr + col * 3, y + col, u + col, v + col, tailMasks);
          if (col < width)
              BgrToYuv444p<align, true>(bgr + col * 3, y + col, u + col, v + col, tailMasks);
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
  //{{{  BgrToRgb
  const __m512i K64_PRMT_P0 = SIMD_MM512_SETR_EPI64(0x7, 0x2, 0x1, 0x4, 0x3, 0x6, 0x5, 0x8);
  const __m512i K64_PRMT_P2 = SIMD_MM512_SETR_EPI64(0x7, 0xA, 0x9, 0xC, 0xB, 0xE, 0xD, 0x8);
  //{{{
  const __m512i K8_SHFL_0S0 = SIMD_MM512_SETR_EPI8(
      0x2, 0x1, 0x0, 0x5, 0x4, 0x3, 0x8, 0x7, 0x6, 0xB, 0xA, 0x9, 0xE, 0xD, 0xC, -1,
      0x0, -1, 0x4, 0x3, 0x2, 0x7, 0x6, 0x5, 0xA, 0x9, 0x8, 0xD, 0xC, 0xB, -1, 0xF,
      -1, 0x3, 0x2, 0x1, 0x6, 0x5, 0x4, 0x9, 0x8, 0x7, 0xC, 0xB, 0xA, 0xF, 0xE, 0xD,
      0x2, 0x1, 0x0, 0x5, 0x4, 0x3, 0x8, 0x7, 0x6, 0xB, 0xA, 0x9, 0xE, 0xD, 0xC, -1);
  //}}}
  //{{{
  const __m512i K8_SHFL_0P0 = SIMD_MM512_SETR_EPI8(
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x9,
      -1, 0x7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x8, -1,
      0x6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x9);
  //}}}
  //{{{
  const __m512i K8_SHFL_1S1 = SIMD_MM512_SETR_EPI8(
      0x0, -1, 0x4, 0x3, 0x2, 0x7, 0x6, 0x5, 0xA, 0x9, 0x8, 0xD, 0xC, 0xB, -1, 0xF,
      -1, 0x3, 0x2, 0x1, 0x6, 0x5, 0x4, 0x9, 0x8, 0x7, 0xC, 0xB, 0xA, 0xF, 0xE, 0xD,
      0x2, 0x1, 0x0, 0x5, 0x4, 0x3, 0x8, 0x7, 0x6, 0xB, 0xA, 0x9, 0xE, 0xD, 0xC, -1,
      0x0, -1, 0x4, 0x3, 0x2, 0x7, 0x6, 0x5, 0xA, 0x9, 0x8, 0xD, 0xC, 0xB, -1, 0xF);
  //}}}
  //{{{
  const __m512i K8_SHFL_1P1 = SIMD_MM512_SETR_EPI8(
      -1, 0x7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x8, -1,
      0x6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x9,
      -1, 0x7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
  //}}}
  //{{{
  const __m512i K8_SHFL_1P2 = SIMD_MM512_SETR_EPI8(
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x8, -1);
  //}}}
  //{{{
  const __m512i K8_SHFL_2S2 = SIMD_MM512_SETR_EPI8(
      -1, 0x3, 0x2, 0x1, 0x6, 0x5, 0x4, 0x9, 0x8, 0x7, 0xC, 0xB, 0xA, 0xF, 0xE, 0xD,
      0x2, 0x1, 0x0, 0x5, 0x4, 0x3, 0x8, 0x7, 0x6, 0xB, 0xA, 0x9, 0xE, 0xD, 0xC, -1,
      0x0, -1, 0x4, 0x3, 0x2, 0x7, 0x6, 0x5, 0xA, 0x9, 0x8, 0xD, 0xC, 0xB, -1, 0xF,
      -1, 0x3, 0x2, 0x1, 0x6, 0x5, 0x4, 0x9, 0x8, 0x7, 0xC, 0xB, 0xA, 0xF, 0xE, 0xD);
  //}}}
  //{{{
  const __m512i K8_SHFL_2P2 = SIMD_MM512_SETR_EPI8(
      0x6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x9,
      -1, 0x7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x8, -1,
      0x6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgrToRgb(const uint8_t * src, uint8_t * dst, const __mmask64 * tails)
  {
      __m512i s0 = Load<align, mask>(src + 0 * A, tails[0]);
      __m512i s1 = Load<align, mask>(src + 1 * A, tails[1]);
      __m512i s2 = Load<align, mask>(src + 2 * A, tails[2]);
      __m512i p0 = _mm512_permutex2var_epi64(s0, K64_PRMT_P0, s1);
      __m512i p1 = _mm512_permutex2var_epi64(s0, K64_PRMT_P2, s1);
      __m512i p2 = _mm512_permutex2var_epi64(s1, K64_PRMT_P2, s2);
      Store<align, mask>(dst + 0 * A, _mm512_or_si512(_mm512_shuffle_epi8(s0, K8_SHFL_0S0), _mm512_shuffle_epi8(p0, K8_SHFL_0P0)), tails[0]);
      Store<align, mask>(dst + 1 * A, _mm512_or_si512(_mm512_or_si512(_mm512_shuffle_epi8(s1, K8_SHFL_1S1),
          _mm512_shuffle_epi8(p1, K8_SHFL_1P1)), _mm512_shuffle_epi8(p2, K8_SHFL_1P2)), tails[1]);
      Store<align, mask>(dst + 2 * A, _mm512_or_si512(_mm512_shuffle_epi8(s2, K8_SHFL_2S2), _mm512_shuffle_epi8(p2, K8_SHFL_2P2)), tails[2]);
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
      __mmask64 tails[3];
      for (size_t i = 0; i < 3; ++i)
          tails[i] = TailMask64(size - aligned - A * i);

      for (size_t row = 0; row < height; ++row)
      {
          size_t i = 0;
          for (; i < aligned; i += A3)
              BgrToRgb<align, false>(bgr + i, rgb + i, tails);
          if (i < size)
              BgrToRgb<align, true>(bgr + i, rgb + i, tails);
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
  //{{{  BgrToBgra
  //{{{
  const __m512i K8_SHUFFLE_BGR_TO_BGRA = SIMD_MM512_SETR_EPI8(
      0x0, 0x1, 0x2, -1, 0x3, 0x4, 0x5, -1, 0x6, 0x7, 0x8, -1, 0x9, 0xA, 0xB, -1,
      0x0, 0x1, 0x2, -1, 0x3, 0x4, 0x5, -1, 0x6, 0x7, 0x8, -1, 0x9, 0xA, 0xB, -1,
      0x0, 0x1, 0x2, -1, 0x3, 0x4, 0x5, -1, 0x6, 0x7, 0x8, -1, 0x9, 0xA, 0xB, -1,
      0x0, 0x1, 0x2, -1, 0x3, 0x4, 0x5, -1, 0x6, 0x7, 0x8, -1, 0x9, 0xA, 0xB, -1);
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgrToBgra(const uint8_t * bgr, uint8_t * bgra, const __m512i & alpha, const __mmask64 * ms)
  {
      __m512i bgr0 = Load<align, mask>(bgr + 0 * A, ms[0]);
      __m512i bgr1 = Load<align, mask>(bgr + 1 * A, ms[1]);
      __m512i bgr2 = Load<align, mask>(bgr + 2 * A, ms[2]);

      const __m512i bgra0 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_0, bgr0);
      const __m512i bgra1 = _mm512_permutex2var_epi32(bgr0, K32_PERMUTE_BGR_TO_BGRA_1, bgr1);
      const __m512i bgra2 = _mm512_permutex2var_epi32(bgr1, K32_PERMUTE_BGR_TO_BGRA_2, bgr2);
      const __m512i bgra3 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_3, bgr2);

      Store<align, mask>(bgra + 0 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra0, K8_SHUFFLE_BGR_TO_BGRA)), ms[3]);
      Store<align, mask>(bgra + 1 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra1, K8_SHUFFLE_BGR_TO_BGRA)), ms[4]);
      Store<align, mask>(bgra + 2 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra2, K8_SHUFFLE_BGR_TO_BGRA)), ms[5]);
      Store<align, mask>(bgra + 3 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra3, K8_SHUFFLE_BGR_TO_BGRA)), ms[6]);
  }
  //}}}
  //{{{
  template <bool align> void BgrToBgra(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
  {
      if (align)
          assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride));

      size_t alignedWidth = AlignLo(width, A);
      __mmask64 tailMasks[7];
      for (size_t c = 0; c < 3; ++c)
          tailMasks[c] = TailMask64((width - alignedWidth) * 3 - A*c);
      for (size_t c = 0; c < 4; ++c)
          tailMasks[3 + c] = TailMask64((width - alignedWidth) * 4 - A*c);
      __m512i _alpha = _mm512_set1_epi32(alpha * 0x1000000);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
            BgrToBgra<align, false>(bgr + 3 * col, bgra + 4 * col, _alpha, tailMasks);
          if (col < width)
            BgrToBgra<align, true>(bgr + 3 * col, bgra + 4 * col, _alpha, tailMasks);
          bgr += bgrStride;
          bgra += bgraStride;
      }
  }
  //}}}

  //{{{
  void BgrToBgra (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
  {
      if (Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride))
          BgrToBgra<true>(bgr, width, height, bgrStride, bgra, bgraStride, alpha);
      else
          BgrToBgra<false>(bgr, width, height, bgrStride, bgra, bgraStride, alpha);
  }
  //}}}

  //}}}

  //{{{  Bgr48pToBgra32
  //{{{
  template <bool align, bool mask> SIMD_INLINE void Bgr48pToBgra32(
      const uint8_t * blue, const uint8_t * green, const uint8_t * red, uint8_t * bgra, __m512i alpha, const __mmask64 * ms)
  {
      __m512i _blue = Load<align, true>(blue, ms[0]);
      __m512i _green = Load<align, true>(green, ms[0]);
      __m512i _red = Load<align, true>(red, ms[0]);

      __m512i bg = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, _mm512_or_si512(_blue, _mm512_slli_epi16(_green, 8)));
      __m512i ra = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, _mm512_or_si512(_red, alpha));

      Store<align, mask>(bgra + 0, _mm512_unpacklo_epi16(bg, ra), ms[1]);
      Store<align, mask>(bgra + A, _mm512_unpackhi_epi16(bg, ra), ms[2]);
  }
  //}}}
  //{{{
  template <bool align> void Bgr48pToBgra32(const uint8_t * blue, size_t blueStride, size_t width, size_t height,
      const uint8_t * green, size_t greenStride, const uint8_t * red, size_t redStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
  {
      if (align)
      {
          assert(Aligned(blue) && Aligned(blueStride));
          assert(Aligned(green) && Aligned(greenStride));
          assert(Aligned(red) && Aligned(redStride));
          assert(Aligned(bgra) && Aligned(bgraStride));
      }

      width *= 2;
      size_t alignedWidth = AlignLo(width, A);
      __mmask64 bodyMask = 0x5555555555555555;
      __mmask64 tailMasks[3];
      tailMasks[0] = TailMask64(width - alignedWidth)&bodyMask;
      for (size_t c = 0; c < 2; ++c)
          tailMasks[1 + c] = TailMask64((width - alignedWidth) * 2 - A*c);
      __m512i _alpha = _mm512_set1_epi16(alpha * 0x100);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              Bgr48pToBgra32<align, false>(blue + col, green + col, red + col, bgra + col * 2, _alpha, &bodyMask);
          if (col < width)
              Bgr48pToBgra32<align, true>(blue + col, green + col, red + col, bgra + col * 2, _alpha, tailMasks);
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
  //{{{  RgbToBgra
  //{{{
  const __m512i K8_SHUFFLE_RGB_TO_BGRA = SIMD_MM512_SETR_EPI8(
      0x2, 0x1, 0x0, -1, 0x5, 0x4, 0x3, -1, 0x8, 0x7, 0x6, -1, 0xB, 0xA, 0x9, -1,
      0x2, 0x1, 0x0, -1, 0x5, 0x4, 0x3, -1, 0x8, 0x7, 0x6, -1, 0xB, 0xA, 0x9, -1,
      0x2, 0x1, 0x0, -1, 0x5, 0x4, 0x3, -1, 0x8, 0x7, 0x6, -1, 0xB, 0xA, 0x9, -1,
      0x2, 0x1, 0x0, -1, 0x5, 0x4, 0x3, -1, 0x8, 0x7, 0x6, -1, 0xB, 0xA, 0x9, -1);
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void RgbToBgra(const uint8_t* rgb, uint8_t* bgra, const __m512i& alpha, const __mmask64* ms)
  {
      __m512i rgb0 = Load<align, mask>(rgb + 0 * A, ms[0]);
      __m512i rgb1 = Load<align, mask>(rgb + 1 * A, ms[1]);
      __m512i rgb2 = Load<align, mask>(rgb + 2 * A, ms[2]);

      const __m512i bgra0 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_0, rgb0);
      const __m512i bgra1 = _mm512_permutex2var_epi32(rgb0, K32_PERMUTE_BGR_TO_BGRA_1, rgb1);
      const __m512i bgra2 = _mm512_permutex2var_epi32(rgb1, K32_PERMUTE_BGR_TO_BGRA_2, rgb2);
      const __m512i bgra3 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_3, rgb2);

      Store<align, mask>(bgra + 0 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra0, K8_SHUFFLE_RGB_TO_BGRA)), ms[3]);
      Store<align, mask>(bgra + 1 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra1, K8_SHUFFLE_RGB_TO_BGRA)), ms[4]);
      Store<align, mask>(bgra + 2 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra2, K8_SHUFFLE_RGB_TO_BGRA)), ms[5]);
      Store<align, mask>(bgra + 3 * A, _mm512_or_si512(alpha, _mm512_shuffle_epi8(bgra3, K8_SHUFFLE_RGB_TO_BGRA)), ms[6]);
  }
  //}}}
  //{{{
  template <bool align> void RgbToBgra(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      if (align)
          assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride));

      size_t alignedWidth = AlignLo(width, A);
      __mmask64 tailMasks[7];
      for (size_t c = 0; c < 3; ++c)
          tailMasks[c] = TailMask64((width - alignedWidth) * 3 - A * c);
      for (size_t c = 0; c < 4; ++c)
          tailMasks[3 + c] = TailMask64((width - alignedWidth) * 4 - A * c);
      __m512i _alpha = _mm512_set1_epi32(alpha * 0x1000000);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              RgbToBgra<align, false>(rgb + 3 * col, bgra + 4 * col, _alpha, tailMasks);
          if (col < width)
              RgbToBgra<align, true>(rgb + 3 * col, bgra + 4 * col, _alpha, tailMasks);
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

  //{{{  BgrToGray
  //{{{
  SIMD_INLINE __m512i PermutedBgrToGray32(__m512i permutedBgr)
  {
      const __m512i b0r0 = _mm512_shuffle_epi8(permutedBgr, K8_SUFFLE_BGR_TO_B0R0);
      const __m512i g000 = _mm512_shuffle_epi8(permutedBgr, K8_SUFFLE_BGR_TO_G000);
      const __m512i weightedSum = _mm512_add_epi32(_mm512_madd_epi16(g000, K16_GREEN_0000), _mm512_madd_epi16(b0r0, K16_BLUE_RED));
      return _mm512_srli_epi32(_mm512_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void BgrToGray(const uint8_t * bgr, uint8_t * gray, const __mmask64 ms[4])
  {
      const __m512i bgr0 = Load<align, mask>(bgr + 0 * A, ms[0]);
      const __m512i bgr1 = Load<align, mask>(bgr + 1 * A, ms[1]);
      const __m512i bgr2 = Load<align, mask>(bgr + 2 * A, ms[2]);

      const __m512i permutedBgr0 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_0, bgr0);
      const __m512i permutedBgr1 = _mm512_permutex2var_epi32(bgr0, K32_PERMUTE_BGR_TO_BGRA_1, bgr1);
      const __m512i permutedBgr2 = _mm512_permutex2var_epi32(bgr1, K32_PERMUTE_BGR_TO_BGRA_2, bgr2);
      const __m512i permutedBgr3 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_3, bgr2);

      __m512i gray0 = PermutedBgrToGray32(permutedBgr0);
      __m512i gray1 = PermutedBgrToGray32(permutedBgr1);
      __m512i gray2 = PermutedBgrToGray32(permutedBgr2);
      __m512i gray3 = PermutedBgrToGray32(permutedBgr3);

      __m512i gray01 = _mm512_packs_epi32(gray0, gray1);
      __m512i gray23 = _mm512_packs_epi32(gray2, gray3);
      __m512i gray0123 = _mm512_packus_epi16(gray01, gray23);
      Store<align, mask>(gray, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, gray0123), ms[3]);
  }
  //}}}
  //{{{
  template <bool align> void BgrToGray(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * gray, size_t grayStride)
  {
      if (align)
          assert(Aligned(gray) && Aligned(grayStride) && Aligned(bgr) && Aligned(bgrStride));

      size_t alignedWidth = AlignLo(width, A);
      __mmask64 tailMasks[4];
      for (size_t c = 0; c < 3; ++c)
          tailMasks[c] = TailMask64((width - alignedWidth) * 3 - A*c);
      tailMasks[3] = TailMask64(width - alignedWidth);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              BgrToGray<align, false>(bgr + col * 3, gray + col, tailMasks);
          if (col < width)
              BgrToGray<align, true>(bgr + col * 3, gray + col, tailMasks);
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
  //{{{  RgbToGray
  //{{{
  SIMD_INLINE __m512i PermutedRgbToGray32(__m512i permutedRgb)
  {
      const __m512i r0b0 = _mm512_shuffle_epi8(permutedRgb, K8_SUFFLE_BGR_TO_B0R0);
      const __m512i g000 = _mm512_shuffle_epi8(permutedRgb, K8_SUFFLE_BGR_TO_G000);
      const __m512i weightedSum = _mm512_add_epi32(_mm512_madd_epi16(g000, K16_GREEN_0000), _mm512_madd_epi16(r0b0, K16_RED_BLUE));
      return _mm512_srli_epi32(_mm512_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
  }
  //}}}
  //{{{
  template <bool align, bool mask> SIMD_INLINE void RgbToGray(const uint8_t* rgb, uint8_t* gray, const __mmask64 ms[4])
  {
      const __m512i rgb0 = Load<align, mask>(rgb + 0 * A, ms[0]);
      const __m512i rgb1 = Load<align, mask>(rgb + 1 * A, ms[1]);
      const __m512i rgb2 = Load<align, mask>(rgb + 2 * A, ms[2]);

      const __m512i permutedRgb0 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_0, rgb0);
      const __m512i permutedRgb1 = _mm512_permutex2var_epi32(rgb0, K32_PERMUTE_BGR_TO_BGRA_1, rgb1);
      const __m512i permutedRgb2 = _mm512_permutex2var_epi32(rgb1, K32_PERMUTE_BGR_TO_BGRA_2, rgb2);
      const __m512i permutedRgb3 = _mm512_permutexvar_epi32(K32_PERMUTE_BGR_TO_BGRA_3, rgb2);

      __m512i gray0 = PermutedRgbToGray32(permutedRgb0);
      __m512i gray1 = PermutedRgbToGray32(permutedRgb1);
      __m512i gray2 = PermutedRgbToGray32(permutedRgb2);
      __m512i gray3 = PermutedRgbToGray32(permutedRgb3);

      __m512i gray01 = _mm512_packs_epi32(gray0, gray1);
      __m512i gray23 = _mm512_packs_epi32(gray2, gray3);
      __m512i gray0123 = _mm512_packus_epi16(gray01, gray23);
      Store<align, mask>(gray, _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, gray0123), ms[3]);
  }
  //}}}
  //{{{
  template <bool align> void RgbToGray(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* gray, size_t grayStride)
  {
      if (align)
          assert(Aligned(gray) && Aligned(grayStride) && Aligned(rgb) && Aligned(rgbStride));

      size_t alignedWidth = AlignLo(width, A);
      __mmask64 tailMasks[4];
      for (size_t c = 0; c < 3; ++c)
          tailMasks[c] = TailMask64((width - alignedWidth) * 3 - A * c);
      tailMasks[3] = TailMask64(width - alignedWidth);
      for (size_t row = 0; row < height; ++row)
      {
          size_t col = 0;
          for (; col < alignedWidth; col += A)
              RgbToGray<align, false>(rgb + col * 3, gray + col, tailMasks);
          if (col < width)
              RgbToGray<align, true>(rgb + col * 3, gray + col, tailMasks);
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
  }
