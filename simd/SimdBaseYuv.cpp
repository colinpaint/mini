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

namespace Simd {
  namespace Base {
    // to bgr
    //{{{
    SIMD_INLINE void Yuv422pToBgr(const uint8_t *y, int u, int v, uint8_t * bgr)
    {
        YuvToBgr(y[0], u, v, bgr);
        YuvToBgr(y[1], u, v, bgr + 3);
    }
    //}}}
    //{{{
    void Yuv420pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
    {
        assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t colUV = 0, colY = 0, colBgr = 0; colY < width; colY += 2, colUV++, colBgr += 6)
            {
                int u_ = u[colUV];
                int v_ = v[colUV];
                Yuv422pToBgr(y + colY, u_, v_, bgr + colBgr);
                Yuv422pToBgr(y + yStride + colY, u_, v_, bgr + bgrStride + colBgr);
            }
            y += 2 * yStride;
            u += uStride;
            v += vStride;
            bgr += 2 * bgrStride;
        }
    }
    //}}}
    //{{{
    void Yuv422pToBgr(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * bgr, size_t bgrStride)
    {
        assert((width % 2 == 0) && (width >= 2));

        for (size_t row = 0; row < height; ++row)
        {
            for (size_t colUV = 0, colY = 0, colBgr = 0; colY < width; colY += 2, colUV++, colBgr += 6)
                Yuv422pToBgr(y + colY, u[colUV], v[colUV], bgr + colBgr);
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
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0, colBgr = 0; col < width; col++, colBgr += 3)
                YuvToBgr(y[col], u[col], v[col], bgr + colBgr);
            y += yStride;
            u += uStride;
            v += vStride;
            bgr += bgrStride;
        }
    }
    //}}}

    // to rgb
    //{{{
    SIMD_INLINE void Yuv422pToRgb(const uint8_t* y, int u, int v, uint8_t* rgb)
    {
        YuvToRgb(y[0], u, v, rgb);
        YuvToRgb(y[1], u, v, rgb + 3);
    }
    //}}}
    //{{{
    void Yuv420pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
        size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
    {
        assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t colUV = 0, colY = 0, colRgb = 0; colY < width; colY += 2, colUV++, colRgb += 6)
            {
                int u_ = u[colUV];
                int v_ = v[colUV];
                Yuv422pToRgb(y + colY, u_, v_, rgb + colRgb);
                Yuv422pToRgb(y + yStride + colY, u_, v_, rgb + rgbStride + colRgb);
            }
            y += 2 * yStride;
            u += uStride;
            v += vStride;
            rgb += 2 * rgbStride;
        }
    }
    //}}}
    //{{{
    void Yuv422pToRgb(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
        size_t width, size_t height, uint8_t* rgb, size_t rgbStride)
    {
        assert((width % 2 == 0) && (width >= 2));

        for (size_t row = 0; row < height; ++row)
        {
            for (size_t colUV = 0, colY = 0, colRgb = 0; colY < width; colY += 2, colUV++, colRgb += 6)
                Yuv422pToRgb(y + colY, u[colUV], v[colUV], rgb + colRgb);
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
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0, colRgb = 0; col < width; col++, colRgb += 3)
                YuvToRgb(y[col], u[col], v[col], rgb + colRgb);
            y += yStride;
            u += uStride;
            v += vStride;
            rgb += rgbStride;
        }
    }
    //}}}

    // Yuva420p to bgra
    //{{{
    SIMD_INLINE void Yuva422pToBgra(const uint8_t * y, int u, int v, const uint8_t * a, uint8_t * bgra)
    {
        YuvToBgra(y[0], u, v, a[0], bgra + 0);
        YuvToBgra(y[1], u, v, a[1], bgra + 4);
    }
    //}}}
    //{{{
    void Yuva420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        const uint8_t * a, size_t aStride, size_t width, size_t height, uint8_t * bgra, size_t bgraStride)
    {
        assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t colUV = 0, colY = 0, colBgra = 0; colY < width; colY += 2, colUV++, colBgra += 8)
            {
                int u_ = u[colUV];
                int v_ = v[colUV];
                Yuva422pToBgra(y + colY, u_, v_, a + colY, bgra + colBgra);
                Yuva422pToBgra(y + yStride + colY, u_, v_, a + aStride + colY, bgra + bgraStride + colBgra);
            }
            y += 2 * yStride;
            u += uStride;
            v += vStride;
            a += 2 * aStride;
            bgra += 2 * bgraStride;
        }
    }
    //}}}

    // Yuv444p to bgraV2
    //{{{
    template <class YuvType> void Yuva444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride,
        const uint8_t* v, size_t vStride, const uint8_t* a, size_t aStride, size_t width, size_t height, uint8_t* bgra, size_t bgraStride)
    {
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0; col < width; col++)
                YuvToBgra<YuvType>(y[col], u[col], v[col], a[col], bgra + col * 4);
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
        case SimdYuvBt601: Yuva444pToBgraV2<Bt601>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
        case SimdYuvBt709: Yuva444pToBgraV2<Bt709>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
        case SimdYuvBt2020: Yuva444pToBgraV2<Bt2020>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
        case SimdYuvTrect871: Yuva444pToBgraV2<Trect871>(y, yStride, u, uStride, v, vStride, a, aStride, width, height, bgra, bgraStride); break;
        default:
            assert(0);
        }
    }
    //}}}

    // yuv to bgra
    //{{{
    SIMD_INLINE void Yuv422pToBgra(const uint8_t *y, int u, int v, int alpha, uint8_t * bgra)
    {
        YuvToBgra(y[0], u, v, alpha, bgra + 0);
        YuvToBgra(y[1], u, v, alpha, bgra + 4);
    }
    //}}}
    //{{{
    void Yuv420pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
    {
        assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t colUV = 0, colY = 0, colBgra = 0; colY < width; colY += 2, colUV++, colBgra += 8)
            {
                int u_ = u[colUV];
                int v_ = v[colUV];
                Yuv422pToBgra(y + colY, u_, v_, alpha, bgra + colBgra);
                Yuv422pToBgra(y + yStride + colY, u_, v_, alpha, bgra + bgraStride + colBgra);
            }
            y += 2 * yStride;
            u += uStride;
            v += vStride;
            bgra += 2 * bgraStride;
        }
    }
    //}}}
    //{{{
    void Yuv422pToBgra(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha)
    {
        assert((width % 2 == 0) && (width >= 2));

        for (size_t row = 0; row < height; ++row)
        {
            for (size_t colUV = 0, colY = 0, colBgra = 0; colY < width; colY += 2, colUV++, colBgra += 8)
                Yuv422pToBgra(y + colY, u[colUV], v[colUV], alpha, bgra + colBgra);
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
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0, colBgra = 0; col < width; col++, colBgra += 4)
                YuvToBgra(y[col], u[col], v[col], alpha, bgra + colBgra);
            y += yStride;
            u += uStride;
            v += vStride;
            bgra += bgraStride;
        }
    }
    //}}}

    // yuv420 to bgraV2
    //{{{
    template <class YuvType> SIMD_INLINE void Yuv422pToBgra(const uint8_t* y, int u, int v, int alpha, uint8_t* bgra)
    {
        YuvToBgra<YuvType>(y[0], u, v, alpha, bgra + 0);
        YuvToBgra<YuvType>(y[1], u, v, alpha, bgra + 4);
    }
    //}}}
    //{{{
    template <class YuvType> void Yuv420pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
        size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
    {
        assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t colUV = 0, colY = 0, colBgra = 0; colY < width; colY += 2, colUV++, colBgra += 8)
            {
                int _u = u[colUV];
                int _v = v[colUV];
                Yuv422pToBgra<YuvType>(y + colY, _u, _v, alpha, bgra + colBgra);
                Yuv422pToBgra<YuvType>(y + yStride + colY, _u, _v, alpha, bgra + bgraStride + colBgra);
            }
            y += 2 * yStride;
            u += uStride;
            v += vStride;
            bgra += 2 * bgraStride;
        }
    }
    //}}}
    //{{{
    void Yuv420pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
        size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType)
    {
        switch (yuvType)
        {
        case SimdYuvBt601: Yuv420pToBgraV2<Bt601>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        case SimdYuvBt709: Yuv420pToBgraV2<Bt709>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        case SimdYuvBt2020: Yuv420pToBgraV2<Bt2020>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        case SimdYuvTrect871: Yuv420pToBgraV2<Trect871>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        default:
            assert(0);
        }
    }
    //}}}

    // yuv444 to bgraV2
    //{{{
    template <class YuvType> void Yuv444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
        size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha)
    {
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0, colBgra = 0; col < width; col++, colBgra += 4)
                YuvToBgra<YuvType>(y[col], u[col], v[col], alpha, bgra + colBgra);
            y += yStride;
            u += uStride;
            v += vStride;
            bgra += bgraStride;
        }
    }
    //}}}
    //{{{
    void Yuv444pToBgraV2(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
        size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType)
    {
        switch (yuvType)
        {
        case SimdYuvBt601: Yuv444pToBgraV2<Bt601>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        case SimdYuvBt709: Yuv444pToBgraV2<Bt709>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        case SimdYuvBt2020: Yuv444pToBgraV2<Bt2020>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        case SimdYuvTrect871: Yuv444pToBgraV2<Trect871>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha); break;
        default:
            assert(0);
        }
    }
    //}}}

    // uyvu to bgr
    //{{{
    template <class YuvType> SIMD_INLINE void Uyvy422ToBgr(const uint8_t* uyvy, uint8_t* bgr)
    {
        uint8_t u = uyvy[0], v = uyvy[2];
        YuvToBgr<YuvType>(uyvy[1], u, v, bgr + 0);
        YuvToBgr<YuvType>(uyvy[3], u, v, bgr + 3);
    }
    //}}}
    //{{{
    template <class YuvType> void Uyvy422ToBgr(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* bgr, size_t bgrStride)
    {
        assert((width % 2 == 0) && (width >= 2));

        size_t sizeUyvy = width * 2;
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t colUyvy = 0, colBgr = 0; colUyvy < sizeUyvy; colUyvy += 4, colBgr += 6)
                Uyvy422ToBgr<YuvType>(uyvy + colUyvy, bgr + colBgr);
            uyvy += uyvyStride;
            bgr += bgrStride;
        }
    }
    //}}}
    //{{{
    void Uyvy422ToBgr(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* bgr, size_t bgrStride, SimdYuvType yuvType)
    {
        switch (yuvType)
        {
        case SimdYuvBt601: Uyvy422ToBgr<Base::Bt601>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
        case SimdYuvBt709: Uyvy422ToBgr<Base::Bt709>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
        case SimdYuvBt2020: Uyvy422ToBgr<Base::Bt2020>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
        case SimdYuvTrect871: Uyvy422ToBgr<Base::Trect871>(uyvy, uyvyStride, width, height, bgr, bgrStride); break;
        default:
            assert(0);
        }
    }
    //}}}

    // uyvy
    //{{{
    SIMD_INLINE void Uyvy422ToYuv420p(const uint8_t* uyvy0, size_t uyvyStride, uint8_t* y0, size_t yStride, uint8_t* u, uint8_t* v)
    {
        const uint8_t* uyvy1 = uyvy0 + uyvyStride;
        uint8_t* y1 = y0 + yStride;
        y0[0] = uyvy0[1];
        y0[1] = uyvy0[3];
        y1[0] = uyvy1[1];
        y1[1] = uyvy1[3];
        u[0] = (uyvy0[0] + uyvy1[0] + 1) / 2;
        v[0] = (uyvy0[2] + uyvy1[2] + 1) / 2;
    }
    //}}}
    //{{{
    void Uyvy422ToYuv420p(const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height, uint8_t* y, size_t yStride, uint8_t* u, size_t uStride, uint8_t* v, size_t vStride)
    {
        assert((width % 2 == 0) && (height % 2 == 0));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t colUyvy = 0, colY = 0, colUV = 0; colY < width; colUyvy += 4, colY += 2, colUV += 1)
                Uyvy422ToYuv420p(uyvy + colUyvy, uyvyStride, y + colY, yStride, u + colUV, v + colUV);
            uyvy += 2 * uyvyStride;
            y += 2 * yStride;
            u += uStride;
            v += vStride;
        }
    }
    //}}}

    // yuv420p to uyvy422
    //{{{
    SIMD_INLINE void Yuv420pToUyvy422(const uint8_t* y0, size_t yStride, const uint8_t* u, const uint8_t* v, uint8_t* uyvy0, size_t uyvyStride)
    {
        const uint8_t* y1 = y0 + yStride;
        uint8_t* uyvy1 = uyvy0 + uyvyStride;
        uyvy0[1] = y0[0];
        uyvy0[3] = y0[1];
        uyvy1[1] = y1[0];
        uyvy1[3] = y1[1];
        uyvy0[0] = u[0];
        uyvy1[0] = u[0];
        uyvy0[2] = v[0];
        uyvy1[2] = v[0];
    }
    //}}}
    //{{{
    void Yuv420pToUyvy422(const uint8_t* y, size_t yStride, const uint8_t* u, size_t uStride, const uint8_t* v, size_t vStride,
        size_t width, size_t height, uint8_t* uyvy, size_t uyvyStride)
    {
        assert((width % 2 == 0) && (height % 2 == 0));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t colY = 0, colUV = 0, colUyvy = 0; colY < width; colY += 2, colUV += 1, colUyvy += 4)
                Yuv420pToUyvy422(y + colY, yStride, u + colUV, v + colUV, uyvy + colUyvy, uyvyStride);
            y += 2 * yStride;
            u += uStride;
            v += vStride;
            uyvy += 2 * uyvyStride;
        }
    }
    //}}}

    // to hue,hsv
    //{{{
    void Yuv444pToHsl(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * hsl, size_t hslStride)
    {
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0, colHsl = 0; col < width; col++, colHsl += 3)
                YuvToHsl(y[col], u[col], v[col], hsl + colHsl);
            y += yStride;
            u += uStride;
            v += vStride;
            hsl += hslStride;
        }
    }
    //}}}
    //{{{
    SIMD_INLINE int YuvToHue(int y, int u, int v)
    {
        int red = YuvToRed(y, v);
        int green = YuvToGreen(y, u, v);
        int blue = YuvToBlue(y, u);

        int max = Max(red, Max(green, blue));
        int min = Min(red, Min(green, blue));
        int range = max - min;

        if (range)
        {
            int dividend;

            if (red == max)
                dividend = green - blue + 6 * range;
            else if (green == max)
                dividend = blue - red + 2 * range;
            else
                dividend = red - green + 4 * range;

            return int(KF_255_DIV_6*float(dividend) / float(range)
            #if defined(_MSC_VER)
                +0.00001f
            #endif
                );
        }
        return 0;
    }
    //}}}
    //{{{
    void Yuv420pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * hue, size_t hueStride)
    {
        assert((width % 2 == 0) && (height % 2 == 0) && (width >= 2) && (height >= 2));

        for (size_t row = 0; row < height; row += 2)
        {
            for (size_t col1 = 0, col2 = 0; col2 < width; col2 += 2, col1++)
            {
                int u_ = u[col1];
                int v_ = v[col1];
                hue[col2] = YuvToHue(y[col2], u_, v_);
                hue[col2 + 1] = YuvToHue(y[col2 + 1], u_, v_);
                hue[col2 + hueStride] = YuvToHue(y[col2 + yStride], u_, v_);
                hue[col2 + hueStride + 1] = YuvToHue(y[col2 + yStride + 1], u_, v_);
            }
            y += 2 * yStride;
            u += uStride;
            v += vStride;
            hue += 2 * hueStride;
        }
    }
    //}}}
    //{{{
    void Yuv444pToHue(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * hue, size_t hueStride)
    {
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0; col < width; ++col)
            {
                hue[col] = YuvToHue(y[col], u[col], v[col]);
            }
            y += yStride;
            u += uStride;
            v += vStride;
            hue += hueStride;
        }
    }
    //}}}
    //{{{
    void Yuv444pToHsv(const uint8_t * y, size_t yStride, const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
        size_t width, size_t height, uint8_t * hsv, size_t hsvStride)
    {
        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0, colHsv = 0; col < width; col++, colHsv += 3)
                YuvToHsv(y[col], u[col], v[col], hsv + colHsv);
            y += yStride;
            u += uStride;
            v += vStride;
            hsv += hsvStride;
        }
    }
    //}}}
    }
  }
