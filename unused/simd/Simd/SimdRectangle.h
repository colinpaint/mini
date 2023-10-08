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
#prgama once
#include "Simd/SimdPoint.h"
#include <algorithm>

namespace Simd {
  template <typename T> struct Rectangle {
    typedef T Type;
    T left;
    T top;
    T right;
    T bottom;

    Rectangle();
    template <typename TL, typename TT, typename TR, typename TB> Rectangle(TL l, TT t, TR r, TB b);
    template <typename TLT, typename TRB> Rectangle(const Point<TLT> & lt, const Point<TRB> & rb);
    template <typename TRB> Rectangle(const Point<TRB> & rb);
    template <class TR, template<class> class TRectangle> Rectangle(const TRectangle<TR> & r);

    ~Rectangle();

    template <class TR, template<class> class TRectangle> operator TRectangle<TR>() const;

    template <typename TR> Rectangle<T> & operator = (const Rectangle<TR> & r);

    template <typename TL> Rectangle<T> & SetLeft(const TL & l);
    template <typename TT> Rectangle<T> & SetTop(const TT & t);
    template <typename TR> Rectangle<T> & SetRight(const TR & r);
    template <typename TB> Rectangle<T> & SetBottom(const TB & b);
    template <typename TP> Rectangle<T> & SetTopLeft(const Point<TP> & topLeft);
    template <typename TP> Rectangle<T> & SetTopRight(const Point<TP> & topRight);
    template <typename TP> Rectangle<T> & SetBottomLeft(const Point<TP> & bottomLeft);
    template <typename TP> Rectangle<T> & SetBottomRight(const Point<TP> & bottomRight);

    T Left() const;
    T Top() const;
    T Right() const;
    T Bottom() const;

    Point<T> TopLeft() const;
    Point<T> TopRight() const;
    Point<T> BottomLeft() const;
    Point<T> BottomRight() const;

    T Width() const;
    T Height() const;
    T Area() const;

    bool Empty() const;
    Point<T> Size() const;
    Point<T> Center() const;

    template <typename TX, typename TY> bool Contains(TX x, TY y) const;
    template <typename TP> bool Contains(const Point<TP> & p) const;
    template <typename TL, typename TT, typename TR, typename TB> bool Contains(TL l, TT t, TR r, TB b) const;
    template <typename TR> bool Contains(const Rectangle <TR> & r) const;

    template <typename TP> Rectangle<T> & Shift(const Point<TP> & shift);
    template <typename TX, typename TY> Rectangle<T> & Shift(TX shiftX, TY shiftY);

    template <typename TP> Rectangle<T> Shifted(const Point<TP> & shift) const;
    template <typename TX, typename TY> Rectangle<T> Shifted(TX shiftX, TY shiftY) const;

    template <typename TB> Rectangle<T> & AddBorder(TB border);
    template <typename TR> Rectangle<T> Intersection(const Rectangle<TR> & r) const;

    template <typename TP> Rectangle<T> & operator &= (const Point<TP> & p);
    template <typename TR> Rectangle<T> & operator &= (const Rectangle<TR> & r);
    template <typename TP> Rectangle<T> & operator |= (const Point<TP> & p);
    template <typename TR> Rectangle<T> & operator |= (const Rectangle<TR> & r);
    template <typename TR> Rectangle<T> & operator += (const Rectangle<TR> & r);
    bool Overlaps(const Rectangle<T> & r) const;
    };

  template <typename T> bool operator == (const Rectangle<T> & r1, const Rectangle<T> & r2);
  template <typename T> bool operator != (const Rectangle<T> & r1, const Rectangle<T> & r2);
  template <class T1, class T2> Rectangle<T1> operator / (const Rectangle<T1> & rect, const T2 & value);
  template <class T1, class T2> Rectangle<T1> operator * (const Rectangle<T1> & rect, const T2 & value);
  template <class T1, class T2> Rectangle<T1> operator * (const T2 & value, const Rectangle<T1> & rect);
  template <typename T> Rectangle<T> operator + (const Rectangle<T> & r1, const Rectangle<T> & r2);

