//{{{
/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2022 Yermalayeu Ihar,
*               2014-2019 Antonenka Mikhail,
*               2019-2019 Facundo Galan.
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
//{{{  includes, platform
#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER) || defined(__CODEGEARC__)
  #define SIMD_API
  #define SIMD_INLINE __forceinline
#elif defined(__GNUC__)
  #define SIMD_API __attribute__ ((visibility ("default")))
  #define SIMD_INLINE inline __attribute__ ((always_inline))
#else
  #error This platform is unsupported!
#endif

#define SIMD_CPP_2011_ENABLE
#define SIMD_CPP_2014_ENABLE
#define SIMD_CPP_2017_ENABLE
//}}}
//{{{  enums
//{{{
typedef enum {
  /*! A Bayer pixel layout (GRBG). */
  SimdBayerLayoutGrbg,
  /*! A Bayer pixel layout (GBRG). */
  SimdBayerLayoutGbrg,
  /*! A Bayer pixel layout (RGGB). */
  SimdBayerLayoutRggb,
  /*! A Bayer pixel layout (BGGR). */
  SimdBayerLayoutBggr,
  } SimdBayerLayoutType;
//}}}
//{{{
typedef enum {
  SimdFalse = 0, /*!< False value. */
  SimdTrue = 1, /*!< True value. */
  } SimdBool;
//}}}
//{{{
typedef enum {
  /*! equal to: a == b */
  SimdCompareEqual,
  /*! equal to: a != b */
  SimdCompareNotEqual,
  /*! equal to: a > b */
  SimdCompareGreater,
  /*! equal to: a >= b */
  SimdCompareGreaterOrEqual,
  /*! equal to: a < b */
  SimdCompareLesser,
  /*! equal to: a <= b */
  SimdCompareLesserOrEqual,
  } SimdCompareType;
//}}}
//{{{
typedef enum {
  SimdCpuInfoSockets,/*!< A number of sockets. */
  SimdCpuInfoCores, /*!< A number of psysical CPU cores. */
  SimdCpuInfoThreads, /*!< A number of logical CPU cores. */
  SimdCpuInfoCacheL1, /*!< A size of level 1 data cache. */
  SimdCpuInfoCacheL2, /*!< A size of level 2 cache. */
  SimdCpuInfoCacheL3, /*!< A size of level 3 cache. */
  SimdCpuInfoSse41, /*!< Availability of SSE4.1 (x86). */
  SimdCpuInfoAvx, /*!< Availability of AVX (x86). */
  SimdCpuInfoAvx2, /*!< Availability of AVX2 (x86). */
  SimdCpuInfoAvx512bw, /*!< Availability of AVX-512BW (x86). */
  SimdCpuInfoAvx512vnni, /*!< Availability of AVX-512VNNI (x86). */
  SimdCpuInfoAvx512bf16, /*!< Availability of AVX-512BF16 (x86). */
  SimdCpuInfoAmx, /*!< Availability of AMX (x86). */
  SimdCpuInfoVmx, /*!< Availability of VMX or Altivec (PowerPC). */
  SimdCpuInfoVsx, /*!< Availability of VSX (PowerPC). */
  SimdCpuInfoNeon, /*!< Availability of NEON (ARM). */
  } SimdCpuInfoType;
