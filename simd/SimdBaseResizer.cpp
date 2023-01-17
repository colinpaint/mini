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
#include "SimdResizerCommon.h"
#include "SimdCopyPixel.h"
#include "SimdUpdate.h"

namespace Simd {
  namespace Base {
    // area
    //{{{
    ResizerByteArea::ResizerByteArea(const ResParam& param)
        : Resizer(param)
    {
        _ay.Resize(_param.dstH + 1);
        _iy.Resize(_param.dstH + 1);
        _ax.Resize(_param.dstW + 1);
        _ix.Resize(_param.dstW + 1);
    }
    //}}}
    //{{{
    void ResizerByteArea::EstimateParams(size_t srcSize, size_t dstSize, size_t range, int32_t* alpha, int32_t* index)
    {
        float scale = (float)srcSize / dstSize;

        for (size_t ds = 0; ds <= dstSize; ++ds)
        {
            float a = (float)ds * scale;
            size_t i = (size_t)::floor(a);
            a -= i;
            if (i == srcSize)
            {
                i--;
                a = 1.0f;
            }
            alpha[ds] = int32_t(range * (1.0f - a) / scale);
            index[ds] = int32_t(i);
        }
    }
    //}}}

    //{{{
    ResizerByteArea1x1::ResizerByteArea1x1(const ResParam & param)
        : ResizerByteArea(param)
    {
        EstimateParams(_param.srcH, _param.dstH, Base::AREA_RANGE, _ay.data, _iy.data);
        EstimateParams(_param.srcW, _param.dstW, Base::AREA_RANGE, _ax.data, _ix.data);
        _by.Resize(AlignHi(_param.srcW * _param.channels, _param.align), false, _param.align);
    }
    //}}}

    //{{{
    template<size_t N, UpdateType update> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t* src, int32_t val, int32_t* dst)
    {
        for (size_t c = 0; c < N; ++c)
            Update<update>(dst + c, src[c] * val);
    }
    //}}}
    //{{{
    template<size_t N, UpdateType update> SIMD_INLINE void ResizerByteArea1x1RowUpdate(const uint8_t* src, size_t size, int32_t val, int32_t* dst)
    {
        if (update == UpdateAdd && val == 0)
            return;
        for (size_t i = 0; i < size; i += N, dst += N)
            ResizerByteArea1x1RowUpdate<N, update>(src + i, val, dst);
    }
    //}}}
    //{{{
    template<size_t N> SIMD_INLINE void ResizerByteArea1x1RowSum(const uint8_t* src, size_t stride, size_t count, size_t size, int32_t curr, int32_t zero, int32_t next, int32_t* dst)
    {
        if (count)
        {
            size_t c = 0;
            ResizerByteArea1x1RowUpdate<N, UpdateSet>(src, size, curr, dst), src += stride, c += 1;
            for (; c < count; c += 1, src += stride)
                ResizerByteArea1x1RowUpdate<N, UpdateAdd>(src, size, zero, dst);
            ResizerByteArea1x1RowUpdate<N, UpdateAdd>(src, size, zero - next, dst);
        }
        else
            ResizerByteArea1x1RowUpdate<N, UpdateSet>(src, size, curr - next, dst);
    }
    //}}}
    //{{{
    template<size_t N> void ResizerByteArea1x1::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
    {
        size_t dstW = _param.dstW, rowSize = _param.srcW * N, rowRest = dstStride - dstW * N;
        const int32_t* iy = _iy.data, * ix = _ix.data, * ay = _ay.data, * ax = _ax.data;
        int32_t ay0 = ay[0], ax0 = ax[0];
        for (size_t dy = 0; dy < _param.dstH; dy++, dst += rowRest)
        {
            int32_t* buf = _by.data;
            size_t yn = iy[dy + 1] - iy[dy];
            ResizerByteArea1x1RowSum<N>(src, srcStride, yn, rowSize, ay[dy], ay0, ay[dy + 1], buf), src += yn * srcStride;
            for (size_t dx = 0; dx < dstW; dx++, dst += N)
            {
                size_t xn = ix[dx + 1] - ix[dx];
                ResizerByteAreaResult<N>(buf, xn, ax[dx], ax0, ax[dx + 1], dst), buf += xn * N;
            }
        }
    }
    //}}}
    //{{{
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
    //}}}