  //{{{
  template <typename T> SIMD_INLINE Rectangle<T>::Rectangle()
      : left(0)
      , top(0)
      , right(0)
      , bottom(0)
  {
  }
  //}}}
  //{{{
  template <typename T> template <typename TL, typename TT, typename TR, typename TB> SIMD_INLINE Rectangle<T>::Rectangle (TL l, TT t, TR r, TB b)
      : left(Convert<T, TL>(l))
      , top(Convert<T, TT>(t))
      , right(Convert<T, TR>(r))
      , bottom(Convert<T, TB>(b))
  {
  }
  //}}}
  //{{{
  template <typename T> template <typename TLT, typename TRB> SIMD_INLINE Rectangle<T>::Rectangle (const Point<TLT> & lt, const Point<TRB> & rb)
      : left(Convert<T, TLT>(lt.x))
      , top(Convert<T, TLT>(lt.y))
      , right(Convert<T, TRB>(rb.x))
      , bottom(Convert<T, TRB>(rb.y))
  {
  }
  //}}}
  //{{{
  template <typename T> template <typename TRB> SIMD_INLINE Rectangle<T>::Rectangle (const Point<TRB> & rb)
      : left(0)
      , top(0)
      , right(Convert<T, TRB>(rb.x))
      , bottom(Convert<T, TRB>(rb.y))
  {
  }
  //}}}
  //{{{
  template <typename T> template <class TR, template<class> class TRectangle> SIMD_INLINE Rectangle<T>::Rectangle (const TRectangle<TR> & r)
      : left(Convert<T, TR>(r.left))
      , top(Convert<T, TR>(r.top))
      , right(Convert<T, TR>(r.right))
      , bottom(Convert<T, TR>(r.bottom))
  {
  }
  //}}}

  //{{{
  template <typename T> SIMD_INLINE Rectangle<T>::~Rectangle()
  {
  }
  //}}}

  //{{{
  template <typename T> template <class TR, template<class> class TRectangle> SIMD_INLINE Rectangle<T>::operator TRectangle<TR>() const
  {
      return TRectangle<TR>(Convert<TR, T>(left), Convert<TR, T>(top),
          Convert<TR, T>(right), Convert<TR, T>(bottom));
  }
  //}}}
  //{{{
  template <typename T> template <typename TR> SIMD_INLINE Rectangle<T>& Rectangle<T>::operator = (const Rectangle<TR> & r)
  {
      left = Convert<T, TR>(r.left);
      top = Convert<T, TR>(r.top);
      right = Convert<T, TR>(r.right);
      bottom = Convert<T, TR>(r.bottom);
      return *this;
  }
  //}}}

