// cDrawTexture.cpp - drawable texture
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS

#include "cDrawTexture.h"

#include <functional>
#include <algorithm>
#include <array>
#include <map>
#include "../common/date.h"
#include "../common/basicTypes.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../include/Simd/SimdLib.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../stb/stb_truetype.h"
#include "freeSansBold.h"

#include "cDrawAA.h"

using namespace std;
//}}}

namespace {
  //{{{
  class cFont {
  public:
    //{{{
    struct sGlyph {
      sGlyph (uint8_t* pixels, int x1, int y1, int width, int height, float advanceWidth, float leftSideBearing)
        : mAlphaTexture (width, height, pixels),
          mX(x1), mY(y1), mAdvanceWidth(advanceWidth), mLeftSideBearing(leftSideBearing) {}

      cAlphaTexture mAlphaTexture;
      int mX;
      int mY;
      float mAdvanceWidth;
      float mLeftSideBearing;
      };
    //}}}

    //{{{
    bool create (const string& name, const string& familyName, const unsigned char* font, float height) {

      if (!stbtt_InitFont (&mInfo, font, 0)) {
        //{{{  error, return
        cLog::log (LOGERROR, fmt::format ("- failed to create font {}", familyName));
        return false;
        }
        //}}}

      mName = name;
      mFamilyName = familyName;
      mLineHeight = height;

      mScale = stbtt_ScaleForPixelHeight (&mInfo, mLineHeight);

      int ascentInt;
      int descentInt;
      stbtt_GetFontVMetrics (&mInfo, &ascentInt, &descentInt, &mLineGap);
      mAscent = ascentInt * mScale;
      mDescent = descentInt * mScale;

      cLog::log (LOGINFO, fmt::format ("- {} lineHeight:{} ascent:{} descent:{} scale:{} {}",
                                       mFamilyName, mLineHeight, mAscent, mDescent, mScale, name));

      return true;
      }
    //}}}

    //{{{
    sGlyph& getChar (uint8_t ch) {

      // is ch in cache map ?
      auto it = mGlyphs.find (ch);
      if (it == mGlyphs.end()) {
        int32_t advanceWidth;
        int32_t leftSideBearing;
        stbtt_GetCodepointHMetrics (&mInfo, ch, &advanceWidth, &leftSideBearing);

        // get bounding box for glyph, usually offset to account for chars that dip above or below the line
        int32_t x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox (&mInfo, ch, mScale, mScale, &x1, &y1, &x2, &y2);

        // create glyph bitmap
        uint32_t width = x2 - x1;
        uint32_t height = y2 - y1;
        uint8_t* pixels = (uint8_t*)cBaseTexture::allocate (width * height);

        // render glyph bitmap
        stbtt_MakeCodepointBitmap (&mInfo, pixels, width, height, width, mScale, mScale, ch);

        // create sGlyph and cache it in mGlyphs map
        it = mGlyphs.emplace (ch, sGlyph (pixels, x1, y1 + (int32_t)mAscent + 2, width, height,
                                          advanceWidth * mScale, leftSideBearing * mScale)).first;
        }

      return it->second;
      }
    //}}}
    //{{{
    float getHeight() const {
      return mLineHeight;
      }
    //}}}
    //{{{
    float getKern (uint8_t lastCh, uint8_t ch) const {
      return stbtt_GetCodepointKernAdvance (&mInfo, lastCh, ch) * mScale;
      }
    //}}}
    //{{{
    uint32_t getNumChars() const {
      return (uint32_t)mGlyphs.size();
      }
    //}}}

  private:
    string mFamilyName;
    string mName;
    float mLineHeight = 0.f;

    float mScale = 0.f;
    float mAscent = 0.f;
    float mDescent = 0.f;
    int mLineGap = 0;

    stbtt_fontinfo mInfo;
    std::map <uint32_t, sGlyph> mGlyphs;
    };
  //}}}
  array <cFont, 4> gFonts;
  array <uint8_t,256> mGamma = {0};
  bool gStaticCreated = false;
  }