//}}}
//{{{
typedef enum {
  /*! An undefined pixel format. */
  SimdPixelFormatNone = 0,
  /*! A 8-bit gray pixel format. */
  SimdPixelFormatGray8,
  /*! A 16-bit (2 8-bit channels) pixel format (UV plane of NV12 pixel format). */
  SimdPixelFormatUv16,
  /*! A 24-bit (3 8-bit channels) BGR (Blue, Green, Red) pixel format. */
  SimdPixelFormatBgr24,
  /*! A 32-bit (4 8-bit channels) BGRA (Blue, Green, Red, Alpha) pixel format. */
  SimdPixelFormatBgra32,
  /*! A single channel 16-bit integer pixel format. */
  SimdPixelFormatInt16,
  /*! A single channel 32-bit integer pixel format. */
  SimdPixelFormatInt32,
  /*! A single channel 64-bit integer pixel format. */
  SimdPixelFormatInt64,
  /*! A single channel 32-bit float point pixel format. */
  SimdPixelFormatFloat,
  /*! A single channel 64-bit float point pixel format. */
  SimdPixelFormatDouble,
  /*! A 8-bit Bayer pixel format (GRBG). */
  SimdPixelFormatBayerGrbg,
  /*! A 8-bit Bayer pixel format (GBRG). */
  SimdPixelFormatBayerGbrg,
  /*! A 8-bit Bayer pixel format (RGGB). */
  SimdPixelFormatBayerRggb,
  /*! A 8-bit Bayer pixel format (BGGR). */
  SimdPixelFormatBayerBggr,
  /*! A 24-bit (3 8-bit channels) HSV (Hue, Saturation, Value) pixel format. */
  SimdPixelFormatHsv24,
  /*! A 24-bit (3 8-bit channels) HSL (Hue, Saturation, Lightness) pixel format. */
  SimdPixelFormatHsl24,
  /*! A 24-bit (3 8-bit channels) RGB (Red, Green, Blue) pixel format. */
  SimdPixelFormatRgb24,
  /*! A 32-bit (4 8-bit channels) RGBA (Red, Green, Blue, Alpha) pixel format. */
  SimdPixelFormatRgba32,
  /*! A 16-bit (2 8-bit channels) UYVY422 pixel format. */
  SimdPixelFormatUyvy16,
  /*! A 32-bit (4 8-bit channels) ARGB (Alpha, Red, Green, Blue) pixel format. */
  SimdPixelFormatArgb32,
  } SimdPixelFormatType;
//}}}
//{{{
typedef enum {
  SimdRecursiveBilateralFilterFast = 0, /*!< Fast implementation of Recursive Bilateral Filter. */
  SimdRecursiveBilateralFilterPrecise = 1, /*!< Precise implementation of Recursive Bilateral Filter. */
  SimdRecursiveBilateralFilterDiffAvg = 0, /*!< Use averaging to estimate result color difference. */
  SimdRecursiveBilateralFilterDiffMax = 2, /*!< Use channel difference maximum to estimate result color difference. */
  SimdRecursiveBilateralFilterDiffSum = 4, /*!< Use saturated sum to estimate result color difference. */
  SimdRecursiveBilateralFilterDiffMask = 6, /*!< Color difference type mask. */
  SimdRecursiveBilateralFilterFmaAvoid = 8, /*!< Not use FMA instructions (for debug purposes). */
  } SimdRecursiveBilateralFilterFlags;
//}}}
//{{{
enum SimdReduceType {
  SimdReduce2x2, /*!< Using of function ::SimdReduceGray2x2 for image reducing. */
  SimdReduce3x3, /*!< Using of function ::SimdReduceGray3x3 for image reducing. */
  SimdReduce4x4, /*!< Using of function ::SimdReduceGray4x4 for image reducing. */
  SimdReduce5x5, /*!< Using of function ::SimdReduceGray5x5 for image reducing. */
  };
//}}}
//{{{
typedef enum {
  SimdResizeChannelByte,
  SimdResizeChannelShort,
  SimdResizeChannelFloat,
  } SimdResizeChannelType;
//}}}
//{{{
typedef enum {
  /*! Nearest method. */
  SimdResizeMethodNearest,
  /*! Nearest Pytorch compatible method. */
  SimdResizeMethodNearestPytorch,
  /*! Bilinear method. */
  SimdResizeMethodBilinear,
  /*! Bilinear Caffe compatible method. It is relevant only for ::SimdResizeChannelFloat (32-bit float channel type).*/
  SimdResizeMethodBilinearCaffe,
  /*! Bilinear Pytorch compatible method. It is relevant only for ::SimdResizeChannelFloat (32-bit float channel type).*/
  SimdResizeMethodBilinearPytorch,
  /*! Bicubic method. */
  SimdResizeMethodBicubic,
  /*! Area method. */
  SimdResizeMethodArea,
  /*! Area method for previously reduced in 2 times image. */
  SimdResizeMethodAreaFast,
  } SimdResizeMethodType;
