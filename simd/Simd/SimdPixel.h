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
#pragma once
#include "Simd/SimdView.hpp"

namespace Simd {
  namespace Pixel {
    struct Bgr24;
    struct Bgra32;
    struct Hsv24;
    struct Hsl24;
    struct Rgb24;
    struct Rgba32;

    //{{{
    struct Bgr24 {
        uint8_t blue; /*!< \brief 8-bit blue channel 24-bit BGR pixel. */
        uint8_t green; /*!< \brief 8-bit green channel 24-bit BGR pixel. */
        uint8_t red; /*!< \brief 8-bit red channel 24-bit BGR pixel. */

        Bgr24(const uint8_t & gray = uint8_t(0));

        Bgr24(const uint8_t & b, const uint8_t & g, const uint8_t & r);
        Bgr24(const Bgra32 & p);
        Bgr24(const Rgb24 & p);
        Bgr24(const Rgba32& p);

        Bgr24(const Bgr24 & p);

        template <template<class> class A> static const Bgr24 & At(const View<A> & view, ptrdiff_t col, ptrdiff_t row);
        template <template<class> class A> static Bgr24 & At(View<A> & view, ptrdiff_t col, ptrdiff_t row);
    };
    //}}}
    //{{{
    struct Bgra32
    {
        uint8_t blue; /*!< \brief 8-bit blue channel 32-bit BGRA pixel. */
        uint8_t green; /*!< \brief 8-bit green channel 32-bit BGRA pixel. */
        uint8_t red; /*!< \brief 8-bit red channel 32-bit BGRA pixel. */
        uint8_t alpha; /*!< \brief 8-bit alpha channel 32-bit BGRA pixel. */

        Bgra32(const uint8_t & gray = uint8_t(0), const uint8_t & a = uint8_t(255));
        Bgra32(const uint8_t & b, const uint8_t & g, const uint8_t & r, const uint8_t & a = uint8_t(255));
        Bgra32(const Bgr24 & p, const uint8_t & a = uint8_t(255));
        Bgra32(const Rgb24 & p, const uint8_t & a = uint8_t(255));
        Bgra32(const Rgba32& p);
        Bgra32(const Bgra32 & p);
        template <template<class> class A> static const Bgra32 & At(const View<A> & view, ptrdiff_t col, ptrdiff_t row);
        template <template<class> class A> static Bgra32 & At(View<A> & view, ptrdiff_t col, ptrdiff_t row);
    };
    //}}}
    //{{{
    struct Hsv24
    {
        uint8_t hue; /*!< \brief 8-bit hue channel 24-bit HSV pixel. */
        uint8_t saturation; /*!< \brief 8-bit saturation channel 24-bit HSV pixel. */
        uint8_t value; /*!< \brief 8-bit value channel 24-bit HSV pixel. */
        Hsv24(const uint8_t & gray = uint8_t(0));

        Hsv24(const uint8_t & h, const uint8_t & s, const uint8_t & v);

        Hsv24(const Hsv24 & p);

        template <template<class> class A> static const Hsv24 & At(const View<A> & view, ptrdiff_t col, ptrdiff_t row);

        template <template<class> class A> static Hsv24 & At(View<A> & view, ptrdiff_t col, ptrdiff_t row);
    };
    //}}}
    //{{{
    struct Hsl24
    {
        uint8_t hue; /*!< \brief 8-bit hue channel 24-bit HSL pixel. */
        uint8_t saturation; /*!< \brief 8-bit saturation channel 24-bit HSL pixel. */
        uint8_t lightness; /*!< \brief 8-bit lightness channel 24-bit HSL pixel. */

        Hsl24(const uint8_t & gray = uint8_t(0));

        Hsl24(const uint8_t & h, const uint8_t & s, const uint8_t & l);
        Hsl24(const Hsl24 & p);
        template <template<class> class A> static const Hsl24 & At(const View<A> & view, ptrdiff_t col, ptrdiff_t row);
        template <template<class> class A> static Hsl24 & At(View<A> & view, ptrdiff_t col, ptrdiff_t row);
    };
    //}}}
    //{{{
    struct Rgb24
    {
        uint8_t red; /*!< \brief 8-bit red channel 24-bit BGR pixel. */
        uint8_t green; /*!< \brief 8-bit green channel 24-bit BGR pixel. */
        uint8_t blue; /*!< \brief 8-bit blue channel 24-bit BGR pixel. */

        Rgb24(const uint8_t & gray = uint8_t(0));
        Rgb24(const uint8_t & r, const uint8_t & g, const uint8_t & b);
        Rgb24(const Bgra32 & p);
        Rgb24(const Bgr24 & p);
        Rgb24(const Rgba32& p);
        Rgb24(const Rgb24 & p);
        template <template<class> class A> static const Rgb24 & At(const View<A> & view, ptrdiff_t col, ptrdiff_t row);
        template <template<class> class A> static Rgb24 & At(View<A> & view, ptrdiff_t col, ptrdiff_t row);
    };
    //}}}
    //{{{
    struct Rgba32
    {
        uint8_t red; /*!< \brief 8-bit red channel 32-bit BGRA pixel. */
        uint8_t green; /*!< \brief 8-bit green channel 32-bit BGRA pixel. */
        uint8_t blue; /*!< \brief 8-bit blue channel 32-bit BGRA pixel. */
        uint8_t alpha; /*!< \brief 8-bit alpha channel 32-bit RGBA pixel. */

        Rgba32(const uint8_t& gray = uint8_t(0), const uint8_t& a = uint8_t(255));
        Rgba32(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a = uint8_t(255));
        Rgba32(const Bgra32& p);
        */
        Rgba32(const Bgr24& p, const uint8_t& a = uint8_t(255));
        Rgba32(const Rgb24& p, const uint8_t& a = uint8_t(255));
        Rgba32(const Rgba32& p);
        template <template<class> class A> static const Rgba32& At(const View<A>& view, ptrdiff_t col, ptrdiff_t row);
        template <template<class> class A> static Rgba32& At(View<A>& view, ptrdiff_t col, ptrdiff_t row);
    };
    //}}}

