//{{{
/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2018 Yermalayeu Ihar.
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
#include "SimdLib.h"
#include <math.h>
#ifdef SIMD_OPENCV_ENABLE
  #include "opencv2/core/core.hpp"
#endif

namespace Simd {
  //{{{
  template <typename T> struct Point {
    typedef T Type; /*!< Type definition. */
    T x; /*!< \brief Specifies the x-coordinate of a point. */
    T y; /*!< \brief Specifies the y-coordinate of a point. */

    Point();
    template <typename TX, typename TY> Point(TX tx, TY ty);
    template <class TP, template<class> class TPoint> Point(const TPoint<TP> & p);
    #ifdef SIMD_OPENCV_ENABLE
      template <class TS> Point(const cv::Size_<TS> & size);
    #endif

    ~Point();

    template <class TP, template<class> class TPoint> operator TPoint<TP>() const;
    template <typename TP> Point & operator = (const Point<TP> & p);
    template <typename TP> Point & operator += (const Point<TP> & p);
    template <typename TP> Point & operator -= (const Point<TP> & p);
    template <typename TA> Point & operator *= (const TA & a);
    Point & operator /= (double a);
    Point operator << (ptrdiff_t shift) const;
    Point operator >> (ptrdiff_t shift) const;
    };
  //}}}

  template <typename T> bool operator == (const Point<T> & p1, const Point<T> & p2);
  template <typename T> bool operator != (const Point<T> & p1, const Point<T> & p2);
  template <typename T> Point<T> operator + (const Point<T> & p1, const Point<T> & p2);
  template <typename T> Point<T> operator - (const Point<T> & p1, const Point<T> & p2);
  template <typename T> Point<T> operator * (const Point<T> & p1, const Point<T> & p2);
  template <typename T> Point<T> operator / (const Point<T> & p1, const Point<T> & p2);
  template <typename T> Point<T> operator - (const Point<T> & p);
  template <typename TP, typename TA> Point<TP> operator / (const Point<TP> & p, const TA & a);
  template <typename TP, typename TA> Point<TP> operator * (const Point<TP> & p, const TA & a);
  template <typename TP, typename TA> Point<TP> operator * (const TA & a, const Point<TP> & p);
  template <typename T> T SquaredDistance(const Point<T> & p1, const Point<T> & p2);
  template <typename T> double Distance(const Point<T> & p1, const Point<T> & p2);
  template <typename T> T DotProduct(const Point<T> & p1, const Point<T> & p2);
  template <typename T> T CrossProduct(const Point<T> & p1, const Point<T> & p2);

  #ifndef SIMD_ROUND
    #define SIMD_ROUND
    SIMD_INLINE int Round (double value) { return (int)(value + (value >= 0 ? 0.5 : -0.5)); }
  #endif

  //{{{
  template <class TD, class TS> SIMD_INLINE TD Convert(TS src)
  {
      return (TD)src;
  }
  //}}}
  //{{{
  template <> SIMD_INLINE ptrdiff_t Convert<ptrdiff_t, double>(double src)
  {
      return Round(src);
  }
  //}}}
  //{{{
  template <> SIMD_INLINE ptrdiff_t Convert<ptrdiff_t, float>(float src)
  {
      return Round(src);
  }

  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T>::Point()
      : x(0)
      , y(0)
  {
  }
  //}}}
  //{{{
  template <typename T> template <typename TX, typename TY> SIMD_INLINE Point<T>::Point(TX tx, TY ty)
      : x(Convert<T, TX>(tx))
      , y(Convert<T, TY>(ty))
  {
  }
  //}}}
  //{{{
  template <typename T> template <class TP, template<class> class TPoint> SIMD_INLINE Point<T>::Point(const TPoint<TP> & p)
      : x(Convert<T, TP>(p.x))
      , y(Convert<T, TP>(p.y))
  {
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T>::~Point()
  {
  }
  //}}}
  //{{{
  template <typename T> template <class TP, template<class> class TPoint> SIMD_INLINE Point<T>::operator TPoint<TP>() const
  {
      return TPoint<TP>(Convert<TP, T>(x), Convert<TP, T>(y));
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Point<T> & Point<T>::operator = (const Point<TP> & p)
  {
      x = Convert<T, TP>(p.x);
      y = Convert<T, TP>(p.y);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Point<T> & Point<T>::operator += (const Point<TP> & p)
  {
      x += Convert<T, TP>(p.x);
      y += Convert<T, TP>(p.y);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Point<T> & Point<T>::operator -= (const Point<TP> & p)
  {
      x -= Convert<T, TP>(p.x);
      y -= Convert<T, TP>(p.y);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TA> SIMD_INLINE Point<T> & Point<T>::operator *= (const TA & a)
  {
      x = Convert<T, TA>(x*a);
      y = Convert<T, TA>(y*a);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> & Point<T>::operator /= (double a)
  {
      x = Convert<T, double>(x / a);
      y = Convert<T, double>(y / a);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> Point<T>::operator << (ptrdiff_t shift) const
  {
      return Point<T>(x << shift, y << shift);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> Point<T>::operator >> (ptrdiff_t shift) const
  {
      return Point<T>(x >> shift, y >> shift);
  }
  //}}}

  // Point<T> utilities implementation:
  //{{{
  template <typename T> SIMD_INLINE bool operator == (const Point<T> & p1, const Point<T> & p2)
  {
      return p1.x == p2.x && p1.y == p2.y;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE bool operator != (const Point<T> & p1, const Point<T> & p2)
  {
      return p1.x != p2.x || p1.y != p2.y;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> operator + (const Point<T> & p1, const Point<T> & p2)
  {
      return Point<T>(p1.x + p2.x, p1.y + p2.y);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> operator - (const Point<T> & p1, const Point<T> & p2)
  {
      return Point<T>(p1.x - p2.x, p1.y - p2.y);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> operator * (const Point<T> & p1, const Point<T> & p2)
  {
      return Point<T>(p1.x * p2.x, p1.y * p2.y);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> operator / (const Point<T> & p1, const Point<T> & p2)
  {
      return Point<T>(p1.x / p2.x, p1.y / p2.y);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> operator - (const Point<T> & p)
  {
      return Point<T>(-p.x, -p.y);
  }
  //}}}
  //{{{
  template <typename TP, typename TA> SIMD_INLINE Point<TP> operator / (const Point<TP> & p, const TA & a)
  {
      return Point<TP>(p.x / a, p.y / a);
  }
  //}}}
  //{{{
  template <typename TP, typename TA> SIMD_INLINE Point<TP> operator * (const Point<TP> & p, const TA & a)
  {
      return Point<TP>(p.x*a, p.y*a);
  }
  //}}}
  //{{{
  template <typename TP, typename TA> SIMD_INLINE Point<TP> operator * (const TA & a, const Point<TP> & p)
  {
      return Point<TP>(p.x*a, p.y*a);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T SquaredDistance(const Point<T> & p1, const Point<T> & p2)
  {
      Point<T> dp = p2 - p1;
      return dp.x*dp.x + dp.y*dp.y;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE double Distance(const Point<T> & p1, const Point<T> & p2)
  {
      return ::sqrt(double(SquaredDistance(p1, p2)));
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T DotProduct(const Point<T> & p1, const Point<T> & p2)
  {
      return (p1.x * p2.x + p1.y * p2.y);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T CrossProduct(const Point<T> & p1, const Point<T> & p2)
  {
      return (p1.x * p2.y - p1.y * p2.x);
  }
  //}}}
  }