//{{{
void cDrawTexture::createStaticResources (float menuTextHeight) {

  if (!gStaticCreated) {
    //{{{  report CPU info
    cLog::log (LOGINFO, fmt::format ("CPU {} cores {} threads L1:{}k L2:{}k L3:{}k {} {} {} {} {} {}",
                                     SimdCpuInfo (SimdCpuInfoCores),
                                     SimdCpuInfo (SimdCpuInfoThreads),
                                     SimdCpuInfo (SimdCpuInfoCacheL1) / 1024,
                                     SimdCpuInfo (SimdCpuInfoCacheL2) / 1024,
                                     SimdCpuInfo (SimdCpuInfoCacheL3) / 1024,
                                     SimdCpuInfo (SimdCpuInfoSse41) ? "Sse4.1" : "",
                                     SimdCpuInfo (SimdCpuInfoAvx) ? "Avx" : "",
                                     SimdCpuInfo (SimdCpuInfoAvx2) ? "Avx2" : "",
                                     SimdCpuInfo (SimdCpuInfoAvx512bw) ? "Avx512" : "",
                                     SimdCpuInfo (SimdCpuInfoAvx512vnni) ? "Avx512vnni" : "",
                                     SimdCpuInfo (SimdCpuInfoAvx512bf16) ? "Avx512bf16" : ""));
    //}}}

    cLog::log(LOGINFO, "create fonts");
    gFonts[0].create ("menu", "freeSansBold", getFreeSansBold(), menuTextHeight);
    gFonts[1].create ("menuLarge", "freeSansBold", getFreeSansBold(), menuTextHeight * 2);
    gFonts[2].create ("console", "freeSansBold", getFreeSansBold(), 12.f);
    gFonts[3].create ("symbol", "freeSansBold", getFreeSansBold(), menuTextHeight);

    for (uint32_t i = 0; i < 256; i++)
      mGamma[i] = uint8_t(pow(double(i) / 255.0, 1.6) * 255.0);

    gStaticCreated = true;
    }
  }
//}}}

//{{{
cDrawTexture::~cDrawTexture() {
  delete mDrawAA;
  }
//}}}
//{{{
uint32_t cDrawTexture::getNumFontChars() const {

  int32_t numChars = 0;
  for (uint32_t i = 0; i < 4; i++)
    numChars += gFonts[i].getNumChars();

  return numChars;
  }
//}}}

// draw shapes
//{{{
void cDrawTexture::drawGradH (const cColor& colorLeft, const cColor& colorRight, const cRect& rect) {
// !!! is the color resolution maths is right !!!
// !!! add clips !!!

  int32_t left = rect.getLeftInt32();
  int32_t top = rect.getTopInt32();
  int32_t xmax = std::min (rect.getRightInt32(), mWidth);
  int32_t ymax = std::min (rect.getBottomInt32(), mHeight);
  int32_t width = rect.getWidthInt32();

  uPixel colorPixelLeft (colorLeft);
  uPixel colorPixelRight (colorRight);

  // draw first line
  int32_t y = top;
  uPixel* dst = getPixels (left, y);
  for (int32_t x = 0; x < xmax - left; x++) {
    uint8_t alpha = mGamma[uint8_t((x * 0xFF) / width)];
    (*dst).rgba.r = colorPixelLeft.rgba.r + ((alpha * (colorPixelRight.rgba.r - colorPixelLeft.rgba.r)) >> 8);
    (*dst).rgba.g = colorPixelLeft.rgba.g + ((alpha * (colorPixelRight.rgba.g - colorPixelLeft.rgba.g)) >> 8);
    (*dst).rgba.b = colorPixelLeft.rgba.b + ((alpha * (colorPixelRight.rgba.b - colorPixelLeft.rgba.b)) >> 8);
    dst++;
    }
  y++;

  // simple copy of first line to subsequent lines
  uPixel* src = getPixels (left, top);
  for (; y < ymax; y++) {
    dst = getPixels (left,y);
    memcpy (dst, src, (xmax - left) * sizeof(uPixel));
    dst += mWidth;
    }
  }