    //{{{
    SIMD_INLINE Bgr24::Bgr24(const uint8_t & gray)
        : blue(gray)
        , green(gray)
        , red(gray)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgr24::Bgr24(const uint8_t & b, const uint8_t & g, const uint8_t & r)
        : blue(b)
        , green(g)
        , red(r)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgr24::Bgr24(const Bgra32 & p)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgr24::Bgr24(const Rgb24 & p)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgr24::Bgr24(const Rgba32& p)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgr24::Bgr24(const Bgr24 & p)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
    {
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE const Bgr24 & Bgr24::At(const View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Bgr24);

        return Simd::At<A, Bgr24>(view, col, row);
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE Bgr24 & Bgr24::At(View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Bgr24);

        return Simd::At<A, Bgr24>(view, col, row);
    }
    //}}}

    //{{{
    SIMD_INLINE Bgra32::Bgra32(const uint8_t & gray, const uint8_t & a)
        : blue(gray)
        , green(gray)
        , red(gray)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgra32::Bgra32(const uint8_t & b, const uint8_t & g, const uint8_t & r, const uint8_t & a)
        : blue(b)
        , green(g)
        , red(r)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgra32::Bgra32(const Bgr24 & p, const uint8_t & a)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgra32::Bgra32(const Rgb24 & p, const uint8_t & a)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgra32::Bgra32(const Rgba32& p)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
        , alpha(p.alpha)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Bgra32::Bgra32(const Bgra32 & p)
        : blue(p.blue)
        , green(p.green)
        , red(p.red)
        , alpha(p.alpha)
    {
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE const Bgra32 & Bgra32::At(const View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Bgra32);

        return Simd::At<A, Bgra32>(view, col, row);
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE Bgra32 & Bgra32::At(View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Bgra32);

        return Simd::At<A, Bgra32>(view, col, row);
    }
    //}}}

    //{{{
    SIMD_INLINE Hsv24::Hsv24(const uint8_t & gray)
        : hue(0)
        , saturation(0)
        , value(gray)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Hsv24::Hsv24(const uint8_t & h, const uint8_t & s, const uint8_t & v)
        : hue(h)
        , saturation(s)
        , value(v)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Hsv24::Hsv24(const Hsv24 & p)
        : hue(p.hue)
        , saturation(p.saturation)
        , value(p.value)
    {
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE const Hsv24 & Hsv24::At(const View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Hsv24);

        return Simd::At<A, Hsv24>(view, col, row);
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE Hsv24 & Hsv24::At(View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Hsv24);

        return Simd::At<A, Hsv24>(view, col, row);
    }
    //}}}

    //{{{
    SIMD_INLINE Hsl24::Hsl24(const uint8_t & gray)
        : hue(0)
        , saturation(0)
        , lightness(gray)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Hsl24::Hsl24(const uint8_t & h, const uint8_t & s, const uint8_t & l)
        : hue(h)
        , saturation(s)
        , lightness(l)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Hsl24::Hsl24(const Hsl24 & p)
        : hue(p.hue)
        , saturation(p.saturation)
        , lightness(p.lightness)
    {
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE const Hsl24 & Hsl24::At(const View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Hsl24);

        return Simd::At<A, Hsl24>(view, col, row);
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE Hsl24 & Hsl24::At(View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Hsl24);

        return Simd::At<A, Hsl24>(view, col, row);
    }
    //}}}

    //{{{
    SIMD_INLINE Rgb24::Rgb24(const uint8_t & gray)
        : red(gray)
        , green(gray)
        , blue(gray)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgb24::Rgb24(const uint8_t & r, const uint8_t & g, const uint8_t & b)
        : red(r)
        , green(g)
        , blue(b)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgb24::Rgb24(const Bgra32 & p)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgb24::Rgb24(const Bgr24 & p)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgb24::Rgb24(const Rgba32& p)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgb24::Rgb24(const Rgb24 & p)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
    {
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE const Rgb24 & Rgb24::At(const View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Rgb24);

        return Simd::At<A, Rgb24>(view, col, row);
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE Rgb24 & Rgb24::At(View<A> & view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Rgb24);

        return Simd::At<A, Rgb24>(view, col, row);
    }
    //}}}

    //{{{
    SIMD_INLINE Rgba32::Rgba32(const uint8_t& gray, const uint8_t& a)
        : red(gray)
        , green(gray)
        , blue(gray)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgba32::Rgba32(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a)
        : red(r)
        , green(g)
        , blue(b)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgba32::Rgba32(const Bgra32& p)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
        , alpha(p.alpha)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgba32::Rgba32(const Bgr24& p, const uint8_t& a)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgba32::Rgba32(const Rgb24& p, const uint8_t& a)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
        , alpha(a)
    {
    }
    //}}}
    //{{{
    SIMD_INLINE Rgba32::Rgba32(const Rgba32& p)
        : red(p.red)
        , green(p.green)
        , blue(p.blue)
        , alpha(p.alpha)
    {
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE const Rgba32& Rgba32::At(const View<A>& view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Rgba32);

        return Simd::At<A, Rgba32>(view, col, row);
    }
    //}}}
    //{{{
    template <template<class> class A> SIMD_INLINE Rgba32& Rgba32::At(View<A>& view, ptrdiff_t col, ptrdiff_t row)
    {
        assert(view.format == View<A>::Rgba32);

        return Simd::At<A, Rgba32>(view, col, row);
    }
    //}}}
    }
  }