    //{{{
    ResizerByteArea2x2::ResizerByteArea2x2(const ResParam& param)
        : ResizerByteArea(param)
    {
        EstimateParams(DivHi(_param.srcH, 2), _param.dstH, Base::AREA_RANGE / 2, _ay.data, _iy.data);
        EstimateParams(DivHi(_param.srcW, 2), _param.dstW, Base::AREA_RANGE / 2, _ax.data, _ix.data);
        _by.Resize(AlignHi(DivHi(_param.srcW, 2) * _param.channels, _param.align) + SIMD_ALIGN, false, _param.align);
    }
    //}}}

    //{{{
    template<size_t N, UpdateType update> SIMD_INLINE void ResizerByteArea2x2RowUpdate(const uint8_t* src0, const uint8_t* src1, size_t size, int32_t val, int32_t* dst)
    {
        if (update == UpdateAdd && val == 0)
            return;
        size_t size2N = AlignLoAny(size, 2 * N);
        size_t i = 0;
        for (; i < size2N; i += 2 * N, dst += N)
            ResizerByteArea2x2RowUpdate<N, N, update>(src0 + i, src1 + i, val, dst);
        if(i < size)
            ResizerByteArea2x2RowUpdate<N, 0, update>(src0 + i, src1 + i, val, dst);
    }
    //}}}
    //{{{
    template<size_t N> SIMD_INLINE void ResizerByteArea2x2RowSum(const uint8_t* src, size_t stride, size_t count, size_t size, int32_t curr, int32_t zero, int32_t next, bool tail, int32_t* dst)
    {
        size_t c = 0;
        if (count)
        {
            ResizerByteArea2x2RowUpdate<N, UpdateSet>(src, src + stride, size, curr, dst), src += 2 * stride, c += 2;
            for (; c < count; c += 2, src += 2 * stride)
                ResizerByteArea2x2RowUpdate<N, UpdateAdd>(src, src + stride, size, zero, dst);
            ResizerByteArea2x2RowUpdate<N, UpdateAdd>(src, tail ? src : src + stride, size, zero - next, dst);
        }
        else
            ResizerByteArea2x2RowUpdate<N, UpdateSet>(src, tail ? src : src + stride, size, curr - next, dst);
    }
    //}}}
    //{{{
    template<size_t N> void ResizerByteArea2x2::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
    {
        size_t dstW = _param.dstW, rowSize = _param.srcW * N, rowRest = dstStride - dstW * N;
        const int32_t* iy = _iy.data, * ix = _ix.data, * ay = _ay.data, * ax = _ax.data;
        int32_t ay0 = ay[0], ax0 = ax[0];
        for (size_t dy = 0; dy < _param.dstH; dy++, dst += rowRest)
        {
            int32_t* buf = _by.data;
            size_t yn = (iy[dy + 1] - iy[dy]) * 2;
            bool tail = (dy == _param.dstH - 1) && (_param.srcH & 1);
            ResizerByteArea2x2RowSum<N>(src, srcStride, yn, rowSize, ay[dy], ay0, ay[dy + 1], tail, buf), src += yn * srcStride;
            for (size_t dx = 0; dx < dstW; dx++, dst += N)
            {
                size_t xn = ix[dx + 1] - ix[dx];
                ResizerByteAreaResult<N>(buf, xn, ax[dx], ax0, ax[dx + 1], dst), buf += xn * N;
            }
        }
    }
    //}}}
    //{{{
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

