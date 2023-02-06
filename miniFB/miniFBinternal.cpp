// MiniFB_internal.cpp
#include <cstdint>
#include "miniFBinternal.h"

//#define kUseBilinearInterpolation
// Only for 32 bits images
//{{{
//static uint32_t interpolate (uint32_t* srcImage, uint32_t x, uint32_t y,
                             //uint32_t srcOffsetX, uint32_t srcOffsetY, uint32_t srcWidth, uint32_t srcHeight, uint32_t srcPitch) {

  //uint32_t incX = x + 1 < srcWidth ? 1 : 0;
  //uint32_t incY = y + 1 < srcHeight ? srcPitch : 0;
  //uint8_t *p00 = (uint8_t*) &srcImage[(srcOffsetX >> 16)];
  //uint8_t *p01 = (uint8_t*) &srcImage[(srcOffsetX >> 16) + incX];
  //uint8_t *p10 = (uint8_t*) &srcImage[(srcOffsetX >> 16) + incY];
  //uint8_t *p11 = (uint8_t*) &srcImage[(srcOffsetX >> 16) + incY + incX];

  //uint32_t wx2 = srcOffsetX & 0xffff;
  //uint32_t wy2 = srcOffsetY & 0xffff;
  //uint32_t wx1 = 0x10000 - wx2;
  //uint32_t wy1 = 0x10000 - wy2;

  //uint32_t w1 = ((uint64_t) wx1 * wy1) >> 16;
  //uint32_t w2 = ((uint64_t) wx2 * wy1) >> 16;
  //uint32_t w3 = ((uint64_t) wx1 * wy2) >> 16;
  //uint32_t w4 = ((uint64_t) wx2 * wy2) >> 16;

  //// If you don't have uint64_t
  ////uint32_t b = (((p00[0] * wx1 + p01[0] * wx2) >> 16) * wy1 + ((p10[0] * wx1 + p11[0] * wx2) >> 16) * wy2) >> 16;
  ////uint32_t g = (((p00[1] * wx1 + p01[1] * wx2) >> 16) * wy1 + ((p10[1] * wx1 + p11[1] * wx2) >> 16) * wy2) >> 16;
  ////uint32_t r = (((p00[2] * wx1 + p01[2] * wx2) >> 16) * wy1 + ((p10[2] * wx1 + p11[2] * wx2) >> 16) * wy2) >> 16;
  ////uint32_t a = (((p00[3] * wx1 + p01[3] * wx2) >> 16) * wy1 + ((p10[3] * wx1 + p11[3] * wx2) >> 16) * wy2) >> 16;

  //uint32_t b = ((p00[0] * w1 + p01[0] * w2) + (p10[0] * w3 + p11[0] * w4)) >> 16;
  //uint32_t g = ((p00[1] * w1 + p01[1] * w2) + (p10[1] * w3 + p11[1] * w4)) >> 16;
  //uint32_t r = ((p00[2] * w1 + p01[2] * w2) + (p10[2] * w3 + p11[2] * w4)) >> 16;
  //uint32_t a = ((p00[3] * w1 + p01[3] * w2) + (p10[3] * w3 + p11[3] * w4)) >> 16;

  //return (a << 24) + (r << 16) + (g << 8) + b;
  //}
//}}}
//{{{
//void stretchImage (uint32_t* srcImage, uint32_t srcX, uint32_t srcY, uint32_t srcWidth, uint32_t srcHeight, uint32_t srcPitch,
                    //uint32_t* dstImage, uint32_t dstX, uint32_t dstY, uint32_t dstWidth, uint32_t dstHeight, uint32_t dstPitch) {

  //if (!srcImage)
    //return;
  //if (!dstImage)
    //return;

  //const uint32_t deltaX = (srcWidth  << 16) / dstWidth;
  //const uint32_t deltaY = (srcHeight << 16) / dstHeight;

  //srcImage += srcX + srcY * srcPitch;
  //dstImage += dstX + dstY * dstPitch;

  //uint32_t srcOffsetY = 0;
  //for (uint32_t y = 0; y < dstHeight; ++y) {
    //uint32_t srcOffsetX = 0;
    //for (uint32_t x = 0; x < dstWidth; ++x) {
      //#if defined(kUseBilinearInterpolation)
        //dstImage[x] = interpolate (srcImage, x+srcX, y+srcY, srcOffsetX, srcOffsetY, srcWidth, srcHeight, srcPitch);
      //#else
        //dstImage[x] = srcImage[srcOffsetX >> 16];
      //#endif
        //srcOffsetX += deltaX;
      //}

    //srcOffsetY += deltaY;
    //if (srcOffsetY >= 0x10000) {
      //srcImage += (srcOffsetY >> 16) * srcPitch;
      //srcOffsetY &= 0xffff;
      //}

    //dstImage += dstPitch;
    //}
  //}