//}}}
//{{{
typedef enum
{
    /*! Gets absolute value for every point of input tensor. */
    SimdSynetUnaryOperation32fAbs,
    /*! Gets exponent for every point of input tensor. */
    SimdSynetUnaryOperation32fExp,
    /*! Gets logarithm for every point of input tensor. */
    SimdSynetUnaryOperation32fLog,
    /*! Gets negative for every point of input tensor. */
    SimdSynetUnaryOperation32fNeg,
    /*! Gets reverse square root for every point of input tensor. */
    SimdSynetUnaryOperation32fRsqrt,
    /*! Gets square root for every point of input tensor. */
    SimdSynetUnaryOperation32fSqrt,
    /*! Gets hyperbolic tangent for every point of input tensor. */
    SimdSynetUnaryOperation32fTanh,
    /*! Gets zero value for every point of input tensor. */
    SimdSynetUnaryOperation32fZero,
} SimdSynetUnaryOperation32fType;
//}}}
//{{{
typedef enum {
  SimdTransformRotate0 = 0, /*!< An original image. The output image has the same size as input image.*/
  SimdTransformRotate90, /*!< Image rotated 90 degrees counterclockwise. The output width and height are equal to the input height and widht. */
  SimdTransformRotate180, /*!< Image rotated 180 degrees counterclockwise. The output image has the same size as input image. */
  SimdTransformRotate270, /*!< Image rotated 270 degrees counterclockwise. The output width and height are equal to the input height and widht. */
  SimdTransformTransposeRotate0, /*!< Transposed image. The output width and height are equal to the input height and widht. */
  SimdTransformTransposeRotate90, /*!< Image transposed and rotated 90 degrees counterclockwise. It is equal to horizontal mirroring of image. The output image has the same size as input image.*/
  SimdTransformTransposeRotate180, /*!< Image transposed and rotated 180 degrees counterclockwise. The output width and height are equal to the input height and widht. */
  SimdTransformTransposeRotate270, /*!< Image transposed and rotated 270 degrees counterclockwise. It is equal to vertical mirroring of image. The output image has the same size as input image.*/
  } SimdTransformType;
//}}}
//{{{
typedef enum {
  SimdYuvUnknown = -1, /*!< Unknown YUV standard. */
  SimdYuvBt601, /*!< Corresponds to BT.601 standard. Uses Kr=0.299, Kb=0.114. Restricts Y to range [16..235], U and V to [16..240]. */
  SimdYuvBt709, /*!< Corresponds to BT.709 standard. Uses Kr=0.2126, Kb=0.0722. Restricts Y to range [16..235], U and V to [16..240]. */
  SimdYuvBt2020, /*!< Corresponds to BT.2020 standard. Uses Kr=0.2627, Kb=0.0593. Restricts Y to range [16..235], U and V to [16..240]. */
  SimdYuvTrect871, /*!< Corresponds to T-REC-T.871 standard. Uses Kr=0.299, Kb=0.114. Y, U and V use full range [0..255]. */
  } SimdYuvType;
//}}}
//{{{
typedef enum {
  SimdWarpAffineDefault = 0, /*!< Default Warp Affine flags. */
  SimdWarpAffineChannelByte = 0, /*!<  8-bit integer channel type. */
  SimdWarpAffineChannelMask = 1, /*!< Bit mask of channel type. */
  SimdWarpAffineInterpNearest = 0, /*!< Nearest pixel interpolation method. */
  SimdWarpAffineInterpBilinear = 2, /*!< Bilinear pixel interpolation method. */
  SimdWarpAffineInterpMask = 2, /*!< Bit mask of pixel interpolation options. */
  SimdWarpAffineBorderConstant = 0, /*!< Nearest pixel interpolation method. */
  SimdWarpAffineBorderTransparent = 4, /*!< Bilinear pixel interpolation method. */
  SimdWarpAffineBorderMask = 4, /*!< Bit mask of pixel interpolation options. */
  } SimdWarpAffineFlags;
//}}}
//}}}

SIMD_API const char* SimdVersion();
SIMD_API size_t SimdCpuInfo (SimdCpuInfoType type);
SIMD_API const char* SimdPerformanceStatistic();
SIMD_API void* SimdAllocate (size_t size, size_t align);
SIMD_API void SimdFree (void* ptr);
SIMD_API size_t SimdAlign (size_t size, size_t align);
SIMD_API size_t SimdAlignment();

