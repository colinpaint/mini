// cTexture.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS

#include "cTexture.h"

#include "../common/date.h"
#include "../common/basicTypes.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../simd/SimdLib.h"

// only implmentation
#define STB_IMAGE_IMPLEMENTATION
#include "../decoders/stb_image.h"

// wuff png decoder
#define WUFFS_IMPLEMENTATION
#define WUFFS_CONFIG__STATIC_FUNCTIONS
#ifdef _WIN32
  #pragma warning(push)
    #pragma warning(disable : 4334) // c++20 shift warning disabled
#endif
#include "../decoders/wuffs-v0.3.c"
#ifdef _WIN32
  #pragma warning(pop)
#endif

using namespace std;
//}}}

namespace {
  //{{{
  void* wuffsDecode (void* pData, size_t data_len, uint32_t& width, uint32_t& height) {

    wuffs_png__decoder* pDec = wuffs_png__decoder__alloc();
    if (!pDec)
      //{{{  failed, cleanup, return nullptr
      return nullptr;
      //}}}

    wuffs_png__decoder__set_quirk_enabled (pDec, WUFFS_BASE__QUIRK_IGNORE_CHECKSUM, true);

    wuffs_base__image_config ic;
    wuffs_base__io_buffer src = wuffs_base__ptr_u8__reader ((uint8_t*)pData, data_len, true);
    wuffs_base__status status = wuffs_png__decoder__decode_image_config (pDec, &ic, &src);

    if (status.repr) {
      //{{{  failed, cleanup, return nullptr
      free (pDec);
      return nullptr;
      }
      //}}}

    width = wuffs_base__pixel_config__width (&ic.pixcfg);
    height = wuffs_base__pixel_config__height (&ic.pixcfg);

    wuffs_base__pixel_config__set (&ic.pixcfg, WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL, WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, width, height);

    uint64_t workbuf_len = wuffs_png__decoder__workbuf_len(pDec).max_incl;
    if (workbuf_len > SIZE_MAX) {
      //{{{  failed, cleanup, return nullptr
      free (pDec);
      return nullptr;
      }
      //}}}

    wuffs_base__slice_u8 workbuf_slice = wuffs_base__make_slice_u8 ((uint8_t*)malloc((size_t)workbuf_len), (size_t)workbuf_len);
    if (!workbuf_slice.ptr) {
      //{{{  failed, cleanup, return nullptr
      free (pDec);
      return nullptr;
      }
      //}}}

    const uint64_t total_pixels = (uint64_t)width * (uint64_t)height;
    if (total_pixels > (SIZE_MAX >> 2U)) {
      //{{{  failed, cleanup, return nullptr
      free (workbuf_slice.ptr);
      free (pDec);
      return nullptr;
      }
      //}}}

    void* pDecode_buf = cBaseTexture::allocate (total_pixels * sizeof(uint32_t));
    if (!pDecode_buf) {
      //{{{  failed, cleanup, return nullptr
      free (workbuf_slice.ptr);
      free (pDec);
      return nullptr;
      }
      //}}}

    wuffs_base__slice_u8 pixbuf_slice = wuffs_base__make_slice_u8 ((uint8_t*)pDecode_buf, (size_t)(total_pixels * sizeof(uint32_t)));

    wuffs_base__pixel_buffer pb;
    status = wuffs_base__pixel_buffer__set_from_slice (&pb, &ic.pixcfg, pixbuf_slice);
    if (status.repr) {
      //{{{  failed, cleanup, return nullptr
      free (workbuf_slice.ptr);
      free (pDecode_buf);
      free (pDec);
      return nullptr;
      }
      //}}}

    status = wuffs_png__decoder__decode_frame (pDec, &pb, &src, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf_slice, NULL);
    if (status.repr) {
      //{{{  failed, cleanup, return nullptr
      free (workbuf_slice.ptr);
      free (pDecode_buf);
      free (pDec);
      return nullptr;
      }
      //}}}

    free (workbuf_slice.ptr);
    free (pDec);

    return pDecode_buf;
    }
  //}}}
  }

//{{{  include wuff png decoder
#define WUFFS_IMPLEMENTATION
#define WUFFS_CONFIG__STATIC_FUNCTIONS

#ifdef _WIN32
  #pragma warning(push)
    #pragma warning(disable : 4334) // c++20 shift warning disabled
#endif

#include "../decoders/wuffs-v0.3.c"

#ifdef _WIN32
  #pragma warning(pop)
#endif
//}}}

// cBaseTexture
//{{{
void* cBaseTexture::allocate (size_t bytes) {

  size_t align = SimdAlignment();
  return SimdAllocate (SimdAlign (bytes, align), align);
  //return malloc (bytes);
  }