//}}}
//{{{
void cDrawTexture::drawGradV (const cColor& colorTop, const cColor& colorBottom, const cRect& rect) {
// !!! is the color resolution maths is right !!!

  int32_t left = rect.getLeftInt32();
  int32_t top = rect.getTopInt32();
  int32_t xmax = std::min (rect.getRightInt32(), mWidth);
  int32_t ymax = std::min (rect.getBottomInt32(), mHeight);
  int32_t height = rect.getHeightInt32();
  int32_t dstStride = mWidth - (xmax - left);

  uPixel colorPixelTop (colorTop);
  uPixel colorPixelBottom (colorBottom);

  uPixel* dst = getPixels (left,top);
  for (int32_t j = 0; j < ymax - top; j++) {
    uint8_t alpha = mGamma[uint8_t((j * 0xFF) / height)];

    uPixel colorPixel;
    colorPixel.rgba.r = colorPixelTop.rgba.r + ((alpha * (colorPixelBottom.rgba.r - colorPixelTop.rgba.r)) >> 8);
    colorPixel.rgba.g = colorPixelTop.rgba.g + ((alpha * (colorPixelBottom.rgba.g - colorPixelTop.rgba.g)) >> 8);
    colorPixel.rgba.b = colorPixelTop.rgba.b + ((alpha * (colorPixelBottom.rgba.b - colorPixelTop.rgba.b)) >> 8);
    colorPixel.rgba.a = 0;

    // draw line of color
    for (int32_t i = left; i < xmax; i++)
      *dst++ = colorPixel.pixel;

    dst += dstStride;
    }
  }
//}}}
//{{{
void cDrawTexture::drawGrad (const cColor& colorTopLeft, const cColor& colorTopRight,
                             const cColor& colorBottomLeft, const cColor& colorBottomRight, const cRect& rect) {
// !!! is the color resolution maths is right !!!
  int32_t left = rect.getLeftInt32();
  int32_t top = rect.getTopInt32();
  int32_t xmax = std::min (rect.getRightInt32(), mWidth);
  int32_t ymax = std::min (rect.getBottomInt32(), mHeight);
  int32_t width = rect.getWidthInt32();
  int32_t height = rect.getHeightInt32();

  uPixel colorPixelTopLeft (colorTopLeft);
  uPixel colorPixelTopRight (colorTopRight);
  uPixel colorPixelBottomLeft (colorBottomLeft);
  uPixel colorPixelBottomRight (colorBottomRight);

  uPixel* dst = getPixels (left, top);

  for (int32_t y = 0; y < ymax - top; y++) {
    // !!! is the color resolution maths is right !!!
    uint8_t alpha = mGamma[uint8_t((y * 0xFF) / height)];

    uPixel colorPixelTop;
    colorPixelTop.rgba.r = colorPixelTopLeft.rgba.r +
                           ((alpha * (colorPixelBottomLeft.rgba.r - colorPixelTopLeft.rgba.r)) >> 8);
    colorPixelTop.rgba.g = colorPixelTopLeft.rgba.g +
                           ((alpha * (colorPixelBottomLeft.rgba.g - colorPixelTopLeft.rgba.g)) >> 8);
    colorPixelTop.rgba.b = colorPixelTopLeft.rgba.b +
                           ((alpha * (colorPixelBottomLeft.rgba.b - colorPixelTopLeft.rgba.b)) >> 8);
    colorPixelTop.rgba.a = 0;

    uPixel colorPixelBottom;
    colorPixelBottom.rgba.r = colorPixelTopRight.rgba.r +
                              ((alpha * (colorPixelBottomRight.rgba.r - colorPixelTopRight.rgba.r)) >> 8);
    colorPixelBottom.rgba.g = colorPixelTopRight.rgba.g +
                              ((alpha * (colorPixelBottomRight.rgba.g - colorPixelTopRight.rgba.g)) >> 8);
    colorPixelBottom.rgba.b = colorPixelTopRight.rgba.b +
                              ((alpha * (colorPixelBottomRight.rgba.b - colorPixelTopRight.rgba.b)) >> 8);
    colorPixelBottom.rgba.a = 0;

    for (int32_t x = 0; x < xmax-left; x++) {
      alpha = mGamma[uint8_t((x * 0xFF) / width)];

      uPixel colorPixel;
      colorPixel.rgba.r = colorPixelTop.rgba.r + ((alpha * (colorPixelBottom.rgba.r - colorPixelTop.rgba.r)) >> 8);
      colorPixel.rgba.g = colorPixelTop.rgba.g + ((alpha * (colorPixelBottom.rgba.g - colorPixelTop.rgba.g)) >> 8);
      colorPixel.rgba.b = colorPixelTop.rgba.b + ((alpha * (colorPixelBottom.rgba.b - colorPixelTop.rgba.b)) >> 8);
      *dst++ = colorPixel.pixel;
      }

    dst += mWidth - (xmax - left);
    }
  }