SIMD_API void SimdRelease (void* context);
SIMD_API size_t SimdGetThreadNumber();
SIMD_API void SimdSetThreadNumber (size_t threadNumber);
SIMD_API void SimdEmpty();
SIMD_API SimdBool SimdGetFastMode();
SIMD_API void SimdSetFastMode (SimdBool value);

// fill
SIMD_API void SimdFill (uint8_t* dst, size_t stride, size_t width, size_t height, size_t pixelSize, uint8_t value);
SIMD_API void SimdFillFrame (uint8_t * dst, size_t stride, size_t width, size_t height, size_t pixelSize,
                             size_t frameLeft, size_t frameTop, size_t frameRight, size_t frameBottom, uint8_t value);
SIMD_API void SimdFillBgr (uint8_t* dst, size_t stride, size_t width, size_t height,
                           uint8_t blue, uint8_t green, uint8_t red);
SIMD_API void SimdFillBgra (uint8_t* dst, size_t stride, size_t width, size_t height,
                            uint8_t blue, uint8_t green, uint8_t red, uint8_t alpha);
SIMD_API void SimdFillPixel (uint8_t* dst, size_t stride, size_t width, size_t height,
                             const uint8_t* pixel, size_t pixelSize);
SIMD_API void SimdFill32f (float * dst, size_t size, const float* value);

// alphaBlending
SIMD_API void SimdAlphaBlending (const uint8_t * src, size_t srcStride, size_t width, size_t height,
                                 size_t channelCount, const uint8_t* alpha, size_t alphaStride,
                                 uint8_t* dst, size_t dstStride);
SIMD_API void SimdAlphaBlending2x (const uint8_t* src0, size_t src0Stride,
                                   const uint8_t* alpha0, size_t alpha0Stride,
                                   const uint8_t* src1, size_t src1Stride,
                                   const uint8_t* alpha1, size_t alpha1Stride,
                                   size_t width, size_t height, size_t channelCount, uint8_t* dst, size_t dstStride);
SIMD_API void SimdAlphaBlendingUniform (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                        size_t channelCount, uint8_t alpha,
                                        uint8_t* dst, size_t dstStride);
SIMD_API void SimdAlphaFilling (uint8_t* dst, size_t dstStride, size_t width, size_t height,
                                const uint8_t* channel, size_t channelCount,
                                const uint8_t* alpha, size_t alphaStride);
SIMD_API void SimdAlphaPremultiply (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                    uint8_t* dst, size_t dstStride,
                                    SimdBool argb);
SIMD_API void SimdAlphaUnpremultiply (const uint8_t* src, size_t srcStride, size_t width, size_t height,
                                      uint8_t* dst, size_t dstStride,
                                      SimdBool argb);

// copy
SIMD_API void SimdCopy (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t pixelSize,
                        uint8_t* dst, size_t dstStride);
SIMD_API void SimdCopyFrame (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t pixelSize,
                             size_t frameLeft, size_t frameTop, size_t frameRight, size_t frameBottom,
                             uint8_t* dst, size_t dstStride);
SIMD_API void SimdFloat32ToUint8 (const float* src, size_t size, const float* lower, const float* upper, uint8_t* dst);
SIMD_API void SimdUint8ToFloat32 (const uint8_t* src, size_t size, const float* lower, const float* upper, float* dst);

//{{{  transform
SIMD_API void SimdTransformImage (const uint8_t* src, size_t srcStride, size_t width, size_t height, size_t pixelSize,
                                  SimdTransformType transform,
                                  uint8_t* dst, size_t dstStride);

SIMD_API void SimdResizeBilinear (const uint8_t* src, size_t srcWidth, size_t srcHeight, size_t srcStride,
                                  uint8_t* dst, size_t dstWidth, size_t dstHeight, size_t dstStride,
                                  size_t channelCount);

SIMD_API void* SimdResizerInit (size_t srcX, size_t srcY, size_t dstX, size_t dstY,
                                size_t channels, SimdResizeChannelType type, SimdResizeMethodType method);
SIMD_API void SimdResizerRun (const void* resizer,
                              const uint8_t* src, size_t srcStride,
                              uint8_t* dst, size_t dstStride);

