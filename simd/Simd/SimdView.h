//{{{
/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2022 Yermalayeu Ihar,
*               2014-2019 Antonenka Mikhail,
*               2018-2019 Dmitry Fedorov,
*               2019-2019 Artur Voronkov,
*               2022-2022 Fabien Spindler,
*               2022-2022 Souriya Trinh.
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
#include "Simd/SimdRectangle.hpp"
#include "Simd/SimdAllocator.hpp"
#include <memory.h>
#include <assert.h>
#include <algorithm>
#include <fstream>

namespace Simd {
  template <template<class> class A> struct View {
    typedef A<uint8_t> Allocator;
    //{{{
    enum Format
    {
        /*! An undefined pixel format. */
        None = 0,
        /*! A 8-bit gray pixel format. */
        Gray8,
        /*! A 16-bit (2 8-bit channels) pixel format (UV plane of NV12 pixel format). */
        Uv16,
        /*! A 24-bit (3 8-bit channels) BGR (Blue, Green, Red) pixel format. */
        Bgr24,
        /*! A 32-bit (4 8-bit channels) BGRA (Blue, Green, Red, Alpha) pixel format. */
        Bgra32,
        /*! A single channel 16-bit integer pixel format. */
        Int16,
        /*! A single channel 32-bit integer pixel format. */
        Int32,
        /*! A single channel 64-bit integer pixel format. */
        Int64,
        /*! A single channel 32-bit float point pixel format. */
        Float,
        /*! A single channel 64-bit float point pixel format. */
        Double,
        /*! A 8-bit Bayer pixel format (GRBG). */
        BayerGrbg,
        /*! A 8-bit Bayer pixel format (GBRG). */
        BayerGbrg,
        /*! A 8-bit Bayer pixel format (RGGB). */
        BayerRggb,
        /*! A 8-bit Bayer pixel format (BGGR). */
        BayerBggr,
        /*! A 24-bit (3 8-bit channels) HSV (Hue, Saturation, Value) pixel format. */
        Hsv24,
        /*! A 24-bit (3 8-bit channels) HSL (Hue, Saturation, Lightness) pixel format. */
        Hsl24,
        /*! A 24-bit (3 8-bit channels) RGB (Red, Green, Blue) pixel format. */
        Rgb24,
        /*! A 32-bit (4 8-bit channels) RGBA (Red, Green, Blue, Alpha) pixel format. */
        Rgba32,
        /*! A 16-bit (2 8-bit channels) UYVY422 pixel format. */
        Uyvy16,
        /*! A 32-bit (4 8-bit channels) ARGB (Alpha, Red, Green, Blue) pixel format. */
        Argb32,
    };
    //}}}
    //{{{
    enum Position
    {
        TopLeft, /*!< A position in the top-left corner. */
        TopCenter, /*!< A position at the top center. */
        TopRight, /*!< A position in the top-right corner. */
        MiddleLeft, /*!< A position of the left in the middle. */
        MiddleCenter, /*!< A central position. */
        MiddleRight, /*!< A position of the right in the middle. */
        BottomLeft, /*!< A position in the bottom-left corner. */
        BottomCenter, /*!< A position at the bottom center. */
        BottomRight, /*!< A position in the bottom-right corner. */
    };
    //}}}

    const size_t width;
    const size_t height;
    const ptrdiff_t stride;
    const Format format;
    uint8_t* const data;

    View();
    View (const View & view);
    View (View&& view) noexcept;
    View (size_t w, size_t h, ptrdiff_t s, Format f, void * d);
    View (size_t w, size_t h, Format f, void * d = NULL, size_t align = Allocator::Alignment());
    View (const Point<ptrdiff_t> & size, Format f);
    ~View();

    View* Clone() const;

    View* Clone (const Rectangle<ptrdiff_t>& rect) const;
    View* Clone (View & buffer) const;
    View & operator = (const View & view);

    View& operator = (View&& view);
    View& Ref();

    void Recreate (size_t w, size_t h, Format f, void * d = NULL, size_t align = Allocator::Alignment());
    void Recreate (const Point<ptrdiff_t> & size, Format f);
    View Region (ptrdiff_t left, ptrdiff_t top, ptrdiff_t right, ptrdiff_t bottom) const;
    View Region (const Point<ptrdiff_t> & topLeft, const Point<ptrdiff_t> & bottomRight) const;
    View Region (const Rectangle<ptrdiff_t> & rect) const;
    View Region (const Point<ptrdiff_t> & size, Position position) const;

    View Flipped() const;
    Point<ptrdiff_t> Size() const;
    size_t DataSize() const;
    size_t Area() const;

    template <class T> const T & At (size_t x, size_t y) const;
    template <class T> T & At (size_t x, size_t y);
    template <class T> const T & At (const Point<ptrdiff_t> & p) const;
    template <class T> T & At (const Point<ptrdiff_t> & p);
    template <class T> const T * Row (size_t row) const;
    template <class T> T * Row (size_t row);

    static size_t PixelSize (Format format);
    size_t PixelSize() const;
    static size_t ChannelSize (Format format);
    size_t ChannelSize() const;
    static size_t ChannelCount (Format format);
    size_t ChannelCount() const;

    void Swap (View & other);
    bool Load (const std::string & path, Format format = None);
    bool Load (const uint8_t * src, size_t size, Format format = None);
    bool Save (const std::string & path, SimdImageFileType type = SimdImageFileUndefined, int quality = 100) const;
    void Clear();
    uint8_t* Release (size_t* size = NULL);
    bool Owner() const;
    void Capture();

  private:
    bool _owner;
    };