//}}}
//{{{
void cDrawTexture::drawGradRadial (const cColor& color, cPoint centre, cPoint radius) {
// simple, with expensive sqrt

  // versions using Chebyshev polynomial approximation, and forward differencing
  //{{{
  //void GradientFill_6 (const color_t& c1, const color_t& c2) {
  //// version using Chebyshev polynomial approximation, and forward differencing

    //double K,dc;

    //// the color delta
    //dc = c2-c1;
    //color_t ce; // the exact color computed for each square
    //color_t * p1, *p2, *p3, *p4; // 4 quadrant pointers
    //double maxDimension = max(height,width);

    //// and constant used in the code....
    //double t1,t2; // temp values
    //t1 = width/maxDimension;
    //t2 = height/maxDimension;
    //K = dc/(sqrt(t1*t1+t2*t2));
    //double delta = 1.0/(maxDimension/2.0); // stepsize

    //// initial pixel relative x coord
    //double alpha = (1.0)/maxDimension;
    //for (int j = 0; j < height/2; j++) {
      //double d, beta; // pixel coords in rectangle [-1,1]x[-1,1]
      //beta = ((double)(height/2-1-j)+0.5)/(maxDimension/2.0);
      //p1 = surface + j*width+width/2;
      //p2 = p1 - 1;
      //p3 = surface + (height - 1 - j)*width+width/2;
      //p4 = p3 - 1;
      //double a0,a1,a2,a3; // polynomial coefficients
      //double j2,r1,r2,r3,r4; // temp values
      //j2 = beta*beta;
      //r1 = sqrt(0.0014485813926750633 + j2);
      //r2 = sqrt(0.0952699361691366900 + j2);
      //r3 = sqrt(0.4779533685342265000 + j2);
      //r4 = sqrt(0.9253281139039617000 + j2);
      //a0 = 1.2568348730314625*r1 - 0.3741514406663722*r2 +
      //0.16704465947982383*r3 - 0.04972809184491411*r4;
      //a1 = -7.196457548543286*r1 + 10.760659484982682*r2 -
      //5.10380523549030050*r3 + 1.53960329905090450*r4;
      //a2 = 12.012829501508346*r1 - 25.001535905017075*r2 +
      //19.3446816555246950*r3 - 6.35597525201596500*r4;
      //a3 = -6.122934917841437*r1 + 14.782072520180590*r2 -
      //14.7820725201805900*r3 + 6.12293491784143700*r4;

      //// forward differencing stuff
      //double d1,d2,d3;

      //// initial color value and differences
      //d = ((a3*alpha+a2)*alpha+a1)*alpha+a0+c1/K;
      //d1 = 3*a3*alpha*alpha*delta + alpha*delta*(2*a2+3*a3*delta) + delta*(a1+a2*delta+a3*delta*delta);
      //d2 = 6*a3*alpha*delta*delta + 2*delta*delta*(a2 + 3*a3*delta);
      //d3 = 6*a3*delta*delta*delta;
      //d *= K; // we can prescale these here
      //d1 *= K;
      //d2 *= K;
      //d3 *= K;
      //for (int i = 0; i < width/2; i++) {
        //// get color and update forward differencing stuff
        //ce = (color_t)(d);
        //d+=d1; d1+=d2; d2+=d3;

        //// now draw 4 pixels
        //*p1++ = ce;
        //*p2-- = ce;
        //*p3++ = ce;
        //*p4-- = ce;
        //}
      //}
    //} // GradientFill_6
  //}}}
  //{{{
  //void GradientFill_7 (const color_t& c1, const color_t& c2) {
  //// stuff above, with fixed point math

    //double K,dc;
    //// the color delta

    //dc = c2-c1;
    //color_t ce; // the exact color computed for each square
    //color_t * p1, *p2, *p3, *p4; // 4 quadrant pointers
    //double maxDimension = max(height,width);

    //// and constants used in the code....
    //double t1,t2; // temp values
    //t1 = width/maxDimension;
    //t2 = height/maxDimension;

    //#define _BITS 24 // bits of fractional point stuff
    //#define _SCALE (1<<_BITS) // size to scale it
    //K = dc/sqrt(t1*t1+t2*t2)*_SCALE;

    //double delta = 2.0/maxDimension; // stepsize
    //double delta2,delta3; // powers of delta
    //delta2 = delta*delta;
    //delta3 = delta2*delta;

    //// initial color value and differences
    //double alpha = 1.0/maxDimension;
    //for (int j = 0; j < height/2; j++) {
      //double d, beta; // pixel coords in rectangle [-1,1]x[-1,1]
      //beta = ((double)(height-1-(j<<1)))/maxDimension;
      //p1 = surface + j*width+width/2;
      //p2 = p1 - 1;
      //p3 = surface + (height - 1 - j)*width+width/2;
      //p4 = p3 - 1;
      //double a0,a1,a2,a3; // polynomial coefficients
      //double j2,r1,r2,r3,r4; // temp values
      //j2 = beta*beta;

      //// numbers from the analysis to create the polynomial
      //r1 = sqrt(0.0014485813926750633 + j2);
      //r2 = sqrt(0.0952699361691366900 + j2);
      //r3 = sqrt(0.4779533685342265000 + j2);
      //r4 = sqrt(0.9253281139039617000 + j2);
      //a0 = 1.2568348730314625*r1 - 0.3741514406663722*r2 +
      //0.16704465947982383*r3 - 0.04972809184491411*r4;
      //a1 = -7.196457548543286*r1 + 10.760659484982682*r2 -
      //5.10380523549030050*r3 + 1.53960329905090450*r4;
      //a2 = 12.012829501508346*r1 - 25.001535905017075*r2 +
      //19.3446816555246950*r3 - 6.35597525201596500*r4;
      //a3 = -6.122934917841437*r1 + 14.782072520180590*r2 -
      //14.7820725201805900*r3 + 6.12293491784143700*r4;

      //// forward differencing variables
      //double d1,d2,d3;

      //// initial color value and differences
      //d = ((a3*alpha+a2)*alpha+a1)*alpha+a0+c1/K*_SCALE;
      //d1 = delta*(3*a3*alpha*alpha + alpha*(2*a2+3*a3*delta) + a2*delta + a3*delta2 + a1);
      //d2 = 2*delta2*(3*a3*(alpha + delta) + a2);
      //d3 = 6*a3*delta3;

      //// now fixed point stuff
      //int color,dc1,dc2,dc3;
      //color = (int)(d*K+0.5); // round to nearest value
      //dc1 = (int)(d1*K+0.5);
      //dc2 = (int)(d2*K+0.5);
      //dc3 = (int)(d3*K+0.5);
      //for (int i = 0; i < width/2; i++) {
        //// get color and update forward differencing stuff
        //ce = (color>>_BITS);
        //color += dc1; dc1 += dc2; dc2 += dc3;
        //// now draw 4 pixels
        //*p1++ = ce;
        //*p2-- = ce;
        //*p3++ = ce;
        //*p4-- = ce;
        //}
      //}

    //#undef _BITS // remove these defines
    //#undef _SCALE
    //} // GradientFill_7
  //}}}

  const int32_t width = radius.getXInt32();
  const int32_t height = radius.getYInt32();
  const float scale = 255.f / width;

  // allocate 4 quadrants texture
  uint8_t* pixels = (uint8_t*)cBaseTexture::allocate (width*2 * height*2);

  // calc tl qudrant
  uint8_t* pixel = pixels;
  for (int32_t y = height - 1; y >= 0; y--) {
    for (int32_t x = width - 1; x >= 0; x--) {
      float distance = sqrtf (float(x * x) + float(y * y)) * scale;
      *pixel++ = distance > 255.0f ? 0 : 255 - uint8_t(distance);
      }
    pixel += width;
    }
  cAlphaTexture alphaTexture (width*2, height*2, pixels);

  // transform to other 3 quadrants
  SimdTransformImage (alphaTexture.getPixels(), width * 2, width, height, 1,
                      SimdTransformTransposeRotate90, alphaTexture.getPixels(width, 0), width*2);
  SimdTransformImage (alphaTexture.getPixels(), width * 2, width, height, 1,
                      SimdTransformTransposeRotate270, alphaTexture.getPixels(0, height), width*2);
  SimdTransformImage (alphaTexture.getPixels(), width * 2, width, height, 1,
                      SimdTransformTransposeRotate180, alphaTexture.getPixels(width, height), width*2);
  // stamp 4 quadrants
  stamp (color, alphaTexture, centre - radius);
  }
