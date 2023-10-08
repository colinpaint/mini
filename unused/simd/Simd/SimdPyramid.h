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
#pragma once
#include "Simd/SimdView.hpp"
#include <vector>

namespace Simd {
  template <template<class> class A> struct Pyramid {
    typedef A<uint8_t> Allocator; 
    Pyramid();
    Pyramid(const Point<ptrdiff_t> & size, size_t levelCount);
    Pyramid(size_t width, size_t height, size_t levelCount);
    void Recreate(Point<ptrdiff_t> size, size_t levelCount);
    void Recreate(size_t width, size_t height, size_t levelCount);
    size_t Size() const;
    View<A> & operator [] (size_t level);
    const View<A> & operator [] (size_t level) const;
    View<A> & At(size_t level);
    const View<A> & At(size_t level) const;
    View<A> & Top();
    const View<A> & Top() const;
    void Swap(Pyramid<A> & pyramid);
  private:
    std::vector< View<A> > _views;
    };

  Point<ptrdiff_t> Scale(Point<ptrdiff_t> size, int scale = 2);

  //{{{
  template <template<class> class A> SIMD_INLINE Pyramid<A>::Pyramid()
  {
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE Pyramid<A>::Pyramid(const Point<ptrdiff_t> & size, size_t levelCount)
  {
      Recreate(size, levelCount);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE Pyramid<A>::Pyramid(size_t width, size_t height, size_t levelCount)
  {
      Recreate(width, height, levelCount);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE void Pyramid<A>::Recreate(Point<ptrdiff_t> size, size_t levelCount)
  {
      if (_views.size() && size == _views[0].Size())
          return;
      _views.resize(levelCount);
      for (size_t level = 0; level < levelCount; ++level)
      {
          _views[level].Recreate(size, View<A>::Gray8);
          size = Scale(size);
      }
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE void Pyramid<A>::Recreate(size_t width, size_t height, size_t levelCount)
  {
      Recreate(Point<ptrdiff_t>(width, height), levelCount);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE size_t Pyramid<A>::Size() const
  {
      return _views.size();
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> & Pyramid<A>::operator [] (size_t level)
  {
      return _views[level];
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE const View<A> & Pyramid<A>::operator [] (size_t level) const
  {
      return _views[level];
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> & Pyramid<A>::At(size_t level)
  {
      return _views[level];
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE const View<A> & Pyramid<A>::At(size_t level) const
  {
      return _views[level];
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> & Pyramid<A>::Top()
  {
      return _views.back();
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE const View<A> & Pyramid<A>::Top() const
  {
      return _views.back();
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE void Pyramid<A>::Swap(Pyramid & pyramid)
  {
      _views.swap(pyramid._views);
  }
  //}}}

  //{{{
  SIMD_INLINE Point<ptrdiff_t> Scale(Point<ptrdiff_t> size, int scale)
  {
      while (scale > 1)
      {
          assert(scale % 2 == 0);
          size.x = (size.x + 1) >> 1;
          size.y = (size.y + 1) >> 1;
          scale >>= 1;
      }
      return size;
  }
  //}}}
  }
