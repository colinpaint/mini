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

namespace Simd {
  #ifdef SIMD_SSE41_ENABLE
    namespace Sse41 {
      //{{{  to YUV
      //{{{
      template <bool align> SIMD_INLINE void LoadBgr(const __m128i * p, __m128i & blue, __m128i & green, __m128i & red)
      {
          __m128i bgr[3];
          bgr[0] = Load<align>(p + 0);
          bgr[1] = Load<align>(p + 1);
          bgr[2] = Load<align>(p + 2);
          blue = BgrToBlue(bgr);
          green = BgrToGreen(bgr);
          red = BgrToRed(bgr);
      }
      //}}}
      //{{{
      SIMD_INLINE __m128i Average16(const __m128i & s0, const __m128i & s1)
      {
          return _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(_mm_maddubs_epi16(s0, K8_01), _mm_maddubs_epi16(s1, K8_01)), K16_0002), 2);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgrToYuv420p(const uint8_t * bgr0, size_t bgrStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v)
      {
          const uint8_t * bgr1 = bgr0 + bgrStride;
          uint8_t * y1 = y0 + yStride;

          __m128i blue[2][2], green[2][2], red[2][2];

          LoadBgr<align>((__m128i*)bgr0 + 0, blue[0][0], green[0][0], red[0][0]);
          Store<align>((__m128i*)y0 + 0, BgrToY8(blue[0][0], green[0][0], red[0][0]));

          LoadBgr<align>((__m128i*)bgr0 + 3, blue[0][1], green[0][1], red[0][1]);
          Store<align>((__m128i*)y0 + 1, BgrToY8(blue[0][1], green[0][1], red[0][1]));

          LoadBgr<align>((__m128i*)bgr1 + 0, blue[1][0], green[1][0], red[1][0]);
          Store<align>((__m128i*)y1 + 0, BgrToY8(blue[1][0], green[1][0], red[1][0]));

          LoadBgr<align>((__m128i*)bgr1 + 3, blue[1][1], green[1][1], red[1][1]);
          Store<align>((__m128i*)y1 + 1, BgrToY8(blue[1][1], green[1][1], red[1][1]));

          blue[0][0] = Average16(blue[0][0], blue[1][0]);
          blue[0][1] = Average16(blue[0][1], blue[1][1]);
          green[0][0] = Average16(green[0][0], green[1][0]);
          green[0][1] = Average16(green[0][1], green[1][1]);
          red[0][0] = Average16(red[0][0], red[1][0]);
          red[0][1] = Average16(red[0][1], red[1][1]);

          Store<align>((__m128i*)u, _mm_packus_epi16(BgrToU16(blue[0][0], green[0][0], red[0][0]), BgrToU16(blue[0][1], green[0][1], red[0][1])));
          Store<align>((__m128i*)v, _mm_packus_epi16(BgrToV16(blue[0][0], green[0][0], red[0][0]), BgrToV16(blue[0][1], green[0][1], red[0][1])));
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

      //{{{
      SIMD_INLINE void Average16(__m128i & a)
      {
          a = _mm_srli_epi16(_mm_add_epi16(_mm_maddubs_epi16(a, K8_01), K16_0001), 1);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgrToYuv422p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v)
      {
          __m128i blue[2], green[2], red[2];

          LoadBgr<align>((__m128i*)bgr + 0, blue[0], green[0], red[0]);
          Store<align>((__m128i*)y + 0, BgrToY8(blue[0], green[0], red[0]));

          LoadBgr<align>((__m128i*)bgr + 3, blue[1], green[1], red[1]);
          Store<align>((__m128i*)y + 1, BgrToY8(blue[1], green[1], red[1]));

          Average16(blue[0]);
          Average16(blue[1]);
          Average16(green[0]);
          Average16(green[1]);
          Average16(red[0]);
          Average16(red[1]);

          Store<align>((__m128i*)u, _mm_packus_epi16(BgrToU16(blue[0], green[0], red[0]), BgrToU16(blue[1], green[1], red[1])));
          Store<align>((__m128i*)v, _mm_packus_epi16(BgrToV16(blue[0], green[0], red[0]), BgrToV16(blue[1], green[1], red[1])));
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

      //{{{
      template <bool align> SIMD_INLINE void BgrToYuv444p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v)
      {
          __m128i blue, green, red;
          LoadBgr<align>((__m128i*)bgr, blue, green, red);
          Store<align>((__m128i*)y, BgrToY8(blue, green, red));
          Store<align>((__m128i*)u, BgrToU8(blue, green, red));
          Store<align>((__m128i*)v, BgrToV8(blue, green, red));
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

      const __m128i K8_SHUFFLE_BGRA_TO_B0R0 = SIMD_MM_SETR_EPI8(0x0, -1, 0x2, -1, 0x4, -1, 0x6, -1, 0x8, -1, 0xA, -1, 0xC, -1, 0xE, -1);
      const __m128i K8_SHUFFLE_BGRA_TO_G000 = SIMD_MM_SETR_EPI8(0x1, -1, -1, -1, 0x5, -1, -1, -1, 0x9, -1, -1, -1, 0xD, -1, -1, -1);
      //{{{
      template <bool align> SIMD_INLINE void LoadPreparedBgra16(const __m128i * bgra, __m128i & b16_r16, __m128i & g16_1)
      {
          __m128i _bgra = Load<align>(bgra);
          b16_r16 = _mm_shuffle_epi8(_bgra, K8_SHUFFLE_BGRA_TO_B0R0);
          g16_1 = _mm_or_si128(_mm_shuffle_epi8(_bgra, K8_SHUFFLE_BGRA_TO_G000), K32_00010000);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m128i LoadAndConvertY16(const __m128i * bgra, __m128i & b16_r16, __m128i & g16_1)
      {
          __m128i _b16_r16[2], _g16_1[2];
          LoadPreparedBgra16<align>(bgra + 0, _b16_r16[0], _g16_1[0]);
          LoadPreparedBgra16<align>(bgra + 1, _b16_r16[1], _g16_1[1]);
          b16_r16 = _mm_hadd_epi32(_b16_r16[0], _b16_r16[1]);
          g16_1 = _mm_hadd_epi32(_g16_1[0], _g16_1[1]);
          return SaturateI16ToU8(_mm_add_epi16(K16_Y_ADJUST, _mm_packs_epi32(BgrToY32(_b16_r16[0], _g16_1[0]), BgrToY32(_b16_r16[1], _g16_1[1]))));
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m128i LoadAndConvertY8(const __m128i * bgra, __m128i b16_r16[2], __m128i g16_1[2])
      {
          return _mm_packus_epi16(LoadAndConvertY16<align>(bgra + 0, b16_r16[0], g16_1[0]), LoadAndConvertY16<align>(bgra + 2, b16_r16[1], g16_1[1]));
      }
      //}}}
      //{{{
      SIMD_INLINE void Average16(__m128i & a, const __m128i & b)
      {
          a = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(a, b), K16_0002), 2);
      }
      //}}}
      //{{{
      SIMD_INLINE __m128i ConvertU16(__m128i b16_r16[2], __m128i g16_1[2])
      {
          return SaturateI16ToU8(_mm_add_epi16(K16_UV_ADJUST, _mm_packs_epi32(BgrToU32(b16_r16[0], g16_1[0]), BgrToU32(b16_r16[1], g16_1[1]))));
      }
      //}}}
      //{{{
      SIMD_INLINE __m128i ConvertV16(__m128i b16_r16[2], __m128i g16_1[2])
      {
          return SaturateI16ToU8(_mm_add_epi16(K16_UV_ADJUST, _mm_packs_epi32(BgrToV32(b16_r16[0], g16_1[0]), BgrToV32(b16_r16[1], g16_1[1]))));
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToYuv420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v)
      {
          const uint8_t * bgra1 = bgra0 + bgraStride;
          uint8_t * y1 = y0 + yStride;

          __m128i _b16_r16[2][2][2], _g16_1[2][2][2];
          Store<align>((__m128i*)y0 + 0, LoadAndConvertY8<align>((__m128i*)bgra0 + 0, _b16_r16[0][0], _g16_1[0][0]));
          Store<align>((__m128i*)y0 + 1, LoadAndConvertY8<align>((__m128i*)bgra0 + 4, _b16_r16[0][1], _g16_1[0][1]));
          Store<align>((__m128i*)y1 + 0, LoadAndConvertY8<align>((__m128i*)bgra1 + 0, _b16_r16[1][0], _g16_1[1][0]));
          Store<align>((__m128i*)y1 + 1, LoadAndConvertY8<align>((__m128i*)bgra1 + 4, _b16_r16[1][1], _g16_1[1][1]));

          Average16(_b16_r16[0][0][0], _b16_r16[1][0][0]);
          Average16(_b16_r16[0][0][1], _b16_r16[1][0][1]);
          Average16(_b16_r16[0][1][0], _b16_r16[1][1][0]);
          Average16(_b16_r16[0][1][1], _b16_r16[1][1][1]);

          Average16(_g16_1[0][0][0], _g16_1[1][0][0]);
          Average16(_g16_1[0][0][1], _g16_1[1][0][1]);
          Average16(_g16_1[0][1][0], _g16_1[1][1][0]);
          Average16(_g16_1[0][1][1], _g16_1[1][1][1]);

          Store<align>((__m128i*)u, _mm_packus_epi16(ConvertU16(_b16_r16[0][0], _g16_1[0][0]), ConvertU16(_b16_r16[0][1], _g16_1[0][1])));
          Store<align>((__m128i*)v, _mm_packus_epi16(ConvertV16(_b16_r16[0][0], _g16_1[0][0]), ConvertV16(_b16_r16[0][1], _g16_1[0][1])));
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
      SIMD_INLINE void Average16(__m128i a[2][2])
      {
          a[0][0] = _mm_srli_epi16(_mm_add_epi16(a[0][0], K16_0001), 1);
          a[0][1] = _mm_srli_epi16(_mm_add_epi16(a[0][1], K16_0001), 1);
          a[1][0] = _mm_srli_epi16(_mm_add_epi16(a[1][0], K16_0001), 1);
          a[1][1] = _mm_srli_epi16(_mm_add_epi16(a[1][1], K16_0001), 1);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToYuv422p(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v)
      {
          __m128i _b16_r16[2][2], _g16_1[2][2];
          Store<align>((__m128i*)y + 0, LoadAndConvertY8<align>((__m128i*)bgra + 0, _b16_r16[0], _g16_1[0]));
          Store<align>((__m128i*)y + 1, LoadAndConvertY8<align>((__m128i*)bgra + 4, _b16_r16[1], _g16_1[1]));

          Average16(_b16_r16);
          Average16(_g16_1);

          Store<align>((__m128i*)u, _mm_packus_epi16(ConvertU16(_b16_r16[0], _g16_1[0]), ConvertU16(_b16_r16[1], _g16_1[1])));
          Store<align>((__m128i*)v, _mm_packus_epi16(ConvertV16(_b16_r16[0], _g16_1[0]), ConvertV16(_b16_r16[1], _g16_1[1])));
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
      SIMD_INLINE __m128i ConvertY16(__m128i b16_r16[2], __m128i g16_1[2])
      {
          return SaturateI16ToU8(_mm_add_epi16(K16_Y_ADJUST, _mm_packs_epi32(BgrToY32(b16_r16[0], g16_1[0]), BgrToY32(b16_r16[1], g16_1[1]))));
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToYuv444p(const uint8_t* bgra, uint8_t* y, uint8_t* u, uint8_t* v)
      {
          __m128i _b16_r16[2][2], _g16_1[2][2];
          LoadPreparedBgra16<align>((__m128i*)bgra + 0, _b16_r16[0][0], _g16_1[0][0]);
          LoadPreparedBgra16<align>((__m128i*)bgra + 1, _b16_r16[0][1], _g16_1[0][1]);
          LoadPreparedBgra16<align>((__m128i*)bgra + 2, _b16_r16[1][0], _g16_1[1][0]);
          LoadPreparedBgra16<align>((__m128i*)bgra + 3, _b16_r16[1][1], _g16_1[1][1]);

          Store<align>((__m128i*)y, _mm_packus_epi16(ConvertY16(_b16_r16[0], _g16_1[0]), ConvertY16(_b16_r16[1], _g16_1[1])));
          Store<align>((__m128i*)u, _mm_packus_epi16(ConvertU16(_b16_r16[0], _g16_1[0]), ConvertU16(_b16_r16[1], _g16_1[1])));
          Store<align>((__m128i*)v, _mm_packus_epi16(ConvertV16(_b16_r16[0], _g16_1[0]), ConvertV16(_b16_r16[1], _g16_1[1])));
      }
      //}}}
      //{{{
      template <bool align> void BgraToYuv444p(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* y, size_t yStride,
          uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
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
      void BgraToYuv444p(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* y, size_t yStride,
          uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
      {
          if (Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride)
              && Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
              BgraToYuv444p<true>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
          else
              BgraToYuv444p<false>(bgra, width, height, bgraStride, y, yStride, u, uStride, v, vStride);
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE void LoadPreparedBgra16(const __m128i * bgra, __m128i & b16_r16, __m128i & g16_1, __m128i & a32)
      {
          __m128i _bgra = Load<align>(bgra);
          b16_r16 = _mm_shuffle_epi8(_bgra, K8_SHUFFLE_BGRA_TO_B0R0);
          g16_1 = _mm_or_si128(_mm_shuffle_epi8(_bgra, K8_SHUFFLE_BGRA_TO_G000), K32_00010000);
          a32 = _mm_and_si128(_mm_srli_si128(_bgra, 3), K32_000000FF);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void LoadAndConvertYA16(const __m128i * bgra, __m128i & b16_r16, __m128i & g16_1, __m128i & y16, __m128i & a16)
      {
          __m128i _b16_r16[2], _g16_1[2], a32[2];
          LoadPreparedBgra16<align>(bgra + 0, _b16_r16[0], _g16_1[0], a32[0]);
          LoadPreparedBgra16<align>(bgra + 1, _b16_r16[1], _g16_1[1], a32[1]);
          b16_r16 = _mm_hadd_epi32(_b16_r16[0], _b16_r16[1]);
          g16_1 = _mm_hadd_epi32(_g16_1[0], _g16_1[1]);
          y16 = SaturateI16ToU8(_mm_add_epi16(K16_Y_ADJUST, _mm_packs_epi32(BgrToY32(_b16_r16[0], _g16_1[0]), BgrToY32(_b16_r16[1], _g16_1[1]))));
          a16 = _mm_packs_epi32(a32[0], a32[1]);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void LoadAndStoreYA(const __m128i * bgra, __m128i b16_r16[2], __m128i g16_1[2], __m128i * y, __m128i * a)
      {
          __m128i y16[2], a16[2];
          LoadAndConvertYA16<align>(bgra + 0, b16_r16[0], g16_1[0], y16[0], a16[0]);
          LoadAndConvertYA16<align>(bgra + 2, b16_r16[1], g16_1[1], y16[1], a16[1]);
          Store<align>(y, _mm_packus_epi16(y16[0], y16[1]));
          Store<align>(a, _mm_packus_epi16(a16[0], a16[1]));
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToYuva420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v, uint8_t * a0, size_t aStride)
      {
          const uint8_t * bgra1 = bgra0 + bgraStride;
          uint8_t * y1 = y0 + yStride;
          uint8_t * a1 = a0 + aStride;

          __m128i _b16_r16[2][2][2], _g16_1[2][2][2];
          LoadAndStoreYA<align>((__m128i*)bgra0 + 0, _b16_r16[0][0], _g16_1[0][0], (__m128i*)y0 + 0, (__m128i*)a0 + 0);
          LoadAndStoreYA<align>((__m128i*)bgra0 + 4, _b16_r16[0][1], _g16_1[0][1], (__m128i*)y0 + 1, (__m128i*)a0 + 1);
          LoadAndStoreYA<align>((__m128i*)bgra1 + 0, _b16_r16[1][0], _g16_1[1][0], (__m128i*)y1 + 0, (__m128i*)a1 + 0);
          LoadAndStoreYA<align>((__m128i*)bgra1 + 4, _b16_r16[1][1], _g16_1[1][1], (__m128i*)y1 + 1, (__m128i*)a1 + 1);

          Average16(_b16_r16[0][0][0], _b16_r16[1][0][0]);
          Average16(_b16_r16[0][0][1], _b16_r16[1][0][1]);
          Average16(_b16_r16[0][1][0], _b16_r16[1][1][0]);
          Average16(_b16_r16[0][1][1], _b16_r16[1][1][1]);

          Average16(_g16_1[0][0][0], _g16_1[1][0][0]);
          Average16(_g16_1[0][0][1], _g16_1[1][0][1]);
          Average16(_g16_1[0][1][0], _g16_1[1][1][0]);
          Average16(_g16_1[0][1][1], _g16_1[1][1][1]);

          Store<align>((__m128i*)u, _mm_packus_epi16(ConvertU16(_b16_r16[0][0], _g16_1[0][0]), ConvertU16(_b16_r16[0][1], _g16_1[0][1])));
          Store<align>((__m128i*)v, _mm_packus_epi16(ConvertV16(_b16_r16[0][0], _g16_1[0][0]), ConvertV16(_b16_r16[0][1], _g16_1[0][1])));
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
      //{{{  to RGB
      const __m128i K8_CVT_00 = SIMD_MM_SETR_EPI8(0x2, 0x1, 0x0, 0x5, 0x4, 0x3, 0x8, 0x7, 0x6, 0xB, 0xA, 0x9, 0xE, 0xD, 0xC, -1);
      const __m128i K8_CVT_01 = SIMD_MM_SETR_EPI8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x1);
      const __m128i K8_CVT_10 = SIMD_MM_SETR_EPI8(-1, 0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
      const __m128i K8_CVT_11 = SIMD_MM_SETR_EPI8(0x0, -1, 0x4, 0x3, 0x2, 0x7, 0x6, 0x5, 0xA, 0x9, 0x8, 0xD, 0xC, 0xB, -1, 0xF);
      const __m128i K8_CVT_12 = SIMD_MM_SETR_EPI8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x0, -1);
      const __m128i K8_CVT_21 = SIMD_MM_SETR_EPI8(0xE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
      const __m128i K8_CVT_22 = SIMD_MM_SETR_EPI8(-1, 0x3, 0x2, 0x1, 0x6, 0x5, 0x4, 0x9, 0x8, 0x7, 0xC, 0xB, 0xA, 0xF, 0xE, 0xD);
      //{{{
      template <bool align> SIMD_INLINE void BgrToRgb(const uint8_t * src, uint8_t * dst)
      {
          __m128i s0 = Load<align>((__m128i*)src + 0);
          __m128i s1 = Load<align>((__m128i*)src + 1);
          __m128i s2 = Load<align>((__m128i*)src + 2);
          Store<align>((__m128i*)dst + 0, _mm_or_si128(_mm_shuffle_epi8(s0, K8_CVT_00), _mm_shuffle_epi8(s1, K8_CVT_01)));
          Store<align>((__m128i*)dst + 1, _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(s0, K8_CVT_10), _mm_shuffle_epi8(s1, K8_CVT_11)), _mm_shuffle_epi8(s2, K8_CVT_12)));
          Store<align>((__m128i*)dst + 2, _mm_or_si128(_mm_shuffle_epi8(s1, K8_CVT_21), _mm_shuffle_epi8(s2, K8_CVT_22)));
      }
      //}}}
      //{{{
      template <bool align> void BgrToRgb(const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* rgb, size_t rgbStride)
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
      void BgrToRgb(const uint8_t* bgr, size_t width, size_t height, size_t bgrStride, uint8_t* rgb, size_t rgbStride)
      {
          if (Aligned(bgr) && Aligned(bgrStride) && Aligned(rgb) && Aligned(rgbStride))
              BgrToRgb<true>(bgr, width, height, bgrStride, rgb, rgbStride);
          else
              BgrToRgb<false>(bgr, width, height, bgrStride, rgb, rgbStride);
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE void BgraToBgrBody(const uint8_t * bgra, uint8_t * bgr, __m128i k[3][2])
      {
          Store<align>((__m128i*)(bgr + 0), _mm_shuffle_epi8(Load<align>((__m128i*)bgra + 0), k[0][0]));
          Store<false>((__m128i*)(bgr + 12), _mm_shuffle_epi8(Load<align>((__m128i*)bgra + 1), k[0][0]));
          Store<false>((__m128i*)(bgr + 24), _mm_shuffle_epi8(Load<align>((__m128i*)bgra + 2), k[0][0]));
          Store<false>((__m128i*)(bgr + 36), _mm_shuffle_epi8(Load<align>((__m128i*)bgra + 3), k[0][0]));
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void BgraToBgr(const uint8_t * bgra, uint8_t * bgr, __m128i k[3][2])
      {
          __m128i bgra0 = Load<align>((__m128i*)bgra + 0);
          __m128i bgra1 = Load<align>((__m128i*)bgra + 1);
          __m128i bgra2 = Load<align>((__m128i*)bgra + 2);
          __m128i bgra3 = Load<align>((__m128i*)bgra + 3);
          Store<align>((__m128i*)bgr + 0, _mm_or_si128(_mm_shuffle_epi8(bgra0, k[0][0]), _mm_shuffle_epi8(bgra1, k[0][1])));
          Store<align>((__m128i*)bgr + 1, _mm_or_si128(_mm_shuffle_epi8(bgra1, k[1][0]), _mm_shuffle_epi8(bgra2, k[1][1])));
          Store<align>((__m128i*)bgr + 2, _mm_or_si128(_mm_shuffle_epi8(bgra2, k[2][0]), _mm_shuffle_epi8(bgra3, k[2][1])));
      }
      //}}}
      //{{{
      template <bool align> void BgraToBgr(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * bgr, size_t bgrStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride));

          size_t widthA = AlignLo(width, A) - A;

          __m128i k[3][2];
          k[0][0] = _mm_setr_epi8(0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1);
          k[0][1] = _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x0, 0x1, 0x2, 0x4);
          k[1][0] = _mm_setr_epi8(0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1, -1, -1, -1, -1);
          k[1][1] = _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9);
          k[2][0] = _mm_setr_epi8(0xA, 0xC, 0xD, 0xE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
          k[2][1] = _mm_setr_epi8(-1, -1, -1, -1, 0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x8, 0x9, 0xA, 0xC, 0xD, 0xE);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < widthA; col += A)
                  BgraToBgrBody<align>(bgra + 4 * col, bgr + 3 * col, k);
              BgraToBgr<align>(bgra + 4 * widthA, bgr + 3 * widthA, k);
              if (widthA + A !=  width)
                  BgraToBgr<false>(bgra + 4 * (width - A), bgr + 3 * (width - A), k);
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

      //{{{
      template <bool align> void BgraToRgb(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgb, size_t rgbStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride));

          size_t widthA = AlignLo(width, A) - A;

          __m128i k[3][2];
          k[0][0] = _mm_setr_epi8(0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1);
          k[0][1] = _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x2, 0x1, 0x0, 0x6);
          k[1][0] = _mm_setr_epi8(0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1, -1, -1, -1, -1);
          k[1][1] = _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9);
          k[2][0] = _mm_setr_epi8(0x8, 0xE, 0xD, 0xC, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
          k[2][1] = _mm_setr_epi8(-1, -1, -1, -1, 0x2, 0x1, 0x0, 0x6, 0x5, 0x4, 0xA, 0x9, 0x8, 0xE, 0xD, 0xC);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < widthA; col += A)
                  BgraToBgrBody<align>(bgra + 4 * col, rgb + 3 * col, k);
              BgraToBgr<align>(bgra + 4 * widthA, rgb + 3 * widthA, k);
              if (widthA + A != width)
                  BgraToBgr<false>(bgra + 4 * (width - A), rgb + 3 * (width - A), k);
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

      const __m128i K8_BGRA_TO_RGBA = SIMD_MM_SETR_EPI8(0x2, 0x1, 0x0, 0x3, 0x6, 0x5, 0x4, 0x7, 0xA, 0x9, 0x8, 0xB, 0xE, 0xD, 0xC, 0xF);
      //{{{
      template <bool align> SIMD_INLINE void BgraToRgba(const uint8_t* bgra, uint8_t* rgba)
      {
          Store<align>((__m128i*)rgba, _mm_shuffle_epi8(Load<align>((__m128i*)bgra), K8_BGRA_TO_RGBA));
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
      //}}}
      //{{{  to Gray
      // !!!fiddling with dups !!!!
      const __m128i K16_BLUE_RED = SIMD_MM_SET2_EPI16(Base::BLUE_TO_GRAY_WEIGHT, Base::RED_TO_GRAY_WEIGHT);
      const __m128i K16_GREEN_0000 = SIMD_MM_SET2_EPI16(Base::GREEN_TO_GRAY_WEIGHT, 0x0000);
      const __m128i K32_ROUND_TERM = SIMD_MM_SET1_EPI32(Base::BGR_TO_GRAY_ROUND_TERM);
      //{{{
      SIMD_INLINE __m128i BgraToGray32_0000(__m128i bgra)
      {
          const __m128i g0a0 = _mm_and_si128(_mm_srli_si128(bgra, 1), K16_00FF);
          const __m128i b0r0 = _mm_and_si128(bgra, K16_00FF);
          const __m128i weightedSum = _mm_add_epi32(_mm_madd_epi16(g0a0, K16_GREEN_0000), _mm_madd_epi16(b0r0, K16_BLUE_RED));
          return _mm_srli_epi32(_mm_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }
      //}}}
      //{{{
      SIMD_INLINE __m128i BgraToGray(__m128i bgra[4])
      {
          const __m128i lo = _mm_packs_epi32(BgraToGray32_0000(bgra[0]), BgraToGray32_0000(bgra[1]));
          const __m128i hi = _mm_packs_epi32(BgraToGray32_0000(bgra[2]), BgraToGray32_0000(bgra[3]));
          return _mm_packus_epi16(lo, hi);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE void Load(const uint8_t* p, __m128i a[4])
      {
          a[0] = Load<align>((__m128i*)p + 0);
          a[1] = Load<align>((__m128i*)p + 1);
          a[2] = Load<align>((__m128i*)p + 2);
          a[3] = Load<align>((__m128i*)p + 3);
      }
      //}}}
      //{{{
      template <bool align> void BgraToGray(const uint8_t *bgra, size_t width, size_t height, size_t bgraStride, uint8_t *gray, size_t grayStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(gray) && Aligned(grayStride));

          size_t alignedWidth = AlignLo(width, A);
          __m128i a[4];
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
              {
                  Sse41::Load<align>(bgra + 4 * col, a);
                  Store<align>((__m128i*)(gray + col), BgraToGray(a));
              }
              if (alignedWidth != width)
              {
                  Sse41::Load<false>(bgra + 4 * (width - A), a);
                  Store<false>((__m128i*)(gray + width - A), BgraToGray(a));
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

      const __m128i K16_RED_BLUE = SIMD_MM_SET2_EPI16(Base::RED_TO_GRAY_WEIGHT, Base::BLUE_TO_GRAY_WEIGHT);
      //{{{
      SIMD_INLINE __m128i RgbaToGray32_0000(__m128i rgba)
      {
          const __m128i g0a0 = _mm_and_si128(_mm_srli_si128(rgba, 1), K16_00FF);
          const __m128i r0b0 = _mm_and_si128(rgba, K16_00FF);
          const __m128i weightedSum = _mm_add_epi32(_mm_madd_epi16(g0a0, K16_GREEN_0000), _mm_madd_epi16(r0b0, K16_RED_BLUE));
          return _mm_srli_epi32(_mm_add_epi32(weightedSum, K32_ROUND_TERM), Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }

      //}}}
      //{{{
      SIMD_INLINE __m128i RgbaToGray(__m128i rgba[4])
      {
          const __m128i lo = _mm_packs_epi32(RgbaToGray32_0000(rgba[0]), RgbaToGray32_0000(rgba[1]));
          const __m128i hi = _mm_packs_epi32(RgbaToGray32_0000(rgba[2]), RgbaToGray32_0000(rgba[3]));
          return _mm_packus_epi16(lo, hi);
      }
      //}}}
      //{{{
      template <bool align> void RgbaToGray(const uint8_t* rgba, size_t width, size_t height, size_t rgbaStride, uint8_t* gray, size_t grayStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(rgba) && Aligned(rgbaStride) && Aligned(gray) && Aligned(grayStride));

          size_t alignedWidth = AlignLo(width, A);
          __m128i a[4];
          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
              {
                  Load<align>(rgba + 4 * col, a);
                  Store<align>((__m128i*)(gray + col), RgbaToGray(a));
              }
              if (alignedWidth != width)
              {
                  Load<false>(rgba + 4 * (width - A), a);
                  Store<false>((__m128i*)(gray + width - A), RgbaToGray(a));
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

      const __m128i K16_GREEN_ROUND = SIMD_MM_SET2_EPI16(Base::GREEN_TO_GRAY_WEIGHT, Base::BGR_TO_GRAY_ROUND_TERM);
      //{{{
      SIMD_INLINE __m128i BgraToGray32(__m128i bgra)
      {
          const __m128i g0a0 = _mm_and_si128(_mm_srli_si128(bgra, 1), K16_00FF);
          const __m128i b0r0 = _mm_and_si128(bgra, K16_00FF);
          const __m128i weightedSum = _mm_add_epi32(_mm_madd_epi16(g0a0, K16_GREEN_ROUND), _mm_madd_epi16(b0r0, K16_BLUE_RED));
          return _mm_srli_epi32(weightedSum, Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m128i BgrToGray(const uint8_t * bgr, __m128i shuffle)
      {
          __m128i bgra[4];
          bgra[0] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(Load<align>((__m128i*)(bgr + 0)), shuffle));
          bgra[1] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(Load<false>((__m128i*)(bgr + 12)), shuffle));
          bgra[2] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(Load<false>((__m128i*)(bgr + 24)), shuffle));
          bgra[3] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(_mm_srli_si128(Load<align>((__m128i*)(bgr + 32)), 4), shuffle));
          return BgraToGray(bgra);
      }
      //}}}
      //{{{
      template <bool align> void BgrToGray(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * gray, size_t grayStride)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(gray) && Aligned(grayStride) && Aligned(bgr) && Aligned(bgrStride));

          size_t alignedWidth = AlignLo(width, A);

          __m128i _shuffle = _mm_setr_epi8(0x0, 0x1, 0x2, -1, 0x3, 0x4, 0x5, -1, 0x6, 0x7, 0x8, -1, 0x9, 0xA, 0xB, -1);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  Store<align>((__m128i*)(gray + col), BgrToGray<align>(bgr + 3 * col, _shuffle));
              if (width != alignedWidth)
                  Store<false>((__m128i*)(gray + width - A), BgrToGray<false>(bgr + 3 * (width - A), _shuffle));
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

      //{{{
      SIMD_INLINE __m128i RgbaToGray32(__m128i rgba)
      {
          const __m128i g0a0 = _mm_and_si128(_mm_srli_si128(rgba, 1), K16_00FF);
          const __m128i r0b0 = _mm_and_si128(rgba, K16_00FF);
          const __m128i weightedSum = _mm_add_epi32(_mm_madd_epi16(g0a0, K16_GREEN_ROUND), _mm_madd_epi16(r0b0, K16_RED_BLUE));
          return _mm_srli_epi32(weightedSum, Base::BGR_TO_GRAY_AVERAGING_SHIFT);
      }
      //}}}
      //{{{
      template <bool align> SIMD_INLINE __m128i RgbToGray(const uint8_t* rgb, __m128i shuffle)
      {
          __m128i rgba[4];
          rgba[0] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(Load<align>((__m128i*)(rgb + 0)), shuffle));
          rgba[1] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(Load<false>((__m128i*)(rgb + 12)), shuffle));
          rgba[2] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(Load<false>((__m128i*)(rgb + 24)), shuffle));
          rgba[3] = _mm_or_si128(K32_01000000, _mm_shuffle_epi8(_mm_srli_si128(Load<align>((__m128i*)(rgb + 32)), 4), shuffle));
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

          __m128i _shuffle = _mm_setr_epi8(0x0, 0x1, 0x2, -1, 0x3, 0x4, 0x5, -1, 0x6, 0x7, 0x8, -1, 0x9, 0xA, 0xB, -1);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  Store<align>((__m128i*)(gray + col), RgbToGray<align>(rgb + 3 * col, _shuffle));
              if (width != alignedWidth)
                  Store<false>((__m128i*)(gray + width - A), RgbToGray<false>(rgb + 3 * (width - A), _shuffle));
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
      //{{{  to BGRA
      //{{{
      template <bool align> SIMD_INLINE void Bgr48pToBgra32(uint8_t* bgra,
          const uint8_t* blue, const uint8_t* green, const uint8_t* red, size_t offset, __m128i alpha)
      {
          __m128i _blue = _mm_and_si128(Load<align>((__m128i*)(blue + offset)), K16_00FF);
          __m128i _green = _mm_and_si128(Load<align>((__m128i*)(green + offset)), K16_00FF);
          __m128i _red = _mm_and_si128(Load<align>((__m128i*)(red + offset)), K16_00FF);

          __m128i bg = _mm_or_si128(_blue, _mm_slli_si128(_green, 1));
          __m128i ra = _mm_or_si128(_red, alpha);

          Store<align>((__m128i*)bgra + 0, _mm_unpacklo_epi16(bg, ra));
          Store<align>((__m128i*)bgra + 1, _mm_unpackhi_epi16(bg, ra));
      }
      //}}}
      //{{{
      template <bool align> void Bgr48pToBgra32(const uint8_t* blue, size_t blueStride, size_t width, size_t height,
          const uint8_t* green, size_t greenStride, const uint8_t* red, size_t redStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
      {
          assert(width >= HA);
          if (align)
          {
              assert(Aligned(blue) && Aligned(blueStride));
              assert(Aligned(green) && Aligned(greenStride));
              assert(Aligned(red) && Aligned(redStride));
              assert(Aligned(bgra) && Aligned(bgraStride));
          }

          __m128i _alpha = _mm_slli_si128(_mm_set1_epi16(alpha), 1);
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
      void Bgr48pToBgra32(const uint8_t* blue, size_t blueStride, size_t width, size_t height,
          const uint8_t* green, size_t greenStride, const uint8_t* red, size_t redStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
      {
          if (Aligned(blue) && Aligned(blueStride) && Aligned(green) && Aligned(greenStride) &&
              Aligned(red) && Aligned(redStride) && Aligned(bgra) && Aligned(bgraStride))
              Bgr48pToBgra32<true>(blue, blueStride, width, height, green, greenStride, red, redStride, bgra, bgraStride, alpha);
          else
              Bgr48pToBgra32<false>(blue, blueStride, width, height, green, greenStride, red, redStride, bgra, bgraStride, alpha);
      }
      //}}}

      //{{{
      template <bool align> SIMD_INLINE void BgrToBgra(const uint8_t * bgr, uint8_t * bgra, __m128i alpha, __m128i shuffle)
      {
          Store<align>((__m128i*)bgra + 0, _mm_or_si128(alpha, _mm_shuffle_epi8(Load<align>((__m128i*)(bgr + 0)), shuffle)));
          Store<align>((__m128i*)bgra + 1, _mm_or_si128(alpha, _mm_shuffle_epi8(Load<false>((__m128i*)(bgr + 12)), shuffle)));
          Store<align>((__m128i*)bgra + 2, _mm_or_si128(alpha, _mm_shuffle_epi8(Load<false>((__m128i*)(bgr + 24)), shuffle)));
          Store<align>((__m128i*)bgra + 3, _mm_or_si128(alpha, _mm_shuffle_epi8(_mm_srli_si128(Load<align>((__m128i*)(bgr + 32)), 4), shuffle)));
      }
      //}}}
      //{{{
      template <bool align> void BgrToBgra(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(bgr) && Aligned(bgrStride));

          size_t alignedWidth = AlignLo(width, A);

          __m128i _alpha = _mm_slli_si128(_mm_set1_epi32(alpha), 3);
          __m128i _shuffle = _mm_setr_epi8(0x0, 0x1, 0x2, -1, 0x3, 0x4, 0x5, -1, 0x6, 0x7, 0x8, -1, 0x9, 0xA, 0xB, -1);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  BgrToBgra<align>(bgr + 3 * col, bgra + 4 * col, _alpha, _shuffle);
              if (width != alignedWidth)
                  BgrToBgra<false>(bgr + 3 * (width - A), bgra + 4 * (width - A), _alpha, _shuffle);
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

      //{{{
      template <bool align> SIMD_INLINE void RgbToBgra(const uint8_t* rgb, uint8_t* bgra, __m128i alpha, __m128i shuffle)
      {
          Store<align>((__m128i*)bgra + 0, _mm_or_si128(alpha, _mm_shuffle_epi8(Load<align>((__m128i*)(rgb + 0)), shuffle)));
          Store<align>((__m128i*)bgra + 1, _mm_or_si128(alpha, _mm_shuffle_epi8(Load<false>((__m128i*)(rgb + 12)), shuffle)));
          Store<align>((__m128i*)bgra + 2, _mm_or_si128(alpha, _mm_shuffle_epi8(Load<false>((__m128i*)(rgb + 24)), shuffle)));
          Store<align>((__m128i*)bgra + 3, _mm_or_si128(alpha, _mm_shuffle_epi8(_mm_srli_si128(Load<align>((__m128i*)(rgb + 32)), 4), shuffle)));
      }
      //}}}
      //{{{
      template <bool align> void RgbToBgra(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
      {
          assert(width >= A);
          if (align)
              assert(Aligned(bgra) && Aligned(bgraStride) && Aligned(rgb) && Aligned(rgbStride));

          size_t alignedWidth = AlignLo(width, A);

          __m128i _alpha = _mm_slli_si128(_mm_set1_epi32(alpha), 3);
          __m128i _shuffle = _mm_setr_epi8(0x2, 0x1, 0x0, -1, 0x5, 0x4, 0x3, -1, 0x8, 0x7, 0x6, -1, 0xB, 0xA, 0x9, -1);

          for (size_t row = 0; row < height; ++row)
          {
              for (size_t col = 0; col < alignedWidth; col += A)
                  RgbToBgra<align>(rgb + 3 * col, bgra + 4 * col, _alpha, _shuffle);
              if (width != alignedWidth)
                  RgbToBgra<false>(rgb + 3 * (width - A), bgra + 4 * (width - A), _alpha, _shuffle);
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
      }
  #endif
  }