//}}}
//{{{
void cDrawTexture::drawEllipse (const cColor& color, cPoint centre, cPoint radius, float width) {

  if (radius.x <= 0.1f)
    return;
  if (radius.y <= 0.1f)
    return;

  addEllipse (centre, radius, width);
  drawEdges (color);
  //{{{  fast
  //if (width > 0.1f) {
    //// outline, ignore width amount
    //float x = 0.f;
    //float y = -radius.y;
    //float err = 2.f - 2.f * radius.x;
    //float k = radius.y / radius.x;

    //uPixel colorPixel (color);
    //do {
      //setPixel (colorPixel, centre + cPoint (-x / k,  y));
      //setPixel (colorPixel, centre + cPoint ( x / k,  y));
      //setPixel (colorPixel, centre + cPoint ( x / k, -y));
      //setPixel (colorPixel, centre + cPoint (-x / k, -y));

      //float e2 = err;
      //if (e2 <= x) {
        //err += (++x * 2.f) + 1.f;
        //if ((-y == x) && (e2 <= y))
          //e2 = 0.f;
        //}

      //if (e2 > y)
        //err += (++y * 2.f) + 1.f;
      //} while (y <= 0.f);
    //}
  //else {
    ////  filled
    //float x1 = 0;
    //float y1 = -radius.x;
    //float err = 2 - 2*radius.x;
    //float k = radius.y / radius.x;

    //do {
      //drawRectangle (color, {centre.x - (x1 / k), centre.y + y1,
                             //centre.x-(x1 / k) + 2.f * (x1 / k) + 1.f, centre.y + y1 + 1.f});
      //drawRectangle (color, {centre.x-(x1 / k), centre.y  - y1,
                             //centre.x-(x1 / k) + 2.f * (x1 / k) + 1.f, centre.y - y1 + 1.f});
      //float e2 = err;
      //if (e2 <= x1) {
        //err += ++x1 * 2 + 1;
        //if (-y1 == centre.x && e2 <= y1)
          //e2 = 0;
        //}
      //if (e2 > y1)
        //err += ++y1*2 + 1;
      //} while (y1 <= 0);
    //}
  //}}}
  }
