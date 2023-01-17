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
#include "SimdResizer.h"
#include "SimdResizerCommon.h"
#include "SimdSet.h"
#include "SimdUpdate.h"
#include "SimdUnpack.h"
#include "SimdCopyPixel.h"

namespace Simd::Sse41 {
  //{{{
  ResizerByteArea1x1::ResizerByteArea1x1 (const ResParam & param)
      : Base::ResizerByteArea1x1(param)
  {
  }
  //}}}

  //{{{
  SIMD_INLINE __m128i SaveLoadTail(const uint8_t * ptr, size_t tail)
  {
      uint8_t buffer[DA];
      _mm_storeu_si128((__m128i*)(buffer), _mm_loadu_si128((__m128i*)(ptr + tail - A)));
      return _mm_loadu_si128((__m128i*)(buffer + A - tail));
  }
  //}}}
  //{{{
  template<UpdateType update> SIMD_INLINE void ResizerByteArea1x1RowUpdate (const uint8_t * src0, size_t size, int32_t a, int32_t * dst)
  {
      if (update == UpdateAdd && a == 0)
          return;
      __m128i alpha = SetInt16(a, a);
      size_t sizeA = AlignLo(size, A);
      size_t i = 0;
      for (; i < sizeA; i += A, dst += A)
      {
          __m128i s0 = _mm_loadu_si128((__m128i*)(src0 + i));
          __m128i i0 = UnpackU8<0>(s0);
          __m128i i1 = UnpackU8<1>(s0);
          Update<update, true>(dst + 0 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i0)));
          Update<update, true>(dst + 1 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i0)));
          Update<update, true>(dst + 2 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i1)));
          Update<update, true>(dst + 3 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i1)));
      }
      if (i < size)
      {
          __m128i s0 = SaveLoadTail(src0 + i, size - i);
          __m128i i0 = UnpackU8<0>(s0);
          __m128i i1 = UnpackU8<1>(s0);
          Update<update, true>(dst + 0 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i0)));
          Update<update, true>(dst + 1 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i0)));
          Update<update, true>(dst + 2 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i1)));
          Update<update, true>(dst + 3 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i1)));
      }
  }
  //}}}
  //{{{
  template<UpdateType update> SIMD_INLINE void ResizerByteArea1x1RowUpdate (const uint8_t * src0, size_t stride, size_t size, int32_t a0, int32_t a1, int32_t * dst)
  {
      __m128i alpha = SetInt16(a0, a1);
      const uint8_t * src1 = src0 + stride;
      size_t sizeA = AlignLo(size, A);
      size_t i = 0;
      for (; i < sizeA; i += A, dst += A)
      {
          __m128i s0 = _mm_loadu_si128((__m128i*)(src0 + i));
          __m128i s1 = _mm_loadu_si128((__m128i*)(src1 + i));
          __m128i i0 = UnpackU8<0>(s0, s1);
          __m128i i1 = UnpackU8<1>(s0, s1);
          Update<update, true>(dst + 0 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i0)));
          Update<update, true>(dst + 1 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i0)));
          Update<update, true>(dst + 2 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i1)));
          Update<update, true>(dst + 3 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i1)));
      }
      if (i < size)
      {
          __m128i s0 = _mm_loadu_si128((__m128i*)(src0 + i));
          __m128i s1 = SaveLoadTail(src1 + i, size - i);
          __m128i i0 = UnpackU8<0>(s0, s1);
          __m128i i1 = UnpackU8<1>(s0, s1);
          Update<update, true>(dst + 0 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i0)));
          Update<update, true>(dst + 1 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i0)));
          Update<update, true>(dst + 2 * F, _mm_madd_epi16(alpha, UnpackU8<0>(i1)));
          Update<update, true>(dst + 3 * F, _mm_madd_epi16(alpha, UnpackU8<1>(i1)));
      }
  }
  //}}}
  //{{{
  SIMD_INLINE void ResizerByteArea1x1RowSum (const uint8_t * src, size_t stride, size_t count, size_t size, int32_t curr, int32_t zero, int32_t next, int32_t * dst)
  {
      if (count)
      {
          size_t i = 0;
          ResizerByteArea1x1RowUpdate<UpdateSet>(src, stride, size, curr, count == 1 ? zero - next : zero, dst), src += 2 * stride, i +=2;
          for (; i < count; i += 2, src += 2 * stride)
              ResizerByteArea1x1RowUpdate<UpdateAdd>(src, stride, size, zero, i == count - 1 ? zero - next : zero, dst);
          if (i == count)
              ResizerByteArea1x1RowUpdate<UpdateAdd>(src, size, zero - next, dst);
      }
      else
          ResizerByteArea1x1RowUpdate<UpdateSet>(src, size, curr - next, dst);
  }
  //}}}
  //{{{
  template<size_t N> void ResizerByteArea1x1::Run (const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      size_t bodyW = _param.dstW - (N == 3 ? 1 : 0), rowSize = _param.srcW * N, rowRest = dstStride - _param.dstW * N;
      const int32_t * iy = _iy.data, * ix = _ix.data, * ay = _ay.data, * ax = _ax.data;
      int32_t ay0 = ay[0], ax0 = ax[0];
      for (size_t dy = 0; dy < _param.dstH; dy++, dst += rowRest)
      {
          int32_t * buf = _by.data;
          size_t yn = iy[dy + 1] - iy[dy];
          ResizerByteArea1x1RowSum(src, srcStride, yn, rowSize, ay[dy], ay0, ay[dy + 1], buf), src += yn * srcStride;
          size_t dx = 0;
          for (; dx < bodyW; dx++, dst += N)
          {
              size_t xn = ix[dx + 1] - ix[dx];
              Sse41::ResizerByteAreaResult<N>(buf, xn, ax[dx], ax0, ax[dx + 1], dst), buf += xn * N;
          }
          for (; dx < _param.dstW; dx++, dst += N)
          {
              size_t xn = ix[dx + 1] - ix[dx];
              Base::ResizerByteAreaResult<N>(buf, xn, ax[dx], ax0, ax[dx + 1], dst), buf += xn * N;
          }
      }
  }
  //}}}
  //{{{
  void ResizerByteArea1x1::Run (const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      switch (_param.channels)
      {
      case 1: Run<1>(src, srcStride, dst, dstStride); return;
      case 2: Run<2>(src, srcStride, dst, dstStride); return;
      case 3: Run<3>(src, srcStride, dst, dstStride); return;
      case 4: Run<4>(src, srcStride, dst, dstStride); return;
      default:
          assert(0);
      }
  }
  //}}}
  //{{{
  ResizerByteArea2x2::ResizerByteArea2x2 (const ResParam& param)
      : Base::ResizerByteArea2x2(param)
  {
  }
  //}}}

  //{{{
  template<size_t N> SIMD_INLINE __m128i ShuffleColor (__m128i val)
  {
      return val;
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i ShuffleColor <2> (__m128i val)
  {
      static const __m128i IDX = SIMD_MM_SETR_EPI8(0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF);
      return _mm_shuffle_epi8(val, IDX);
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i ShuffleColor <4> (__m128i val)
  {
      static const __m128i IDX = SIMD_MM_SETR_EPI8(0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF);
      return _mm_shuffle_epi8(val, IDX);
  }
  //}}}
  //{{{
  template<size_t N> SIMD_INLINE __m128i SaveLoadTail2x2 (const uint8_t* ptr, size_t tail)
  {
      uint8_t buffer[DA];
      _mm_storeu_si128((__m128i*)(buffer), LoadAfterLast<N>(_mm_loadu_si128((__m128i*)(ptr + tail - A))));
      return _mm_loadu_si128((__m128i*)(buffer + A - tail - N));
  }
  //}}}
  //{{{
  template<size_t N, UpdateType update> SIMD_INLINE void ResizerByteArea2x2RowUpdateColor (const uint8_t* src0, const uint8_t* src1, size_t size, int32_t val, int32_t* dst)
  {
      if (update == UpdateAdd && val == 0)
          return;
      size_t size4F = AlignLoAny(size, 4 * F);
      __m128i _val = _mm_set1_epi16(val);
      size_t i = 0;
      for (; i < size4F; i += 4 * F, dst += 2 * F)
      {
          __m128i s0 = _mm_maddubs_epi16(ShuffleColor<N>(_mm_loadu_si128((__m128i*)(src0 + i))), K8_01);
          __m128i s1 = _mm_maddubs_epi16(ShuffleColor<N>(_mm_loadu_si128((__m128i*)(src1 + i))), K8_01);
          Update<update, false>(dst + 0, _mm_madd_epi16(_mm_unpacklo_epi16(s0, s1), _val));
          Update<update, false>(dst + F, _mm_madd_epi16(_mm_unpackhi_epi16(s0, s1), _val));
      }
      if (i < size)
      {
          size_t tail = size - i;
          __m128i s0 = _mm_maddubs_epi16(ShuffleColor<N>(SaveLoadTail2x2<N>(src0 + i, tail)), K8_01);
          __m128i s1 = _mm_maddubs_epi16(ShuffleColor<N>(SaveLoadTail2x2<N>(src1 + i, tail)), K8_01);
          Update<update, false>(dst + 0, _mm_madd_epi16(_mm_unpacklo_epi16(s0, s1), _val));
          Update<update, false>(dst + F, _mm_madd_epi16(_mm_unpackhi_epi16(s0, s1), _val));
      }
  }
  //}}}
  //{{{
  template<size_t N> SIMD_INLINE void ResizerByteArea2x2RowSum (const uint8_t* src, size_t stride, size_t count, size_t size, int32_t curr, int32_t zero, int32_t next, bool tail, int32_t* dst)
  {
      size_t c = 0;
      if (count)
      {
          ResizerByteArea2x2RowUpdateColor<N, UpdateSet>(src, src + stride, size, curr, dst), src += 2 * stride, c += 2;
          for (; c < count; c += 2, src += 2 * stride)
              ResizerByteArea2x2RowUpdateColor<N, UpdateAdd>(src, src + stride, size, zero, dst);
          ResizerByteArea2x2RowUpdateColor<N, UpdateAdd>(src, tail ? src : src + stride, size, zero - next, dst);
      }
      else
          ResizerByteArea2x2RowUpdateColor<N, UpdateSet>(src, tail ? src : src + stride, size, curr - next, dst);
  }
  //}}}
  //{{{
  SIMD_INLINE void SaveLoadTailBgr2x2 (const uint8_t* ptr, size_t tail, __m128i * val)
  {
      uint8_t buffer[3 * A] = { 0 };
      _mm_storeu_si128((__m128i*)(buffer + 0), _mm_loadu_si128((__m128i*)(ptr + tail - 24)));
      _mm_storeu_si128((__m128i*)(buffer + 11), LoadAfterLast<3>(_mm_loadu_si128((__m128i*)(ptr + tail - 16))));
      val[0] = _mm_loadu_si128((__m128i*)(buffer + 24 - tail));
      val[1] = _mm_loadu_si128((__m128i*)(buffer + 32 - tail));
  }
  //}}}
  //{{{
  template<UpdateType update> SIMD_INLINE void ResizerByteArea2x2RowUpdateBgr (const uint8_t* src0, const uint8_t* src1, size_t size, int32_t val, int32_t* dst)
  {
      if (update == UpdateAdd && val == 0)
          return;
      size_t i = 0;
      __m128i _val = _mm_set1_epi32(val);
      static const __m128i K8_BGR0 = SIMD_MM_SETR_EPI8(0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, -1, -1, -1, -1, 0x7, 0xA, 0x8, 0xB);
      static const __m128i K8_BGR1 = SIMD_MM_SETR_EPI8(0x4, 0x7, 0x5, 0x8, -1, -1, -1, -1, 0x6, 0x9, 0xA, 0xD, 0xB, 0xE, 0xC, 0xF);
      size_t size24 = AlignLoAny(size, 24);
      for (; i < size24; i += 24, dst += 12)
      {
          __m128i s00 = _mm_maddubs_epi16(_mm_shuffle_epi8(Load<false>((__m128i*)(src0 + i + 0)), K8_BGR0), K8_01);
          __m128i s01 = _mm_maddubs_epi16(_mm_shuffle_epi8(Load<false>((__m128i*)(src0 + i + 8)), K8_BGR1), K8_01);
          __m128i s10 = _mm_maddubs_epi16(_mm_shuffle_epi8(Load<false>((__m128i*)(src1 + i + 0)), K8_BGR0), K8_01);
          __m128i s11 = _mm_maddubs_epi16(_mm_shuffle_epi8(Load<false>((__m128i*)(src1 + i + 8)), K8_BGR1), K8_01);
          __m128i s0 = _mm_add_epi16(s00, s10);
          __m128i s1 = _mm_add_epi16(s01, s11);
          Update<update, false>(dst + 0 * F, _mm_madd_epi16(_mm_cvtepi16_epi32(s0), _val));
          Update<update, false>(dst + 1 * F, _mm_madd_epi16(_mm_cvtepi16_epi32(_mm_alignr_epi8(s1, s0, 12)), _val));
          Update<update, false>(dst + 2 * F, _mm_madd_epi16(_mm_cvtepi16_epi32(_mm_srli_si128(s1, 8)), _val));
      }
      if (i < size)
      {
          size_t tail = size - i;
          __m128i s[4];
          SaveLoadTailBgr2x2(src0 + i, tail, s + 0);
          SaveLoadTailBgr2x2(src1 + i, tail, s + 2);
          __m128i s0 = _mm_add_epi16(
              _mm_maddubs_epi16(_mm_shuffle_epi8(s[0], K8_BGR0), K8_01),
              _mm_maddubs_epi16(_mm_shuffle_epi8(s[2], K8_BGR0), K8_01));
          __m128i s1 = _mm_add_epi16(
              _mm_maddubs_epi16(_mm_shuffle_epi8(s[1], K8_BGR1), K8_01),
              _mm_maddubs_epi16(_mm_shuffle_epi8(s[3], K8_BGR1), K8_01));
          Update<update, false>(dst + 0 * F, _mm_madd_epi16(_mm_cvtepi16_epi32(s0), _val));
          Update<update, false>(dst + 1 * F, _mm_madd_epi16(_mm_cvtepi16_epi32(_mm_alignr_epi8(s1, s0, 12)), _val));
          Update<update, false>(dst + 2 * F, _mm_madd_epi16(_mm_cvtepi16_epi32(_mm_srli_si128(s1, 8)), _val));
      }
  }
  //}}}
  //{{{
  template<> SIMD_INLINE void ResizerByteArea2x2RowSum <3> (const uint8_t* src, size_t stride, size_t count, size_t size, int32_t curr, int32_t zero, int32_t next, bool tail, int32_t* dst)
  {
      size_t c = 0;
      if (count)
      {
          ResizerByteArea2x2RowUpdateBgr<UpdateSet>(src, src + stride, size, curr, dst), src += 2 * stride, c += 2;
          for (; c < count; c += 2, src += 2 * stride)
              ResizerByteArea2x2RowUpdateBgr<UpdateAdd>(src, src + stride, size, zero, dst);
          ResizerByteArea2x2RowUpdateBgr<UpdateAdd>(src, tail ? src : src + stride, size, zero - next, dst);
      }
      else
          ResizerByteArea2x2RowUpdateBgr<UpdateSet>(src, tail ? src : src + stride, size, curr - next, dst);
  }
  //}}}
  //{{{
  template<size_t N> void ResizerByteArea2x2::Run (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      size_t bodyW = _param.dstW - (N == 3 ? 1 : 0), rowSize = _param.srcW * N, rowRest = dstStride - _param.dstW * N;
      const int32_t* iy = _iy.data, * ix = _ix.data, * ay = _ay.data, * ax = _ax.data;
      int32_t ay0 = ay[0], ax0 = ax[0];
      for (size_t dy = 0; dy < _param.dstH; dy++, dst += rowRest)
      {
          int32_t* buf = _by.data;
          size_t yn = (iy[dy + 1] - iy[dy]) * 2;
          bool tail = (dy == _param.dstH - 1) && (_param.srcH & 1);
          ResizerByteArea2x2RowSum<N>(src, srcStride, yn, rowSize, ay[dy], ay0, ay[dy + 1], tail, buf), src += yn * srcStride;
          size_t dx = 0;
          for (; dx < bodyW; dx++, dst += N)
          {
              size_t xn = ix[dx + 1] - ix[dx];
              Sse41::ResizerByteAreaResult<N>(buf, xn, ax[dx], ax0, ax[dx + 1], dst), buf += xn * N;
          }
          for (; dx < _param.dstW; dx++, dst += N)
          {
              size_t xn = ix[dx + 1] - ix[dx];
              Base::ResizerByteAreaResult<N>(buf, xn, ax[dx], ax0, ax[dx + 1], dst), buf += xn * N;
          }
      }
  }
  //}}}
  //{{{
  void ResizerByteArea2x2::Run (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      switch (_param.channels)
      {
      case 1: Run<1>(src, srcStride, dst, dstStride); return;
      case 2: Run<2>(src, srcStride, dst, dstStride); return;
      case 3: Run<3>(src, srcStride, dst, dstStride); return;
      case 4: Run<4>(src, srcStride, dst, dstStride); return;
      default:
          assert(0);
      }
  }
  //}}}

  // nearest
  //{{{
  ResizerNearest::ResizerNearest (const ResParam& param)
      : Base::ResizerNearest(param)
      , _blocks(0)
  {
  }
  //}}}
  //{{{
  size_t ResizerNearest::BlockCountMax (size_t align)
  {
      return (size_t)::ceil(float(Simd::Max(_param.srcW, _param.dstW) * _param.PixelSize()) / (align - _param.PixelSize()));
  }
  //}}}
  //{{{
  void ResizerNearest::EstimateParams()
  {
      if (_blocks)
          return;
      Base::ResizerNearest::EstimateParams();
      const size_t pixelSize = _param.PixelSize();
      if (pixelSize *_param.dstW < A || pixelSize * _param.srcW < A)
          return;
      if (pixelSize < 4 && _param.srcW < 4 * _param.dstW)
          _blocks = BlockCountMax(A);
      float scale = (float)_param.srcW / _param.dstW;
      if (_blocks)
      {
          _tails = 0;
          _ix16x1.Resize(_blocks);
          _tail16x1.Resize((size_t)::ceil(A * scale / pixelSize));
          size_t dstRowSize = _param.dstW * pixelSize;
          int block = 0;
          _ix16x1[0].src = 0;
          _ix16x1[0].dst = 0;
          for (int dstIndex = 0; dstIndex < (int)_param.dstW; ++dstIndex)
          {
              int srcIndex = _ix[dstIndex] / (int)pixelSize;
              int dst = dstIndex * (int)pixelSize - _ix16x1[block].dst;
              int src = srcIndex * (int)pixelSize - _ix16x1[block].src;
              if (src >= A - pixelSize || dst >= A - pixelSize)
              {
                  block++;
                  _ix16x1[block].src = srcIndex * (int)pixelSize;
                  _ix16x1[block].dst = dstIndex * (int)pixelSize;
                  if (_ix16x1[block].dst > dstRowSize - A)
                  {
                      _tail16x1[_tails] = LeftNotZero8i(dstRowSize - _ix16x1[block].dst);
                      _tails++;
                  }
                  dst = 0;
                  src = srcIndex * (int)pixelSize - _ix16x1[block].src;
              }
              for(size_t i = 0; i < pixelSize; ++i)
                  _ix16x1[block].shuffle[dst + i] = uint8_t(src + i);
          }
          _blocks = block + 1;
      }
  }
  //}}}
  //{{{
  void ResizerNearest::Shuffle16x1 (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      size_t body = _blocks - _tails;
      for (size_t dy = 0; dy < _param.dstH; dy++)
      {
          const uint8_t* srcRow = src + _iy[dy] * srcStride;
          size_t i = 0, t = 0;
          for (; i < body; ++i)
          {
             const IndexShuffle16x1& index = _ix16x1[i];
              __m128i _src = _mm_loadu_si128((__m128i*)(srcRow + index.src));
              __m128i _shuffle = _mm_loadu_si128((__m128i*) & index.shuffle);
              _mm_storeu_si128((__m128i*)(dst + index.dst), _mm_shuffle_epi8(_src, _shuffle));
          }
          for (; i < _blocks; ++i, t++)
          {
              const IndexShuffle16x1& index = _ix16x1[i];
              __m128i _src = _mm_loadu_si128((__m128i*)(srcRow + index.src));
              __m128i _shuffle = _mm_loadu_si128((__m128i*) & index.shuffle);
              StoreMasked<false>((__m128i*)(dst + index.dst), _mm_shuffle_epi8(_src, _shuffle), _tail16x1[t]);
          }
          dst += dstStride;
      }
  }
  //}}}
  //{{{
  SIMD_INLINE void CopyPixel12 (const uint8_t* src, uint8_t* dst)
  {
      __m128i val = _mm_loadu_si128((__m128i*)src);
      _mm_storeu_si128((__m128i*)dst, val);
  }
  //}}}
  //{{{
  void ResizerNearest::Resize12 (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      size_t body = _param.dstW - 1;
      for (size_t dy = 0; dy < _param.dstH; dy++)
      {
          const uint8_t* srcRow = src + _iy[dy] * srcStride;
          size_t dx = 0, offset = 0;
          for (; dx < body; dx++, offset += 12)
              CopyPixel12(srcRow + _ix[dx], dst + offset);
          Base::CopyPixel<12>(srcRow + _ix[dx], dst + offset);
          dst += dstStride;
      }
  }
  //}}}
  //{{{
  void ResizerNearest::Run (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      EstimateParams();
      if (_blocks)
          Shuffle16x1(src, srcStride, dst, dstStride);
      else if (_pixelSize == 12)
          Resize12(src, srcStride, dst, dstStride);
      else
          Base::ResizerNearest::Run(src, srcStride, dst, dstStride);
  }
  //}}}
  //{{{
  ResizerByteBilinear::ResizerByteBilinear (const ResParam& param)
      : Base::ResizerByteBilinear(param)
      , _blocks(0)
  {
  }
  //}}}
  //{{{
  size_t ResizerByteBilinear::BlockCountMax (size_t align)
  {
      return (size_t)Simd::Max(::ceil(float(_param.srcW) / (align - 1)), ::ceil(float(_param.dstW) * 2.0f / align));
  }
  //}}}
  //{{{
  void ResizerByteBilinear::EstimateParams()
  {
      if (_ax.data)
          return;
      if (_param.channels == 1 && _param.srcW < 4 * _param.dstW)
          _blocks = BlockCountMax(A);
      float scale = (float)_param.srcW / _param.dstW;
      _ax.Resize(AlignHi(_param.dstW, A) * _param.channels * 2, false, _param.align);
      uint8_t* alphas = _ax.data;
      if (_blocks)
      {
          _ixg.Resize(_blocks);
          int block = 0;
          _ixg[0].src = 0;
          _ixg[0].dst = 0;
          for (int dstIndex = 0; dstIndex < (int)_param.dstW; ++dstIndex)
          {
              float alpha = (float)((dstIndex + 0.5) * scale - 0.5);
              int srcIndex = (int)::floor(alpha);
              alpha -= srcIndex;

              if (srcIndex < 0)
              {
                  srcIndex = 0;
                  alpha = 0;
              }

              if (srcIndex > (int)_param.srcW - 2)
              {
                  srcIndex = (int)_param.srcW - 2;
                  alpha = 1;
              }

              int dst = 2 * dstIndex - _ixg[block].dst;
              int src = srcIndex - _ixg[block].src;
              if (src >= A - 1 || dst >= A)
              {
                  block++;
                  _ixg[block].src = Simd::Min(srcIndex, int(_param.srcW - A));
                  _ixg[block].dst = 2 * dstIndex;
                  dst = 0;
                  src = srcIndex - _ixg[block].src;
              }
              _ixg[block].shuffle[dst] = src;
              _ixg[block].shuffle[dst + 1] = src + 1;

              alphas[1] = (uint8_t)(alpha * Base::FRACTION_RANGE + 0.5);
              alphas[0] = (uint8_t)(Base::FRACTION_RANGE - alphas[1]);
              alphas += 2;
          }
          _blocks = block + 1;
      }
      else
      {
          _ix.Resize(_param.dstW);
          for (size_t i = 0; i < _param.dstW; ++i)
          {
              float alpha = (float)((i + 0.5) * scale - 0.5);
              ptrdiff_t index = (ptrdiff_t)::floor(alpha);
              alpha -= index;

              if (index < 0)
              {
                  index = 0;
                  alpha = 0;
              }

              if (index > (ptrdiff_t)_param.srcW - 2)
              {
                  index = _param.srcW - 2;
                  alpha = 1;
              }

              _ix[i] = (int)index;
              alphas[1] = (uint8_t)(alpha * Base::FRACTION_RANGE + 0.5);
              alphas[0] = (uint8_t)(Base::FRACTION_RANGE - alphas[1]);
              for (size_t channel = 1; channel < _param.channels; channel++)
                  ((uint16_t*)alphas)[channel] = *(uint16_t*)alphas;
              alphas += 2 * _param.channels;
          }
      }
      size_t size = AlignHi(_param.dstW, _param.align) * _param.channels * 2 + SIMD_ALIGN;
      _bx[0].Resize(size, false, _param.align);
      _bx[1].Resize(size, false, _param.align);
  }
  //}}}

  // bylinear
  template <size_t N> void ResizerByteBilinearInterpolateX (const __m128i* alpha, __m128i* buffer);
  //{{{
  template <> SIMD_INLINE void ResizerByteBilinearInterpolateX <1> (const __m128i* alpha, __m128i* buffer)
  {
      _mm_store_si128(buffer, _mm_maddubs_epi16(_mm_load_si128(buffer), _mm_load_si128(alpha)));
  }
  //}}}

  const __m128i K8_SHUFFLE_X2 = SIMD_MM_SETR_EPI8 (0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF);
  //{{{
  SIMD_INLINE void ResizerByteBilinearInterpolateX2 (const __m128i* alpha, __m128i* buffer)
  {
      __m128i src = _mm_shuffle_epi8(_mm_load_si128(buffer), K8_SHUFFLE_X2);
      _mm_store_si128(buffer, _mm_maddubs_epi16(src, _mm_load_si128(alpha)));
  }
  //}}}
  //{{{
  template <> SIMD_INLINE void ResizerByteBilinearInterpolateX <2> (const __m128i* alpha, __m128i* buffer)
  {
      ResizerByteBilinearInterpolateX2(alpha + 0, buffer + 0);
      ResizerByteBilinearInterpolateX2(alpha + 1, buffer + 1);
  }

  //}}}
  const __m128i K8_SHUFFLE_X3_00 = SIMD_MM_SETR_EPI8 (0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, 0x7, 0xA, 0x8, 0xB, 0xC, 0xF, 0xD, -1);
  const __m128i K8_SHUFFLE_X3_01 = SIMD_MM_SETR_EPI8 (-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x0);
  const __m128i K8_SHUFFLE_X3_10 = SIMD_MM_SETR_EPI8 (0xE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
  const __m128i K8_SHUFFLE_X3_11 = SIMD_MM_SETR_EPI8 (-1, 0x1, 0x2, 0x5, 0x3, 0x6, 0x4, 0x7, 0x8, 0xB, 0x9, 0xC, 0xA, 0xD, 0xE, -1);
  const __m128i K8_SHUFFLE_X3_12 = SIMD_MM_SETR_EPI8 (-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x1);
  const __m128i K8_SHUFFLE_X3_21 = SIMD_MM_SETR_EPI8 (0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
  const __m128i K8_SHUFFLE_X3_22 = SIMD_MM_SETR_EPI8 (-1, 0x2, 0x0, 0x3, 0x4, 0x7, 0x5, 0x8, 0x6, 0x9, 0xA, 0xD, 0xB, 0xE, 0xC, 0xF);
  //{{{
  template <> SIMD_INLINE void ResizerByteBilinearInterpolateX <3> (const __m128i* alpha, __m128i* buffer)
  {
      __m128i src[3], shuffled[3];
      src[0] = _mm_load_si128(buffer + 0);
      src[1] = _mm_load_si128(buffer + 1);
      src[2] = _mm_load_si128(buffer + 2);
      shuffled[0] = _mm_shuffle_epi8(src[0], K8_SHUFFLE_X3_00);
      shuffled[0] = _mm_or_si128(shuffled[0], _mm_shuffle_epi8(src[1], K8_SHUFFLE_X3_01));
      _mm_store_si128(buffer + 0, _mm_maddubs_epi16(shuffled[0], _mm_load_si128(alpha + 0)));
      shuffled[1] = _mm_shuffle_epi8(src[0], K8_SHUFFLE_X3_10);
      shuffled[1] = _mm_or_si128(shuffled[1], _mm_shuffle_epi8(src[1], K8_SHUFFLE_X3_11));
      shuffled[1] = _mm_or_si128(shuffled[1], _mm_shuffle_epi8(src[2], K8_SHUFFLE_X3_12));
      _mm_store_si128(buffer + 1, _mm_maddubs_epi16(shuffled[1], _mm_load_si128(alpha + 1)));
      shuffled[2] = _mm_shuffle_epi8(src[1], K8_SHUFFLE_X3_21);
      shuffled[2] = _mm_or_si128(shuffled[2], _mm_shuffle_epi8(src[2], K8_SHUFFLE_X3_22));
      _mm_store_si128(buffer + 2, _mm_maddubs_epi16(shuffled[2], _mm_load_si128(alpha + 2)));
  }
  //}}}
  const __m128i K8_SHUFFLE_X4 = SIMD_MM_SETR_EPI8 (0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF);
  //{{{
  SIMD_INLINE void ResizerByteBilinearInterpolateX4 (const __m128i* alpha, __m128i* buffer)
  {
      __m128i src = _mm_shuffle_epi8(_mm_load_si128(buffer), K8_SHUFFLE_X4);
      _mm_store_si128(buffer, _mm_maddubs_epi16(src, _mm_load_si128(alpha)));
  }
  //}}}
  //{{{
  template <> SIMD_INLINE void ResizerByteBilinearInterpolateX <4> (const __m128i* alpha, __m128i* buffer)
  {
      ResizerByteBilinearInterpolateX4(alpha + 0, buffer + 0);
      ResizerByteBilinearInterpolateX4(alpha + 1, buffer + 1);
      ResizerByteBilinearInterpolateX4(alpha + 2, buffer + 2);
      ResizerByteBilinearInterpolateX4(alpha + 3, buffer + 3);
  }
  //}}}

  const __m128i K16_FRACTION_ROUND_TERM = SIMD_MM_SET1_EPI16(Base::BILINEAR_ROUND_TERM);
  //{{{
  template<bool align> SIMD_INLINE __m128i ResizerByteBilinearInterpolateY (const __m128i* pbx0, const __m128i* pbx1, __m128i alpha[2])
  {
      __m128i sum = _mm_add_epi16(_mm_mullo_epi16(Load<align>(pbx0), alpha[0]), _mm_mullo_epi16(Load<align>(pbx1), alpha[1]));
      return _mm_srli_epi16(_mm_add_epi16(sum, K16_FRACTION_ROUND_TERM), Base::BILINEAR_SHIFT);
  }
  //}}}
  //{{{
  template<bool align> SIMD_INLINE void ResizerByteBilinearInterpolateY (const uint8_t* bx0, const uint8_t* bx1, __m128i alpha[2], uint8_t* dst)
  {
      __m128i lo = ResizerByteBilinearInterpolateY<align>((__m128i*)bx0 + 0, (__m128i*)bx1 + 0, alpha);
      __m128i hi = ResizerByteBilinearInterpolateY<align>((__m128i*)bx0 + 1, (__m128i*)bx1 + 1, alpha);
      Store<false>((__m128i*)dst, _mm_packus_epi16(lo, hi));
  }
  //}}}
  //{{{
  template<size_t N> void ResizerByteBilinear::Run (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      struct One { uint8_t val[N * 1]; };
      struct Two { uint8_t val[N * 2]; };

      size_t size = 2 * _param.dstW * N;
      size_t aligned = AlignHi(size, DA) - DA;
      const size_t step = A * N;
      ptrdiff_t previous = -2;
      __m128i a[2];
      uint8_t* bx[2] = { _bx[0].data, _bx[1].data };
      const uint8_t* ax = _ax.data;
      const int32_t* ix = _ix.data;
      size_t dstW = _param.dstW;

      for (size_t yDst = 0; yDst < _param.dstH; yDst++, dst += dstStride)
      {
          a[0] = _mm_set1_epi16(int16_t(Base::FRACTION_RANGE - _ay[yDst]));
          a[1] = _mm_set1_epi16(int16_t(_ay[yDst]));

          ptrdiff_t sy = _iy[yDst];
          int k = 0;

          if (sy == previous)
              k = 2;
          else if (sy == previous + 1)
          {
              Swap(bx[0], bx[1]);
              k = 1;
          }

          previous = sy;

          for (; k < 2; k++)
          {
              Two* pb = (Two*)bx[k];
              const One* psrc = (const One*)(src + (sy + k) * srcStride);
              for (size_t x = 0; x < dstW; x++)
                  pb[x] = *(Two*)(psrc + ix[x]);

              uint8_t* pbx = bx[k];
              for (size_t i = 0; i < size; i += step)
                  ResizerByteBilinearInterpolateX<N>((__m128i*)(ax + i), (__m128i*)(pbx + i));
          }

          for (size_t ib = 0, id = 0; ib < aligned; ib += DA, id += A)
              ResizerByteBilinearInterpolateY<true>(bx[0] + ib, bx[1] + ib, a, dst + id);
          size_t i = size - DA;
          ResizerByteBilinearInterpolateY<false>(bx[0] + i, bx[1] + i, a, dst + i / 2);
      }
  }
  //}}}
  //{{{
  template <class Idx> SIMD_INLINE void ResizerByteBilinearLoadGrayInterpolated (const uint8_t* src, const Idx& index, const uint8_t* alpha, uint8_t* dst)
  {
      __m128i _src = _mm_loadu_si128((__m128i*)(src + index.src));
      __m128i _shuffle = _mm_loadu_si128((__m128i*) & index.shuffle);
      __m128i _alpha = _mm_loadu_si128((__m128i*)(alpha + index.dst));
      _mm_storeu_si128((__m128i*)(dst + index.dst), _mm_maddubs_epi16(_mm_shuffle_epi8(_src, _shuffle), _alpha));
  }
  //}}}
  //{{{
  void ResizerByteBilinear::RunG (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      size_t bufW = AlignHi(_param.dstW, A) * 2;
      size_t size = 2 * _param.dstW;
      size_t aligned = AlignHi(size, DA) - DA;
      size_t blocks = _blocks;
      ptrdiff_t previous = -2;
      __m128i a[2];
      uint8_t* bx[2] = { _bx[0].data, _bx[1].data };
      const uint8_t* ax = _ax.data;
      const Idx* ixg = _ixg.data;

      for (size_t yDst = 0; yDst < _param.dstH; yDst++, dst += dstStride)
      {
          a[0] = _mm_set1_epi16(int16_t(Base::FRACTION_RANGE - _ay[yDst]));
          a[1] = _mm_set1_epi16(int16_t(_ay[yDst]));

          ptrdiff_t sy = _iy[yDst];
          int k = 0;

          if (sy == previous)
              k = 2;
          else if (sy == previous + 1)
          {
              Swap(bx[0], bx[1]);
              k = 1;
          }

          previous = sy;

          for (; k < 2; k++)
          {
              const uint8_t* psrc = src + (sy + k) * srcStride;
              uint8_t* pdst = bx[k];
              for (size_t i = 0; i < blocks; ++i)
                  ResizerByteBilinearLoadGrayInterpolated(psrc, ixg[i], ax, pdst);
          }

          for (size_t ib = 0, id = 0; ib < aligned; ib += DA, id += A)
              ResizerByteBilinearInterpolateY<true>(bx[0] + ib, bx[1] + ib, a, dst + id);
          size_t i = size - DA;
          ResizerByteBilinearInterpolateY<false>(bx[0] + i, bx[1] + i, a, dst + i / 2);
      }
  }
  //}}}
  //{{{
  void ResizerByteBilinear::Run (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      assert(_param.dstW >= A);

      EstimateParams();
      switch (_param.channels)
      {
      case 1:
          if (_blocks)
              RunG(src, srcStride, dst, dstStride);
          else
              Run<1>(src, srcStride, dst, dstStride);
          break;
      case 2: Run<2>(src, srcStride, dst, dstStride); break;
      case 3: Run<3>(src, srcStride, dst, dstStride); break;
      case 4: Run<4>(src, srcStride, dst, dstStride); break;
      default:
          assert(0);
      }
  }
  //}}}

  //{{{
  ResizerShortBilinear::ResizerShortBilinear (const ResParam& param)
      : Base::ResizerShortBilinear(param)
  {
  }
  //}}}
  //{{{
  template<size_t N> void ResizerShortBilinear::RunB (const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
  {
      size_t rs = _param.dstW * N;
      float* pbx[2] = { _bx[0].data, _bx[1].data };
      int32_t prev = -2;
      size_t rs3 = AlignLoAny(rs - 1, 3);
      size_t rs4 = AlignLo(rs, 4);
      size_t rs8 = AlignLo(rs, 8);
      __m128 _1 = _mm_set1_ps(1.0f);
      for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
      {
          float fy1 = _ay[dy];
          float fy0 = 1.0f - fy1;
          int32_t sy = _iy[dy];
          int32_t k = 0;

          if (sy == prev)
              k = 2;
          else if (sy == prev + 1)
          {
              Swap(pbx[0], pbx[1]);
              k = 1;
          }

          prev = sy;

          for (; k < 2; k++)
          {
              float* pb = pbx[k];
              const uint16_t* ps = src + (sy + k) * srcStride;
              size_t dx = 0;
              if (N == 1)
              {
                  for (; dx < rs4; dx += 4)
                  {
                      __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                      __m128 fx0 = _mm_sub_ps(_1, fx1);
                      _mm_storeu_ps(pb + dx, BilColS1(ps, _ix.data + dx, fx0, fx1));
                  }
              }
              if (N == 2)
              {
                  for (; dx < rs4; dx += 4)
                  {
                      __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                      __m128 fx0 = _mm_sub_ps(_1, fx1);
                      _mm_storeu_ps(pb + dx, BilColS2(ps, _ix.data + dx, fx0, fx1));
                  }
              }
              if (N == 3)
              {
                  for (; dx < rs3; dx += 3)
                  {
                      __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                      __m128 fx0 = _mm_sub_ps(_1, fx1);
                      _mm_storeu_ps(pb + dx, BilColS3(ps + _ix[dx], fx0, fx1));
                  }
              }
              if (N == 4)
              {
                  for (; dx < rs4; dx += 4)
                  {
                      __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                      __m128 fx0 = _mm_sub_ps(_1, fx1);
                      _mm_storeu_ps(pb + dx, BilColS4(ps + _ix[dx], fx0, fx1));
                  }
              }
              for (; dx < rs; dx++)
              {
                  int32_t sx = _ix[dx];
                  float fx = _ax[dx];
                  pb[dx] = ps[sx] * (1.0f - fx) + ps[sx + N] * fx;
              }
          }

          size_t dx = 0;
          __m128 _fy0 = _mm_set1_ps(fy0);
          __m128 _fy1 = _mm_set1_ps(fy1);
          for (; dx < rs8; dx += 8)
          {
              __m128 m00 = _mm_mul_ps(_mm_loadu_ps(pbx[0] + dx + 0), _fy0);
              __m128 m01 = _mm_mul_ps(_mm_loadu_ps(pbx[1] + dx + 0), _fy1);
              __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m00, m01));
              __m128 m10 = _mm_mul_ps(_mm_loadu_ps(pbx[0] + dx + 4), _fy0);
              __m128 m11 = _mm_mul_ps(_mm_loadu_ps(pbx[1] + dx + 4), _fy1);
              __m128i i1 = _mm_cvttps_epi32(_mm_add_ps(m10, m11));
              _mm_storeu_si128((__m128i*)(dst + dx), _mm_packus_epi32(i0, i1));
          }
          for (; dx < rs4; dx += 4)
          {
              __m128 m0 = _mm_mul_ps(_mm_loadu_ps(pbx[0] + dx), _fy0);
              __m128 m1 = _mm_mul_ps(_mm_loadu_ps(pbx[1] + dx), _fy1);
              __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m0, m1));
              _mm_storel_epi64((__m128i*)(dst + dx), _mm_packus_epi32(i0, K_ZERO));
          }
          for (; dx < rs; dx++)
              dst[dx] = Round(pbx[0][dx] * fy0 + pbx[1][dx] * fy1);
      }
  }
  //}}}
  //{{{
  template<size_t N> void ResizerShortBilinear::RunS (const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
  {
      size_t rs = _param.dstW * N;
      size_t rs3 = AlignLoAny(rs - 1, 3);
      size_t rs6 = AlignLoAny(rs - 1, 6);
      size_t rs4 = AlignLo(rs, 4);
      size_t rs8 = AlignLo(rs, 8);
      __m128 _1 = _mm_set1_ps(1.0f);
      for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
      {
          float fy1 = _ay[dy];
          float fy0 = 1.0f - fy1;
          int32_t sy = _iy[dy];
          const uint16_t* ps0 = src + (sy + 0) * srcStride;
          const uint16_t* ps1 = src + (sy + 1) * srcStride;
          size_t dx = 0;
          __m128 _fy0 = _mm_set1_ps(fy0);
          __m128 _fy1 = _mm_set1_ps(fy1);
          if (N == 1)
          {
              for (; dx < rs8; dx += 8)
              {
                  __m128 fx01 = _mm_loadu_ps(_ax.data + dx + 0);
                  __m128 fx00 = _mm_sub_ps(_1, fx01);
                  __m128 m00 = _mm_mul_ps(BilColS1(ps0, _ix.data + dx + 0, fx00, fx01), _fy0);
                  __m128 m01 = _mm_mul_ps(BilColS1(ps1, _ix.data + dx + 0, fx00, fx01), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m00, m01));
                  __m128 fx11 = _mm_loadu_ps(_ax.data + dx + 4);
                  __m128 fx10 = _mm_sub_ps(_1, fx11);
                  __m128 m10 = _mm_mul_ps(BilColS1(ps0, _ix.data + dx + 4, fx10, fx11), _fy0);
                  __m128 m11 = _mm_mul_ps(BilColS1(ps1, _ix.data + dx + 4, fx10, fx11), _fy1);
                  __m128i i1 = _mm_cvttps_epi32(_mm_add_ps(m10, m11));
                  _mm_storeu_si128((__m128i*)(dst + dx), _mm_packus_epi32(i0, i1));
              }
              for (; dx < rs4; dx += 4)
              {
                  __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                  __m128 fx0 = _mm_sub_ps(_1, fx1);
                  __m128 m0 = _mm_mul_ps(BilColS1(ps0, _ix.data + dx, fx0, fx1), _fy0);
                  __m128 m1 = _mm_mul_ps(BilColS1(ps1, _ix.data + dx, fx0, fx1), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m0, m1));
                  _mm_storel_epi64((__m128i*)(dst + dx), _mm_packus_epi32(i0, K_ZERO));
              }
          }
          if (N == 2)
          {
              for (; dx < rs8; dx += 8)
              {
                  __m128 fx01 = _mm_loadu_ps(_ax.data + dx + 0);
                  __m128 fx00 = _mm_sub_ps(_1, fx01);
                  __m128 m00 = _mm_mul_ps(BilColS2(ps0, _ix.data + dx + 0, fx00, fx01), _fy0);
                  __m128 m01 = _mm_mul_ps(BilColS2(ps1, _ix.data + dx + 0, fx00, fx01), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m00, m01));
                  __m128 fx11 = _mm_loadu_ps(_ax.data + dx + 4);
                  __m128 fx10 = _mm_sub_ps(_1, fx11);
                  __m128 m10 = _mm_mul_ps(BilColS2(ps0, _ix.data + dx + 4, fx10, fx11), _fy0);
                  __m128 m11 = _mm_mul_ps(BilColS2(ps1, _ix.data + dx + 4, fx10, fx11), _fy1);
                  __m128i i1 = _mm_cvttps_epi32(_mm_add_ps(m10, m11));
                  _mm_storeu_si128((__m128i*)(dst + dx), _mm_packus_epi32(i0, i1));
              }
              for (; dx < rs4; dx += 4)
              {
                  __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                  __m128 fx0 = _mm_sub_ps(_1, fx1);
                  __m128 m0 = _mm_mul_ps(BilColS2(ps0, _ix.data + dx, fx0, fx1), _fy0);
                  __m128 m1 = _mm_mul_ps(BilColS2(ps1, _ix.data + dx, fx0, fx1), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m0, m1));
                  _mm_storel_epi64((__m128i*)(dst + dx), _mm_packus_epi32(i0, K_ZERO));
              }
          }
          if (N == 3)
          {
              for (; dx < rs6; dx += 6)
              {
                  __m128 fx01 = _mm_loadu_ps(_ax.data + dx + 0);
                  __m128 fx00 = _mm_sub_ps(_1, fx01);
                  __m128 m00 = _mm_mul_ps(BilColS3(ps0 + _ix[dx + 0], fx00, fx01), _fy0);
                  __m128 m01 = _mm_mul_ps(BilColS3(ps1 + _ix[dx + 0], fx00, fx01), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m00, m01));
                  __m128 fx11 = _mm_loadu_ps(_ax.data + dx + 3);
                  __m128 fx10 = _mm_sub_ps(_1, fx11);
                  __m128 m10 = _mm_mul_ps(BilColS3(ps0 + _ix[dx + 3], fx10, fx11), _fy0);
                  __m128 m11 = _mm_mul_ps(BilColS3(ps1 + _ix[dx + 3], fx10, fx11), _fy1);
                  __m128i i1 = _mm_cvttps_epi32(_mm_add_ps(m10, m11));
                  _mm_storeu_si128((__m128i*)(dst + dx), _mm_shuffle_epi8(_mm_packus_epi32(i0, i1), RSB_3_P));
              }
              for (; dx < rs3; dx += 3)
              {
                  __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                  __m128 fx0 = _mm_sub_ps(_1, fx1);
                  __m128 m0 = _mm_mul_ps(BilColS3(ps0 + _ix[dx], fx0, fx1), _fy0);
                  __m128 m1 = _mm_mul_ps(BilColS3(ps1 + _ix[dx], fx0, fx1), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m0, m1));
                  _mm_storel_epi64((__m128i*)(dst + dx), _mm_packus_epi32(i0, K_ZERO));
              }
          }
          if (N == 4)
          {
              for (; dx < rs8; dx += 8)
              {
                  __m128 fx01 = _mm_loadu_ps(_ax.data + dx + 0);
                  __m128 fx00 = _mm_sub_ps(_1, fx01);
                  __m128 m00 = _mm_mul_ps(BilColS4(ps0 + _ix[dx + 0], fx00, fx01), _fy0);
                  __m128 m01 = _mm_mul_ps(BilColS4(ps1 + _ix[dx + 0], fx00, fx01), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m00, m01));
                  __m128 fx11 = _mm_loadu_ps(_ax.data + dx + 4);
                  __m128 fx10 = _mm_sub_ps(_1, fx11);
                  __m128 m10 = _mm_mul_ps(BilColS4(ps0 + _ix[dx + 4], fx10, fx11), _fy0);
                  __m128 m11 = _mm_mul_ps(BilColS4(ps1 + _ix[dx + 4], fx10, fx11), _fy1);
                  __m128i i1 = _mm_cvttps_epi32(_mm_add_ps(m10, m11));
                  _mm_storeu_si128((__m128i*)(dst + dx), _mm_packus_epi32(i0, i1));
              }
              for (; dx < rs4; dx += 4)
              {
                  __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                  __m128 fx0 = _mm_sub_ps(_1, fx1);
                  __m128 m0 = _mm_mul_ps(BilColS4(ps0 + _ix[dx], fx0, fx1), _fy0);
                  __m128 m1 = _mm_mul_ps(BilColS4(ps1 + _ix[dx], fx0, fx1), _fy1);
                  __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m0, m1));
                  _mm_storel_epi64((__m128i*)(dst + dx), _mm_packus_epi32(i0, K_ZERO));
              }
          }
          for (; dx < rs; dx++)
          {
              int32_t sx = _ix[dx];
              float fx1 = _ax[dx];
              float fx0 = 1.0f - fx1;
              float r0 = ps0[sx] * fx0 + ps0[sx + N] * fx1;
              float r1 = ps1[sx] * fx0 + ps1[sx + N] * fx1;
              dst[dx] = Round(r0 * fy0 + r1 * fy1);
          }
      }
  }
  //}}}
  //{{{
  void ResizerShortBilinear::Run (const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
  {
      bool sparse = _param.dstH * 2.0 <= _param.srcH;
      switch (_param.channels)
      {
      case 1: sparse ? RunS<1>(src, srcStride, dst, dstStride) : RunB<1>(src, srcStride, dst, dstStride); return;
      case 2: sparse ? RunS<2>(src, srcStride, dst, dstStride) : RunB<2>(src, srcStride, dst, dstStride); return;
      case 3: sparse ? RunS<3>(src, srcStride, dst, dstStride) : RunB<3>(src, srcStride, dst, dstStride); return;
      case 4: sparse ? RunS<4>(src, srcStride, dst, dstStride) : RunB<4>(src, srcStride, dst, dstStride); return;
      default:
          assert(0);
      }
  }
  //}}}

  // bicubic
  //{{{
  ResizerByteBicubic::ResizerByteBicubic (const ResParam& param)
      : Base::ResizerByteBicubic(param)
  {
  }
  //}}}
  //{{{
  void ResizerByteBicubic::EstimateIndexAlphaY()
  {
      size_t sizeD = _param.dstH, sizeS = _param.srcH;
      _iy.Resize(sizeD);
      _ay.Resize(sizeD * 4);
      float scale = float(sizeS) / float(sizeD);
      size_t i = 0, sizeDF = AlignLo(sizeD, F);
      int32_t* ay = _ay.data;
      if (sizeDF)
      {
          __m128i _i = _mm_setr_epi32(0, 1, 2, 3);
          __m128 _scale = _mm_set1_ps(scale);
          __m128 _0 = _mm_set1_ps(0.0f);
          __m128 _05 = _mm_set1_ps(0.5f);
          __m128 _1 = _mm_set1_ps(1.0f);
          __m128 _2 = _mm_set1_ps(2.0f);
          __m128 _1_6 = _mm_set1_ps(1.0f / 6.0f);
          __m128 _max = _mm_set1_ps(float(sizeS - 2));
          __m128 _range = _mm_set1_ps(float(Base::BICUBIC_RANGE));

          for (; i < sizeDF; i += F, ay += 4 * F)
          {
              __m128 _pos = _mm_sub_ps(_mm_mul_ps(_mm_add_ps(_mm_cvtepi32_ps(_i), _05), _scale), _05);
              __m128 idx = _mm_round_ps(_pos, _MM_FROUND_FLOOR);
              __m128 d = _mm_sub_ps(_pos, idx);

              __m128 minMask = _mm_cmplt_ps(idx, _0);
              idx = _mm_blendv_ps(idx, _0, minMask);
              d = _mm_blendv_ps(d, _0, minMask);

              __m128 maxMask = _mm_cmpgt_ps(idx, _max);
              idx = _mm_blendv_ps(idx, _max, maxMask);
              d = _mm_blendv_ps(d, _1, maxMask);

              _mm_storeu_si128((__m128i*)(_iy.data + i), _mm_cvtps_epi32(idx));

              __m128i a0 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(_2, d), _mm_sub_ps(_1, d)), _mm_mul_ps(d, _1_6))));
              __m128i a1 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(d, _2), _mm_add_ps(_1, d)), _mm_mul_ps(_mm_sub_ps(_1, d), _05))));
              __m128i a2 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(d, _2), _mm_add_ps(_1, d)), _mm_mul_ps(d, _05))));
              __m128i a3 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_add_ps(_1, d), _mm_sub_ps(_1, d)), _mm_mul_ps(d, _1_6))));
              __m128i a00 = _mm_unpacklo_epi32(a0, a2);
              __m128i a01 = _mm_unpacklo_epi32(a1, a3);
              __m128i a10 = _mm_unpackhi_epi32(a0, a2);
              __m128i a11 = _mm_unpackhi_epi32(a1, a3);
              _mm_storeu_si128((__m128i*)ay + 0, _mm_unpacklo_epi32(a00, a01));
              _mm_storeu_si128((__m128i*)ay + 1, _mm_unpackhi_epi32(a00, a01));
              _mm_storeu_si128((__m128i*)ay + 2, _mm_unpacklo_epi32(a10, a11));
              _mm_storeu_si128((__m128i*)ay + 3, _mm_unpackhi_epi32(a10, a11));

              _i = _mm_add_epi32(_i, K32_00000004);
          }
      }
      for (; i < sizeD; ++i, ay += 4)
      {
          float pos = (float)((i + 0.5f) * scale - 0.5f);
          int idx = (int)::floor(pos);
          float d = pos - idx;
          if (idx < 0)
          {
              idx = 0;
              d = 0.0f;
          }
          if (idx > (int)sizeS - 2)
          {
              idx = (int)sizeS - 2;
              d = 1.0f;
          }
          _iy[i] = idx;
          ay[0] = Round(Base::BICUBIC_RANGE * (2.0f - d) * (1.0f - d) * d / 6.0f);
          ay[1] = Round(Base::BICUBIC_RANGE * (d - 2.0f) * (d + 1.0f) * (1.0f - d) / 2.0f);
          ay[2] = Round(Base::BICUBIC_RANGE * (d - 2.0f) * (d + 1.0f) * d / 2.0f);
          ay[3] = Round(Base::BICUBIC_RANGE * (1.0f + d) * (1.0f - d) * d / 6.0f);
      }
  }
  //}}}
  //{{{
  void ResizerByteBicubic::EstimateIndexAlphaX()
  {
      size_t sizeD = _param.dstW, sizeS = _param.srcW;
      _ix.Resize(sizeD);
      _ax.Resize(sizeD * 4);
      float scale = float(sizeS) / float(sizeD);
      size_t i = 0, sizeDF = AlignLo(sizeD, F);
      int8_t* ax = _ax.data;
      if (sizeDF)
      {
          static const __m128i _SHUFFLE = SIMD_MM_SETR_EPI8(0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF);
          __m128i _i = _mm_setr_epi32(0, 1, 2, 3);
          __m128 _scale = _mm_set1_ps(scale);
          __m128 _0 = _mm_set1_ps(0.0f);
          __m128 _05 = _mm_set1_ps(0.5f);
          __m128 _1 = _mm_set1_ps(1.0f);
          __m128 _2 = _mm_set1_ps(2.0f);
          __m128 _1_6 = _mm_set1_ps(1.0f / 6.0f);
          __m128 _max = _mm_set1_ps(float(sizeS - 2));
          __m128 _range = _mm_set1_ps(float(Base::BICUBIC_RANGE));
          __m128i _channels = _mm_set1_epi32((int)_param.channels);

          for (; i < sizeDF; i += F, ax += 4 * F)
          {
              __m128 _pos = _mm_sub_ps(_mm_mul_ps(_mm_add_ps(_mm_cvtepi32_ps(_i), _05), _scale), _05);
              __m128 idx = _mm_round_ps(_pos, _MM_FROUND_FLOOR);
              __m128 d = _mm_sub_ps(_pos, idx);

              __m128 minMask = _mm_cmplt_ps(idx, _0);
              idx = _mm_blendv_ps(idx, _0, minMask);
              d = _mm_blendv_ps(d, _0, minMask);

              __m128 maxMask = _mm_cmpgt_ps(idx, _max);
              idx = _mm_blendv_ps(idx, _max, maxMask);
              d = _mm_blendv_ps(d, _1, maxMask);

              _mm_storeu_si128((__m128i*)(_ix.data + i), _mm_mullo_epi32(_mm_cvtps_epi32(idx), _channels));

              __m128i a0 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(_2, d), _mm_sub_ps(_1, d)), _mm_mul_ps(d, _1_6))));
              __m128i a1 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(d, _2), _mm_add_ps(_1, d)), _mm_mul_ps(_mm_sub_ps(_1, d), _05))));
              __m128i a2 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(d, _2), _mm_add_ps(_1, d)), _mm_mul_ps(d, _05))));
              __m128i a3 = _mm_cvtps_epi32(_mm_mul_ps(_range, _mm_mul_ps(_mm_mul_ps(_mm_add_ps(_1, d), _mm_sub_ps(_1, d)), _mm_mul_ps(d, _1_6))));
              _mm_storeu_si128((__m128i*)ax, _mm_shuffle_epi8(_mm_packs_epi16(_mm_packs_epi32(a0, a1), _mm_packs_epi32(a2, a3)), _SHUFFLE));

              _i = _mm_add_epi32(_i, K32_00000004);
          }
      }
      for (; i < sizeD; ++i, ax += 4)
      {
          float pos = (float)((i + 0.5f) * scale - 0.5f);
          int idx = (int)::floor(pos);
          float d = pos - idx;
          if (idx < 0)
          {
              idx = 0;
              d = 0.0f;
          }
          if (idx > (int)sizeS - 2)
          {
              idx = (int)sizeS - 2;
              d = 1.0f;
          }
          _ix[i] = idx * (int)_param.channels;
          ax[0] = (int8_t)Round(Base::BICUBIC_RANGE * (2.0f - d) * (1.0f - d) * d / 6.0f);
          ax[1] = (int8_t)Round(Base::BICUBIC_RANGE * (d - 2.0f) * (d + 1.0f) * (1.0f - d) / 2.0f);
          ax[2] = (int8_t)Round(Base::BICUBIC_RANGE * (d - 2.0f) * (d + 1.0f) * d / 2.0f);
          ax[3] = (int8_t)Round(Base::BICUBIC_RANGE * (1.0f + d) * (1.0f - d) * d / 6.0f);
      }
  }

  //}}}
  //{{{
  void ResizerByteBicubic::ResizerByteBicubic::Init (bool sparse)
  {
      if (_iy.data)
          return;
      EstimateIndexAlphaY();
      EstimateIndexAlphaX();
      if (!sparse)
      {
          for (int i = 0; i < 4; ++i)
              _bx[i].Resize(_param.dstW * _param.channels);
      }
      _sxl = (_param.srcW - 2) * _param.channels;
      for (_xn = 0; _ix[_xn] == 0; _xn++);
      for (_xt = _param.dstW; _ix[_xt - 1] == _sxl; _xt--);
  }
  //}}}

  template<int N> __m128i LoadAx(const int8_t* ax);
  //{{{
  template<> SIMD_INLINE __m128i LoadAx <1> (const int8_t* ax)
  {
      return _mm_loadu_si128((__m128i*)ax);
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i LoadAx <2> (const int8_t* ax)
  {
      return _mm_shuffle_epi32(_mm_loadl_epi64((__m128i*)ax), 0x50);
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i LoadAx <3> (const int8_t* ax)
  {
      return _mm_set1_epi32(*(int32_t*)ax);
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i LoadAx <4> (const int8_t* ax)
  {
      return _mm_set1_epi32(*(int32_t*)ax);
  }
  //}}}

  template<int N> __m128i CubicSumX (const uint8_t* src, const int32_t* ix, __m128i ax, __m128i ay);
  //{{{
  template<> SIMD_INLINE __m128i CubicSumX <1> (const uint8_t* src, const int32_t* ix, __m128i ax, __m128i ay)
  {
      __m128i _src = _mm_setr_epi32(*(int32_t*)(src + ix[0]), *(int32_t*)(src + ix[1]), *(int32_t*)(src + ix[2]), *(int32_t*)(src + ix[3]));
      return _mm_madd_epi16(_mm_maddubs_epi16(_src, ax), ay);
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i CubicSumX <2> (const uint8_t* src, const int32_t* ix, __m128i ax, __m128i ay)
  {
      static const __m128i SHUFFLE = SIMD_MM_SETR_EPI8(0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF);
      __m128i _src = _mm_shuffle_epi8(Load((__m128i*)(src + ix[0]), (__m128i*)(src + ix[1])), SHUFFLE);
      return _mm_madd_epi16(_mm_maddubs_epi16(_src, ax), ay);
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i CubicSumX <3> (const uint8_t* src, const int32_t* ix, __m128i ax, __m128i ay)
  {
      static const __m128i SHUFFLE = SIMD_MM_SETR_EPI8(0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1);
      __m128i _src = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(src + ix[0])), SHUFFLE);
      return _mm_madd_epi16(_mm_maddubs_epi16(_src, ax), ay);
  }
  //}}}
  //{{{
  template<> SIMD_INLINE __m128i CubicSumX <4> (const uint8_t* src, const int32_t* ix, __m128i ax, __m128i ay)
  {
      static const __m128i SHUFFLE = SIMD_MM_SETR_EPI8(0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF);
      __m128i _src = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(src + ix[0])), SHUFFLE);
      return _mm_madd_epi16(_mm_maddubs_epi16(_src, ax), ay);
  }
  //}}}
  //{{{
  template<int N> SIMD_INLINE void BicubicInt (const uint8_t* src0, const uint8_t* src1, const uint8_t* src2, const uint8_t* src3, const int32_t* ix, const int8_t* ax, const __m128i* ay, uint8_t* dst)
  {
      static const __m128i ROUND = SIMD_MM_SET1_EPI32(Base::BICUBIC_ROUND);
      __m128i _ax = LoadAx<N>(ax);
      __m128i say0 = CubicSumX<N>(src0 - N, ix, _ax, ay[0]);
      __m128i say1 = CubicSumX<N>(src1 - N, ix, _ax, ay[1]);
      __m128i say2 = CubicSumX<N>(src2 - N, ix, _ax, ay[2]);
      __m128i say3 = CubicSumX<N>(src3 - N, ix, _ax, ay[3]);
      __m128i sum = _mm_add_epi32(_mm_add_epi32(say0, say1), _mm_add_epi32(say2, say3));
      __m128i dst0 = _mm_srai_epi32(_mm_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
      *((int32_t*)(dst)) = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(dst0, K_ZERO), K_ZERO));
  }
  //}}}
  //{{{
  template<int N> void ResizerByteBicubic::RunS (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      assert(_xn == 0 && _xt == _param.dstW);
      size_t step = 4 / N;
      size_t body = AlignLoAny(_param.dstW - (N == 3 ? 1 : 0), step);
      for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
      {
          size_t sy = _iy[dy];
          const uint8_t* src1 = src + sy * srcStride;
          const uint8_t* src2 = src1 + srcStride;
          const uint8_t* src0 = sy ? src1 - srcStride : src1;
          const uint8_t* src3 = sy < _param.srcH - 2 ? src2 + srcStride : src2;
          const int32_t* ay = _ay.data + dy * 4;
          __m128i ays[4];
          ays[0] = _mm_set1_epi16(ay[0]);
          ays[1] = _mm_set1_epi16(ay[1]);
          ays[2] = _mm_set1_epi16(ay[2]);
          ays[3] = _mm_set1_epi16(ay[3]);
          size_t dx = 0;
          for (; dx < body; dx += step)
              BicubicInt<N>(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * N);
          for (; dx < _param.dstW; dx++)
              Base::BicubicInt<N, -1, 2>(src0, src1, src2, src3, _ix[dx], _ax.data + dx * 4, ay, dst + dx * N);
      }
  }
  //}}}

  template<int F> SIMD_INLINE void PixelCubicSumX (const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst);
  //{{{
  template<> SIMD_INLINE void PixelCubicSumX <1> (const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
  {
      __m128i _src = _mm_setr_epi32(*(int32_t*)(src + ix[0]), *(int32_t*)(src + ix[1]), *(int32_t*)(src + ix[2]), *(int32_t*)(src + ix[3]));
      __m128i _ax = _mm_loadu_si128((__m128i*)ax);
      _mm_storeu_si128((__m128i*)dst, _mm_madd_epi16(_mm_maddubs_epi16(_src, _ax), K16_0001));
  }
  //}}}
  //{{{
  template<> SIMD_INLINE void PixelCubicSumX <2> (const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
  {
      static const __m128i SHUFFLE = SIMD_MM_SETR_EPI8(0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF);
      __m128i _src = _mm_shuffle_epi8(Load((__m128i*)(src + ix[0]), (__m128i*)(src + ix[1])), SHUFFLE);
      __m128i _ax = _mm_shuffle_epi32(_mm_loadl_epi64((__m128i*)ax), 0x50);
      _mm_storeu_si128((__m128i*)dst, _mm_madd_epi16(_mm_maddubs_epi16(_src, _ax), K16_0001));
  }
  //}}}
  //{{{
  template<> SIMD_INLINE void PixelCubicSumX <3> (const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
  {
      static const __m128i SHUFFLE = SIMD_MM_SETR_EPI8(0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1);
      __m128i _src = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(src + ix[0])), SHUFFLE);
      __m128i _ax = _mm_set1_epi32(*(int32_t*)ax);
      _mm_storeu_si128((__m128i*)dst, _mm_madd_epi16(_mm_maddubs_epi16(_src, _ax), K16_0001));
  }
  //}}}
  //{{{
  template<> SIMD_INLINE void PixelCubicSumX <4> (const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
  {
      static const __m128i SHUFFLE = SIMD_MM_SETR_EPI8(0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF);
      __m128i _src = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(src + ix[0])), SHUFFLE);
      __m128i _ax = _mm_set1_epi32(*(int32_t*)ax);
      _mm_storeu_si128((__m128i*)dst, _mm_madd_epi16(_mm_maddubs_epi16(_src, _ax), K16_0001));
  }
  //}}}
  //{{{
  template<int N> SIMD_INLINE void RowCubicSumX (const uint8_t* src, size_t nose, size_t body, size_t tail, const int32_t* ix, const int8_t* ax, int32_t* dst)
  {
      size_t step = 4 / N;
      size_t bodyS = nose + AlignLoAny(body - nose, step);

      size_t dx = 0;
      for (; dx < nose; dx++, ax += 4, dst += N)
          Base::PixelCubicSumX<N, 0, 2>(src + ix[dx], ax, dst);
      for (; dx < bodyS; dx += step, ax += 4 * step, dst += N * step)
          PixelCubicSumX<N>(src - N, ix + dx, ax, dst);
      for (; dx < body; dx++, ax += 4, dst += N)
          Base::PixelCubicSumX<N, -1, 2>(src + ix[dx], ax, dst);
      for (; dx < tail; dx++, ax += 4, dst += N)
          Base::PixelCubicSumX<N, -1, 1>(src + ix[dx], ax, dst);
  }
  //}}}
  //{{{
  SIMD_INLINE void BicubicRowInt (const int32_t* src0, const int32_t* src1, const int32_t* src2, const int32_t* src3, size_t n, const int32_t* ay, uint8_t* dst)
  {
      size_t nF = AlignLo(n, F);
      size_t i = 0;
      if (nF)
      {
          static const __m128i ROUND = SIMD_MM_SET1_EPI32(Base::BICUBIC_ROUND);
          __m128i ay0 = _mm_set1_epi32(ay[0]);
          __m128i ay1 = _mm_set1_epi32(ay[1]);
          __m128i ay2 = _mm_set1_epi32(ay[2]);
          __m128i ay3 = _mm_set1_epi32(ay[3]);
          for (; i < nF; i += F)
          {
              __m128i say0 = _mm_mullo_epi32(_mm_loadu_si128((__m128i*)(src0 + i)), ay0);
              __m128i say1 = _mm_mullo_epi32(_mm_loadu_si128((__m128i*)(src1 + i)), ay1);
              __m128i say2 = _mm_mullo_epi32(_mm_loadu_si128((__m128i*)(src2 + i)), ay2);
              __m128i say3 = _mm_mullo_epi32(_mm_loadu_si128((__m128i*)(src3 + i)), ay3);
              __m128i sum = _mm_add_epi32(_mm_add_epi32(say0, say1), _mm_add_epi32(say2, say3));
              __m128i dst0 = _mm_srai_epi32(_mm_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
             *((int32_t*)(dst + i)) = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(dst0, K_ZERO), K_ZERO));
          }
      }
      for (; i < n; ++i)
      {
          int32_t sum = ay[0] * src0[i] + ay[1] * src1[i] + ay[2] * src2[i] + ay[3] * src3[i];
          dst[i] = Base::RestrictRange((sum + Base::BICUBIC_ROUND) >> Base::BICUBIC_SHIFT, 0, 255);
      }
  }
  //}}}
  //{{{
  template<int N> void ResizerByteBicubic::RunB (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      int32_t prev = -1;
      for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
      {
          int32_t sy = _iy[dy], next = prev;
          for (int32_t curr = sy - 1, end = sy + 3; curr < end; ++curr)
          {
              if (curr < prev)
                  continue;
              const uint8_t* ps = src + RestrictRange(curr, 0, (int)_param.srcH - 1) * srcStride;
              int32_t* pb = _bx[(curr + 1) & 3].data;
              RowCubicSumX<N>(ps, _xn, _xt, _param.dstW, _ix.data, _ax.data, pb);
              next++;
          }
          prev = next;

          const int32_t* ay = _ay.data + dy * 4;
          int32_t* pb0 = _bx[(sy + 0) & 3].data;
          int32_t* pb1 = _bx[(sy + 1) & 3].data;
          int32_t* pb2 = _bx[(sy + 2) & 3].data;
          int32_t* pb3 = _bx[(sy + 3) & 3].data;
          BicubicRowInt(pb0, pb1, pb2, pb3, _bx[0].size, ay, dst);
      }
  }
  //}}}
  //{{{
  void ResizerByteBicubic::Run (const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      bool sparse = _param.dstH * 3.0 <= _param.srcH;
      Init(sparse);
      switch (_param.channels)
      {
      case 1: sparse ? RunS<1>(src, srcStride, dst, dstStride) : RunB<1>(src, srcStride, dst, dstStride); return;
      case 2: sparse ? RunS<2>(src, srcStride, dst, dstStride) : RunB<2>(src, srcStride, dst, dstStride); return;
      case 3: sparse ? RunS<3>(src, srcStride, dst, dstStride) : RunB<3>(src, srcStride, dst, dstStride); return;
      case 4: sparse ? RunS<4>(src, srcStride, dst, dstStride) : RunB<4>(src, srcStride, dst, dstStride); return;
      default:
          assert(0);
      }
  }
  //}}}

  //{{{
  void* ResizerInit (size_t srcX, size_t srcY, size_t dstX, size_t dstY,
                      size_t channels, SimdResizeChannelType type, SimdResizeMethodType method)
  {
      ResParam param(srcX, srcY, dstX, dstY, channels, type, method, sizeof(__m128i));
      if (param.IsNearest())
          return new ResizerNearest(param);
      else if (param.IsByteBilinear() && dstX >= A)
          return new ResizerByteBilinear(param);
      else if (param.IsShortBilinear())
          return new ResizerShortBilinear(param);
      else if (param.IsByteBicubic())
          return new ResizerByteBicubic(param);
      else if (param.IsByteArea2x2())
          return new ResizerByteArea2x2(param);
      else if (param.IsByteArea1x1())
          return new ResizerByteArea1x1(param);
      else
          return Base::ResizerInit(srcX, srcY, dstX, dstY, channels, type, method);
  }
  //}}}
  }