    // nearest
    //{{{
    ResizerNearest::ResizerNearest(const ResParam& param)
        : Resizer(param)
        , _pixelSize(0)
    {
    }
    //}}}
    //{{{
    void ResizerNearest::EstimateIndex(size_t srcSize, size_t dstSize, size_t channelSize, size_t channels, int32_t* indices)
    {
        if (_param.method == SimdResizeMethodNearest)
        {
            float scale = (float)srcSize / dstSize;
            for (size_t i = 0; i < dstSize; ++i)
            {
                float alpha = (i + 0.5f) * scale;
                int index = RestrictRange((int)::floor(alpha), 0, (int)srcSize - 1);
                for (size_t c = 0; c < channels; c++)
                {
                    size_t offset = i * channels + c;
                    indices[offset] = (int32_t)((channels * index + c) * channelSize);
                }
            }
        }
        else if (_param.method == SimdResizeMethodNearestPytorch)
        {
            for (size_t i = 0; i < dstSize; ++i)
            {
                int index = RestrictRange((int)(i * srcSize / dstSize), 0, (int)srcSize - 1);
                for (size_t c = 0; c < channels; c++)
                {
                    size_t offset = i * channels + c;
                    indices[offset] = (int32_t)((channels * index + c) * channelSize);
                }
            }
        }
        else
            assert(0);
    }
    //}}}
    //{{{
    void ResizerNearest::EstimateParams()
    {
        if (_pixelSize)
            return;

        _pixelSize = _param.PixelSize();

        _iy.Resize(_param.dstH, false, _param.align);
        EstimateIndex(_param.srcH, _param.dstH, 1, 1, _iy.data);

        _ix.Resize(_param.dstW, false, _param.align);
        EstimateIndex(_param.srcW, _param.dstW, _pixelSize, 1, _ix.data);
    }
    //}}}
    //{{{
    void ResizerNearest::Resize(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
    {
        for (size_t dy = 0; dy < _param.dstH; dy++)
        {
            const uint8_t* srcRow = src + _iy[dy] * srcStride;
            for (size_t dx = 0, offset = 0; dx < _param.dstW; dx++, offset += _pixelSize)
                memcpy(dst + offset, srcRow + _ix[dx], _pixelSize);
            dst += dstStride;
        }
    }
    //}}}
    //{{{
    template<size_t N> void ResizerNearest::Resize(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
    {
        for (size_t dy = 0; dy < _param.dstH; dy++)
        {
            const uint8_t * srcRow = src + _iy[dy] * srcStride;
            for (size_t dx = 0, offset = 0; dx < _param.dstW; dx++, offset += N)
                CopyPixel<N>(srcRow + _ix[dx], dst + offset);
            dst += dstStride;
        }
    }
    //}}}
    //{{{
    void ResizerNearest::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
    {
        EstimateParams();
        switch (_pixelSize)
        {
        case 1: Resize<1>(src, srcStride, dst, dstStride); break;
        case 2: Resize<2>(src, srcStride, dst, dstStride); break;
        case 3: Resize<3>(src, srcStride, dst, dstStride); break;
        case 4: Resize<4>(src, srcStride, dst, dstStride); break;
        case 6: Resize<6>(src, srcStride, dst, dstStride); break;
        case 8: Resize<8>(src, srcStride, dst, dstStride); break;
        case 12: Resize<12>(src, srcStride, dst, dstStride); break;
        default:
            Resize(src, srcStride, dst, dstStride);
        }
    }
    //}}}

    // bilinear
    //{{{
    ResizerByteBilinear::ResizerByteBilinear(const ResParam & param)
        : Resizer(param)
    {
        _ay.Resize(_param.dstH);
        _iy.Resize(_param.dstH);
        EstimateIndexAlpha(_param.srcH, _param.dstH, 1, _iy.data, _ay.data);
    }
    //}}}
    //{{{
    void ResizerByteBilinear::EstimateIndexAlpha(size_t srcSize, size_t dstSize, size_t channels, int32_t * indices, int32_t * alphas)
    {
        float scale = (float)srcSize / dstSize;

        for (size_t i = 0; i < dstSize; ++i)
        {
            float alpha = (float)((i + 0.5f)*scale - 0.5f);
            ptrdiff_t index = (ptrdiff_t)::floor(alpha);
            alpha -= index;

            if (index < 0)
            {
                index = 0;
                alpha = 0;
            }

            if (index >(ptrdiff_t)srcSize - 2)
            {
                index = srcSize - 2;
                alpha = 1;
            }

            for (size_t c = 0; c < channels; c++)
            {
                size_t offset = i * channels + c;
                indices[offset] = (int32_t)(channels*index + c);
                alphas[offset] = (int32_t)(alpha * FRACTION_RANGE + 0.5f);
            }
        }
    }
    //}}}
    //{{{
    void ResizerByteBilinear::Run(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
    {
        size_t cn =  _param.channels;
        size_t rs = _param.dstW * cn;
        if (_ax.data == 0)
        {
            _ax.Resize(rs);
            _ix.Resize(rs);
            EstimateIndexAlpha(_param.srcW, _param.dstW, cn, _ix.data, _ax.data);
            _bx[0].Resize(rs);
            _bx[1].Resize(rs);
        }
        int32_t * pbx[2] = { _bx[0].data, _bx[1].data };
        int32_t prev = -2;
        for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
        {
            int32_t fy = _ay[dy];
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
                int32_t * pb = pbx[k];
                const uint8_t * ps = src + (sy + k)*srcStride;
                for (size_t dx = 0; dx < rs; dx++)
                {
                    int32_t sx = _ix[dx];
                    int32_t fx = _ax[dx];
                    int32_t t = ps[sx];
                    pb[dx] = (t << LINEAR_SHIFT) + (ps[sx + cn] - t)*fx;
                }
            }

            if (fy == 0)
                for (size_t dx = 0; dx < rs; dx++)
                    dst[dx] = ((pbx[0][dx] << LINEAR_SHIFT) + BILINEAR_ROUND_TERM) >> BILINEAR_SHIFT;
            else if (fy == FRACTION_RANGE)
                for (size_t dx = 0; dx < rs; dx++)
                    dst[dx] = ((pbx[1][dx] << LINEAR_SHIFT) + BILINEAR_ROUND_TERM) >> BILINEAR_SHIFT;
            else
            {
                for (size_t dx = 0; dx < rs; dx++)
                {
                    int32_t t = pbx[0][dx];
                    dst[dx] = ((t << LINEAR_SHIFT) + (pbx[1][dx] - t)*fy + BILINEAR_ROUND_TERM) >> BILINEAR_SHIFT;
                }
            }
        }
    }
    //}}}

    //{{{
    ResizerShortBilinear::ResizerShortBilinear(const ResParam& param)
        : Resizer(param)
    {
        _ay.Resize(_param.dstH, false, _param.align);
        _iy.Resize(_param.dstH, false, _param.align);
        EstimateIndexAlpha(_param.srcH, _param.dstH, 1, _iy.data, _ay.data);
        size_t rs = _param.dstW * _param.channels + SIMD_ALIGN;
        _ax.Resize(rs, false, _param.align);
        _ix.Resize(rs, false, _param.align);
        EstimateIndexAlpha(_param.srcW, _param.dstW, _param.channels, _ix.data, _ax.data);
        _bx[0].Resize(rs, false, _param.align);
        _bx[1].Resize(rs, false, _param.align);
    }
    //}}}
    //{{{
    void ResizerShortBilinear::EstimateIndexAlpha(size_t srcSize, size_t dstSize, size_t channels, int32_t* indices, float* alphas)
    {
        float scale = (float)srcSize / dstSize;
        for (size_t i = 0; i < dstSize; ++i)
        {
            float alpha = (float)((i + 0.5f) * scale - 0.5f);
            ptrdiff_t index = (ptrdiff_t)::floor(alpha);
            alpha -= index;
            if (index < 0)
            {
                index = 0;
                alpha = 0;
            }
            if (index > (ptrdiff_t)srcSize - 2)
            {
                index = srcSize - 2;
                alpha = 1;
            }
            for (size_t c = 0; c < channels; c++)
            {
                size_t offset = i * channels + c;
                indices[offset] = (int32_t)(channels * index + c);
                alphas[offset] = alpha;
            }
        }
    }
    //}}}
    //{{{
    void ResizerShortBilinear::Run(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
    {
        Run((const uint16_t*)src, srcStride / sizeof(uint16_t), (uint16_t*)dst, dstStride / sizeof(uint16_t));
    }
    //}}}
    //{{{
    template<size_t N> void ResizerShortBilinear::RunB(const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
    {
        size_t rs = _param.dstW * N;
        float* pbx[2] = { _bx[0].data, _bx[1].data };
        int32_t prev = -2;
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
                for (size_t dx = 0; dx < rs; dx++)
                {
                    int32_t sx = _ix[dx];
                    float fx = _ax[dx];
                    pb[dx] = ps[sx] * (1.0f - fx) + ps[sx + N] * fx;
                }
            }
            for (size_t dx = 0; dx < rs; dx++)
                dst[dx] = Round(pbx[0][dx] * fy0 + pbx[1][dx] * fy1);
        }
    }
    //}}}
    //{{{
    template<size_t N> void ResizerShortBilinear::RunS(const uint16_t* src, size_t srcStride, uint16_t* dst, size_t dstStride)
    {
        size_t rs = _param.dstW * N;
        for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
        {
            float fy1 = _ay[dy];
            float fy0 = 1.0f - fy1;
            int32_t sy = _iy[dy];
            const uint16_t* ps0 = src + (sy + 0) * srcStride;
            const uint16_t* ps1 = src + (sy + 1) * srcStride;
            for (size_t dx = 0; dx < rs; dx++)
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
    //}}}

    //{{{
    ResizerFloatBilinear::ResizerFloatBilinear(const ResParam & param)
        : Resizer(param)
    {
        _ay.Resize(_param.dstH, false, _param.align);
        _iy.Resize(_param.dstH, false, _param.align);
        EstimateIndexAlpha(_param.srcH, _param.dstH, 1, _iy.data, _ay.data);
        size_t rs = _param.dstW * _param.channels;
        _ax.Resize(rs, false, _param.align);
        _ix.Resize(rs, false, _param.align);
        EstimateIndexAlpha(_param.srcW, _param.dstW, _param.channels, _ix.data, _ax.data);
        _bx[0].Resize(rs, false, _param.align);
        _bx[1].Resize(rs, false, _param.align);
    }
    //}}}
    //{{{
    void ResizerFloatBilinear::EstimateIndexAlpha(size_t srcSize, size_t dstSize, size_t channels, int32_t * indices, float * alphas)
    {
        if (_param.method == SimdResizeMethodBilinear)
        {
            float scale = (float)srcSize / dstSize;
            for (size_t i = 0; i < dstSize; ++i)
            {
                float alpha = (float)((i + 0.5f) * scale - 0.5f);
                ptrdiff_t index = (ptrdiff_t)::floor(alpha);
                alpha -= index;
                if (index < 0)
                {
                    index = 0;
                    alpha = 0;
                }
                if (index > (ptrdiff_t)srcSize - 2)
                {
                    index = srcSize - 2;
                    alpha = 1;
                }
                for (size_t c = 0; c < channels; c++)
                {
                    size_t offset = i * channels + c;
                    indices[offset] = (int32_t)(channels * index + c);
                    alphas[offset] = alpha;
                }
            }
        }
        else if (_param.method == SimdResizeMethodBilinearCaffe)
        {
            float scale = dstSize > 1 ? float(srcSize - 1) / float(dstSize - 1) : 0.0f;
            for (size_t i = 0; i < dstSize; ++i)
            {
                float alpha = float(i) * scale;
                ptrdiff_t index = (ptrdiff_t)::floor(alpha);
                alpha -= index;
                if (index > (ptrdiff_t)srcSize - 2)
                {
                    index = srcSize - 2;
                    alpha = 1;
                }
                for (size_t c = 0; c < channels; c++)
                {
                    size_t offset = i * channels + c;
                    indices[offset] = (int32_t)(channels * index + c);
                    alphas[offset] = alpha;
                }
            }
        }
        else if (_param.method == SimdResizeMethodBilinearPytorch)
        {
            float scale = (float)srcSize / dstSize;
            for (size_t i = 0; i < dstSize; ++i)
            {
                float alpha = float(i) * scale;
                ptrdiff_t index = (ptrdiff_t)::floor(alpha);
                alpha -= index;
                if (index < 0)
                {
                    index = 0;
                    alpha = 0;
                }
                if (index > (ptrdiff_t)srcSize - 2)
                {
                    index = srcSize - 2;
                    alpha = 1;
                }
                for (size_t c = 0; c < channels; c++)
                {
                    size_t offset = i * channels + c;
                    indices[offset] = (int32_t)(channels * index + c);
                    alphas[offset] = alpha;
                }
            }
        }
        else
            assert(0);
    }
    //}}}
    //{{{
    void ResizerFloatBilinear::Run(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
    {
        Run((const float*)src, srcStride / sizeof(float), (float*)dst, dstStride / sizeof(float));
    }
    //}}}
    //{{{
    void ResizerFloatBilinear::Run(const float * src, size_t srcStride, float * dst, size_t dstStride)
    {
        size_t cn = _param.channels;
        size_t rs = _param.dstW * cn;
        float * pbx[2] = { _bx[0].data, _bx[1].data };
        int32_t prev = -2;
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
                float * pb = pbx[k];
                const float * ps = src + (sy + k)*srcStride;
                for (size_t dx = 0; dx < rs; dx++)
                {
                    int32_t sx = _ix[dx];
                    float fx = _ax[dx];
                    pb[dx] = ps[sx]*(1.0f - fx) + ps[sx + cn]*fx;
                }
            }

            for (size_t dx = 0; dx < rs; dx++)
                dst[dx] = pbx[0][dx]*fy0 + pbx[1][dx]*fy1;
        }
    }
    //}}}

    // bicubic
    //{{{
    ResizerByteBicubic::ResizerByteBicubic(const ResParam & param)
        : Resizer(param)
    {
    }
    //}}}
    //{{{
    void ResizerByteBicubic::EstimateIndexAlpha(size_t sizeS, size_t sizeD, size_t N, Array32i& index, Array32i& alpha)
    {
        index.Resize(sizeD);
        alpha.Resize(sizeD * 4);
        float scale = float(sizeS) / float(sizeD);
        for (size_t i = 0; i < sizeD; ++i)
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
            index[i] = idx * (int)N;
            alpha[i * 4 + 0] = Round(BICUBIC_RANGE * (2.0f - d) * (1.0f - d) * d / 6.0f);
            alpha[i * 4 + 1] = Round(BICUBIC_RANGE * (d - 2.0f) * (d + 1.0f) * (1.0f - d) / 2.0f);
            alpha[i * 4 + 2] = Round(BICUBIC_RANGE * (d - 2.0f) * (d + 1.0f) * d / 2.0f);
            alpha[i * 4 + 3] = Round(BICUBIC_RANGE * (1.0f + d) * (1.0f - d) * d / 6.0f);
        }
    }
    //}}}
    //{{{
    void ResizerByteBicubic::Init(bool sparse)
    {
        if (_iy.data)
            return;
        EstimateIndexAlpha(_param.srcH, _param.dstH, 1, _iy, _ay);
        EstimateIndexAlpha(_param.srcW, _param.dstW, _param.channels, _ix, _ax);
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

    //{{{
    template<int N, int F, int L> SIMD_INLINE int32_t CubicSumX(const uint8_t* src, const int32_t* ax)
    {
        return ax[0] * src[F * N] + ax[1] * src[0 * N] + ax[2] * src[1 * N] + ax[3] * src[L * N];
    }
    //}}}
    //{{{
    template<int N, int F, int L> SIMD_INLINE void BicubicInt(const uint8_t* src0, const uint8_t* src1,
        const uint8_t* src2, const uint8_t* src3, size_t sx, const int32_t* ax, const int32_t* ay, uint8_t * dst)
    {
        for (size_t c = 0; c < N; ++c)
        {
            int32_t rs0 = CubicSumX<N, F, L>(src0 + sx + c, ax);
            int32_t rs1 = CubicSumX<N, F, L>(src1 + sx + c, ax);
            int32_t rs2 = CubicSumX<N, F, L>(src2 + sx + c, ax);
            int32_t rs3 = CubicSumX<N, F, L>(src3 + sx + c, ax);
            int32_t sum = ay[0] * rs0 + ay[1] * rs1 + ay[2] * rs2 + ay[3] * rs3;
            dst[c] = RestrictRange((sum + BICUBIC_ROUND) >> BICUBIC_SHIFT, 0, 255);
        }
    }
    //}}}
    //{{{
    template<int N> void ResizerByteBicubic::RunS(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
    {
        for (size_t dy = 0; dy < _param.dstH; dy++, dst += dstStride)
        {
            size_t sy = _iy[dy];
            const uint8_t* src1 = src + sy * srcStride;
            const uint8_t* src2 = src1 + srcStride;
            const uint8_t* src0 = sy ? src1 - srcStride : src1;
            const uint8_t* src3 = sy < _param.srcH - 2 ? src2 + srcStride : src2;
            const int32_t* ay = _ay.data + dy * 4;
            size_t dx = 0;
            for (; dx < _xn; dx++)
                BicubicInt<N, 0, 2>(src0, src1, src2, src3, _ix[dx], _ax.data + dx * 4, ay, dst + dx * N);
            for (; dx < _xt; dx++)
                BicubicInt<N, -1, 2>(src0, src1, src2, src3, _ix[dx], _ax.data + dx * 4, ay, dst + dx * N);
            for (; dx < _param.dstW; dx++)
                BicubicInt<N, -1, 1>(src0, src1, src2, src3, _ix[dx], _ax.data + dx * 4, ay, dst + dx * N);
        }
    }
    //}}}

    //{{{
    template<int N, int F, int L> SIMD_INLINE void PixelCubicSumX(const uint8_t* src, const int32_t* ax, int32_t* dst)
    {
        for (size_t c = 0; c < N; ++c)
            dst[c] = CubicSumX<N, F, L>(src + c, ax);
    }
    //}}}
    //{{{
    template<int N> SIMD_INLINE void RowCubicSumX(const uint8_t* src, size_t nose, size_t body, size_t tail, const int32_t* ix, const int32_t* ax, int32_t* dst)
    {
        size_t dx = 0;
        for (; dx < nose; dx++, ax += 4, dst += N)
            PixelCubicSumX<N, 0, 2>(src + ix[dx], ax, dst);
        for (; dx < body; dx++, ax += 4, dst += N)
            PixelCubicSumX<N, -1, 2>(src + ix[dx], ax, dst);
        for (; dx < tail; dx++, ax += 4, dst += N)
            PixelCubicSumX<N, -1, 1>(src + ix[dx], ax, dst);
    }
    //}}}
    //{{{
    SIMD_INLINE void BicubicRowInt(const int32_t* src0, const int32_t* src1, const int32_t* src2, const int32_t* src3, size_t n, const int32_t* ay, uint8_t* dst)
    {
        for (size_t i = 0; i < n; ++i)
        {
            int32_t sum = ay[0] * src0[i] + ay[1] * src1[i] + ay[2] * src2[i] + ay[3] * src3[i];
            dst[i] = RestrictRange((sum + BICUBIC_ROUND) >> BICUBIC_SHIFT, 0, 255);
        }
    }
    //}}}
    //{{{
    template<int N> void ResizerByteBicubic::RunB(const uint8_t* src, size_t srcStride, uint8_t* dst, size_t dstStride)
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
    void ResizerByteBicubic::Run(const uint8_t * src, size_t srcStride, uint8_t * dst, size_t dstStride)
    {
        bool sparse = _param.dstH * 4.0 <= _param.srcH;
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
    void * ResizerInit(size_t srcX, size_t srcY, size_t dstX, size_t dstY, size_t channels, SimdResizeChannelType type, SimdResizeMethodType method)
    {
        ResParam param(srcX, srcY, dstX, dstY, channels, type, method, sizeof(void*));
        if (param.IsNearest())
            return new ResizerNearest(param);
        else if (param.IsByteBilinear())
            return new ResizerByteBilinear(param);
        else if (param.IsShortBilinear())
            return new ResizerShortBilinear(param);
        else if (param.IsFloatBilinear())
            return new ResizerFloatBilinear(param);
        else if (param.IsByteBicubic())
            return new ResizerByteBicubic(param);
        else if (param.IsByteArea2x2())
            return new ResizerByteArea2x2(param);
        else if (param.IsByteArea1x1())
            return new ResizerByteArea1x1(param);
        else
            return NULL;
    }
    //}}}
    }
  }