//}}}
//{{{
void cDrawTexture::drawLine (const cColor& color, cPoint point1, cPoint point2, float width) {

  (void)width;
  addLine (point1, point2, width);
  drawEdges (color);
  //{{{  fast
  //uPixel colorPixel (color);

  //int32_t deltax = (int32_t)abs (point2.x - point1.x); // The difference between the x's
  //int32_t deltay = (int32_t)abs (point2.y - point1.y); // The difference between the y's

  //// ???? could use fixed point maths ????
  //cPoint p = point1;
  //cPoint inc1 ((point2.x >= point1.x) ? 1 : -1, (point2.y >= point1.y) ? 1 : -1);
  //cPoint inc2 = inc1;

  //int32_t numAdd = (deltax >= deltay) ? deltay : deltax;
  //int32_t den = (deltax >= deltay) ? deltax : deltay;
  //if (deltax >= deltay) { // There is at least one x-value for every y-value
    //inc1.x = 0;           // Don't change the x when numerator >= denominator
    //inc2.y = 0;           // Don't change the y for every iteration
    //}
  //else {                  // There is at least one y-value for every x-value
    //inc2.x = 0;           // Don't change the x for every iteration
    //inc1.y = 0;           // Don't change the y when numerator >= denominator
    //}

  //int32_t num = den / 2;
  //int32_t numPixels = den;
  //for (int32_t pixel = 0; pixel <= numPixels; pixel++) {
    //setPixel (colorPixel, p);
    //num += numAdd;    // Increase the numerator by the top of the fraction
    //if (num >= den) { // Check if numerator >= denominator
      //num -= den;     // Calculate the new numerator value
      //p += inc1;
      //}
    //p += inc2;
    //}
  //}}}
  }