  //{{{
  template <typename T> template <typename TL> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetLeft(const TL & l)
  {
      left = Convert<T, TL>(l);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TT> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetTop(const TT & t)
  {
      top = Convert<T, TT>(t);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TR> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetRight(const TR & r)
  {
      right = Convert<T, TR>(r);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TB> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetBottom(const TB & b)
  {
      bottom = Convert<T, TB>(b);
      return *this;
  }
  //}}}

  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetTopLeft(const Point<TP> & topLeft)
  {
      left = Convert<T, TP>(topLeft.x);
      top = Convert<T, TP>(topLeft.y);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetTopRight(const Point<TP> & topRight)
  {
      right = Convert<T, TP>(topRight.x);
      top = Convert<T, TP>(topRight.y);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetBottomLeft(const Point<TP> & bottomLeft)
  {
      left = Convert<T, TP>(bottomLeft.x);
      bottom = Convert<T, TP>(bottomLeft.y);
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T>& Rectangle<T>::SetBottomRight(const Point<TP> & bottomRight)
  {
      right = Convert<T, TP>(bottomRight.x);
      bottom = Convert<T, TP>(bottomRight.y);
      return *this;
  }
  //}}}

  //{{{
  template <typename T> SIMD_INLINE T Rectangle<T>::Left() const
  {
      return left;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T Rectangle<T>::Top() const
  {
      return top;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T Rectangle<T>::Right() const
  {
      return right;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T Rectangle<T>::Bottom() const
  {
      return bottom;
  }
  //}}}

  //{{{
  template <typename T> SIMD_INLINE Point<T> Rectangle<T>::TopLeft() const
  {
      return Point<T>(left, top);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> Rectangle<T>::TopRight() const
  {
      return Point<T>(right, top);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> Rectangle<T>::BottomLeft() const
  {
      return Point<T>(left, bottom);
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> Rectangle<T>::BottomRight() const
  {
      return Point<T>(right, bottom);
  }
  //}}}

  //{{{
  template <typename T> SIMD_INLINE bool Rectangle<T>::Empty() const
  {
      return Area() == 0;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T Rectangle<T>::Width() const
  {
      return right - left;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T Rectangle<T>::Height() const
  {
      return bottom - top;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE T Rectangle<T>::Area() const
  {
      return Width()*Height();
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> Rectangle<T>::Size() const
  {
      return Point<T>(Width(), Height());
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE Point<T> Rectangle<T>::Center() const
  {
      return Point<T>((left + right) / 2.0, (top + bottom) / 2.0);
  }
  //}}}

  // contains
  //{{{
  template <typename T> template <typename TX, typename TY> SIMD_INLINE bool Rectangle<T>::Contains (TX x, TY y) const
  {
      Point<T> p(x, y);
      return p.x >= left && p.x < right && p.y >= top && p.y < bottom;
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE bool Rectangle<T>::Contains (const Point<TP> & p) const
  {
      return Contains(p.x, p.y);
  }
  //}}}
  //{{{
  template <typename T> template <typename TL, typename TT, typename TR, typename TB> SIMD_INLINE bool Rectangle<T>::Contains (TL l, TT t, TR r, TB b) const
  {
      Rectangle<T> rect(l, t, r, b);
      return rect.left >= left && rect.right <= right && rect.top >= top && rect.bottom <= bottom;
  }
  //}}}
  //{{{
  template <typename T> template <typename TR> SIMD_INLINE bool Rectangle<T>::Contains (const Rectangle <TR> & r) const
  {
      return Contains(r.left, r.top, r.right, r.bottom);
  }
  //}}}

  // shift
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T>& Rectangle<T>::Shift (const Point<TP> & shift)
  {
      return Shift(shift.x, shift.y);
  }
  //}}}
  //{{{
  template <typename T> template <typename TX, typename TY> SIMD_INLINE Rectangle<T>& Rectangle<T>::Shift (TX shiftX, TY shiftY)
  {
      Point<T> shift(shiftX, shiftY);
      left += shift.x;
      top += shift.y;
      right += shift.x;
      bottom += shift.y;
      return *this;
  }
  //}}}
  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T> Rectangle<T>::Shifted (const Point<TP> & shift) const
  {
      return Shifted(shift.x, shift.y);
  }
  //}}}
  //{{{
  template <typename T> template <typename TX, typename TY> SIMD_INLINE Rectangle<T> Rectangle<T>::Shifted (TX shiftX, TY shiftY) const
  {
      Point<T> shift(shiftX, shiftY);
      return Rectangle<T>(left + shift.x, top + shift.y, right + shift.x, bottom + shift.y);
  }
  //}}}

  //{{{
  template <typename T> template <typename TB> SIMD_INLINE Rectangle<T>& Rectangle<T>::AddBorder (TB border) {

    T _border = Convert<T, TB>(border);
    left -= _border;
    top -= _border;
    right += _border;
    bottom += _border;
    return *this;
    }
  //}}}
  //{{{
  template <typename T> template <typename TR> SIMD_INLINE Rectangle<T> Rectangle<T>::Intersection (const Rectangle<TR>& rect) const {

    Rectangle<T> _rect(rect);
    T l = std::max(left, _rect.left);
    T t = std::max(top, _rect.top);
    T r = std::max(l, std::min(right, _rect.right));
    T b = std::max(t, std::min(bottom, _rect.bottom));
    return Rectangle(l, t, r, b);
    }
  //}}}

  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T>& Rectangle<T>::operator &= (const Point<TP> & p)
  {
    Point<T> _p(p);

    if (Contains (_p)) {
      left = _p.x;
      top = _p.y;
      right = _p.x + 1;
      bottom = _p.y + 1;
      }
    else {
      bottom = top;
      right = left;
      }

    return *this;
    }
  //}}}
  //{{{
  template <typename T> template <typename TR> SIMD_INLINE Rectangle<T>& Rectangle<T>::operator &= (const Rectangle<TR> & r)
  {
    if (Empty())
      return *this;

    if (r.Empty())
      return this->operator=(r);

    Rectangle<T> _r(r);
    if (left < _r.left)
      left = std::min(_r.left, right);

    if (top < _r.top)
      top = std::min(_r.top, bottom);

    if (right > _r.right)
      right = std::max(_r.right, left);

    if (bottom > _r.bottom)
      bottom = std::max(_r.bottom, top);

    return *this;
    }
  //}}}

  //{{{
  template <typename T> template <typename TP> SIMD_INLINE Rectangle<T>& Rectangle<T>::operator |= (const Point<TP> & p) {

    Point<T> _p(p);
    if (Empty()) {
      left = _p.x;
      top = _p.y;
      right = _p.x + 1;
      bottom = _p.y + 1;
      }
    else {
      if (left > _p.x)
        left = _p.x;
      if (top > _p.y)
        top = _p.y;
      if (right <= _p.x)
        right = _p.x + 1;
      if (bottom <= _p.y)
        bottom = _p.y + 1;
      }

    return *this;
    }
  //}}}
  //{{{
  template <typename T> template <typename TR> SIMD_INLINE Rectangle<T>& Rectangle<T>::operator |= (const Rectangle<TR>& r) {

    if (Empty())
      return this->operator=(r);

    if (r.Empty())
      return *this;

    Rectangle<T> _r(r);
    left = std::min(left, _r.left);
    top = std::min(top, _r.top);
    right = std::max(right, _r.right);
    bottom = std::max(bottom, _r.bottom);
    return *this;
    }
  //}}}

  //{{{
  template <typename T> template <typename TR> SIMD_INLINE Rectangle<T>& Rectangle<T>::operator += (const Rectangle<TR> & r)
  {
      left += Convert<T, TR>(r.left);
      top += Convert<T, TR>(r.top);
      right += Convert<T, TR>(r.right);
      bottom += Convert<T, TR>(r.bottom);
      return *this;
  }
  //}}}

  //{{{
  template <typename T> SIMD_INLINE bool Rectangle<T>::Overlaps (const Rectangle<T>& r) const
  {
      bool lr = left < r.right;
      bool rl = right > r.left;
      bool tb = top < r.bottom;
      bool bt = bottom > r.top;
      return (lr == rl) && (tb == bt);
  }
  //}}}

  //{{{
  template <typename T> SIMD_INLINE bool operator == (const Rectangle<T>& r1, const Rectangle<T>& r2)
  {
      return r1.left == r2.left && r1.top == r2.top && r1.right == r2.right && r1.bottom == r2.bottom;
  }
  //}}}
  //{{{
  template <typename T> SIMD_INLINE bool operator != (const Rectangle<T>& r1, const Rectangle<T>& r2)
  {
      return r1.left != r2.left || r1.top != r2.top || r1.right != r2.right || r1.bottom != r2.bottom;
  }
  //}}}

  //{{{
  template <class T1, class T2> SIMD_INLINE Rectangle<T1> operator / (const Rectangle<T1>& rect, const T2& value)
  {
      return Rectangle<T1>(rect.left / value, rect.top / value, rect.right / value, rect.bottom / value);
  }
  //}}}
  //{{{
  template <class T1, class T2> SIMD_INLINE Rectangle<T1> operator * (const Rectangle<T1>& rect, const T2& value)
  {
      return Rectangle<T1>(rect.left*value, rect.top*value, rect.right*value, rect.bottom*value);
  }
  //}}}
  //{{{
  template <class T1, class T2> SIMD_INLINE Rectangle<T1> operator * (const T2& value, const Rectangle<T1>& rect)
  {
      return Rectangle<T1>(rect.left*value, rect.top*value, rect.right*value, rect.bottom*value);
  }
  //}}}
  //{{{
  template <class T> SIMD_INLINE Rectangle<T> operator + (const Rectangle<T>& r1, const Rectangle<T>& r2)
  {
      return Rectangle<T>(r1.left + r2.left, r1.top + r2.top, r1.right + r2.right, r1.bottom + r2.bottom);
  }
  //}}}
  }