SIMD_API void* SimdWarpAffineInit (size_t srcW, size_t srcH, size_t srcS,
                                   size_t dstW, size_t dstH, size_t dstS,
                                   size_t channels, const float* mat, SimdWarpAffineFlags flags,
                                   const uint8_t * border);
SIMD_API void SimdWarpAffineRun (const void* context, const uint8_t* src, uint8_t* dst);
//}}}
//{{{  interleave
SIMD_API void SimdDeinterleaveUv (const uint8_t * uv, size_t uvStride, size_t width, size_t height,
                                  uint8_t * u, size_t uStride, uint8_t * v, size_t vStride);
SIMD_API void SimdDeinterleaveBgr (const uint8_t * bgr, size_t bgrStride, size_t width, size_t height,
                                   uint8_t * b, size_t bStride,
                                   uint8_t * g, size_t gStride,
                                   uint8_t * r, size_t rStride);
SIMD_API void SimdDeinterleaveBgra (const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
                                    uint8_t * b, size_t bStride,
                                    uint8_t * g, size_t gStride,
                                    uint8_t * r, size_t rStride,
                                    uint8_t * a, size_t aStride);

SIMD_API void SimdInterleaveBgr (const uint8_t* b, size_t bStride,
                                 const uint8_t* g, size_t gStride,
                                 const uint8_t* r, size_t rStride,
                                 size_t width, size_t height,
                                 uint8_t * bgr, size_t bgrStride);
SIMD_API void SimdInterleaveBgra (const uint8_t* b, size_t bStride,
                                  const uint8_t* g, size_t gStride,
                                  const uint8_t* r, size_t rStride,
                                  const uint8_t* a, size_t aStride,
                                  size_t width, size_t height,
                                  uint8_t * bgra, size_t bgraStride);
//}}}

//{{{  rgb
SIMD_API void SimdRgbToBgra (const uint8_t* rgb, size_t width, size_t height, size_t rgbStride,
                            uint8_t* bgra, size_t bgraStride, uint8_t alpha);
SIMD_API void SimdRgbToGray (const uint8_t* rgb, size_t width, size_t height, size_t rgbStride,
                             uint8_t* gray, size_t grayStride);
SIMD_API void SimdRgbaToGray (const uint8_t* rgba, size_t width, size_t height, size_t rgbaStride,
                              uint8_t* gray, size_t grayStride);
//}}}
//{{{  bgr
SIMD_API void SimdBgrToBgra (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * bgra, size_t bgraStride, uint8_t alpha);
SIMD_API void SimdBgr48pToBgra32 (const uint8_t * blue, size_t blueStride, size_t width, size_t height,
                                  const uint8_t * green, size_t greenStride,
                                  const uint8_t * red, size_t redStride,
                                  uint8_t * bgra, size_t bgraStride,
                                  uint8_t alpha);

SIMD_API void SimdBgrToHsl (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride,
                            uint8_t * hsl, size_t hslStride);
SIMD_API void SimdBgrToHsv (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride,
                            uint8_t * hsv, size_t hsvStride);
SIMD_API void SimdBgrToRgb (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride,
                            uint8_t * rgb, size_t rgbStride);

SIMD_API void SimdBgrToYuv420p (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride,
                                uint8_t * y, size_t yStride, uint8_t * u, size_t uStride, uint8_t * v, size_t vStride);
SIMD_API void SimdBgrToYuv422p (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride,
                                uint8_t * y, size_t yStride, uint8_t * u, size_t uStride, uint8_t * v, size_t vStride);
SIMD_API void SimdBgrToYuv444p (const uint8_t * bgr, size_t width, size_t height, size_t bgrStride,
                                uint8_t * y, size_t yStride, uint8_t * u, size_t uStride, uint8_t * v, size_t vStride);
//}}}
//{{{  bgra 
SIMD_API void SimdBgraToBgr (const uint8_t * bgra, size_t width, size_t height, size_t bgraStride,
                             uint8_t * bgr, size_t bgrStride);
SIMD_API void SimdBgraToGray (const uint8_t * bgra, size_t width, size_t height, size_t bgraStride,
                              uint8_t * gray, size_t grayStride);