//}}}

//{{{
void cDrawTexture::drawBorder (const cColor& color, const cRect& rect, float width) {

  drawRectangle (color, {rect.left,          rect.top,            rect.right,        rect.top + width});
  drawRectangle (color, {rect.left,          rect.top + width,    rect.left + width, rect.bottom - width});
  drawRectangle (color, {rect.right - width, rect.top + width,    rect.right,        rect.bottom - width});
  drawRectangle (color, {rect.left,          rect.bottom - width, rect.right,        rect.bottom});
  }
//}}}
//{{{
void cDrawTexture::drawRounded (const cColor& color, const cRect& rect, float radius) {
  drawRectangle (color, rect); (void)radius;
  }
//}}}

// draw text
//{{{
cPoint cDrawTexture::measureText (cPoint size, const std::string& text, uint32_t font) {

  (void)size;
  (void)font;
  float width = 0.f;

  uint8_t lastCh = 0;
  for (auto ch : text) {
    if (lastCh)
      width += gFonts[font].getKern (lastCh, ch);
    cFont::sGlyph& glyph = gFonts[font].getChar (ch);
    width += glyph.mAdvanceWidth;
    lastCh = ch;
    }

  return cPoint (width, gFonts[font].getHeight());
  }
//}}}
//{{{
cPoint cDrawTexture::drawText (const cColor& color, const cRect& rect, const string& text, uint32_t font) {

  (void)font;
  cPoint point = rect.getTL();

  uint8_t lastCh = 0;

  for (auto ch : text) {
    if (lastCh)
      point.x += gFonts[font].getKern (lastCh, ch);

    cFont::sGlyph& glyph = gFonts[font].getChar (ch);
    stamp (color, glyph.mAlphaTexture, point + cPoint (glyph.mX, glyph.mY));
    point.x += glyph.mAdvanceWidth;

    lastCh = ch;
    }

  return cPoint (point.x - rect.getTL().x, gFonts[font].getHeight());
  }