//}}}
//{{{
void calcDstFactor (sWindowData *window_data, uint32_t width, uint32_t height) {

  if (window_data->dst_width == 0)
    window_data->dst_width = width;

  window_data->factor_x     = (float) window_data->dst_offset_x / (float) width;
  window_data->factor_width = (float) window_data->dst_width    / (float) width;

  if (window_data->dst_height == 0)
    window_data->dst_height = height;

  window_data->factor_y      = (float) window_data->dst_offset_y / (float) height;
  window_data->factor_height = (float) window_data->dst_height   / (float) height;
  }
//}}}
//{{{
void resizeDst (sWindowData *window_data, uint32_t width, uint32_t height) {

  window_data->dst_offset_x = (uint32_t) (width  * window_data->factor_x);
  window_data->dst_offset_y = (uint32_t) (height * window_data->factor_y);
  window_data->dst_width    = (uint32_t) (width  * window_data->factor_width);
  window_data->dst_height   = (uint32_t) (height * window_data->factor_height);
  }
//}}}

double g_timer_frequency;
double g_timer_resolution;
double g_time_for_frame = 1.0 / 60.0;
bool g_use_hardware_sync = false;

extern uint64_t timerTick();
extern void timerInit();

// target
//{{{
void setTargetFps (uint32_t fps) {

  if (fps == 0)
    g_time_for_frame = 0;
  else
      g_time_for_frame = 1.0 / fps;
  setTargetFpsAux();
  }
//}}}
//{{{
unsigned getTargetFps() {

  if (g_time_for_frame == 0)
    return 0;
  else
    return (unsigned) (1.0 / g_time_for_frame);
  }
//}}}

// timer
//{{{
struct sMiniFBtimer* timerCreate() {
    static int  once = 1;
    sMiniFBtimer   *tmr;

    if(once) {
      once = 0;
      timerInit();
      }

    tmr = (sMiniFBtimer*)malloc(sizeof(sMiniFBtimer));
    timerReset (tmr);

    return tmr;
}
//}}}
//{{{
void timerDestroy (struct sMiniFBtimer *tmr) {
    if(tmr != 0x0) {
        free(tmr);
    }
}
//}}}
//{{{
void timerReset (struct sMiniFBtimer *tmr) {
    if(tmr == 0x0)
        return;

    tmr->start_time    = timerTick();
    tmr->delta_counter = tmr->start_time;
    tmr->time          = 0;
}
//}}}

//{{{
double timerNow (struct sMiniFBtimer *tmr) {
    uint64_t    counter;

    if(tmr == 0x0)
        return 0.0;

    counter         = timerTick();
    tmr->time      += (counter - tmr->start_time);
    tmr->start_time = counter;

    return tmr->time * g_timer_resolution;
}
//}}}
//{{{
double timerDelta (struct sMiniFBtimer *tmr) {
    int64_t     counter;
    uint64_t    delta;

    if(tmr == 0x0)
        return 0.0;

    counter            = timerTick();
    delta              = (counter - tmr->delta_counter);
    tmr->delta_counter = counter;

    return delta * g_timer_resolution;
}
//}}}
//{{{
double timerGetFrequency() {
    return g_timer_frequency;
}
//}}}
//{{{
double timerGetResolution() {
    return g_timer_resolution;
}
//}}}