//}}}
//{{{
void cBaseTexture::deAllocate (void* allocation) {

  SimdFree (allocation);
  //free (allocation);
  }
//}}}

// cAlphaTexture
//{{{
cAlphaTexture cAlphaTexture::create (uint32_t width, uint32_t height, void* pixels, bool freePixels) {
// static create

  (void)freePixels;

  //cLog::log (LOGINFO, fmt::format ("cAlphaTexture::create {}x{}", width, height));
  return cAlphaTexture (width, height, pixels);
  }
//}}}
//{{{
cAlphaTexture::~cAlphaTexture() {
  //release();
  }
//}}}

//{{{
void cAlphaTexture::createPixels (int32_t width, int32_t height) {

  mWidth = width;
  mHeight = height;
  mPixels = (uint8_t*)allocate (width * height * sizeof(uint8_t));
  }
//}}}
//{{{
void cAlphaTexture::release() {
  deAllocate (mPixels);
  }
//}}}

// - draw simple
//{{{
void cAlphaTexture::clear (uint8_t alpha) {
  memset (mPixels, alpha, mWidth * mHeight);
  }
//}}}
//{{{
void cAlphaTexture::drawRectangle (uint8_t alpha, const cRect& rect) {

  // convert to int32, clip to bitmap
  cClipRect clipRect (rect, mWidth, mHeight);
  if (clipRect.empty)
    return;

  uint8_t* dst = getPixels (clipRect.left, clipRect.top);

  SimdFillPixel (dst, mWidth, clipRect.getWidth(), clipRect.getHeight(), &alpha, 1);

  // code
  //for (int32_t j = clipRect.top; j < clipRect.bottom; j++) {
  //  memset (dst, alpha, clipRect.getWidth());
  //  dst += mWidth;
  //  }
  }
//}}}
//{{{
void cAlphaTexture::drawRectangleUnclipped (uint8_t alpha, const cRect& rect) {

  SimdFillPixel (getPixels (rect.getLeftInt32(), rect.getTopInt32()), mWidth,
                 rect.getWidthInt32(), rect.getHeightInt32(), &alpha, 1);

  // code
  //uint8_t* dst = getPixels (rect.left, rect.top);
  //for (int32_t j = clipRect.top; j < clipRect.bottom; j++) {
  //  memset (dst, alpha, clipRect.getWidth());
  //  dst += mWidth;
  //  }
  }
//}}}

// cTexture - statics
//{{{
cTexture cTexture::createLoad (const string& name) {
// static create from file

  // CRUDE - fix with size enquiry !!!
  constexpr uint32_t kMaxFileSize = 20000000;
  uint8_t* fileBuf = new uint8_t [kMaxFileSize];

  FILE* file = fopen (name.c_str(), "rb");
  uint32_t fileBufLen = (uint32_t)fread (fileBuf, 1, kMaxFileSize, file);
  fclose (file);

  return createDecode (fileBuf, fileBufLen);
  }
//}}}
//{{{
cTexture cTexture::createDecode (uint8_t* buffer, uint32_t bufferSize) {

  uint32_t unsignedWidth;
  uint32_t unsignedHeight;
  uint8_t* pixels = (uint8_t*)wuffsDecode (buffer, bufferSize, unsignedWidth, unsignedHeight);
  if (pixels)
    return create ( (int32_t)unsignedWidth, (int32_t)unsignedHeight, pixels, true);

  int32_t width;
  int32_t height;
  int32_t channels;
  pixels = (uint8_t*)stbi_load_from_memory (buffer, bufferSize, &width, &height, &channels, 4);
  if (pixels)
    return create (width, height, pixels, true);

  return cTexture();
  }
//}}}
//{{{
cTexture cTexture::create (uint32_t width, uint32_t height, void* pixels, bool freePixels) {
// static create from pixels

  (void)freePixels;

  //cLog::log (LOGINFO, fmt::format ("cTexture::create {}x{}", width, height));
  return cTexture (width, height, (uPixel*)pixels);
  }
//}}}

// - members
//{{{
cTexture::~cTexture() {
  //release();
  }
//}}}

//{{{
void cTexture::createPixels (int32_t width, int32_t height) {

  mWidth = width;
  mHeight = height;
  mPixels = (uPixel*)allocate (width * height * sizeof(uPixel));
  }
//}}}
//{{{
void cTexture::release() {
  deAllocate (mPixels);
  }
//}}}

// - draw simple
//{{{
void cTexture::clear (const cColor& color) {

  uPixel colorPixel (color);
  SimdFillPixel ((uint8_t*)getPixels(), mWidth * 4, mWidth, mHeight, (uint8_t*)(&colorPixel), 4);

  // code
  //if ((colorPixel.rgba.r == colorPixel.rgba.g) && (colorPixel.rgba.r == colorPixel.rgba.b)) // cheat, alpha ignored ?
  //  memset (mPixels, colorPixel.rgba.r, mWidth * mHeight * 4);
  //else {
  //  uPixel* pixel = getPixels();
  //  for (int32_t i = 0; i < mWidth * mHeight; i++)
  //    *pixel++ = colorPixel;
  //  }
  }