//}}}
//{{{
cPoint cDrawTexture::drawTextShadow (const cColor& color, const cRect& rect, const std::string& text, uint32_t font) {
// crude

  (void)font;

  cPoint point = rect.getTL() + cPoint (2.f, 2.f);
  uint8_t lastCh = 0;
  for (auto ch : text) {
    if (lastCh)
      point.x += gFonts[font].getKern (lastCh, ch);
    cFont::sGlyph& glyph = gFonts[font].getChar (ch);
    stamp (kBlack, glyph.mAlphaTexture, point + cPoint (glyph.mX, glyph.mY));
    point.x += glyph.mAdvanceWidth;
    lastCh = ch;
    }

  point = rect.getTL();
  lastCh = 0;
  for (auto ch : text) {
    if (lastCh)
      point.x += gFonts[font].getKern (lastCh, ch);
    cFont::sGlyph& glyph = gFonts[font].getChar (ch);
    stamp (color, glyph.mAlphaTexture, point + cPoint (glyph.mX, glyph.mY));
    point.x += glyph.mAdvanceWidth;
    lastCh = ch;
    }

  return cPoint (point.x - rect.getTL().x, gFonts[font].getHeight());
  }
//}}}

// draw AA
//{{{
void cDrawTexture::addEdgeFrom (cPoint point) {

  if (!mDrawAA)
    mDrawAA = new cDrawAA();

  mDrawAA->addEdgeFrom (point);
  }
//}}}
//{{{
void cDrawTexture::addEdgeTo (cPoint point) {
  mDrawAA->addEdgeTo (point);
  }
//}}}
//{{{
void cDrawTexture::addLine (cPoint point1, cPoint point2, float width) {

  cPoint perp = (point2 - point1).perp() * width;
  addEdgeFrom (point1 + perp);
  addEdgeTo (point2 + perp);
  addEdgeTo (point2 - perp);
  addEdgeTo (point1 - perp);
  }
//}}}
//{{{
void cDrawTexture::addArrowHead (cPoint point1, cPoint point2, float width) {

  cPoint perp = (point2 - point1).perp() * width;
  addEdgeFrom (point1 + perp);
  addEdgeTo (point2);
  addEdgeTo (point1 - perp);
  }
//}}}
//{{{
void cDrawTexture::addTriangle (cPoint point1, cPoint point2, cPoint point3) {

  addEdgeFrom (point1);
  addEdgeTo (point2);
  addEdgeTo (point3);
  }
//}}}
//{{{
void cDrawTexture::addEllipse (cPoint centre, cPoint radius, float outlineWidth) {
// !!! tart this up !!!!

  constexpr float kAngleToRadians = kPi / 180.f;
  constexpr int kSteps = 16;
  constexpr float fstep = 360.f / kSteps;

  // clockwise ellipse
  addEdgeFrom (centre + cPoint(radius.x, 0.f));

  float angle = 0.f;
  angle += fstep;
  while (angle < 360.f) {
    float radians = angle * kAngleToRadians;
    addEdgeTo (centre + cPoint (cos(radians) * radius.x, sin(radians) * radius.x));
    angle += fstep;
    }

  if ((outlineWidth > 0.f) && (outlineWidth < radius.x)) {
    // anti clockwise ellipse cuts hole
    radius -= outlineWidth;
    addEdgeFrom (centre + cPoint(radius.x, 0.f));

    angle -= fstep;
    while (angle > 0.f) {
      float radians = angle * kAngleToRadians;
      addEdgeTo (centre + cPoint (cos(radians) * radius.x, sin(radians) * radius.x));
      angle -= fstep;
      }
    }
  }
//}}}
//{{{
void cDrawTexture::drawEdges (const cColor& color) {
// draw antiAliased - lambda callback single line spans of src alpha

  mDrawAA->draw (getWidth(), getHeight(), true,
                 [&](uint8_t* src, int32_t dstx, int32_t dsty, uint32_t numPixels) {
                   cAlphaTexture alphaTexture (numPixels, 1, src);
                   stamp (color, alphaTexture, {dstx, dsty});
                   }
                 );
  }
//}}}