SIMD_API void SimdBgraToRgb (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride,
                             uint8_t* rgb, size_t rgbStride);
SIMD_API void SimdBgraToRgba (const uint8_t* bgra, size_t width, size_t height, size_t bgraStride,
                              uint8_t* rgba, size_t rgbaStride);

SIMD_API void SimdBgraToYuv420p (const uint8_t * bgra, size_t width, size_t height, size_t bgraStride,
                                 uint8_t * y, size_t yStride,
                                 uint8_t * u, size_t uStride,
                                 uint8_t * v, size_t vStride);
SIMD_API void SimdBgraToYuv420pV2 (const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
                                   uint8_t* y, size_t yStride,
                                   uint8_t* u, size_t uStride,
                                   uint8_t* v, size_t vStride,
                                   SimdYuvType yuvType);

SIMD_API void SimdBgraToYuv422p (const uint8_t * bgra, size_t width, size_t height, size_t bgraStride,
                                 uint8_t * y, size_t yStride,
                                 uint8_t * u, size_t uStride,
                                 uint8_t * v, size_t vStride);
SIMD_API void SimdBgraToYuv444p (const uint8_t * bgra, size_t width, size_t height, size_t bgraStride,
                                 uint8_t * y, size_t yStride,
                                 uint8_t * u, size_t uStride,
                                 uint8_t * v, size_t vStride);
SIMD_API void SimdBgraToYuv444pV2 (const uint8_t* bgra, size_t bgraStride, size_t width, size_t height,
                                   uint8_t* y, size_t yStride,
                                   uint8_t* u, size_t uStride,
                                   uint8_t* v, size_t vStride,
                                   SimdYuvType yuvType);
SIMD_API void SimdBgraToYuva420p (const uint8_t * bgra, size_t bgraStride, size_t width, size_t height,
                                  uint8_t * y, size_t yStride,
                                  uint8_t * u, size_t uStride,
                                  uint8_t * v, size_t vStride,
                                  uint8_t * a, size_t aStride);

//}}}
//{{{  gray 
SIMD_API void SimdGrayToBgr (const uint8_t *gray, size_t width, size_t height, size_t grayStride,
                             uint8_t* bgr, size_t bgrStride);
SIMD_API void SimdGrayToBgra (const uint8_t *gray, size_t width, size_t height, size_t grayStride,
                              uint8_t* bgra, size_t bgraStride, uint8_t alpha);

SIMD_API void SimdInt16ToGray (const uint8_t * src, size_t width, size_t height, size_t srcStride,
                               uint8_t* dst, size_t dstStride);
//}}}
//{{{  yuv
SIMD_API void SimdYuva420pToBgra (const uint8_t * y, size_t yStride,
                                  const uint8_t * u, size_t uStride,
                                  const uint8_t * v, size_t vStride,
                                  const uint8_t * a, size_t aStride,
                                  size_t width, size_t height,
                                  uint8_t * bgra, size_t bgraStride);
SIMD_API void SimdYuva444pToBgraV2 (const uint8_t* y, size_t yStride,
                                    const uint8_t* u, size_t uStride,
                                    const uint8_t* v, size_t vStride,
                                    const uint8_t* a, size_t aStride,
                                    size_t width, size_t height,
                                    uint8_t* bgra, size_t bgraStride, SimdYuvType yuvType);
SIMD_API void SimdYuv420pToBgr (const uint8_t * y, size_t yStride,
                                const uint8_t * u, size_t uStride,
                                const uint8_t * v, size_t vStride,
                                size_t width, size_t height, uint8_t * bgr, size_t bgrStride);
SIMD_API void SimdYuv422pToBgr (const uint8_t * y, size_t yStride,
                                const uint8_t * u, size_t uStride,
                                const uint8_t * v, size_t vStride,
                                size_t width, size_t height, uint8_t * bgr, size_t bgrStride);
SIMD_API void SimdYuv444pToBgr (const uint8_t * y, size_t yStride,
                                const uint8_t * u, size_t uStride,
                                const uint8_t * v, size_t vStride,
                                size_t width, size_t height, uint8_t * bgr, size_t bgrStride);