//}}}
//{{{
void cTexture::drawRectangle (const cColor& color, const cRect& rect) {

  // convert to int32, clip to bitmap
  cClipRect clipRect (rect, mWidth, mHeight);
  if (clipRect.empty)
    return;

  uPixel colorPixel (color);
  uint8_t* dst = (uint8_t*)getPixels (clipRect.left, clipRect.top);
  SimdFillPixel (dst, mWidth*4, clipRect.getWidth(), clipRect.getHeight(), (uint8_t*)(&colorPixel), 4);

  // code
  //uPixel* dst = getPixels (clipRect.left, clipRect.top);
  //uint32_t dstStride = mWidth - clipRect.getWidth();
  // for (int32_t j = clipRect.top; j < clipRect.bottom; j++) {
  //  for (int32_t i = clipRect.left; i < clipRect.right; i++)
  //    *dst++ = colorPixel;
  //  dst += dstStride;
  //  }
  }
//}}}
//{{{
void cTexture::drawRectangleUnclipped (const cColor& color, const cRect& rect) {
// useful for waveform drawing

  uPixel colorPixel (color);

  uint8_t* dst = (uint8_t*)getPixels (rect.getLeftInt32(), rect.getTopInt32());
  SimdFillPixel (dst, mWidth*4, rect.getWidthInt32(), rect.getHeightInt32(), (uint8_t*)(&colorPixel), 4);

  // code
  //uint8_t* dst = getPixels (rect.getLeftInt32(), rect.getTopInt32());
  //for (int32_t j = rect.getTopInt32(); j < rect.getBottomInt32(); j++) {
  //  memset (dst, alpha, rect.getWidthInt32()());
  //  dst += mWidth;
  //  }
  }
//}}}

// - blit texture
//{{{
void cTexture::blit (cTexture srcTexture, const cRect& dstRect) {
// blit clipped by srcTexture size and dstRect and our texture size

  if (srcTexture.empty())
    return;

  cClipRect clipRect (dstRect, mWidth, mHeight);
  if (clipRect.empty)
    return;

  // calc width to copy
  int32_t width = min (srcTexture.getWidth() - clipRect.srcLeft, clipRect.getWidth());
  if (width <= 0)
    return;

  // calc height to copy
  int32_t height = min (srcTexture.getHeight() - clipRect.srcTop, clipRect.getHeight());
  if (height <= 0)
    return;

  SimdCopy ((uint8_t*)srcTexture.getPixels (clipRect.srcLeft, clipRect.srcTop),
            srcTexture.getWidth()*4, width, height, 4,
            (uint8_t*)getPixels (clipRect.left, clipRect.top), mWidth*4);

  // SimdCopy code
  //int32_t memcpyBytes = min (clipRect.getWidth(), texture.getWidth()) * sizeof (uPixel);
  //for (int32_t j = clipRect.top; j < clipRect.bottom; j++) {
  //  memcpy (dst, src, memcpyBytes);
  //  dst += mWidth;
  //  src += texture.getWidth();
  //  }
  }
//}}}
//{{{
void cTexture::blit (cTexture srcTexture, const cRect& dstRect, const cRect& clip) {
// blit, clipped by texture size and clip rectangle

  if (srcTexture.empty())
    return;

  cClipRect clipRect (dstRect, clip);
  if (clipRect.empty)
    return;

  // calc width to copy
  int32_t width = min (srcTexture.getWidth() - clipRect.srcLeft, clipRect.getWidth());
  if (width <= 0)
    return;

  // calc height to copy
  int32_t height = min (srcTexture.getHeight() - clipRect.srcTop, clipRect.getHeight());
  if (height <= 0)
    return;

  SimdCopy ((uint8_t*)srcTexture.getPixels (clipRect.srcLeft, clipRect.srcTop),
            srcTexture.getWidth()*4, width, height, 4,
            (uint8_t*)getPixels (clipRect.left, clipRect.top), mWidth*4);
  }