  template <template<class> class A, class T> const T & At (const View<A> & view, size_t x, size_t y);
  template <template<class> class A, class T> T & At (View<A> & view, size_t x, size_t y);
  template <template<class> class A, template<class> class B> bool EqualSize (const View<A> & a, const View<B> & b);
  template <template<class> class A> bool EqualSize (const View<A> & a, const View<A> & b, const View<A> & c);
  template <template<class> class A> bool EqualSize (const View<A> & a, const View<A> & b, const View<A> & c, const View<A> & d);
  template <template<class> class A, template<class> class B> bool Compatible (const View<A> & a, const View<B> & b);
  template <template<class> class A> bool Compatible (const View<A> & a, const View<A> & b, const View<A> & c);
  template <template<class> class A> bool Compatible (const View<A> & a, const View<A> & b, const View<A> & c, const View<A> & d);
  template <template<class> class A> bool Compatible (const View<A> & a, const View<A> & b, const View<A> & c, const View<A> & d, const View<A> & e);

  //{{{
  template <template<class> class A> SIMD_INLINE View<A>::View()
      : width(0)
      , height(0)
      , stride(0)
      , format(None)
      , data(NULL)
      , _owner(false)
  {
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>::View (const View<A> & view)
      : width(view.width)
      , height(view.height)
      , stride(view.stride)
      , format(view.format)
      , data(view.data)
      , _owner(false)
  {
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>::View (View<A> && view) noexcept
      : width(0)
      , height(0)
      , stride(0)
      , format(None)
      , data(NULL)
      , _owner(false)
  {
      Swap(view);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>::View (size_t w, size_t h, ptrdiff_t s, Format f, void * d)
      : width(w)
      , height(h)
      , stride(s)
      , format(f)
      , data((uint8_t*)d)
      , _owner(false)
  {
      if (data == NULL && height && width && stride && format != None)
      {
          *(void**)&data = Allocator::Allocate(height*stride, Allocator::Alignment());
          _owner = true;
      }
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>::View (size_t w, size_t h, Format f, void * d, size_t align)
      : width(0)
      , height(0)
      , stride(0)
      , format(None)
      , data(NULL)
      , _owner(false)
  {
      Recreate(w, h, f, d, align);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>::View (const Point<ptrdiff_t> & size, Format f)
      : width(0)
      , height(0)
      , stride(0)
      , format(None)
      , data(NULL)
      , _owner(false)
  {
      Recreate(size.x, size.y, f);
  }

  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>::~View()
  {
      if (_owner && data)
      {
          Allocator::Free(data);
      }
  }
  //}}}

  //{{{
  template <template<class> class A> SIMD_INLINE View<A> * View<A>::Clone() const
  {
      View<A> * view = new View<A>(width, height, format);
      size_t size = width*PixelSize();
      for (size_t row = 0; row < height; ++row)
          memcpy(view->data + view->stride*row, data + stride*row, size);
      return view;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>* View<A>::Clone (const Rectangle<ptrdiff_t>& rect) const
  {
      return Region(rect).Clone();
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> * View<A>::Clone (View & buffer) const
  {
      if (buffer.width < width || buffer.height < height)
          buffer.Recreate(width, height, format);

      View<A> * view = new View<A>(width, height, format, buffer.data);
      size_t size = width*PixelSize();
      for (size_t row = 0; row < height; ++row)
          memcpy(view->data + view->stride*row, data + stride*row, size);
      return view;
  }

  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> & View<A>::operator = (const View<A> & view)
  {
      if (this != &view)
      {
          if (_owner && data)
          {
              Allocator::Free(data);
              assert(0);
          }
          *(size_t*)&width = view.width;
          *(size_t*)&height = view.height;
          *(Format*)&format = view.format;
          *(ptrdiff_t*)&stride = view.stride;
          *(unsigned char**)&data = view.data;
          _owner = false;
      }
      return *this;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A>& View<A>::operator = (View<A>&& view)
  {
      if (this != &view)
      {
          Swap(view);
          view.Clear();
      }
      return *this;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> & View<A>::Ref()
  {
      return *this;
  }
  //}}}

  //{{{
  template <template<class> class A> SIMD_INLINE void View<A>::Recreate (size_t w, size_t h, Format f, void * d, size_t align)
  {
      if (_owner && data)
      {
          Allocator::Free(data);
          *(void**)&data = NULL;
          _owner = false;
      }
      *(size_t*)&width = w;
      *(size_t*)&height = h;
      *(Format*)&format = f;
      *(ptrdiff_t*)&stride = Allocator::Align(width*PixelSize(format), align);
      if (d)
      {
          *(void**)&data = Allocator::Align(d, align);
          _owner = false;
      }
      else if(height && stride)
      {
          *(void**)&data = Allocator::Allocate(height*stride, align);
          _owner = true;
      }
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE void View<A>::Recreate (const Point<ptrdiff_t> & size, Format f)
  {
      Recreate(size.x, size.y, f);
  }
  //}}}

  //{{{
  template <template<class> class A> SIMD_INLINE View<A> View<A>::Region (ptrdiff_t left, ptrdiff_t top, ptrdiff_t right, ptrdiff_t bottom) const
  {
      if (data != NULL && right >= left && bottom >= top)
      {
          left = std::min<ptrdiff_t>(std::max<ptrdiff_t>(left, 0), width);
          top = std::min<ptrdiff_t>(std::max<ptrdiff_t>(top, 0), height);
          right = std::min<ptrdiff_t>(std::max<ptrdiff_t>(right, 0), width);
          bottom = std::min<ptrdiff_t>(std::max<ptrdiff_t>(bottom, 0), height);
          return View<A>(right - left, bottom - top, stride, format, data + top*stride + left*PixelSize(format));
      }
      else
          return View<A>();
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> View<A>::Region (const Point<ptrdiff_t> & topLeft, const Point<ptrdiff_t> & bottomRight) const
  {
      return Region(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> View<A>::Region (const Rectangle<ptrdiff_t> & rect) const
  {
      return Region(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> View<A>::Region (const Point<ptrdiff_t> & size, Position position) const
  {
      ptrdiff_t w = width, h = height;
      switch (position)
      {
      case TopLeft:
          return Region(0, 0, size.x, size.y);
      case TopCenter:
          return Region((w - size.x) / 2, 0, (w + size.x) / 2, size.y);
      case TopRight:
          return Region(w - size.x, 0, w, size.y);
      case MiddleLeft:
          return Region(0, (h - size.y) / 2, size.x, (h + size.y) / 2);
      case MiddleCenter:
          return Region((w - size.x) / 2, (h - size.y) / 2, (w + size.x) / 2, (h + size.y) / 2);
      case MiddleRight:
          return Region(w - size.x, (h - size.y) / 2, w, (h + size.y) / 2);
      case BottomLeft:
          return Region(0, h - size.y, size.x, h);
      case BottomCenter:
          return Region((w - size.x) / 2, h - size.y, (w + size.x) / 2, h);
      case BottomRight:
          return Region(w - size.x, h - size.y, w, h);
      default:
          assert(0);
      }
      return View<A>();
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE View<A> View<A>::Flipped() const
  {
      return View<A>(width, height, -stride, format, data + (height - 1)*stride);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE Point<ptrdiff_t> View<A>::Size() const
  {
      return Point<ptrdiff_t>(width, height);
  }
  //}}}

  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::DataSize() const
  {
      return stride*height;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::Area() const
  {
      return width*height;
  }
  //}}}

  //{{{
  template <template<class> class A> template<class T> SIMD_INLINE const T & View<A>::At(size_t x, size_t y) const
  {
      assert(x < width && y < height);
      return ((const T*)(data + y*stride))[x];
  }
  //}}}
  //{{{
  template <template<class> class A> template<class T> SIMD_INLINE T & View<A>::At(size_t x, size_t y)
  {
      assert(x < width && y < height);
      return ((T*)(data + y*stride))[x];
  }
  //}}}
  //{{{
  template <template<class> class A> template<class T> SIMD_INLINE const T & View<A>::At(const Point<ptrdiff_t> & p) const
  {
      return At<T>(p.x, p.y);
  }
  //}}}
  //{{{
  template <template<class> class A> template<class T> SIMD_INLINE T & View<A>::At(const Point<ptrdiff_t> & p)
  {
      return At<T>(p.x, p.y);
  }
  //}}}
  //{{{
  template <template<class> class A> template<class T> SIMD_INLINE const T * View<A>::Row(size_t row) const
  {
      assert(row < height);
      return ((const T*)(data + row*stride));
  }
  //}}}
  //{{{
  template <template<class> class A> template<class T> SIMD_INLINE T * View<A>::Row(size_t row)
  {
      assert(row < height);
      return ((T*)(data + row*stride));
  }
  //}}}

  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::PixelSize (Format format)
  {
      switch (format)
      {
      case None:      return 0;
      case Gray8:     return 1;
      case Uv16:      return 2;
      case Bgr24:     return 3;
      case Bgra32:    return 4;
      case Int16:     return 2;
      case Int32:     return 4;
      case Int64:     return 8;
      case Float:     return 4;
      case Double:    return 8;
      case BayerGrbg: return 1;
      case BayerGbrg: return 1;
      case BayerRggb: return 1;
      case BayerBggr: return 1;
      case Hsv24:     return 3;
      case Hsl24:     return 3;
      case Rgb24:     return 3;
      case Rgba32:    return 4;
      case Uyvy16:    return 2;
      case Argb32:    return 4;
      default: assert(0); return 0;
      }
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::PixelSize() const
  {
      return PixelSize(format);
  }
  //}}}

  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::ChannelSize (Format format)
  {
      switch (format)
      {
      case None:      return 0;
      case Gray8:     return 1;
      case Uv16:      return 1;
      case Bgr24:     return 1;
      case Bgra32:    return 1;
      case Int16:     return 2;
      case Int32:     return 4;
      case Int64:     return 8;
      case Float:     return 4;
      case Double:    return 8;
      case BayerGrbg: return 1;
      case BayerGbrg: return 1;
      case BayerRggb: return 1;
      case BayerBggr: return 1;
      case Hsv24:     return 1;
      case Hsl24:     return 1;
      case Rgb24:     return 1;
      case Rgba32:    return 1;
      case Uyvy16:    return 1;
      case Argb32:    return 1;
      default: assert(0); return 0;
      }
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::ChannelSize() const
  {
      return ChannelSize(format);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::ChannelCount(Format format)
  {
      switch (format)
      {
      case None:      return 0;
      case Gray8:     return 1;
      case Uv16:      return 2;
      case Bgr24:     return 3;
      case Bgra32:    return 4;
      case Int16:     return 1;
      case Int32:     return 1;
      case Int64:     return 1;
      case Float:     return 1;
      case Double:    return 1;
      case BayerGrbg: return 1;
      case BayerGbrg: return 1;
      case BayerRggb: return 1;
      case BayerBggr: return 1;
      case Hsv24:     return 3;
      case Hsl24:     return 3;
      case Rgb24:     return 3;
      case Rgba32:    return 4;
      case Uyvy16:    return 2;
      case Argb32:    return 4;
      default: assert(0); return 0;
      }
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE size_t View<A>::ChannelCount() const
  {
      return ChannelCount(format);
  }
  //}}}

  //{{{
  template <template<class> class A> SIMD_INLINE void View<A>::Swap (View<A> & other)
  {
      std::swap((size_t&)width, (size_t&)other.width);
      std::swap((size_t&)height, (size_t&)other.height);
      std::swap((ptrdiff_t&)stride, (ptrdiff_t&)other.stride);
      std::swap((Format&)format, (Format&)other.format);
      std::swap((uint8_t*&)data, (uint8_t*&)other.data);
      std::swap((bool&)_owner, (bool&)other._owner);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool View<A>::Load (const std::string & path, Format format_)
  {
      Clear();
      (Format&)format = format_;
      *(uint8_t**)&data = SimdImageLoadFromFile(path.c_str(), (size_t*)&stride, (size_t*)&width, (size_t*)&height, (SimdPixelFormatType*)&format);
      if (data)
          _owner = true;
      else
          (Format&)format = None;
      return _owner;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool View<A>::Load (const uint8_t * src, size_t size, Format format_)
  {
      Clear();
      (Format&)format = format_;
      *(uint8_t**)&data = SimdImageLoadFromMemory(src, size, (size_t*)&stride, (size_t*)&width, (size_t*)&height, (SimdPixelFormatType*)&format);
      if (data)
          _owner = true;
      else
          (Format&)format = None;
      return _owner;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool View<A>::Save (const std::string & path, SimdImageFileType type, int quality) const
  {
      return SimdImageSaveToFile(data, stride, width, height, (SimdPixelFormatType)format, type, quality, path.c_str()) == SimdTrue;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE void View<A>::Clear()
  {
      if (_owner && data)
          Allocator::Free(data);
      *(void**)&data = nullptr;
      _owner = false;
      *(size_t*)&width = 0;
      *(size_t*)&height = 0;
      *(ptrdiff_t *)&stride = 0;
      *(Format*)&format = Format::None;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE uint8_t* View<A>::Release (size_t* size)
  {
      uint8_t* released = data;
      if (size)
          *size = DataSize();
      _owner = false;
      Clear();
      return released;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool View<A>::Owner() const
  {
      return _owner;
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE void View<A>::Capture()
  {
      if (data && _owner == false)
      {
          View<A> copy(width, height, format);
          size_t size = width * PixelSize();
          for (size_t row = 0; row < height; ++row)
              memcpy(copy.data + copy.stride * row, data + stride * row, size);
          Swap(copy);
      }
  }
  //}}}

  //{{{
  template <template<class> class A, class T> const T & At(const View<A> & view, size_t x, size_t y)
  {
      assert(x < view.width && y < view.height);

      return ((const T*)(view.data + y*view.stride))[x];
  }
  //}}}
  //{{{
  template <template<class> class A, class T> T & At(View<A> & view, size_t x, size_t y)
  {
      assert(x < view.width && y < view.height);

      return ((T*)(view.data + y*view.stride))[x];
  }
  //}}}
  //{{{
  template <template<class> class A, template<class> class B> SIMD_INLINE bool EqualSize(const View<A> & a, const View<B> & b)
  {
      return
          (a.width == b.width && a.height == b.height);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool EqualSize(const View<A> & a, const View<A> & b, const View<A> & c)
  {
      return
          (a.width == b.width && a.height == b.height) &&
          (a.width == c.width && a.height == c.height);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool EqualSize(const View<A> & a, const View<A> & b, const View<A> & c, const View<A> & d)
  {
      return
          (a.width == b.width && a.height == b.height) &&
          (a.width == c.width && a.height == c.height) &&
          (a.width == d.width && a.height == d.height);
  }
  //}}}
  //{{{
  template <template<class> class A, template<class> class B> SIMD_INLINE bool Compatible(const View<A> & a, const View<B> & b)
  {
      typedef typename View<A>::Format Format;

      return
          (a.width == b.width && a.height == b.height && a.format == (Format)b.format);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool Compatible(const View<A> & a, const View<A> & b, const View<A> & c)
  {
      return
          (a.width == b.width && a.height == b.height && a.format == b.format) &&
          (a.width == c.width && a.height == c.height && a.format == c.format);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool Compatible(const View<A> & a, const View<A> & b, const View<A> & c, const View<A> & d)
  {
      return
          (a.width == b.width && a.height == b.height && a.format == b.format) &&
          (a.width == c.width && a.height == c.height && a.format == c.format) &&
          (a.width == d.width && a.height == d.height && a.format == d.format);
  }
  //}}}
  //{{{
  template <template<class> class A> SIMD_INLINE bool Compatible(const View<A> & a, const View<A> & b, const View<A> & c, const View<A> & d, const View<A> & e)
  {
      return
          (a.width == b.width && a.height == b.height && a.format == b.format) &&
          (a.width == c.width && a.height == c.height && a.format == c.format) &&
          (a.width == d.width && a.height == d.height && a.format == d.format) &&
          (a.width == e.width && a.height == e.height && a.format == e.format);
  }
  //}}}
  }
