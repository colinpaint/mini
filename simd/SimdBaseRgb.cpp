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
#include "SimdDefs.h"
#include "SimdConversion.h"
#include "SimdYuvToBgr.h"

namespace Simd::Base {
  //{{{
  void BgrToHsl(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * hsl, size_t hslStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          const uint8_t * pBgr = bgr + row*bgrStride;
          uint8_t * pHsl = hsl + row*hslStride;
          for (const uint8_t * pBgrEnd = pBgr + width * 3; pBgr < pBgrEnd; pBgr += 3, pHsl += 3)
          {
              BgrToHsl(pBgr[0], pBgr[1], pBgr[2], pHsl);
          }
      }
  }
  //}}}
  //{{{
  void BgrToHsv(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * hsv, size_t hsvStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          const uint8_t * pBgr = bgr + row*bgrStride;
          uint8_t * pHsv = hsv + row*hsvStride;
          for (const uint8_t * pBgrEnd = pBgr + width * 3; pBgr < pBgrEnd; pBgr += 3, pHsv += 3)
          {
              BgrToHsv(pBgr[0], pBgr[1], pBgr[2], pHsv);
          }
      }
  }
  //}}}
  //{{{
  void BgrToRgb(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * rgb, size_t rgbStride)
  {
      size_t size = width * 3;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t i = 0; i < size; i += 3)
          {
              rgb[i + 0] = bgr[i + 2];
              rgb[i + 1] = bgr[i + 1];
              rgb[i + 2] = bgr[i + 0];
          }
          bgr += bgrStride;
          rgb += rgbStride;
      }
  }
  //}}}
  //{{{
  SIMD_INLINE void BgrToYuv420p(const uint8_t * bgr0, size_t bgrStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v)
  {
      const uint8_t * bgr1 = bgr0 + bgrStride;
      uint8_t * y1 = y0 + yStride;

      y0[0] = BgrToY(bgr0[0], bgr0[1], bgr0[2]);
      y0[1] = BgrToY(bgr0[3], bgr0[4], bgr0[5]);
      y1[0] = BgrToY(bgr1[0], bgr1[1], bgr1[2]);
      y1[1] = BgrToY(bgr1[3], bgr1[4], bgr1[5]);

      int blue = Average(bgr0[0], bgr0[3], bgr1[0], bgr1[3]);
      int green = Average(bgr0[1], bgr0[4], bgr1[1], bgr1[4]);
      int red = Average(bgr0[2], bgr0[5], bgr1[2], bgr1[5]);

      u[0] = BgrToU(blue, green, red);
      v[0] = BgrToV(blue, green, red);
  }
  //}}}
  //{{{
  void BgrToYuv420p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colBgr = 0; colY < width; colY += 2, colUV++, colBgr += 6)
          {
              BgrToYuv420p(bgr + colBgr, bgrStride, y + colY, yStride, u + colUV, v + colUV);
          }
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          bgr += 2 * bgrStride;
      }
  }
  //}}}

  //{{{
  SIMD_INLINE void BgrToYuv422p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v)
  {
      y[0] = BgrToY(bgr[0], bgr[1], bgr[2]);
      y[1] = BgrToY(bgr[3], bgr[4], bgr[5]);

      int blue = Average(bgr[0], bgr[3]);
      int green = Average(bgr[1], bgr[4]);
      int red = Average(bgr[2], bgr[5]);

      u[0] = BgrToU(blue, green, red);
      v[0] = BgrToV(blue, green, red);
  }
  //}}}
  //{{{
  void BgrToYuv422p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert((width % 2 == 0) && (width >= 2));

      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colUV = 0, colY = 0, colBgr = 0; colY < width; colY += 2, colUV++, colBgr += 6)
              BgrToYuv422p(bgr + colBgr, y + colY, u + colUV, v + colUV);
          y += yStride;
          u += uStride;
          v += vStride;
          bgr += bgrStride;
      }
  }
  //}}}

  //{{{
  SIMD_INLINE void BgrToYuv444p(const uint8_t * bgr, uint8_t * y, uint8_t * u, uint8_t * v)
  {
      const int blue = bgr[0], green = bgr[1], red = bgr[2];
      y[0] = BgrToY(blue, green, red);
      u[0] = BgrToU(blue, green, red);
      v[0] = BgrToV(blue, green, red);
  }
  //}}}
  //{{{
  void BgrToYuv444p(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0, colBgr = 0; col < width; ++col, colBgr += 3)
              BgrToYuv444p(bgr + colBgr, y + col, u + col, v + col);
          y += yStride;
          u += uStride;
          v += vStride;
          bgr += bgrStride;
      }
  }
  //}}}
  //{{{
  void BgraToGray(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * gray, size_t grayStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          const uint8_t * pBgra = bgra + row*bgraStride;
          uint8_t * pGray = gray + row*grayStride;
          for (const uint8_t *pGrayEnd = pGray + width; pGray < pGrayEnd; pGray += 1, pBgra += 4)
          {
              *pGray = BgrToGray(pBgra[0], pBgra[1], pBgra[2]);
          }
      }
  }
  //}}}

  //{{{
  void RgbaToGray(const uint8_t* rgba, size_t width, size_t height, size_t rgbaStride, uint8_t* gray, size_t grayStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          const uint8_t* pRgba = rgba + row * rgbaStride;
          uint8_t* pGray = gray + row * grayStride;
          for (const uint8_t* pGrayEnd = pGray + width; pGray < pGrayEnd; pGray += 1, pRgba += 4)
          {
              *pGray = BgrToGray(pRgba[2], pRgba[1], pRgba[0]);
          }
      }
  }
  //}}}
  //{{{
  SIMD_INLINE void BgraToYuv420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v)
  {
      const uint8_t * bgra1 = bgra0 + bgraStride;
      uint8_t * y1 = y0 + yStride;

      y0[0] = BgrToY(bgra0[0], bgra0[1], bgra0[2]);
      y0[1] = BgrToY(bgra0[4], bgra0[5], bgra0[6]);
      y1[0] = BgrToY(bgra1[0], bgra1[1], bgra1[2]);
      y1[1] = BgrToY(bgra1[4], bgra1[5], bgra1[6]);

      int blue = Average(bgra0[0], bgra0[4], bgra1[0], bgra1[4]);
      int green = Average(bgra0[1], bgra0[5], bgra1[1], bgra1[5]);
      int red = Average(bgra0[2], bgra0[6], bgra1[2], bgra1[6]);

      u[0] = BgrToU(blue, green, red);
      v[0] = BgrToV(blue, green, red);
  }
  //}}}
  //{{{
  void BgraToYuv420p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colBgra = 0; colY < width; colY += 2, colUV++, colBgra += 8)
              BgraToYuv420p(bgra + colBgra, bgraStride, y + colY, yStride, u + colUV, v + colUV);
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          bgra += 2 * bgraStride;
      }
  }
  //}}}

  //{{{
  SIMD_INLINE void BgraToYuv422p(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v)
  {
      y[0] = BgrToY(bgra[0], bgra[1], bgra[2]);
      y[1] = BgrToY(bgra[4], bgra[5], bgra[6]);

      int blue = Average(bgra[0], bgra[4]);
      int green = Average(bgra[1], bgra[5]);
      int red = Average(bgra[2], bgra[6]);

      u[0] = BgrToU(blue, green, red);
      v[0] = BgrToV(blue, green, red);
  }
  //}}}
  //{{{
  void BgraToYuv422p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      assert((width % 2 == 0) && (width >= 2));

      for (size_t row = 0; row < height; ++row)
      {
          for (size_t colUV = 0, colY = 0, colBgra = 0; colY < width; colY += 2, colUV++, colBgra += 8)
              BgraToYuv422p(bgra + colBgra, y + colY, u + colUV, v + colUV);
          y += yStride;
          u += uStride;
          v += vStride;
          bgra += bgraStride;
      }
  }
  //}}}

  //{{{
  SIMD_INLINE void BgraToYuv444p(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v)
  {
      const int blue = bgra[0], green = bgra[1], red = bgra[2];
      y[0] = BgrToY(blue, green, red);
      u[0] = BgrToU(blue, green, red);
      v[0] = BgrToV(blue, green, red);
  }
  //}}}
  //{{{
  void BgraToYuv444p(const uint8_t * bgra, size_t width, size_t height, size_t bgraStride, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0, colBgra = 0; col < width; ++col, colBgra += 4)
              BgraToYuv444p(bgra + colBgra, y + col, u + col, v + col);
          y += yStride;
          u += uStride;
          v += vStride;
          bgra += bgraStride;
      }
  }
  //}}}

  //{{{
  SIMD_INLINE void BgraToYuva420p(const uint8_t * bgra0, size_t bgraStride, uint8_t * y0, size_t yStride, uint8_t * u, uint8_t * v, uint8_t * a0, size_t aStride)
  {
      const uint8_t * bgra1 = bgra0 + bgraStride;
      uint8_t * y1 = y0 + yStride;
      uint8_t * a1 = a0 + aStride;

      y0[0] = BgrToY(bgra0[0], bgra0[1], bgra0[2]);
      y0[1] = BgrToY(bgra0[4], bgra0[5], bgra0[6]);
      y1[0] = BgrToY(bgra1[0], bgra1[1], bgra1[2]);
      y1[1] = BgrToY(bgra1[4], bgra1[5], bgra1[6]);

      int blue = Average(bgra0[0], bgra0[4], bgra1[0], bgra1[4]);
      int green = Average(bgra0[1], bgra0[5], bgra1[1], bgra1[5]);
      int red = Average(bgra0[2], bgra0[6], bgra1[2], bgra1[6]);

      u[0] = BgrToU(blue, green, red);
      v[0] = BgrToV(blue, green, red);

      a0[0] = bgra0[3];
      a0[1] = bgra0[7];
      a1[0] = bgra1[3];
      a1[1] = bgra1[7];
  }
  //}}}
  //{{{
  void BgraToYuva420p(const uint8_t * bgra, size_t bgraStride, size_t width, size_t height, uint8_t * y, size_t yStride,
      uint8_t * u, size_t uStride, uint8_t * v, size_t vStride, uint8_t * a, size_t aStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colYA = 0, colBgra = 0; colYA < width; colYA += 2, colUV++, colBgra += 8)
              BgraToYuva420p(bgra + colBgra, bgraStride, y + colYA, yStride, u + colUV, v + colUV, a + colYA, aStride);
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          a += 2 * aStride;
          bgra += 2 * bgraStride;
      }
  }
  //}}}

  //{{{
  template <class YuvType> SIMD_INLINE void BgraToYuv444pV2(const uint8_t* bgra, uint8_t* y, uint8_t* u, uint8_t* v)
  {
      const int blue = bgra[0], green = bgra[1], red = bgra[2];
      y[0] = BgrToY<YuvType>(blue, green, red);
      u[0] = BgrToU<YuvType>(blue, green, red);
      v[0] = BgrToV<YuvType>(blue, green, red);
  }
  //}}}
  //{{{
  template <class YuvType> void BgraToYuv444pV2(const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
      uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0, colBgra = 0; col < width; ++col, colBgra += 4)
              BgraToYuv444pV2<YuvType>(bgra + colBgra, y + col, u + col, v + col);
          bgra += bgraStride;
          y += yStride;
          u += uStride;
          v += vStride;
      }
  }
  //}}}
  //{{{
  void BgraToYuv444pV2(const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
      uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride, SimdYuvType yuvType)
  {
      switch (yuvType)
      {
      case SimdYuvBt601: BgraToYuv444pV2<Bt601>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      case SimdYuvBt709: BgraToYuv444pV2<Bt709>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      case SimdYuvBt2020: BgraToYuv444pV2<Bt2020>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      case SimdYuvTrect871: BgraToYuv444pV2<Trect871>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      default:
          assert(0);
      }
  }
  //}}}

  //{{{
  template <class YuvType> SIMD_INLINE void BgraToYuv420pV2(const uint8_t* bgra0, size_t bgraStride, uint8_t* y0, size_t yStride, uint8_t* u, uint8_t* v)
  {
      const uint8_t* bgra1 = bgra0 + bgraStride;
      uint8_t* y1 = y0 + yStride;

      y0[0] = BgrToY<YuvType>(bgra0[0], bgra0[1], bgra0[2]);
      y0[1] = BgrToY<YuvType>(bgra0[4], bgra0[5], bgra0[6]);
      y1[0] = BgrToY<YuvType>(bgra1[0], bgra1[1], bgra1[2]);
      y1[1] = BgrToY<YuvType>(bgra1[4], bgra1[5], bgra1[6]);

      int blue = Average(bgra0[0], bgra0[4], bgra1[0], bgra1[4]);
      int green = Average(bgra0[1], bgra0[5], bgra1[1], bgra1[5]);
      int red = Average(bgra0[2], bgra0[6], bgra1[2], bgra1[6]);

      u[0] = BgrToU<YuvType>(blue, green, red);
      v[0] = BgrToV<YuvType>(blue, green, red);
  }
  //}}}
  //{{{
  template <class YuvType> void BgraToYuv420pV2(const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
      uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
  {
      assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

      for (size_t row = 0; row < height; row += 2)
      {
          for (size_t colUV = 0, colY = 0, colBgra = 0; colY < width; colY += 2, colUV++, colBgra += 8)
              BgraToYuv420pV2<YuvType>(bgra + colBgra, bgraStride, y + colY, yStride, u + colUV, v + colUV);
          y += 2 * yStride;
          u += uStride;
          v += vStride;
          bgra += 2 * bgraStride;
      }
  }
  //}}}
  //{{{
  void BgraToYuv420pV2(const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
      uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride, SimdYuvType yuvType)
  {
      switch (yuvType)
      {
      case SimdYuvBt601: BgraToYuv420pV2<Bt601>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      case SimdYuvBt709: BgraToYuv420pV2<Bt709>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      case SimdYuvBt2020: BgraToYuv420pV2<Bt2020>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      case SimdYuvTrect871: BgraToYuv420pV2<Trect871>(bgra, bgraStride, width, height, y, yStride, u, uStride, v, vStride); break;
      default:
          assert(0);
      }
  }
  //}}}
  //{{{
  void BgrToBgra(const uint8_t *bgr, size_t size, uint8_t *bgra, bool fillAlpha, bool lastRow, uint8_t alpha)
  {
      if (fillAlpha)
      {
      #ifdef SIMD_BIG_ENDIAN
          const int32_t alphaMask = alpha;
      #else
          const int32_t alphaMask = alpha << 24;
      #endif
          for (size_t i = (lastRow ? 1 : 0); i < size; ++i, bgr += 3, bgra += 4)
          {
              *(int32_t*)bgra = (*(int32_t*)bgr) | alphaMask;
          }
          if (lastRow)
          {
              bgra[0] = bgr[0];
              bgra[1] = bgr[1];
              bgra[2] = bgr[2];
              bgra[3] = alpha;
          }
      }
      else
      {
          for (size_t i = (lastRow ? 1 : 0); i < size; ++i, bgr += 3, bgra += 4)
          {
              *(int32_t*)bgra = (*(int32_t*)bgr);
          }
          if (lastRow)
          {
              bgra[0] = bgr[0];
              bgra[1] = bgr[1];
              bgra[2] = bgr[2];
          }
      }
  }
  //}}}
  //{{{
  void BgrToBgra(const uint8_t *bgr, size_t width, size_t height, size_t bgrStride, uint8_t *bgra, size_t bgraStride, uint8_t alpha)
  {
      for (size_t row = 1; row < height; ++row)
      {
          BgrToBgra(bgr, width, bgra, true, false, alpha);
          bgr += bgrStride;
          bgra += bgraStride;
      }
      BgrToBgra(bgr, width, bgra, true, true, alpha);
  }
  //}}}
  //{{{
  void Bgr48pToBgra32(const uint8_t * blue, size_t blueStride, size_t width, size_t height,
      const uint8_t * green, size_t greenStride, const uint8_t * red, size_t redStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
  {
      for (size_t row = 0; row < height; ++row)
      {
          const uint8_t * pBlue = blue;
          const uint8_t * pGreen = green;
          const uint8_t * pRed = red;
          uint8_t * pBgra = bgra;
          for (size_t col = 0; col < width; ++col)
          {
          #ifdef SIMD_BIG_ENDIAN
              pBgra[0] = pBlue[1];
              pBgra[1] = pGreen[1];
              pBgra[2] = pRed[1];
          #else
              pBgra[0] = pBlue[0];
              pBgra[1] = pGreen[0];
              pBgra[2] = pRed[0];
          #endif
              pBgra[3] = alpha;
              pBlue += 2;
              pGreen += 2;
              pRed += 2;
              pBgra += 4;
          }
          blue += blueStride;
          green += greenStride;
          red += redStride;
          bgra += bgraStride;
      }
  }
  //}}}
  //{{{
  void RgbToBgra(const uint8_t * rgb, size_t width, size_t height, size_t rgbStride, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
  {
      size_t rgbGap = rgbStride - width * 3;
      size_t bgraGap = bgraStride - width * 4;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < width; ++col, rgb += 3, bgra += 4)
          {
              bgra[0] = rgb[2];
              bgra[1] = rgb[1];
              bgra[2] = rgb[0];
              bgra[3] = alpha;
          }
          rgb += rgbGap;
          bgra += bgraGap;
      }
  }
  //}}}
  //{{{
  void BgrToGray(const uint8_t *bgr, size_t width, size_t height, size_t bgrStride, uint8_t *gray, size_t grayStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          const uint8_t * pBgr = bgr + row*bgrStride;
          uint8_t * pGray = gray + row*grayStride;
          for (const uint8_t *pGrayEnd = pGray + width; pGray < pGrayEnd; pGray += 1, pBgr += 3)
          {
              *pGray = BgrToGray(pBgr[0], pBgr[1], pBgr[2]);
          }
      }
  }
  //}}}

  //{{{
  void RgbToGray(const uint8_t* rgb, size_t width, size_t height, size_t rgbStride, uint8_t* gray, size_t grayStride)
  {
      for (size_t row = 0; row < height; ++row)
      {
          const uint8_t* pRgb = rgb + row * rgbStride;
          uint8_t* pGray = gray + row * grayStride;
          for (const uint8_t* pGrayEnd = pGray + width; pGray < pGrayEnd; pGray += 1, pRgb += 3)
          {
              *pGray = BgrToGray(pRgb[2], pRgb[1], pRgb[0]);
          }
      }
  }
  //}}}
  //{{{
  void BgraToBgr(const uint8_t *bgra, size_t size, uint8_t *bgr, bool lastRow)
  {
      for (size_t i = (lastRow ? 1 : 0); i < size; ++i, bgr += 3, bgra += 4)
      {
          *(int32_t*)bgr = (*(int32_t*)bgra);
      }
      if (lastRow)
      {
          bgr[0] = bgra[0];
          bgr[1] = bgra[1];
          bgr[2] = bgra[2];
      }
  }
  //}}}
  //{{{
  void BgraToBgr(const uint8_t *bgra, size_t width, size_t height, size_t bgraStride, uint8_t *bgr, size_t bgrStride)
  {
      for (size_t row = 1; row < height; ++row)
      {
          BgraToBgr(bgra, width, bgr, false);
          bgr += bgrStride;
          bgra += bgraStride;
      }
      BgraToBgr(bgra, width, bgr, true);
  }
  //}}}
  //{{{
  void BgraToRgb(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgb, size_t rgbStride)
  {
      size_t bgraGap = bgraStride - width * 4;
      size_t rgbGap = rgbStride - width * 3;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < width; ++col, bgra += 4, rgb += 3)
          {
              rgb[2] = bgra[0];
              rgb[1] = bgra[1];
              rgb[0] = bgra[2];
          }
          bgra += bgraGap;
          rgb += rgbGap;
      }
  }
  //}}}
  //{{{
  void BgraToRgba(const uint8_t* bgra, size_t width, size_t height, size_t bgraStride, uint8_t* rgba, size_t rgbaStride)
  {
      size_t bgraGap = bgraStride - width * 4;
      size_t rgbaGap = rgbaStride - width * 4;
      for (size_t row = 0; row < height; ++row)
      {
          for (size_t col = 0; col < width; ++col, bgra += 4, rgba += 4)
          {
              rgba[2] = bgra[0];
              rgba[1] = bgra[1];
              rgba[0] = bgra[2];
              rgba[3] = bgra[3];
          }
          bgra += bgraGap;
          rgba += rgbaGap;
      }
  }
  //}}}
  }