//}}}
//{{{
void cTexture::blitSize (cTexture srcTexture, const cRect& dstRect) {

  if (srcTexture.empty())
    return;

  cClipRect clipRect (dstRect, mWidth, mHeight);
  if (clipRect.empty)
    return;

  uint8_t* src = (uint8_t*)srcTexture.getPixels (clipRect.srcLeft, clipRect.srcTop);
  uint8_t* dst = (uint8_t*)getPixels (clipRect.left, clipRect.top);

  //SimdResizeMethodNearest
  //SimdResizeMethodBilinear,
  //SimdResizeMethodBicubic,
  //SimdResizeMethodArea,
  //SimdResizeMethodAreaFast,
  void* context = SimdResizerInit (srcTexture.getWidth(), srcTexture.getHeight(),
                                   clipRect.getWidth(), clipRect.getHeight(), 4,
                                   SimdResizeChannelByte, SimdResizeMethodBilinear);
  SimdResizerRun (context, src, srcTexture.getWidth() * 4, dst, mWidth * 4);
  SimdRelease (context);
  }
//}}}
//{{{
void cTexture::blitAffine (cTexture srcTexture, const cRect& dstRect, const cMatrix3x2& matrix) {

  if (srcTexture.empty())
    return;

  cClipRect clipRect (dstRect, mWidth, mHeight);
  if (clipRect.empty)
    return;

  uPixel colorPixel (kBlack);
  uint8_t* src = (uint8_t*)srcTexture.getPixels (clipRect.srcLeft, clipRect.srcTop);
  uint8_t* dst = (uint8_t*)getPixels (clipRect.left, clipRect.top);

  //SimdWarpAffineDefault = 0,           /*!< Default Warp Affine flags. */
  //SimdWarpAffineChannelByte = 0,       /*!<  8-bit integer channel type. */
  //SimdWarpAffineInterpNearest = 0,     /*!< Nearest pixel interpolation method. */
  //SimdWarpAffineInterpBilinear = 2,    /*!< Bilinear pixel interpolation method. */
  //SimdWarpAffineBorderConstant = 0,    /*!< Nearest pixel interpolation method. */
  //SimdWarpAffineBorderTransparent = 4, /*!< Bilinear pixel interpolation method. */
  void* context = SimdWarpAffineInit (srcTexture.getWidth(), srcTexture.getHeight(), srcTexture.getWidth() * 4,
                                      clipRect.getWidth(), clipRect.getHeight(), mWidth * 4, 4,
                                      (float*)(&matrix),
                                      SimdWarpAffineFlags (SimdWarpAffineInterpBilinear | SimdWarpAffineBorderTransparent),
                                      (uint8_t*)(&colorPixel));
  SimdWarpAffineRun (context, src, dst);
  SimdRelease (context);
  }
//}}}
//{{{
void cTexture::blitAffine (cTexture srcTexture, const cRect& dstRect, float size, float angle, float x, float y) {

  if (srcTexture.empty())
    return;

  cMatrix3x2 matrix = cMatrix3x2::createTranslate (x, y);
  matrix.scale (size);
  matrix.rotate (angle);
  matrix.translate (-srcTexture.getWidth()/2.f, -srcTexture.getHeight()/2.f);

  blitAffine (srcTexture, dstRect, matrix);
  }
//}}}

// - stamp alphaTexture with color
//{{{
void cTexture::stamp (const cColor& color, cAlphaTexture& alphaTexture, const cPoint& point) {

  cClipRect clipRect ({point.x, point.y,
                       point.x + (float)alphaTexture.getWidth(), point.y + (float)alphaTexture.getHeight()},
                      mWidth, mHeight);
  if (clipRect.empty)
    return;

  uPixel colorPixel (color);
  uint8_t* src = alphaTexture.getPixels (clipRect.srcLeft, clipRect.srcTop);
  uint8_t* dst = (uint8_t*)getPixels (clipRect.left, clipRect.top);

  SimdAlphaFilling (dst, mWidth*4,
                    clipRect.getWidth(), clipRect.getHeight(),
                    (uint8_t*)(&colorPixel), 4, src, alphaTexture.getWidth());

  // code
  //int32_t srcStride = alphaTexture.getWidth() - clipRect.getWidth();
  //int32_t dstStride = mWidth - clipRect.getWidth();
  // for (int32_t j = clipRect.top; j < clipRect.bottom; j++) {
  //   for (int32_t i = clipRect.left; i < clipRect.right; i++) {
  //     uint8_t aFgnd = *src++;
  //     if (aFgnd) {
  //      // !!! is the color resolution maths is right !!!
  //      uint8_t aBgnd = ~aFgnd; // 1-k
  //      (*dst).rgba.r = (((*dst).rgba.r * aBgnd) + (colorPixel.rgba.r * aFgnd)) >> 8;
  //      (*dst).rgba.g = (((*dst).rgba.g * aBgnd) + (colorPixel.rgba.g * aFgnd)) >> 8;
  //      (*dst).rgba.b = (((*dst).rgba.b * aBgnd) + (colorPixel.rgba.b * aFgnd)) >> 8;
  //      }
  //    dst++;
  //    }
  //  src += srcStride;
  //  dst += dstStride;
  //    }
  }
//}}}