SIMD_API void SimdYuv420pToBgra (const uint8_t * y, size_t yStride,
                                 const uint8_t * u, size_t uStride,
                                 const uint8_t * v, size_t vStride,
                                 size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha);
SIMD_API void SimdYuv420pToBgraV2 (const uint8_t* y, size_t yStride,
                                   const uint8_t* u, size_t uStride,
                                   const uint8_t* v, size_t vStride,
                                   size_t width, size_t height, uint8_t* bgra, size_t bgraStride, uint8_t alpha, SimdYuvType yuvType);
SIMD_API void SimdYuv422pToBgra (const uint8_t * y, size_t yStride,
                                 const uint8_t * u, size_t uStride,
                                 const uint8_t * v, size_t vStride,
                                 size_t width, size_t height, uint8_t * bgra, size_t bgraStride, uint8_t alpha);
SIMD_API void SimdYuv444pToBgra (const uint8_t * y, size_t yStride,
                                 const uint8_t * u, size_t uStride, const uint8_t * v, size_t vStride,
                                 size_t width, size_t height,
                                 uint8_t * bgra, size_t bgraStride,
                                 uint8_t alpha);
SIMD_API void SimdYuv444pToBgraV2 (const uint8_t* y, size_t yStride,
                                   const uint8_t* u, size_t uStride,
                                   const uint8_t* v, size_t vStride,
                                   size_t width, size_t height,
                                   uint8_t* bgra, size_t bgraStride,
                                   uint8_t alpha, SimdYuvType yuvType);
SIMD_API void SimdYuv444pToHsl (const uint8_t * y, size_t yStride,
                                const uint8_t * u, size_t uStride,
                                const uint8_t * v, size_t vStride,
                                size_t width, size_t height,
                                uint8_t * hsl, size_t hslStride);
SIMD_API void SimdYuv444pToHsv (const uint8_t * y, size_t yStride,
                                const uint8_t * u, size_t uStride,
                                const uint8_t * v, size_t vStride,
                                size_t width, size_t height,
                                uint8_t * hsv, size_t hsvStride);
SIMD_API void SimdYuv420pToHue (const uint8_t * y, size_t yStride,
                                const uint8_t * u, size_t uStride,
                                const uint8_t * v, size_t vStride,
                                size_t width, size_t height,
                                uint8_t * hue, size_t hueStride);
SIMD_API void SimdYuv444pToHue (const uint8_t * y, size_t yStride,
                                const uint8_t * u, size_t uStride,
                                const uint8_t * v, size_t vStride,
                                size_t width, size_t height,
                                uint8_t * hue, size_t hueStride);
SIMD_API void SimdYuv420pToRgb (const uint8_t* y, size_t yStride,
                                const uint8_t* u, size_t uStride,
                                const uint8_t* v, size_t vStride,
                                size_t width, size_t height,
                                uint8_t* rgb, size_t rgbStride);
SIMD_API void SimdYuv422pToRgb (const uint8_t* y, size_t yStride,
                                const uint8_t* u, size_t uStride,
                                const uint8_t* v, size_t vStride,
                                size_t width, size_t height,
                                uint8_t* rgb, size_t rgbStride);
SIMD_API void SimdYuv444pToRgb (const uint8_t* y, size_t yStride,
                                const uint8_t* u, size_t uStride,
                                const uint8_t* v, size_t vStride,
                                size_t width, size_t height,
                                uint8_t* rgb, size_t rgbStride);
SIMD_API void SimdYuv420pToUyvy422 (const uint8_t* y, size_t yStride,
                                    const uint8_t* u, size_t uStride,
                                    const uint8_t* v, size_t vStride,
                                    size_t width, size_t height,
                                    uint8_t* uyvy, size_t uyvyStride);
//}}}
//{{{  uyvy 
SIMD_API void SimdUyvy422ToBgr (const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height,
                                uint8_t* bgr, size_t bgrStride, SimdYuvType yuvType);
SIMD_API void SimdUyvy422ToYuv420p (const uint8_t* uyvy, size_t uyvyStride, size_t width, size_t height,
                                    uint8_t* y, size_t yStride,
                                    uint8_t* u, size_t uStride,
                                    uint8_t* v, size_t vStride);
//}}}
