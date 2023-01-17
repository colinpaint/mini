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
#include "SimdResizer.h"
#include "SimdStore.h"
#include "SimdSet.h"
#include "SimdUpdate.h"
#include "SimdUnpack.h"
#include "SimdResizerCommon.h"

namespace Simd::Avx512bw {
  //{{{  area
  ResizerByteArea1x1::ResizerByteArea1x1(const ResParam & param)
      : Avx2::ResizerByteArea1x1(param)
  {
  }

  template<UpdateType update, bool mask> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t * src0, __m512i alpha, int32_t * dst, __mmask64 tail = -1)
  {
      __m512i s0 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<false, mask>(src0, tail)));
      __m512i i0 = UnpackU8<0>(s0);
      __m512i i1 = UnpackU8<1>(s0);
      Update<update, true>(dst + 0 * F, _mm512_madd_epi16(alpha, UnpackU8<0>(i0)));
      Update<update, true>(dst + 1 * F, _mm512_madd_epi16(alpha, UnpackU8<1>(i0)));
      Update<update, true>(dst + 2 * F, _mm512_madd_epi16(alpha, UnpackU8<0>(i1)));
      Update<update, true>(dst + 3 * F, _mm512_madd_epi16(alpha, UnpackU8<1>(i1)));
  }

  template<UpdateType update> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t * src0, size_t size, size_t aligned, int32_t a, int32_t * dst, __mmask64 tail)
  {
      __m512i alpha = SetInt16(a, a);
      size_t i = 0;
      for (; i < aligned; i += A, dst += A, src0 += A)
          ResizerByteArea1x1RowUpdate<update, false>(src0, alpha, dst);
      if(i < size)
          ResizerByteArea1x1RowUpdate<update, true>(src0, alpha, dst, tail);
  }

  template<UpdateType update, bool mask> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t * src0, const uint8_t * src1, __m512i alpha, int32_t * dst, __mmask64 tail = -1)
  {
      __m512i s0 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<false, mask>(src0, tail)));
      __m512i s1 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<false, mask>(src1, tail)));
      __m512i i0 = UnpackU8<0>(s0, s1);
      __m512i i1 = UnpackU8<1>(s0, s1);
      Update<update, true>(dst + 0 * F, _mm512_madd_epi16(alpha, UnpackU8<0>(i0)));
      Update<update, true>(dst + 1 * F, _mm512_madd_epi16(alpha, UnpackU8<1>(i0)));
      Update<update, true>(dst + 2 * F, _mm512_madd_epi16(alpha, UnpackU8<0>(i1)));
      Update<update, true>(dst + 3 * F, _mm512_madd_epi16(alpha, UnpackU8<1>(i1)));
  }

  template<UpdateType update> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t * src0, size_t stride, size_t size, size_t aligned, int32_t a0, int32_t a1, int32_t * dst, __mmask64 tail = -1)
  {
      __m512i alpha = SetInt16(a0, a1);
      const uint8_t * src1 = src0 + stride;
      size_t i = 0;
      for (; i < aligned; i += A, dst += A)
          ResizerByteArea1x1RowUpdate<update, false>(src0 + i, src1 + i, alpha, dst);
      if (i < size)
          ResizerByteArea1x1RowUpdate<update, true>(src0 + i, src1 + i, alpha, dst, tail);
  }

  template<UpdateType update, bool mask> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t * src0, const uint8_t * src1,
      const uint8_t * src2, const uint8_t * src3, __m512i a01, __m512i a23, int32_t * dst, __mmask64 tail = -1)
  {
      __m512i s0 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<false, mask>(src0, tail)));
      __m512i s1 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<false, mask>(src1, tail)));
      __m512i t010 = _mm512_maddubs_epi16(UnpackU8<0>(s0, s1), a01);
      __m512i t011 = _mm512_maddubs_epi16(UnpackU8<1>(s0, s1), a01);
      __m512i s2 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<false, mask>(src2, tail)));
      __m512i s3 = _mm512_permutexvar_epi32(K32_PERMUTE_FOR_TWO_UNPACK, (Load<false, mask>(src3, tail)));
      __m512i t230 = _mm512_maddubs_epi16(UnpackU8<0>(s2, s3), a23);
      __m512i t231 = _mm512_maddubs_epi16(UnpackU8<1>(s2, s3), a23);
      Update<update, true>(dst + 0 * F, _mm512_madd_epi16(K16_0001, UnpackU16<0>(t010, t230)));
      Update<update, true>(dst + 1 * F, _mm512_madd_epi16(K16_0001, UnpackU16<1>(t010, t230)));
      Update<update, true>(dst + 2 * F, _mm512_madd_epi16(K16_0001, UnpackU16<0>(t011, t231)));
      Update<update, true>(dst + 3 * F, _mm512_madd_epi16(K16_0001, UnpackU16<1>(t011, t231)));
  }

  template<UpdateType update> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t * src0, size_t stride, size_t size, size_t aligned, int32_t a0, int32_t a12, int32_t a3, int32_t * dst, __mmask64 tail = -1)
  {
      __m512i a01 = SetInt8(a0, a12);
      __m512i a23 = SetInt8(a12, a3);
      const uint8_t * src1 = src0 + stride;
      const uint8_t * src2 = src1 + stride;
      const uint8_t * src3 = src2 + stride;
      size_t i = 0;
      for (; i < aligned; i += A, dst += A)
          ResizerByteArea1x1RowUpdate<update, false>(src0 + i, src1 + i, src2 + i, src3 + i, a01, a23, dst);
      if (i < size)
          ResizerByteArea1x1RowUpdate<update, true>(src0 + i, src1 + i, src2 + i, src3 + i, a01, a23, dst, tail);
  }

  SIMD_INLINE void ResizerByteArea1x1RowSum(const uint8_t * src, size_t stride, size_t count, size_t size, size_t aligned, int32_t curr, int32_t zero, int32_t next, int32_t * dst, __mmask64 tail)
  {
      if (count)
      {
          size_t i = 0;
          ResizerByteArea1x1RowUpdate<UpdateSet>(src, stride, size, aligned, curr, count == 1 ? zero - next : zero, dst, tail), src += 2 * stride, i += 2;
          for (; i < count; i += 2, src += 2 * stride)
              ResizerByteArea1x1RowUpdate<UpdateAdd>(src, stride, size, aligned, zero, i == count - 1 ? zero - next : zero, dst, tail);
          if (i == count)
              ResizerByteArea1x1RowUpdate<UpdateAdd>(src, size, aligned, zero - next, dst, tail);
      }
      else
          ResizerByteArea1x1RowUpdate<UpdateSet>(src, size, aligned, curr - next, dst, tail);
  }

  template<size_t N> void ResizerByteArea1x1::Run(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
  {
      size_t bodyW = _param.dstW - (N == 3 ? 1 : 0), rowSize = _param.srcW * N, rowRest = dstStride - _param.dstW * N;
      const int32_t * iy = _iy.data, *ix = _ix.data, *ay = _ay.data, *ax = _ax.data;
      int32_t ay0 = ay[0], ax0 = ax[0];
      size_t rowSizeA = AlignLo(rowSize, A);
      __mmask64 tail = TailMask64(rowSize - rowSizeA);
      for (size_t dy = 0; dy < _param.dstH; dy++, dst += rowRest)
      {
          int32_t * buf = _by.data;
          size_t yn = iy[dy + 1] - iy[dy];
          ResizerByteArea1x1RowSum(src, srcStride, yn, rowSize, rowSizeA, ay[dy], ay0, ay[dy + 1], buf, tail), src += yn * srcStride;
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

  void ResizerByteArea1x1::Run(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
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

  //---------------------------------------------------------------------------------------------

  ResizerByteArea2x2::ResizerByteArea2x2(const ResParam& param)
      : Avx2::ResizerByteArea2x2(param)
  {
  }

  template<size_t N> SIMD_INLINE __m512i ShuffleColor(__m512i val)
  {
      return val;
  }

  template<> SIMD_INLINE __m512i ShuffleColor<2>(__m512i val)
  {
      static const __m512i IDX = SIMD_MM512_SETR_EPI8(
          0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF,
          0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF,
          0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF,
          0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF);
      return _mm512_shuffle_epi8(val, IDX);
  }

  template<> SIMD_INLINE __m512i ShuffleColor<4>(__m512i val)
  {
      static const __m512i IDX = SIMD_MM512_SETR_EPI8(
          0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF,
          0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF,
          0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF,
          0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF);
      return _mm512_shuffle_epi8(val, IDX);
  }

  template<size_t N, UpdateType update> SIMD_INLINE void ResizerByteArea2x2RowUpdateColor(const uint8_t* src0, const uint8_t* src1, size_t size, int32_t val, int32_t* dst)
  {
      if (update == UpdateAdd && val == 0)
          return;
      size_t size2N = AlignLoAny(size, 2 * N);
      size_t i = 0;
      size_t size4F = AlignLoAny(size, 4 * F);
      __m512i _val = _mm512_set1_epi16(val);
      for (; i < size4F; i += 4 * F, dst += 2 * F)
      {
          __m512i s0 = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, Load<false>(src0 + i));
          __m512i s1 = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, Load<false>(src1 + i));
          s0 = _mm512_maddubs_epi16(ShuffleColor<N>(s0), K8_01);
          s1 = _mm512_maddubs_epi16(ShuffleColor<N>(s1), K8_01);
          Update<update, false>(dst + 0, _mm512_madd_epi16(_mm512_unpacklo_epi16(s0, s1), _val));
          Update<update, false>(dst + F, _mm512_madd_epi16(_mm512_unpackhi_epi16(s0, s1), _val));
      }
      if (size4F < size2N)
      {
          ptrdiff_t srcTail = size2N - size4F, dstTail = srcTail / 2;
          __mmask64 srcMask = TailMask64(srcTail);
          __m512i s0 = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<false, true>(src0 + i, srcMask)));
          __m512i s1 = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_UNPACK, (Load<false, true>(src1 + i, srcMask)));
          s0 = _mm512_maddubs_epi16(ShuffleColor<N>(s0), K8_01);
          s1 = _mm512_maddubs_epi16(ShuffleColor<N>(s1), K8_01);
          Update<update, false, true>(dst + 0, _mm512_madd_epi16(_mm512_unpacklo_epi16(s0, s1), _val), TailMask16(dstTail - 0));
          Update<update, false, true>(dst + F, _mm512_madd_epi16(_mm512_unpackhi_epi16(s0, s1), _val), TailMask16(dstTail - F));
          i += srcTail;
          dst += dstTail;
      }
      if (i < size)
          Base::ResizerByteArea2x2RowUpdate<N, 0, update>(src0 + i, src1 + i, val, dst);
  }

  template<size_t N> SIMD_INLINE void ResizerByteArea2x2RowSum(const uint8_t* src, size_t stride, size_t count, size_t size, int32_t curr, int32_t zero, int32_t next, bool tail, int32_t* dst)
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

  template<UpdateType update> SIMD_INLINE void ResizerByteArea2x2RowUpdateBgr(const uint8_t* src0, const uint8_t* src1, size_t size, int32_t val, int32_t* dst)
  {
      if (update == UpdateAdd && val == 0)
          return;
      size_t size6 = AlignLoAny(size, 6);
      size_t i = 0;
      static const __m256i K32_PRM0 = SIMD_MM256_SETR_EPI32(0x0, 0x1, 0x2, 0x0, 0x3, 0x4, 0x5, 0x0);
      static const __m256i K32_PRM1 = SIMD_MM256_SETR_EPI32(0x2, 0x3, 0x4, 0x0, 0x5, 0x6, 0x7, 0x0);
      static const __m256i K8_SHFL = SIMD_MM256_SETR_EPI8(
          0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, 0x7, 0xA, 0x8, 0xB, -1, -1, -1, -1,
          0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, 0x7, 0xA, 0x8, 0xB, -1, -1, -1, -1);
      static const __m256i K32_PRM2 = SIMD_MM256_SETR_EPI32(0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x3, 0x7);
      static const __m256i K32_PRM3 = SIMD_MM256_SETR_EPI32(0x2, 0x4, 0x5, 0x6, 0x3, 0x7, 0x0, 0x1);
      __m256i _val = _mm256_set1_epi32(val);
      size_t size48 = AlignLoAny(size, 48);
      for (; i < size48; i += 48, dst += 24)
      {
          __m256i s00 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(Avx2::Load<false>((__m256i*)(src0 + i + 0 * Avx2::F)), K32_PRM0), K8_SHFL);
          __m256i s01 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(Avx2::Load<false>((__m256i*)(src0 + i + 2 * Avx2::F)), K32_PRM1), K8_SHFL);
          __m256i s10 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(Avx2::Load<false>((__m256i*)(src1 + i + 0 * Avx2::F)), K32_PRM0), K8_SHFL);
          __m256i s11 = _mm256_shuffle_epi8(_mm256_permutevar8x32_epi32(Avx2::Load<false>((__m256i*)(src1 + i + 2 * Avx2::F)), K32_PRM1), K8_SHFL);
          __m256i s0 = _mm256_add_epi16(_mm256_maddubs_epi16(s00, Avx2::K8_01), _mm256_maddubs_epi16(s10, Avx2::K8_01));
          __m256i s1 = _mm256_add_epi16(_mm256_maddubs_epi16(s01, Avx2::K8_01), _mm256_maddubs_epi16(s11, Avx2::K8_01));
          __m256i d0 = _mm256_permutevar8x32_epi32(s0, K32_PRM2);
          __m256i d2 = _mm256_permutevar8x32_epi32(s1, K32_PRM3);
          Avx2::Update<update, false>(dst + 0 * Avx2::F, _mm256_madd_epi16(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(d0)), _val));
          Avx2::Update<update, false>(dst + 1 * Avx2::F, _mm256_madd_epi16(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(_mm256_or_si256(d0, d2), 1)), _val));
          Avx2::Update<update, false>(dst + 2 * Avx2::F, _mm256_madd_epi16(_mm256_cvtepi16_epi32(_mm256_castsi256_si128(d2)), _val));
      }
      for (; i < size6; i += 6, dst += 3)
          Base::ResizerByteArea2x2RowUpdate<3, 3, update>(src0 + i, src1 + i, val, dst);
      if (i < size)
          Base::ResizerByteArea2x2RowUpdate<3, 0, update>(src0 + i, src1 + i, val, dst);
  }

  template<> SIMD_INLINE void ResizerByteArea2x2RowSum<3>(const uint8_t* src, size_t stride, size_t count, size_t size, int32_t curr, int32_t zero, int32_t next, bool tail, int32_t* dst)
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

  template<size_t N> void ResizerByteArea2x2::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
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

  void ResizerByteArea2x2::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
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
  //{{{  nearest
  ResizerNearest::ResizerNearest(const ResParam& param)
      : Avx2::ResizerNearest(param)
  {
  }

  void ResizerNearest::EstimateParams()
  {
      if (_blocks)
          return;
      Base::ResizerNearest::EstimateParams();
      const size_t pixelSize = _param.PixelSize();
      if (pixelSize * _param.dstW < A || pixelSize * _param.srcW < A)
          return;
      if (pixelSize < 4 && _param.srcW < 4 * _param.dstW)
          _blocks = BlockCountMax(A);
      float scale = (float)_param.srcW / _param.dstW;
      if (_blocks)
      {
          _tails = 0;
          _ix32x2.Resize(_blocks);
          _tail32x2.Resize((size_t)::ceil(A * scale / pixelSize));
          size_t dstRowSize = _param.dstW * pixelSize;
          int block = 0;
          _ix32x2[0].src = 0;
          _ix32x2[0].dst = 0;
          for (int dstIndex = 0; dstIndex < (int)_param.dstW; ++dstIndex)
          {
              int srcIndex = _ix[dstIndex] / (int)pixelSize;
              int dst = dstIndex * (int)pixelSize - _ix32x2[block].dst;
              int src = srcIndex * (int)pixelSize - _ix32x2[block].src;
              if (src >= A - pixelSize || dst >= A - pixelSize)
              {
                  block++;
                  _ix32x2[block].src = srcIndex * (int)pixelSize;
                  _ix32x2[block].dst = dstIndex * (int)pixelSize;
                  if (_ix32x2[block].dst > dstRowSize - A)
                  {
                      _tail32x2[_tails] = TailMask32((dstRowSize - _ix32x2[block].dst) / 2);
                      _tails++;
                  }
                  dst = 0;
                  src = srcIndex * (int)pixelSize - _ix32x2[block].src;
              }
              for (size_t i = 0; i < pixelSize; i += 2)
                  _ix32x2[block].shuffle[(dst + i) / 2] = uint16_t((src + i) / 2);
          }
          _blocks = block + 1;
      }
  }

  SIMD_INLINE void Gather4(const int32_t* src, const int32_t* idx, int32_t* dst, __mmask16 mask = -1)
  {
      __m512i _idx = _mm512_maskz_loadu_epi32(mask, idx);
      __m512i val = _mm512_i32gather_epi32(_idx, src, 1);
      _mm512_mask_storeu_epi32(dst, mask, val);
  }

  void ResizerNearest::Gather4(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      size_t body = AlignLo(_param.dstW, F);
      __mmask16 tail = TailMask16(_param.dstW - body);
      for (size_t dy = 0; dy < _param.dstH; dy++)
      {
          const int32_t* srcRow = (int32_t*)(src + _iy[dy] * srcStride);
          size_t dx = 0;
          for (; dx < body; dx += F)
              Avx512bw::Gather4(srcRow, _ix.data + dx, (int32_t*)dst + dx);
          if (tail)
              Avx512bw::Gather4(srcRow, _ix.data + dx, (int32_t*)dst + dx, tail);
          dst += dstStride;
      }
  }

  SIMD_INLINE void Gather8(const int64_t* src, const int32_t* idx, int64_t* dst)
  {
      __m256i _idx = _mm256_loadu_si256((__m256i*)idx);
      __m512i val = _mm512_i32gather_epi64(_idx, src, 1);
      _mm512_storeu_si512((__m512i*)dst, val);
  }

  void ResizerNearest::Gather8(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      size_t body = AlignLo(_param.dstW, 8);
      size_t tail = _param.dstW - 8;
      for (size_t dy = 0; dy < _param.dstH; dy++)
      {
          const int64_t* srcRow = (int64_t*)(src + _iy[dy] * srcStride);
          for (size_t dx = 0; dx < body; dx += 8)
              Avx512bw::Gather8(srcRow, _ix.data + dx, (int64_t*)dst + dx);
          Avx512bw::Gather8(srcRow, _ix.data + tail, (int64_t*)dst + tail);
          dst += dstStride;
      }
  }

  void ResizerNearest::Shuffle32x2(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      size_t body = _blocks - _tails;
      for (size_t dy = 0; dy < _param.dstH; dy++)
      {
          const uint8_t* srcRow = src + _iy[dy] * srcStride;
          size_t i = 0, t = 0;
          for (; i < body; ++i)
          {
              const IndexShuffle32x2& index = _ix32x2[i];
              __m512i _src = _mm512_loadu_si512((__m512i*)(srcRow + index.src));
              __m512i _shuffle = _mm512_loadu_si512((__m512i*) & index.shuffle);
              _mm512_storeu_si512((__m512i*)(dst + index.dst), _mm512_permutexvar_epi16(_shuffle, _src));
          }
          for (; i < _blocks; ++i, t++)
          {
              const IndexShuffle32x2& index = _ix32x2[i];
              __m512i _src = _mm512_loadu_si512((__m512i*)(srcRow + index.src));
              __m512i _shuffle = _mm512_loadu_si512((__m512i*)&index.shuffle);
              _mm512_mask_storeu_epi16(dst + index.dst, _tail32x2[t], _mm512_permutexvar_epi16(_shuffle, _src));
          }
          dst += dstStride;
      }
  }

  void ResizerNearest::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
  {
      EstimateParams();
      if (_blocks)
          Shuffle32x2(src, srcStride, dst, dstStride);
      else
      {
          Avx2::ResizerNearest::EstimateParams();
          if (_pixelSize == 4)
              Gather4(src, srcStride, dst, dstStride);
          else if (_pixelSize == 8)
              Gather8(src, srcStride, dst, dstStride);
          else
              Avx2::ResizerNearest::Run(src, srcStride, dst, dstStride);
      }
  }

  bool ResizerNearest::Preferable(const ResParam& param)
  {
      const size_t pixelSize = param.PixelSize();
      return
          (pixelSize == 4 || (pixelSize == 8 && param.dstW >= F)) ||
          ((pixelSize & 1) == 0 && pixelSize < 8 && param.srcW < 8 * param.dstW);
  }
  //}}}
  //{{{  bilinear
          ResizerByteBilinear::ResizerByteBilinear(const ResParam & param)
              : Avx2::ResizerByteBilinear(param)
          {
          }

          template <size_t N> void ResizerByteBilinearInterpolateX(const uint8_t * alpha, uint8_t * buffer);

          template <> SIMD_INLINE void ResizerByteBilinearInterpolateX<1>(const uint8_t * alpha, uint8_t * buffer)
          {
              __m512i _buffer = Load<true>(buffer);
              Store<true>(buffer, _mm512_maddubs_epi16(_buffer, Load<true>(alpha)));
          }

          const __m512i K8_SHUFFLE_X2 = SIMD_MM512_SETR_EPI8(
              0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF,
              0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF,
              0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF,
              0x0, 0x2, 0x1, 0x3, 0x4, 0x6, 0x5, 0x7, 0x8, 0xA, 0x9, 0xB, 0xC, 0xE, 0xD, 0xF);

          SIMD_INLINE void ResizerByteBilinearInterpolateX2(const uint8_t * alpha, uint8_t * buffer)
          {
              __m512i _buffer = _mm512_shuffle_epi8(Load<true>(buffer), K8_SHUFFLE_X2);
              Store<true>(buffer, _mm512_maddubs_epi16(_buffer, Load<true>(alpha)));
          }

          template <> SIMD_INLINE void ResizerByteBilinearInterpolateX<2>(const uint8_t * alpha, uint8_t * buffer)
          {
              ResizerByteBilinearInterpolateX2(alpha + 0, buffer + 0);
              ResizerByteBilinearInterpolateX2(alpha + A, buffer + A);
          }

          const __m512i K8_SHUFFLE_X3_00 = SIMD_MM512_SETR_EPI8(
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              0xE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
          const __m512i K8_SHUFFLE_X3_01 = SIMD_MM512_SETR_EPI8(
              0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, 0x7, 0xA, 0x8, 0xB, 0xC, 0xF, 0xD, -1,
              -1, 0x1, 0x2, 0x5, 0x3, 0x6, 0x4, 0x7, 0x8, 0xB, 0x9, 0xC, 0xA, 0xD, 0xE, -1,
              -1, 0x2, 0x0, 0x3, 0x4, 0x7, 0x5, 0x8, 0x6, 0x9, 0xA, 0xD, 0xB, 0xE, 0xC, 0xF,
              0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, 0x7, 0xA, 0x8, 0xB, 0xC, 0xF, 0xD, -1);
          const __m512i K8_SHUFFLE_X3_02 = SIMD_MM512_SETR_EPI8(
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x0,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x0);

          const __m512i K8_SHUFFLE_X3_10 = SIMD_MM512_SETR_EPI8(
              0xE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              0xE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
          const __m512i K8_SHUFFLE_X3_11 = SIMD_MM512_SETR_EPI8(
              -1, 0x1, 0x2, 0x5, 0x3, 0x6, 0x4, 0x7, 0x8, 0xB, 0x9, 0xC, 0xA, 0xD, 0xE, -1,
              -1, 0x2, 0x0, 0x3, 0x4, 0x7, 0x5, 0x8, 0x6, 0x9, 0xA, 0xD, 0xB, 0xE, 0xC, 0xF,
              0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, 0x7, 0xA, 0x8, 0xB, 0xC, 0xF, 0xD, -1,
              -1, 0x1, 0x2, 0x5, 0x3, 0x6, 0x4, 0x7, 0x8, 0xB, 0x9, 0xC, 0xA, 0xD, 0xE, -1);
          const __m512i K8_SHUFFLE_X3_12 = SIMD_MM512_SETR_EPI8(
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x0,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x1);

          const __m512i K8_SHUFFLE_X3_20 = SIMD_MM512_SETR_EPI8(
              0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              0xE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              0xF, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
          const __m512i K8_SHUFFLE_X3_21 = SIMD_MM512_SETR_EPI8(
              -1, 0x2, 0x0, 0x3, 0x4, 0x7, 0x5, 0x8, 0x6, 0x9, 0xA, 0xD, 0xB, 0xE, 0xC, 0xF,
              0x0, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x9, 0x7, 0xA, 0x8, 0xB, 0xC, 0xF, 0xD, -1,
              -1, 0x1, 0x2, 0x5, 0x3, 0x6, 0x4, 0x7, 0x8, 0xB, 0x9, 0xC, 0xA, 0xD, 0xE, -1,
              -1, 0x2, 0x0, 0x3, 0x4, 0x7, 0x5, 0x8, 0x6, 0x9, 0xA, 0xD, 0xB, 0xE, 0xC, 0xF);
          const __m512i K8_SHUFFLE_X3_22 = SIMD_MM512_SETR_EPI8(
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x0,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x1,
              -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

          template <> SIMD_INLINE void ResizerByteBilinearInterpolateX<3>(const uint8_t * alpha, uint8_t * buffer)
          {
              __m512i src[3], shuffled;
              src[0] = Load<true>(buffer + 0 * A);
              src[1] = Load<true>(buffer + 1 * A);
              src[2] = Load<true>(buffer + 2 * A);

              shuffled = _mm512_shuffle_epi8(_mm512_alignr_epi32(src[0], src[0], 12), K8_SHUFFLE_X3_00);
              shuffled = _mm512_or_si512(shuffled, _mm512_shuffle_epi8(src[0], K8_SHUFFLE_X3_01));
              shuffled = _mm512_or_si512(shuffled, _mm512_shuffle_epi8(_mm512_alignr_epi32(src[1], src[0], 4), K8_SHUFFLE_X3_02));
              Store<true>(buffer + 0 * A, _mm512_maddubs_epi16(shuffled, Load<true>(alpha + 0 * A)));

              shuffled = _mm512_shuffle_epi8(_mm512_alignr_epi32(src[1], src[0], 12), K8_SHUFFLE_X3_10);
              shuffled = _mm512_or_si512(shuffled, _mm512_shuffle_epi8(src[1], K8_SHUFFLE_X3_11));
              shuffled = _mm512_or_si512(shuffled, _mm512_shuffle_epi8(_mm512_alignr_epi32(src[2], src[1], 4), K8_SHUFFLE_X3_12));
              Store<true>(buffer + 1 * A, _mm512_maddubs_epi16(shuffled, Load<true>(alpha + 1 * A)));

              shuffled = _mm512_shuffle_epi8(_mm512_alignr_epi32(src[2], src[1], 12), K8_SHUFFLE_X3_20);
              shuffled = _mm512_or_si512(shuffled, _mm512_shuffle_epi8(src[2], K8_SHUFFLE_X3_21));
              shuffled = _mm512_or_si512(shuffled, _mm512_shuffle_epi8(_mm512_alignr_epi32(src[2], src[2], 4), K8_SHUFFLE_X3_22));
              Store<true>(buffer + 2 * A, _mm512_maddubs_epi16(shuffled, Load<true>(alpha + 2 * A)));
          }

          const __m512i K8_SHUFFLE_X4 = SIMD_MM512_SETR_EPI8(
              0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF,
              0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF,
              0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF,
              0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x8, 0xC, 0x9, 0xD, 0xA, 0xE, 0xB, 0xF);

          SIMD_INLINE void ResizerByteBilinearInterpolateX4(const uint8_t * alpha, uint8_t * buffer)
          {
              __m512i _buffer = _mm512_shuffle_epi8(Load<true>(buffer), K8_SHUFFLE_X4);
              Store<true>(buffer, _mm512_maddubs_epi16(_buffer, Load<true>(alpha)));
          }

          template <> SIMD_INLINE void ResizerByteBilinearInterpolateX<4>(const uint8_t * alpha, uint8_t * buffer)
          {
              ResizerByteBilinearInterpolateX4(alpha + 0 * A, buffer + 0 * A);
              ResizerByteBilinearInterpolateX4(alpha + 1 * A, buffer + 1 * A);
              ResizerByteBilinearInterpolateX4(alpha + 2 * A, buffer + 2 * A);
              ResizerByteBilinearInterpolateX4(alpha + 3 * A, buffer + 3 * A);
          }

          const __m512i K16_FRACTION_ROUND_TERM = SIMD_MM512_SET1_EPI16(Base::BILINEAR_ROUND_TERM);

          template<bool align> SIMD_INLINE __m512i ResizerByteBilinearInterpolateY(const uint8_t * pbx0, const uint8_t * pbx1, __m512i alpha[2])
          {
              __m512i sum = _mm512_add_epi16(_mm512_mullo_epi16(Load<align>(pbx0), alpha[0]), _mm512_mullo_epi16(Load<align>(pbx1), alpha[1]));
              return _mm512_srli_epi16(_mm512_add_epi16(sum, K16_FRACTION_ROUND_TERM), Base::BILINEAR_SHIFT);
          }

          template<bool align> SIMD_INLINE void ResizerByteBilinearInterpolateY(const uint8_t * bx0, const uint8_t * bx1, __m512i alpha[2], uint8_t * dst)
          {
              __m512i lo = ResizerByteBilinearInterpolateY<align>(bx0 + 0, bx1 + 0, alpha);
              __m512i hi = ResizerByteBilinearInterpolateY<align>(bx0 + A, bx1 + A, alpha);
              Store<false>(dst, _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_packus_epi16(lo, hi)));
          }

          template <size_t N> SIMD_INLINE void ResizerByteBilinearGather(const uint8_t * src, const int * idx, size_t size, uint8_t * dst)
          {
              struct Src { uint8_t channels[N * 1]; };
              struct Dst { uint8_t channels[N * 2]; };
              const Src * s = (const Src *)src;
              Dst * d = (Dst*)dst;
              for (size_t i = 0; i < size; i++)
                  d[i] = *(Dst *)(s + idx[i]);
          }

          template <> SIMD_INLINE void ResizerByteBilinearGather<2>(const uint8_t * src, const int * idx, size_t size, uint8_t * dst)
          {
              for (size_t i = 0; i < size; i += 16)
              {
  #if defined(__GNUC__) &&  __GNUC__ < 6
                  _mm512_storeu_si512(dst + 4 * i, _mm512_i32gather_epi32(_mm512_loadu_si512(idx + i), (const int *)src, 2));
  #else
                  _mm512_storeu_si512(dst + 4 * i, _mm512_i32gather_epi32(_mm512_loadu_si512(idx + i), src, 2));
  #endif
              }
          }

          template <> SIMD_INLINE void ResizerByteBilinearGather<4>(const uint8_t * src, const int * idx, size_t size, uint8_t * dst)
          {
              for (size_t i = 0; i < size; i += 8)
              {
  #if defined(__GNUC__) &&  __GNUC__ < 6
                  _mm512_storeu_si512(dst + 8 * i, _mm512_i32gather_epi64(_mm256_loadu_si256((__m256i*)(idx + i)), (const long long int*)src, 4));
  #else
                  _mm512_storeu_si512(dst + 8 * i, _mm512_i32gather_epi64(_mm256_loadu_si256((__m256i*)(idx + i)), src, 4));
  #endif
              }
          }

          template<size_t N> void ResizerByteBilinear::Run(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
          {
              struct One { uint8_t val[N * 1]; };
              struct Two { uint8_t val[N * 2]; };

              size_t size = 2 * _param.dstW*N;
              size_t aligned = AlignHi(size, DA) - DA;
              const size_t step = A * N;
              ptrdiff_t previous = -2;
              __m512i a[2];
              uint8_t * bx[2] = { _bx[0].data, _bx[1].data };
              const uint8_t * ax = _ax.data;
              const int32_t * ix = _ix.data;
              size_t dstW = _param.dstW;

              for (size_t yDst = 0; yDst < _param.dstH; yDst++, dst += dstStride)
              {
                  a[0] = _mm512_set1_epi16(int16_t(Base::FRACTION_RANGE - _ay[yDst]));
                  a[1] = _mm512_set1_epi16(int16_t(_ay[yDst]));

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
                      ResizerByteBilinearGather<N>(src + (sy + k)*srcStride, ix, dstW, bx[k]);

                      uint8_t * pbx = bx[k];
                      for (size_t i = 0; i < size; i += step)
                          ResizerByteBilinearInterpolateX<N>(ax + i, pbx + i);
                  }

                  for (size_t ib = 0, id = 0; ib < aligned; ib += DA, id += A)
                      ResizerByteBilinearInterpolateY<true>(bx[0] + ib, bx[1] + ib, a, dst + id);
                  size_t i = size - DA;
                  ResizerByteBilinearInterpolateY<false>(bx[0] + i, bx[1] + i, a, dst + i / 2);
              }
          }

          void ResizerByteBilinear::RunG(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
          {
              size_t bufW = AlignHi(_param.dstW, A) * 2;
              size_t size = 2 * _param.dstW;
              size_t aligned = AlignHi(size, DA) - DA;
              size_t blocks = _blocks;
              ptrdiff_t previous = -2;
              __m512i a[2];
              uint8_t * bx[2] = { _bx[0].data, _bx[1].data };
              const uint8_t * ax = _ax.data;
              const Idx * ixg = _ixg.data;

              for (size_t yDst = 0; yDst < _param.dstH; yDst++, dst += dstStride)
              {
                  a[0] = _mm512_set1_epi16(int16_t(Base::FRACTION_RANGE - _ay[yDst]));
                  a[1] = _mm512_set1_epi16(int16_t(_ay[yDst]));

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
                      const uint8_t * psrc = src + (sy + k)*srcStride;
                      uint8_t * pdst = bx[k];
                      for (size_t i = 0; i < blocks; ++i)
                          Avx2::ResizerByteBilinearLoadGrayInterpolated(psrc, ixg[i], ax, pdst);
                  }

                  for (size_t ib = 0, id = 0; ib < aligned; ib += DA, id += A)
                      ResizerByteBilinearInterpolateY<true>(bx[0] + ib, bx[1] + ib, a, dst + id);
                  size_t i = size - DA;
                  ResizerByteBilinearInterpolateY<false>(bx[0] + i, bx[1] + i, a, dst + i / 2);
              }
          }

          void ResizerByteBilinear::Run(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
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

          //-----------------------------------------------------------------------------------------

          ResizerShortBilinear::ResizerShortBilinear(const ResParam& param)
              : Avx2::ResizerShortBilinear(param)
          {
          }

          const __m512i RSB_1_0 = SIMD_MM512_SETR_EPI8(
              0x0, 0x1, -1, -1, 0x4, 0x5, -1, -1, 0x8, 0x9, -1, -1, 0xC, 0xD, -1, -1,
              0x0, 0x1, -1, -1, 0x4, 0x5, -1, -1, 0x8, 0x9, -1, -1, 0xC, 0xD, -1, -1,
              0x0, 0x1, -1, -1, 0x4, 0x5, -1, -1, 0x8, 0x9, -1, -1, 0xC, 0xD, -1, -1,
              0x0, 0x1, -1, -1, 0x4, 0x5, -1, -1, 0x8, 0x9, -1, -1, 0xC, 0xD, -1, -1);
          const __m512i RSB_1_1 = SIMD_MM512_SETR_EPI8(
              0x2, 0x3, -1, -1, 0x6, 0x7, -1, -1, 0xA, 0xB, -1, -1, 0xE, 0xF, -1, -1,
              0x2, 0x3, -1, -1, 0x6, 0x7, -1, -1, 0xA, 0xB, -1, -1, 0xE, 0xF, -1, -1,
              0x2, 0x3, -1, -1, 0x6, 0x7, -1, -1, 0xA, 0xB, -1, -1, 0xE, 0xF, -1, -1,
              0x2, 0x3, -1, -1, 0x6, 0x7, -1, -1, 0xA, 0xB, -1, -1, 0xE, 0xF, -1, -1);

          SIMD_INLINE __m512 BilColS1(const uint16_t* src, const int32_t* idx, __m512 fx0, __m512 fx1, __mmask16 tail = -1)
          {
              __m512i s = _mm512_mask_i32gather_epi32(K_ZERO, tail, _mm512_maskz_loadu_epi32(tail, idx), (int*)src, 2);
              __m512 m0 = _mm512_mul_ps(fx0, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_1_0)));
              __m512 m1 = _mm512_mul_ps(fx1, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_1_1)));
              return _mm512_add_ps(m0, m1);
          }

          const __m512i RSB_2_0 = SIMD_MM512_SETR_EPI8(
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1);
          const __m512i RSB_2_1 = SIMD_MM512_SETR_EPI8(
              0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1,
              0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1,
              0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1,
              0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1);

          SIMD_INLINE __m512 BilColS2(const uint16_t* src, const int32_t* idx, __m512 fx0, __m512 fx1)
          {
              __m512i s = _mm512_setr_epi64(
                  *(uint64_t*)(src + idx[0]), *(uint64_t*)(src + idx[2]),
                  *(uint64_t*)(src + idx[4]), *(uint64_t*)(src + idx[6]),
                  *(uint64_t*)(src + idx[8]), *(uint64_t*)(src + idx[10]),
                  *(uint64_t*)(src + idx[12]), *(uint64_t*)(src + idx[14]));
              __m512 m0 = _mm512_mul_ps(fx0, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_2_0)));
              __m512 m1 = _mm512_mul_ps(fx1, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_2_1)));
              return _mm512_add_ps(m0, m1);
          }

          SIMD_INLINE __m512 BilColS2(const uint16_t* src, const int32_t* idx, __m512 fx0, __m512 fx1, __mmask16 tail)
          {
              __m512i s = _mm512_i64gather_epi64(_mm512_and_epi32(_mm512_maskz_loadu_epi32(tail, idx), K64_00000000FFFFFFFF), (long long int*)src, 2);
              __m512 m0 = _mm512_mul_ps(fx0, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_2_0)));
              __m512 m1 = _mm512_mul_ps(fx1, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_2_1)));
              return _mm512_add_ps(m0, m1);
          }

          const __m512i RSB_3_0 = SIMD_MM512_SETR_EPI8(
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, -1, -1, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, -1, -1, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, -1, -1, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, -1, -1, -1, -1);
          const __m512i RSB_3_1 = SIMD_MM512_SETR_EPI8(
              0x6, 0x7, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, -1, -1, -1, -1,
              0x6, 0x7, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, -1, -1, -1, -1,
              0x6, 0x7, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, -1, -1, -1, -1,
              0x6, 0x7, -1, -1, 0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, -1, -1, -1, -1);
          const __m512i RSB_3_P1 = SIMD_MM512_SETR_EPI32(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 15, 15, 15);

          SIMD_INLINE __m512 BilColS3(const uint16_t* src, const int32_t* idx, __m512 fx0, __m512 fx1)
          {
              __m512i s = Load<false>((__m128i*)(src + idx[0]), (__m128i*)(src + idx[3]), (__m128i*)(src + idx[6]), (__m128i*)(src + idx[9]));
              __m512 m0 = _mm512_mul_ps(fx0, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_3_0)));
              __m512 m1 = _mm512_mul_ps(fx1, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_3_1)));
              return _mm512_permutexvar_ps(RSB_3_P1, _mm512_add_ps(m0, m1));
          }

          const __m512i RSB_4_0 = SIMD_MM512_SETR_EPI8(
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1,
              0x0, 0x1, -1, -1, 0x2, 0x3, -1, -1, 0x4, 0x5, -1, -1, 0x6, 0x7, -1, -1);
          const __m512i RSB_4_1 = SIMD_MM512_SETR_EPI8(
              0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1,
              0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1,
              0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1,
              0x8, 0x9, -1, -1, 0xA, 0xB, -1, -1, 0xC, 0xD, -1, -1, 0xE, 0xF, -1, -1);

          SIMD_INLINE __m512 BilColS4(const uint16_t* src, const int32_t* idx, __m512 fx0, __m512 fx1)
          {
              __m512i s = Load<false>((__m128i*)(src + idx[0]), (__m128i*)(src + idx[4]), (__m128i*)(src + idx[8]), (__m128i*)(src + idx[12]));
              __m512 m0 = _mm512_mul_ps(fx0, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_4_0)));
              __m512 m1 = _mm512_mul_ps(fx1, _mm512_cvtepi32_ps(_mm512_shuffle_epi8(s, RSB_4_1)));
              return _mm512_add_ps(m0, m1);
          }

          template<size_t N> void ResizerShortBilinear::RunB(const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
          {
              size_t rs = _param.dstW * N;
              float* pbx[2] = { _bx[0].data, _bx[1].data };
              int32_t prev = -2;
              size_t rs12 = AlignLoAny(rs - 1, 12);
              size_t rs16 = AlignLo(rs, 16);
              size_t rs32 = AlignLo(rs, 32);
              __mmask16 tail16 = TailMask16(rs - rs16);
              __m512 _1 = _mm512_set1_ps(1.0f);
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
                          for (; dx < rs16; dx += 16)
                          {
                              __m512 fx1 = _mm512_loadu_ps(_ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_storeu_ps(pb + dx, BilColS1(ps, _ix.data + dx, fx0, fx1));
                          }
                          if (dx < rs)
                          {
                              __m512 fx1 = _mm512_maskz_loadu_ps(tail16, _ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_mask_storeu_ps(pb + dx, tail16, BilColS1(ps, _ix.data + dx, fx0, fx1, tail16));
                          }
                      }
                      if (N == 2)
                      {
                          for (; dx < rs16; dx += 16)
                          {
                              __m512 fx1 = _mm512_loadu_ps(_ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_storeu_ps(pb + dx, BilColS2(ps, _ix.data + dx, fx0, fx1));
                          }
                          if (dx < rs)
                          {
                              __m512 fx1 = _mm512_maskz_loadu_ps(tail16, _ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_mask_storeu_ps(pb + dx, tail16, BilColS2(ps, _ix.data + dx, fx0, fx1, tail16));
                          }
                      }
                      if (N == 3)
                      {
                          for (; dx < rs12; dx += 12)
                          {
                              __m512 fx1 = Load<false>(_ax.data + dx, _ax.data + dx + 3, _ax.data + dx + 6, _ax.data + dx + 9);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_storeu_ps(pb + dx, BilColS3(ps, _ix.data + dx, fx0, fx1));
                          }
                          for (; dx < rs; dx += 3)
                          {
                              __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                              __m128 fx0 = _mm_sub_ps(_mm512_castps512_ps128(_1), fx1);
                              _mm_storeu_ps(pb + dx, Sse41::BilColS3(ps + _ix[dx], fx0, fx1));
                          }
                      }
                      if (N == 4)
                      {
                          for (; dx < rs16; dx += 16)
                          {
                              __m512 fx1 = _mm512_loadu_ps(_ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_storeu_ps(pb + dx, BilColS4(ps, _ix.data + dx, fx0, fx1));
                          }
                          for (; dx < rs; dx += 4)
                          {
                              __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                              __m128 fx0 = _mm_sub_ps(_mm512_castps512_ps128(_1), fx1);
                              _mm_storeu_ps(pb + dx, Sse41::BilColS4(ps + _ix[dx], fx0, fx1));
                          }
                      }
                  }

                  size_t dx = 0;
                  __m512 _fy0 = _mm512_set1_ps(fy0);
                  __m512 _fy1 = _mm512_set1_ps(fy1);
                  for (; dx < rs32; dx += 32)
                  {
                      __m512 m00 = _mm512_mul_ps(_mm512_loadu_ps(pbx[0] + dx + 0), _fy0);
                      __m512 m01 = _mm512_mul_ps(_mm512_loadu_ps(pbx[1] + dx + 0), _fy1);
                      __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m00, m01));
                      __m512 m10 = _mm512_mul_ps(_mm512_loadu_ps(pbx[0] + dx + 16), _fy0);
                      __m512 m11 = _mm512_mul_ps(_mm512_loadu_ps(pbx[1] + dx + 16), _fy1);
                      __m512i i1 = _mm512_cvttps_epi32(_mm512_add_ps(m10, m11));
                      _mm512_storeu_si512(dst + dx, PackU32ToI16(i0, i1));
                  }
                  for (; dx < rs16; dx += 16)
                  {
                      __m512 m0 = _mm512_mul_ps(_mm512_loadu_ps(pbx[0] + dx), _fy0);
                      __m512 m1 = _mm512_mul_ps(_mm512_loadu_ps(pbx[1] + dx), _fy1);
                      __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                      _mm256_storeu_si256((__m256i*)(dst + dx), _mm512_castsi512_si256(PackU32ToI16(i0)));
                  }
                  if (dx < rs)
                  {
                      __m512 m0 = _mm512_mul_ps(_mm512_maskz_loadu_ps(tail16, pbx[0] + dx), _fy0);
                      __m512 m1 = _mm512_mul_ps(_mm512_maskz_loadu_ps(tail16, pbx[1] + dx), _fy1);
                      __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                      _mm256_mask_storeu_epi16((__m256i*)(dst + dx), tail16, _mm512_castsi512_si256(PackU32ToI16(i0)));
                  }
              }
          }

          const __m512i RSB_3_P2 = SIMD_MM512_SETR_EPI32(0, 1, 4, 5, 8, 9, 2, 3, 6, 7, 10, 11, 12, 13, 14, 15);

          SIMD_INLINE __m512i PackU32ToI16Rsb3(__m512i lo, __m512i hi)
          {
              return _mm512_permutexvar_epi32(RSB_3_P2, _mm512_packus_epi32(lo, hi));
          }

          template<size_t N> void ResizerShortBilinear::RunS(const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
          {
              size_t rs = _param.dstW * N;
              size_t rs12 = AlignLoAny(rs - 1, 12);
              size_t rs24 = AlignLoAny(rs - 1, 24);
              size_t rs16 = AlignLo(rs, 16);
              size_t rs32 = AlignLo(rs, 32);
              __mmask16 tail16 = TailMask16(rs - rs16);
              __m512 _1 = _mm512_set1_ps(1.0f);
              for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
              {
                  float fy1 = _ay[dy];
                  float fy0 = 1.0f - fy1;
                  int32_t sy = _iy[dy];
                  const uint16_t* ps0 = src + (sy + 0) * srcStride;
                  const uint16_t* ps1 = src + (sy + 1) * srcStride;
                  size_t dx = 0;
                  __m512 _fy0 = _mm512_set1_ps(fy0);
                  __m512 _fy1 = _mm512_set1_ps(fy1);
                  if (N == 1)
                  {
                      for (; dx < rs32; dx += 32)
                      {
                          __m512 fx01 = _mm512_loadu_ps(_ax.data + dx + 0);
                          __m512 fx00 = _mm512_sub_ps(_1, fx01);
                          __m512 m00 = _mm512_mul_ps(BilColS1(ps0, _ix.data + dx + 0, fx00, fx01), _fy0);
                          __m512 m01 = _mm512_mul_ps(BilColS1(ps1, _ix.data + dx + 0, fx00, fx01), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m00, m01));
                          __m512 fx11 = _mm512_loadu_ps(_ax.data + dx + 16);
                          __m512 fx10 = _mm512_sub_ps(_1, fx11);
                          __m512 m10 = _mm512_mul_ps(BilColS1(ps0, _ix.data + dx + 16, fx10, fx11), _fy0);
                          __m512 m11 = _mm512_mul_ps(BilColS1(ps1, _ix.data + dx + 16, fx10, fx11), _fy1);
                          __m512i i1 = _mm512_cvttps_epi32(_mm512_add_ps(m10, m11));
                          _mm512_storeu_si512(dst + dx, PackU32ToI16(i0, i1));
                      }
                      for (; dx < rs16; dx += 16)
                      {
                          __m512 fx1 = _mm512_loadu_ps(_ax.data + dx);
                          __m512 fx0 = _mm512_sub_ps(_1, fx1);
                          __m512 m0 = _mm512_mul_ps(BilColS1(ps0, _ix.data + dx, fx0, fx1), _fy0);
                          __m512 m1 = _mm512_mul_ps(BilColS1(ps1, _ix.data + dx, fx0, fx1), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                          _mm256_storeu_si256((__m256i*)(dst + dx), _mm512_castsi512_si256(PackU32ToI16(i0)));
                      }
                      if (dx < rs)
                      {
                          __m512 fx1 = _mm512_maskz_loadu_ps(tail16, _ax.data + dx);
                          __m512 fx0 = _mm512_sub_ps(_1, fx1);
                          __m512 m0 = _mm512_mul_ps(BilColS1(ps0, _ix.data + dx, fx0, fx1, tail16), _fy0);
                          __m512 m1 = _mm512_mul_ps(BilColS1(ps1, _ix.data + dx, fx0, fx1, tail16), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                          _mm256_mask_storeu_epi16(dst + dx, tail16, _mm512_castsi512_si256(PackU32ToI16(i0)));
                      }
                  }
                  if (N == 2)
                  {
                      for (; dx < rs32; dx += 32)
                      {
                          __m512 fx01 = _mm512_loadu_ps(_ax.data + dx + 0);
                          __m512 fx00 = _mm512_sub_ps(_1, fx01);
                          __m512 m00 = _mm512_mul_ps(BilColS2(ps0, _ix.data + dx + 0, fx00, fx01), _fy0);
                          __m512 m01 = _mm512_mul_ps(BilColS2(ps1, _ix.data + dx + 0, fx00, fx01), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m00, m01));
                          __m512 fx11 = _mm512_loadu_ps(_ax.data + dx + 16);
                          __m512 fx10 = _mm512_sub_ps(_1, fx11);
                          __m512 m10 = _mm512_mul_ps(BilColS2(ps0, _ix.data + dx + 16, fx10, fx11), _fy0);
                          __m512 m11 = _mm512_mul_ps(BilColS2(ps1, _ix.data + dx + 16, fx10, fx11), _fy1);
                          __m512i i1 = _mm512_cvttps_epi32(_mm512_add_ps(m10, m11));
                          _mm512_storeu_si512(dst + dx, PackU32ToI16(i0, i1));
                      }
                      for (; dx < rs16; dx += 16)
                      {
                          __m512 fx1 = _mm512_loadu_ps(_ax.data + dx);
                          __m512 fx0 = _mm512_sub_ps(_1, fx1);
                          __m512 m0 = _mm512_mul_ps(BilColS2(ps0, _ix.data + dx, fx0, fx1), _fy0);
                          __m512 m1 = _mm512_mul_ps(BilColS2(ps1, _ix.data + dx, fx0, fx1), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                          _mm256_storeu_si256((__m256i*)(dst + dx), _mm512_castsi512_si256(PackU32ToI16(i0)));
                      }
                      if (dx < rs)
                      {
                          __m512 fx1 = _mm512_maskz_loadu_ps(tail16, _ax.data + dx);
                          __m512 fx0 = _mm512_sub_ps(_1, fx1);
                          __m512 m0 = _mm512_mul_ps(BilColS2(ps0, _ix.data + dx, fx0, fx1, tail16), _fy0);
                          __m512 m1 = _mm512_mul_ps(BilColS2(ps1, _ix.data + dx, fx0, fx1, tail16), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                          _mm256_mask_storeu_epi16(dst + dx, tail16, _mm512_castsi512_si256(PackU32ToI16(i0)));
                      }
                  }
                  if (N == 3)
                  {
                      for (; dx < rs24; dx += 24)
                      {
                          __m512 fx01 = Load<false>(_ax.data + dx + 0, _ax.data + dx + 3, _ax.data + dx + 6, _ax.data + dx + 9);
                          __m512 fx00 = _mm512_sub_ps(_1, fx01);
                          __m512 m00 = _mm512_mul_ps(BilColS3(ps0, _ix.data + dx, fx00, fx01), _fy0);
                          __m512 m01 = _mm512_mul_ps(BilColS3(ps1, _ix.data + dx, fx00, fx01), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m00, m01));
                          __m512 fx11 = Load<false>(_ax.data + dx + 12, _ax.data + dx + 15, _ax.data + dx + 18, _ax.data + dx + 21);
                          __m512 fx10 = _mm512_sub_ps(_1, fx11);
                          __m512 m10 = _mm512_mul_ps(BilColS3(ps0, _ix.data + dx + 12, fx10, fx11), _fy0);
                          __m512 m11 = _mm512_mul_ps(BilColS3(ps1, _ix.data + dx + 12, fx10, fx11), _fy1);
                          __m512i i1 = _mm512_cvttps_epi32(_mm512_add_ps(m10, m11));
                          _mm512_storeu_si512((__m512i*)(dst + dx), PackU32ToI16Rsb3(i0, i1));
                      }
                      for (; dx < rs12; dx += 12)
                      {
                          __m512 fx1 = Load<false>(_ax.data + dx, _ax.data + dx + 3, _ax.data + dx + 6, _ax.data + dx + 9);
                          __m512 fx0 = _mm512_sub_ps(_1, fx1);
                          __m512 m0 = _mm512_mul_ps(BilColS3(ps0, _ix.data + dx, fx0, fx1), _fy0);
                          __m512 m1 = _mm512_mul_ps(BilColS3(ps1, _ix.data + dx, fx0, fx1), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                          _mm256_storeu_si256((__m256i*)(dst + dx), _mm512_castsi512_si256(PackU32ToI16(i0)));
                      }
                      for (; dx < rs; dx += 3)
                      {
                          __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                          __m128 fx0 = _mm_sub_ps(_mm512_castps512_ps128(_1), fx1);
                          __m128 m0 = _mm_mul_ps(Sse41::BilColS3(ps0 + _ix[dx], fx0, fx1), _mm512_castps512_ps128(_fy0));
                          __m128 m1 = _mm_mul_ps(Sse41::BilColS3(ps1 + _ix[dx], fx0, fx1), _mm512_castps512_ps128(_fy1));
                          __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m0, m1));
                          _mm_mask_storeu_epi16((__m128i*)(dst + dx), 0x7, _mm_packus_epi32(i0, Sse41::K_ZERO));
                      }
                  }
                  if (N == 4)
                  {
                      for (; dx < rs32; dx += 32)
                      {
                          __m512 fx01 = _mm512_loadu_ps(_ax.data + dx + 0);
                          __m512 fx00 = _mm512_sub_ps(_1, fx01);
                          __m512 m00 = _mm512_mul_ps(BilColS4(ps0, _ix.data + dx + 0, fx00, fx01), _fy0);
                          __m512 m01 = _mm512_mul_ps(BilColS4(ps1, _ix.data + dx + 0, fx00, fx01), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m00, m01));
                          __m512 fx11 = _mm512_loadu_ps(_ax.data + dx + 16);
                          __m512 fx10 = _mm512_sub_ps(_1, fx11);
                          __m512 m10 = _mm512_mul_ps(BilColS4(ps0, _ix.data + dx + 16, fx10, fx11), _fy0);
                          __m512 m11 = _mm512_mul_ps(BilColS4(ps1, _ix.data + dx + 16, fx10, fx11), _fy1);
                          __m512i i1 = _mm512_cvttps_epi32(_mm512_add_ps(m10, m11));
                          _mm512_storeu_si512((__m512i*)(dst + dx), PackU32ToI16(i0, i1));
                      }
                      for (; dx < rs16; dx += 16)
                      {
                          __m512 fx1 = _mm512_loadu_ps(_ax.data + dx);
                          __m512 fx0 = _mm512_sub_ps(_1, fx1);
                          __m512 m0 = _mm512_mul_ps(BilColS4(ps0, _ix.data + dx, fx0, fx1), _fy0);
                          __m512 m1 = _mm512_mul_ps(BilColS4(ps1, _ix.data + dx, fx0, fx1), _fy1);
                          __m512i i0 = _mm512_cvttps_epi32(_mm512_add_ps(m0, m1));
                          _mm256_storeu_si256((__m256i*)(dst + dx), _mm512_castsi512_si256(PackU32ToI16(i0)));
                      }
                      for (; dx < rs; dx += 4)
                      {
                          __m128 fx1 = _mm_loadu_ps(_ax.data + dx);
                          __m128 fx0 = _mm_sub_ps(_mm512_castps512_ps128(_1), fx1);
                          __m128 m0 = _mm_mul_ps(Sse41::BilColS4(ps0 + _ix[dx], fx0, fx1), _mm512_castps512_ps128(_fy0));
                          __m128 m1 = _mm_mul_ps(Sse41::BilColS4(ps1 + _ix[dx], fx0, fx1), _mm512_castps512_ps128(_fy1));
                          __m128i i0 = _mm_cvttps_epi32(_mm_add_ps(m0, m1));
                          _mm_storel_epi64((__m128i*)(dst + dx), _mm_packus_epi32(i0, Sse41::K_ZERO));
                      }
                  }
              }
          }

          void ResizerShortBilinear::Run(const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
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

          //-----------------------------------------------------------------------------------------

          ResizerFloatBilinear::ResizerFloatBilinear(const ResParam& param)
              : Base::ResizerFloatBilinear(param)
          {
          }

          void ResizerFloatBilinear::Run(const float* src, size_t srcStride, float* dst, size_t dstStride)
          {
              size_t cn = _param.channels;
              size_t rs = _param.dstW * cn;
              float* pbx[2] = { _bx[0].data, _bx[1].data };
              int32_t prev = -2;
              size_t rsa = AlignLo(rs, Avx512bw::F);
              __mmask16 tail = TailMask16(rs - rsa);
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
                      const float* ps = src + (sy + k) * srcStride;
                      size_t dx = 0;
                      if (cn == 1)
                      {
                          __m512 _1 = _mm512_set1_ps(1.0f);
                          for (; dx < rsa; dx += Avx512bw::F)
                          {
                              __m512i idx = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_load_si512(_ix.data + dx));
                              __m512 sp0 = _mm512_castpd_ps(_mm512_i32gather_pd(_mm512_extracti64x4_epi64(idx, 0), (double*)ps, 4));
                              __m512 sp1 = _mm512_castpd_ps(_mm512_i32gather_pd(_mm512_extracti64x4_epi64(idx, 1), (double*)ps, 4));
                              __m512 fx1 = _mm512_load_ps(_ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              __m512 s0 = _mm512_shuffle_ps(sp0, sp1, 0x88);
                              __m512 s1 = _mm512_shuffle_ps(sp0, sp1, 0xDD);
                              _mm512_store_ps(pb + dx, _mm512_fmadd_ps(s0, fx0, _mm512_mul_ps(s1, fx1)));
                          }
                          if (dx < rs)
                          {
                              __m512i idx = _mm512_permutexvar_epi64(K64_PERMUTE_FOR_PACK, _mm512_maskz_load_epi32(tail, _ix.data + dx));
                              __m512 sp0 = _mm512_castpd_ps(_mm512_i32gather_pd(_mm512_extracti64x4_epi64(idx, 0), (double*)ps, 4));
                              __m512 sp1 = _mm512_castpd_ps(_mm512_i32gather_pd(_mm512_extracti64x4_epi64(idx, 1), (double*)ps, 4));
                              __m512 fx1 = _mm512_maskz_load_ps(tail, _ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              __m512 s0 = _mm512_shuffle_ps(sp0, sp1, 0x88);
                              __m512 s1 = _mm512_shuffle_ps(sp0, sp1, 0xDD);
                              _mm512_mask_store_ps(pb + dx, tail, _mm512_fmadd_ps(s0, fx0, _mm512_mul_ps(s1, fx1)));
                          }
                      }
                      else if (cn == 3 && rs > 3)
                      {
                          __m256 _1 = _mm256_set1_ps(1.0f);
                          size_t rs3 = rs - 3;
                          size_t rs6 = AlignLoAny(rs3, 6);
                          for (; dx < rs6; dx += 6)
                          {
                              __m256 s0 = Avx::Load<false>(ps + _ix[dx + 0] + 0, ps + _ix[dx + 3] + 0);
                              __m256 s1 = Avx::Load<false>(ps + _ix[dx + 0] + 3, ps + _ix[dx + 3] + 3);
                              __m256 fx1 = Avx::Load<false>(_ax.data + dx + 0, _ax.data + dx + 3);
                              __m256 fx0 = _mm256_sub_ps(_1, fx1);
                              Avx::Store<false>(pb + dx + 0, pb + dx + 3, _mm256_fmadd_ps(fx0, s0, _mm256_mul_ps(fx1, s1)));
                          }
                          for (; dx < rs3; dx += 3)
                          {
                              __m128 s0 = _mm_loadu_ps(ps + _ix[dx] + 0);
                              __m128 s1 = _mm_loadu_ps(ps + _ix[dx] + 3);
                              __m128 fx1 = _mm_set1_ps(_ax.data[dx]);
                              __m128 fx0 = _mm_sub_ps(_mm256_castps256_ps128(_1), fx1);
                              _mm_storeu_ps(pb + dx, _mm_add_ps(_mm_mul_ps(fx0, s0), _mm_mul_ps(fx1, s1)));
                          }
                          for (; dx < rs; dx++)
                          {
                              int32_t sx = _ix[dx];
                              float fx = _ax[dx];
                              pb[dx] = ps[sx] * (1.0f - fx) + ps[sx + cn] * fx;
                          }
                      }
                      else
                      {
                          __m512 _1 = _mm512_set1_ps(1.0f);
                          __m512i _cn = _mm512_set1_epi32((int)cn);
                          for (; dx < rsa; dx += Avx512bw::F)
                          {
                              __m512i i0 = _mm512_load_si512(_ix.data + dx);
                              __m512i i1 = _mm512_add_epi32(i0, _cn);
                              __m512 s0 = _mm512_i32gather_ps(i0, ps, 4);
                              __m512 s1 = _mm512_i32gather_ps(i1, ps, 4);
                              __m512 fx1 = _mm512_load_ps(_ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_store_ps(pb + dx, _mm512_fmadd_ps(s0, fx0, _mm512_mul_ps(s1, fx1)));
                          }
                          if (dx < rs)
                          {
                              __m512i i0 = _mm512_maskz_load_epi32(tail, _ix.data + dx);
                              __m512i i1 = _mm512_add_epi32(i0, _cn);
                              __m512 s0 = _mm512_i32gather_ps(i0, ps, 4);
                              __m512 s1 = _mm512_i32gather_ps(i1, ps, 4);
                              __m512 fx1 = _mm512_maskz_load_ps(tail, _ax.data + dx);
                              __m512 fx0 = _mm512_sub_ps(_1, fx1);
                              _mm512_mask_store_ps(pb + dx, tail, _mm512_fmadd_ps(s0, fx0, _mm512_mul_ps(s1, fx1)));
                          }
                      }
                  }
                  size_t dx = 0;
                  __m512 _fy0 = _mm512_set1_ps(fy0);
                  __m512 _fy1 = _mm512_set1_ps(fy1);
                  for (; dx < rsa; dx += Avx512bw::F)
                  {
                      __m512 b0 = _mm512_loadu_ps(pbx[0] + dx);
                      __m512 b1 = _mm512_loadu_ps(pbx[1] + dx);
                      _mm512_storeu_ps(dst + dx, _mm512_fmadd_ps(b0, _fy0, _mm512_mul_ps(b1, _fy1)));
                  }
                  if (dx < rs)
                  {
                      __m512 b0 = _mm512_maskz_loadu_ps(tail, pbx[0] + dx);
                      __m512 b1 = _mm512_maskz_loadu_ps(tail, pbx[1] + dx);
                      _mm512_mask_storeu_ps(dst + dx, tail, _mm512_fmadd_ps(b0, _fy0, _mm512_mul_ps(b1, _fy1)));
                  }
              }
          }
  //}}}
  //{{{  bicubic
          ResizerByteBicubic::ResizerByteBicubic(const ResParam& param)
              : Avx2::ResizerByteBicubic(param)
          {
          }

  #ifndef SIMD_AVX512BW_RESIZER_BYTE_BICUBIC_MSVS_COMPER_ERROR
          template<int N> __m512i LoadAx(const int8_t* ax);

          template<> SIMD_INLINE __m512i LoadAx<1>(const int8_t* ax)
          {
              return _mm512_loadu_si512((__m512i*)ax);
          }

          template<> SIMD_INLINE __m512i LoadAx<2>(const int8_t* ax)
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7);
              return _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi256_si512(_mm256_loadu_si256((__m256i*)ax)));
          }

          template<> SIMD_INLINE __m512i LoadAx<3>(const int8_t* ax)
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 0, 0, 0, 0);
              return _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi128_si512(_mm_loadu_si128((__m128i*)ax)));
          }

          template<> SIMD_INLINE __m512i LoadAx<4>(const int8_t* ax)
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
              return _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi128_si512(_mm_loadu_si128((__m128i*)ax)));
          }

          template<int N> __m512i CubicSumX(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay);

          template<> SIMD_INLINE __m512i CubicSumX<1>(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay)
          {
              __m512i _src = _mm512_i32gather_epi32(_mm512_loadu_si512((__m512i*)ix), (int32_t*)src, 1);
              return  _mm512_madd_epi16(_mm512_maddubs_epi16(_src, ax), ay);
          }

          template<> SIMD_INLINE __m512i CubicSumX<2>(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay)
          {
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF);
              __m512i _src = _mm512_shuffle_epi8(_mm512_i32gather_epi64(_mm256_loadu_si256((__m256i*)ix), (long long*)src, 1), SHUFFLE);
              return _mm512_madd_epi16(_mm512_maddubs_epi16(_src, ax), ay);
          }

          template<> SIMD_INLINE __m512i CubicSumX<3>(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay)
          {
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1);
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 0, 0, 0, 0);
              __m512i _src = _mm512_permutexvar_epi32(PERMUTE, _mm512_shuffle_epi8(
                  Load<false>((__m128i*)(src + ix[0]), (__m128i*)(src + ix[1]),
                      (__m128i*)(src + ix[2]), (__m128i*)(src + ix[3])), SHUFFLE));
              return _mm512_madd_epi16(_mm512_maddubs_epi16(_src, ax), ay);
          }

          template<> SIMD_INLINE __m512i CubicSumX<4>(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay)
          {
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF);
              __m512i _src = _mm512_shuffle_epi8(Load<false>((__m128i*)(src + ix[0]),
                  (__m128i*)(src + ix[1]), (__m128i*)(src + ix[2]), (__m128i*)(src + ix[3])), SHUFFLE);
              return _mm512_madd_epi16(_mm512_maddubs_epi16(_src, ax), ay);
          }

          template <int N> SIMD_INLINE void StoreBicubicInt(__m512i val, uint8_t* dst)
          {
              _mm_storeu_si128((__m128i*)dst, _mm512_cvtusepi32_epi8(_mm512_max_epi32(val, _mm512_setzero_si512())));
          }

          template <int N> SIMD_INLINE void BicubicInt(const uint8_t* src0, const uint8_t* src1, const uint8_t* src2, const uint8_t* src3, const int32_t* ix, const int8_t* ax, const __m512i* ay, uint8_t* dst)
          {
              static const __m512i ROUND = SIMD_MM512_SET1_EPI32(Base::BICUBIC_ROUND);
              __m512i _ax = LoadAx<N>(ax);
              __m512i say0 = CubicSumX<N>(src0 - N, ix, _ax, ay[0]);
              __m512i say1 = CubicSumX<N>(src1 - N, ix, _ax, ay[1]);
              __m512i say2 = CubicSumX<N>(src2 - N, ix, _ax, ay[2]);
              __m512i say3 = CubicSumX<N>(src3 - N, ix, _ax, ay[3]);
              __m512i sum = _mm512_add_epi32(_mm512_add_epi32(say0, say1), _mm512_add_epi32(say2, say3));
              __m512i dst0 = _mm512_srai_epi32(_mm512_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
              StoreBicubicInt<N>(dst0, dst);
          }

          template<int N> void ResizerByteBicubic::RunS(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              assert(_xn == 0 && _xt == _param.dstW);
              size_t step = 4 / N * 4;
              size_t body = AlignLoAny(_param.dstW, step);
              for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
              {
                  size_t sy = _iy[dy];
                  const uint8_t* src1 = src + sy * srcStride;
                  const uint8_t* src2 = src1 + srcStride;
                  const uint8_t* src0 = sy ? src1 - srcStride : src1;
                  const uint8_t* src3 = sy < _param.srcH - 2 ? src2 + srcStride : src2;
                  const int32_t* ay = _ay.data + dy * 4;
                  __m512i ays[4];
                  ays[0] = _mm512_set1_epi16(ay[0]);
                  ays[1] = _mm512_set1_epi16(ay[1]);
                  ays[2] = _mm512_set1_epi16(ay[2]);
                  ays[3] = _mm512_set1_epi16(ay[3]);
                  size_t dx = 0;
                  for (; dx < body; dx += step)
                      BicubicInt<N>(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * N);
                  for (; dx < _param.dstW; dx++)
                      Base::BicubicInt<N, -1, 2>(src0, src1, src2, src3, _ix[dx], _ax.data + dx * 4, ay, dst + dx * N);
              }
          }

          //-----------------------------------------------------------------------------------------

          SIMD_INLINE __m512i LoadAx1(const int8_t* ax, __mmask16 mask)
          {
              return _mm512_maskz_loadu_epi32(mask, ax);
          }

          SIMD_INLINE __m512i CubicSumX1(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay, __mmask16 mask)
          {
              __m512i _src = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask, _mm512_maskz_loadu_epi32(mask, ix), (int32_t*)src, 1);
              return  _mm512_madd_epi16(_mm512_maddubs_epi16(_src, ax), ay);
          }

          SIMD_INLINE void BicubicInt1(const uint8_t* src0, const uint8_t* src1, const uint8_t* src2, const uint8_t* src3,
              const int32_t* ix, const int8_t* ax, const __m512i* ay, uint8_t* dst, __mmask16 mask)
          {
              static const __m512i ROUND = SIMD_MM512_SET1_EPI32(Base::BICUBIC_ROUND);
              __m512i _ax = LoadAx1(ax, mask);
              __m512i say0 = CubicSumX1(src0 - 1, ix, _ax, ay[0], mask);
              __m512i say1 = CubicSumX1(src1 - 1, ix, _ax, ay[1], mask);
              __m512i say2 = CubicSumX1(src2 - 1, ix, _ax, ay[2], mask);
              __m512i say3 = CubicSumX1(src3 - 1, ix, _ax, ay[3], mask);
              __m512i sum = _mm512_add_epi32(_mm512_add_epi32(say0, say1), _mm512_add_epi32(say2, say3));
              __m512i dst0 = _mm512_srai_epi32(_mm512_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
              _mm_mask_storeu_epi8(dst, mask, _mm512_cvtusepi32_epi8(_mm512_max_epi32(dst0, _mm512_setzero_si512())));
          }

          template<> void ResizerByteBicubic::RunS<1>(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              assert(_xn == 0 && _xt == _param.dstW);
              size_t step = 16;
              size_t body = AlignLoAny(_param.dstW, step);
              __mmask16 tail = TailMask16(_param.dstW - body);
              for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
              {
                  size_t sy = _iy[dy];
                  const uint8_t* src1 = src + sy * srcStride;
                  const uint8_t* src2 = src1 + srcStride;
                  const uint8_t* src0 = sy ? src1 - srcStride : src1;
                  const uint8_t* src3 = sy < _param.srcH - 2 ? src2 + srcStride : src2;
                  const int32_t* ay = _ay.data + dy * 4;
                  __m512i ays[4];
                  ays[0] = _mm512_set1_epi16(ay[0]);
                  ays[1] = _mm512_set1_epi16(ay[1]);
                  ays[2] = _mm512_set1_epi16(ay[2]);
                  ays[3] = _mm512_set1_epi16(ay[3]);
                  size_t dx = 0;
                  for (; dx < body; dx += step)
                      BicubicInt<1>(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx);
                  if(tail)
                      BicubicInt1(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx, tail);
              }
          }

          //-----------------------------------------------------------------------------------------

          SIMD_INLINE __m512i LoadAx2(const int8_t* ax, __mmask8 mask = __mmask8(-1))
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7);
              return _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi256_si512(_mm256_maskz_loadu_epi32(mask, ax)));
          }

          SIMD_INLINE __m512i CubicSumX2(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay, __mmask8 mask = __mmask8(-1))
          {
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF);
              __m512i _src = _mm512_mask_i32gather_epi64(_mm512_setzero_si512(), mask, _mm256_maskz_loadu_epi32(mask, ix), (long long*)src, 1);
              return _mm512_madd_epi16(_mm512_maddubs_epi16(_mm512_shuffle_epi8(_src, SHUFFLE), ax), ay);
          }

          SIMD_INLINE void BicubicInt2(const uint8_t* src0, const uint8_t* src1, const uint8_t* src2, const uint8_t* src3,
              const int32_t* ix, const int8_t* ax, const __m512i* ay, uint8_t* dst, __mmask8 mask = __mmask8(-1))
          {
              static const __m512i ROUND = SIMD_MM512_SET1_EPI32(Base::BICUBIC_ROUND);
              __m512i _ax = LoadAx2(ax, mask);
              __m512i say0 = CubicSumX2(src0 - 2, ix, _ax, ay[0], mask);
              __m512i say1 = CubicSumX2(src1 - 2, ix, _ax, ay[1], mask);
              __m512i say2 = CubicSumX2(src2 - 2, ix, _ax, ay[2], mask);
              __m512i say3 = CubicSumX2(src3 - 2, ix, _ax, ay[3], mask);
              __m512i sum = _mm512_add_epi32(_mm512_add_epi32(say0, say1), _mm512_add_epi32(say2, say3));
              __m512i dst0 = _mm512_srai_epi32(_mm512_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
              _mm_mask_storeu_epi16((int16_t*)dst, mask, _mm512_cvtusepi32_epi8(_mm512_max_epi32(dst0, _mm512_setzero_si512())));
          }

          template<> void ResizerByteBicubic::RunS<2>(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              assert(_xn == 0 && _xt == _param.dstW);
              size_t step = 8;
              size_t body = AlignLoAny(_param.dstW, step);
              __mmask8 tail = TailMask8(_param.dstW - body);
              for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
              {
                  size_t sy = _iy[dy];
                  const uint8_t* src1 = src + sy * srcStride;
                  const uint8_t* src2 = src1 + srcStride;
                  const uint8_t* src0 = sy ? src1 - srcStride : src1;
                  const uint8_t* src3 = sy < _param.srcH - 2 ? src2 + srcStride : src2;
                  const int32_t* ay = _ay.data + dy * 4;
                  __m512i ays[4];
                  ays[0] = _mm512_set1_epi16(ay[0]);
                  ays[1] = _mm512_set1_epi16(ay[1]);
                  ays[2] = _mm512_set1_epi16(ay[2]);
                  ays[3] = _mm512_set1_epi16(ay[3]);
                  size_t dx = 0;
                  for (; dx < body; dx += step)
                      BicubicInt2(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * 2);
                  if (tail)
                      BicubicInt2(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * 2, tail);
              }
          }

          //-----------------------------------------------------------------------------------------

          SIMD_INLINE __m512i LoadAx3(const int8_t* ax, __mmask8 srcMask)
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 0, 0, 0, 0);
              return _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi128_si512(_mm_maskz_loadu_epi32(srcMask, ax)));
          }

          SIMD_INLINE __m512i CubicSumX3(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay, __mmask8 * srcMask)
          {
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1);
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 0, 0, 0, 0);
              __m128i src0 = _mm_maskz_loadu_epi32(srcMask[0], src + ix[0]);
              __m128i src1 = _mm_maskz_loadu_epi32(srcMask[1], src + ix[1]);
              __m128i src2 = _mm_maskz_loadu_epi32(srcMask[2], src + ix[2]);
              __m128i src3 = _mm_maskz_loadu_epi32(srcMask[3], src + ix[3]);
              __m512i _src = _mm512_permutexvar_epi32(PERMUTE, _mm512_shuffle_epi8(Set(src0, src1, src2, src3), SHUFFLE));
              return _mm512_madd_epi16(_mm512_maddubs_epi16(_src, ax), ay);
          }

          SIMD_INLINE void BicubicInt3(const uint8_t* src0, const uint8_t* src1, const uint8_t* src2, const uint8_t* src3,
              const int32_t* ix, const int8_t* ax, const __m512i* ay, uint8_t* dst, __mmask8 srcMask[5], __mmask16 dstMask)
          {
              static const __m512i ROUND = SIMD_MM512_SET1_EPI32(Base::BICUBIC_ROUND);
              __m512i _ax = LoadAx3(ax, srcMask[4]);
              __m512i say0 = CubicSumX3(src0 - 3, ix, _ax, ay[0], srcMask);
              __m512i say1 = CubicSumX3(src1 - 3, ix, _ax, ay[1], srcMask);
              __m512i say2 = CubicSumX3(src2 - 3, ix, _ax, ay[2], srcMask);
              __m512i say3 = CubicSumX3(src3 - 3, ix, _ax, ay[3], srcMask);
              __m512i sum = _mm512_add_epi32(_mm512_add_epi32(say0, say1), _mm512_add_epi32(say2, say3));
              __m512i dst0 = _mm512_srai_epi32(_mm512_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
              _mm_mask_storeu_epi8(dst, dstMask, _mm512_cvtusepi32_epi8(_mm512_max_epi32(dst0, _mm512_setzero_si512())));
          }

          template<> void ResizerByteBicubic::RunS<3>(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              assert(_xn == 0 && _xt == _param.dstW);
              size_t step = 4;
              size_t body = AlignLoAny(_param.dstW - 1, step), tail = _param.dstW - body;
              __mmask8 srcMaskTail[5];
              srcMaskTail[0] = tail > 0 ? 0x7 : 0x0;
              srcMaskTail[1] = tail > 1 ? 0x7 : 0x0;
              srcMaskTail[2] = tail > 2 ? 0x7 : 0x0;
              srcMaskTail[3] = tail > 3 ? 0x7 : 0x0;
              srcMaskTail[4] = TailMask8(tail);
              __mmask16 dstMaskTail = TailMask16(tail * 3);
              for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
              {
                  size_t sy = _iy[dy];
                  const uint8_t* src1 = src + sy * srcStride;
                  const uint8_t* src2 = src1 + srcStride;
                  const uint8_t* src0 = sy ? src1 - srcStride : src1;
                  const uint8_t* src3 = sy < _param.srcH - 2 ? src2 + srcStride : src2;
                  const int32_t* ay = _ay.data + dy * 4;
                  __m512i ays[4];
                  ays[0] = _mm512_set1_epi16(ay[0]);
                  ays[1] = _mm512_set1_epi16(ay[1]);
                  ays[2] = _mm512_set1_epi16(ay[2]);
                  ays[3] = _mm512_set1_epi16(ay[3]);
                  size_t dx = 0;
                  for (; dx < body; dx += step)
                      BicubicInt<3>(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * 3);
                  if (tail)
                      BicubicInt3(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * 3, srcMaskTail, dstMaskTail);
              }
          }

          //-----------------------------------------------------------------------------------------

          SIMD_INLINE __m512i LoadAx4(const int8_t* ax, __mmask8 srcMask)
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
              return _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi128_si512(_mm_maskz_loadu_epi32(srcMask, ax)));
          }

          SIMD_INLINE __m512i CubicSumX4(const uint8_t* src, const int32_t* ix, __m512i ax, __m512i ay, __mmask8* srcMask)
          {
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF);
              __m128i src0 = _mm_maskz_loadu_epi32(srcMask[0], src + ix[0]);
              __m128i src1 = _mm_maskz_loadu_epi32(srcMask[1], src + ix[1]);
              __m128i src2 = _mm_maskz_loadu_epi32(srcMask[2], src + ix[2]);
              __m128i src3 = _mm_maskz_loadu_epi32(srcMask[3], src + ix[3]);
              __m512i _src = _mm512_shuffle_epi8(Set(src0, src1, src2, src3), SHUFFLE);
              return _mm512_madd_epi16(_mm512_maddubs_epi16(_src, ax), ay);
          }

          SIMD_INLINE void BicubicInt4(const uint8_t* src0, const uint8_t* src1, const uint8_t* src2, const uint8_t* src3,
              const int32_t* ix, const int8_t* ax, const __m512i* ay, uint8_t* dst, __mmask8 srcMask[5], __mmask16 dstMask)
          {
              static const __m512i ROUND = SIMD_MM512_SET1_EPI32(Base::BICUBIC_ROUND);
              __m512i _ax = LoadAx4(ax, srcMask[4]);
              __m512i say0 = CubicSumX4(src0 - 4, ix, _ax, ay[0], srcMask);
              __m512i say1 = CubicSumX4(src1 - 4, ix, _ax, ay[1], srcMask);
              __m512i say2 = CubicSumX4(src2 - 4, ix, _ax, ay[2], srcMask);
              __m512i say3 = CubicSumX4(src3 - 4, ix, _ax, ay[3], srcMask);
              __m512i sum = _mm512_add_epi32(_mm512_add_epi32(say0, say1), _mm512_add_epi32(say2, say3));
              __m512i dst0 = _mm512_srai_epi32(_mm512_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
              _mm_mask_storeu_epi8(dst, dstMask, _mm512_cvtusepi32_epi8(_mm512_max_epi32(dst0, _mm512_setzero_si512())));
          }

          template<> void ResizerByteBicubic::RunS<4>(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              assert(_xn == 0 && _xt == _param.dstW);
              size_t step = 4;
              size_t body = AlignLoAny(_param.dstW, step), tail = _param.dstW - body;
              __mmask8 srcMaskTail[5];
              srcMaskTail[0] = tail > 0 ? 0xF : 0x0;
              srcMaskTail[1] = tail > 1 ? 0xF : 0x0;
              srcMaskTail[2] = tail > 2 ? 0xF : 0x0;
              srcMaskTail[3] = tail > 3 ? 0xF : 0x0;
              srcMaskTail[4] = TailMask8(tail);
              __mmask16 dstMaskTail = TailMask16(tail * 4);
              for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
              {
                  size_t sy = _iy[dy];
                  const uint8_t* src1 = src + sy * srcStride;
                  const uint8_t* src2 = src1 + srcStride;
                  const uint8_t* src0 = sy ? src1 - srcStride : src1;
                  const uint8_t* src3 = sy < _param.srcH - 2 ? src2 + srcStride : src2;
                  const int32_t* ay = _ay.data + dy * 4;
                  __m512i ays[4];
                  ays[0] = _mm512_set1_epi16(ay[0]);
                  ays[1] = _mm512_set1_epi16(ay[1]);
                  ays[2] = _mm512_set1_epi16(ay[2]);
                  ays[3] = _mm512_set1_epi16(ay[3]);
                  size_t dx = 0;
                  for (; dx < body; dx += step)
                      BicubicInt<4>(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * 4);
                  if (tail)
                      BicubicInt4(src0, src1, src2, src3, _ix.data + dx, _ax.data + dx * 4, ays, dst + dx * 4, srcMaskTail, dstMaskTail);
              }
          }

          //-----------------------------------------------------------------------------------------

          template<int F> SIMD_INLINE void PixelCubicSumX(const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst);

          template<> SIMD_INLINE void PixelCubicSumX<1>(const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
          {
              __m512i _src = _mm512_i32gather_epi32(_mm512_loadu_si512((__m512i*)ix), (int32_t*)src, 1);
              __m512i _ax = _mm512_loadu_si512((__m512i*)ax);
              _mm512_storeu_si512((__m512i*)dst, _mm512_madd_epi16(_mm512_maddubs_epi16(_src, _ax), K16_0001));
          }

          template<> SIMD_INLINE void PixelCubicSumX<2>(const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7);
              __m512i _ax = _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi256_si512(_mm256_loadu_si256((__m256i*)ax)));
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF,
                  0x0, 0x2, 0x4, 0x6, 0x1, 0x3, 0x5, 0x7, 0x8, 0xA, 0xC, 0xE, 0x9, 0xB, 0xD, 0xF);
              __m512i _src = _mm512_shuffle_epi8(_mm512_i32gather_epi64(_mm256_loadu_si256((__m256i*)ix), (long long*)src, 1), SHUFFLE);
              _mm512_storeu_si512((__m512i*)dst, _mm512_madd_epi16(_mm512_maddubs_epi16(_src, _ax), K16_0001));
          }

          template<> SIMD_INLINE void PixelCubicSumX<3>(const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
          {
              static const __m512i PERM_1 = SIMD_MM512_SETR_EPI32(0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 0, 0, 0, 0);
              __m512i _ax = _mm512_permutexvar_epi32(PERM_1, _mm512_castsi128_si512(_mm_loadu_si128((__m128i*)ax)));
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1,
                  0x0, 0x3, 0x6, 0x9, 0x1, 0x4, 0x7, 0xA, 0x2, 0x5, 0x8, 0xB, -1, -1, -1, -1);
              static const __m512i PERM_2 = SIMD_MM512_SETR_EPI32(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 0, 0, 0, 0);
              __m512i _src = _mm512_permutexvar_epi32(PERM_2, _mm512_shuffle_epi8(
                  Load<false>((__m128i*)(src + ix[0]), (__m128i*)(src + ix[1]), (__m128i*)(src + ix[2]), (__m128i*)(src + ix[3])), SHUFFLE));
              _mm512_storeu_si512((__m512i*)dst, _mm512_madd_epi16(_mm512_maddubs_epi16(_src, _ax), K16_0001));
          }

          template<> SIMD_INLINE void PixelCubicSumX<4>(const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst)
          {
              static const __m512i PERMUTE = SIMD_MM512_SETR_EPI32(0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3);
              __m512i _ax = _mm512_permutexvar_epi32(PERMUTE, _mm512_castsi128_si512(_mm_loadu_si128((__m128i*)ax)));
              static const __m512i SHUFFLE = SIMD_MM512_SETR_EPI8(
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF,
                  0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF);
              __m512i _src = _mm512_shuffle_epi8(Load<false>((__m128i*)(src + ix[0]), (__m128i*)(src + ix[1]), (__m128i*)(src + ix[2]), (__m128i*)(src + ix[3])), SHUFFLE);
              _mm512_storeu_si512((__m512i*)dst, _mm512_madd_epi16(_mm512_maddubs_epi16(_src, _ax), K16_0001));
          }

          template<int N> SIMD_INLINE void RowCubicSumX(const uint8_t* src, size_t nose, size_t body, size_t tail, const int32_t* ix, const int8_t* ax, int32_t* dst)
          {
              size_t step = 4 / N * 4;
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

          SIMD_INLINE void BicubicRowInt(const int32_t* src0, const int32_t* src1, const int32_t* src2, const int32_t* src3, const int32_t* ay, size_t body, __mmask16 tail, uint8_t* dst)
          {
              static const __m512i ROUND = SIMD_MM512_SET1_EPI32(Base::BICUBIC_ROUND);
              __m512i ay0 = _mm512_set1_epi32(ay[0]);
              __m512i ay1 = _mm512_set1_epi32(ay[1]);
              __m512i ay2 = _mm512_set1_epi32(ay[2]);
              __m512i ay3 = _mm512_set1_epi32(ay[3]);
              size_t i = 0;
              for (; i < body; i += F)
              {
                  __m512i say0 = _mm512_mullo_epi32(_mm512_loadu_si512((__m512i*)(src0 + i)), ay0);
                  __m512i say1 = _mm512_mullo_epi32(_mm512_loadu_si512((__m512i*)(src1 + i)), ay1);
                  __m512i say2 = _mm512_mullo_epi32(_mm512_loadu_si512((__m512i*)(src2 + i)), ay2);
                  __m512i say3 = _mm512_mullo_epi32(_mm512_loadu_si512((__m512i*)(src3 + i)), ay3);
                  __m512i sum = _mm512_add_epi32(_mm512_add_epi32(say0, say1), _mm512_add_epi32(say2, say3));
                  __m512i dst0 = _mm512_srai_epi32(_mm512_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
                  _mm_storeu_si128((__m128i*)(dst + i), _mm512_cvtusepi32_epi8(_mm512_max_epi32(dst0, _mm512_setzero_si512())));
              }
              if (tail)
              {
                  __m512i say0 = _mm512_mullo_epi32(_mm512_maskz_loadu_epi32(tail, src0 + i), ay0);
                  __m512i say1 = _mm512_mullo_epi32(_mm512_maskz_loadu_epi32(tail, src1 + i), ay1);
                  __m512i say2 = _mm512_mullo_epi32(_mm512_maskz_loadu_epi32(tail, src2 + i), ay2);
                  __m512i say3 = _mm512_mullo_epi32(_mm512_maskz_loadu_epi32(tail, src3 + i), ay3);
                  __m512i sum = _mm512_add_epi32(_mm512_add_epi32(say0, say1), _mm512_add_epi32(say2, say3));
                  __m512i dst0 = _mm512_srai_epi32(_mm512_add_epi32(sum, ROUND), Base::BICUBIC_SHIFT);
                  _mm_mask_storeu_epi8(dst + i, tail, _mm512_cvtusepi32_epi8(_mm512_max_epi32(dst0, _mm512_setzero_si512())));
              }
          }

          template<int N> void ResizerByteBicubic::RunB(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              size_t rowBody = AlignLo(_bx[0].size, F);
              __mmask16 rowTail = TailMask16(_bx[0].size - rowBody);

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
                  BicubicRowInt(pb0, pb1, pb2, pb3, ay, rowBody, rowTail, dst);
              }
          }

          //-----------------------------------------------------------------------------------------

          SIMD_INLINE void PixelCubicSumX1(const uint8_t* src, const int32_t* ix, const int8_t* ax, int32_t* dst, __mmask16 mask)
          {
              __m512i _src = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask, _mm512_maskz_loadu_epi32(mask, ix), (int32_t*)src, 1);
              __m512i _ax = _mm512_maskz_loadu_epi32(mask, ax);
              _mm512_mask_storeu_epi32(dst, mask, _mm512_madd_epi16(_mm512_maddubs_epi16(_src, _ax), K16_0001));
          }

          SIMD_INLINE void RowCubicSumX1(const uint8_t* src, size_t nose, size_t body, size_t tail, const int32_t* ix, const int8_t* ax, int32_t* dst)
          {
              size_t bodyS = nose + AlignLoAny(body - nose, 16);
              size_t bodyTail = body - bodyS;
              __mmask16 bodyTailMask = TailMask16(bodyTail);

              size_t dx = 0;
              for (; dx < nose; dx++, ax += 4, dst += 1)
                  Base::PixelCubicSumX<1, 0, 2>(src + ix[dx], ax, dst);
              for (; dx < bodyS; dx += 16, ax += 64, dst += 16)
                  PixelCubicSumX<1>(src - 1, ix + dx, ax, dst);
              for (; dx < body; dx += bodyTail, ax += 4 * bodyTail, dst += bodyTail)
                  PixelCubicSumX1(src - 1, ix + dx, ax, dst, bodyTailMask);
              for (; dx < tail; dx++, ax += 4, dst += 1)
                  Base::PixelCubicSumX<1, -1, 1>(src + ix[dx], ax, dst);
          }

          template<> void ResizerByteBicubic::RunB<1>(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              size_t rowBody = AlignLo(_bx[0].size, F);
              __mmask16 rowTail = TailMask16(_bx[0].size - rowBody);

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
                      RowCubicSumX1(ps, _xn, _xt, _param.dstW, _ix.data, _ax.data, pb);
                      next++;
                  }
                  prev = next;

                  const int32_t* ay = _ay.data + dy * 4;
                  int32_t* pb0 = _bx[(sy + 0) & 3].data;
                  int32_t* pb1 = _bx[(sy + 1) & 3].data;
                  int32_t* pb2 = _bx[(sy + 2) & 3].data;
                  int32_t* pb3 = _bx[(sy + 3) & 3].data;
                  BicubicRowInt(pb0, pb1, pb2, pb3, ay, rowBody, rowTail, dst);
              }
          }

          //-----------------------------------------------------------------------------------------

          void ResizerByteBicubic::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
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
  #else // SIMD_AVX512BW_RESIZER_BYTE_BICUBIC_MSVS_COMPER_ERROR
          void ResizerByteBicubic::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
          {
              Avx2::ResizerByteBicubic::Run(src, srcStride, dst, dstStride);
          }
  #endif // SIMD_AVX512BW_RESIZER_BYTE_BICUBIC_MSVS_COMPER_ERROR
  //}}}
  //{{{
  void * ResizerInit(size_t srcX, size_t srcY, size_t dstX, size_t dstY, size_t channels, SimdResizeChannelType type, SimdResizeMethodType method)
  {
      ResParam param(srcX, srcY, dstX, dstY, channels, type, method, sizeof(__m512i));
      if (param.IsNearest() && ResizerNearest::Preferable(param))
          return new ResizerNearest(param);
      else if (param.IsByteBilinear() && dstX >= A)
          return new ResizerByteBilinear(param);
      else if (param.IsShortBilinear() && dstX > F)
          return new ResizerShortBilinear(param);
      if (param.IsFloatBilinear())
          return new ResizerFloatBilinear(param);
      else if (param.IsByteBicubic())
          return new ResizerByteBicubic(param);
      else if (param.IsByteArea2x2())
          return new ResizerByteArea2x2(param);
      else if (param.IsByteArea1x1())
          return new ResizerByteArea1x1(param);
      else
          return Avx2::ResizerInit(srcX, srcY, dstX, dstY, channels, type, method);
  }
  //}}}
  }
