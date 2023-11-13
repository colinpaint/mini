// cSongLoaderBox.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <string>
#include <array>
#include <mutex>

#include "../common/basicTypes.h"
#include "../common/date.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../gui/cWindow.h"

#include "../song/cSong.h"
#include "../song/cSongLoader.h"

#include "cSongLoaderBox.h"

using namespace std;
//}}}

//{{{
cSongLoaderBox::cSongLoaderBox (cWindow& window, float width, float height, cSongLoader& songLoader)
    : cBox("songLoader", window, width, height), mSongLoader(songLoader) {
  mPin = true;
  }
//}}}

//{{{
bool cSongLoaderBox::pick (bool inClient, cPoint pos, bool& change) {
// overide rectangle pick, no song no pick

  if (mSongLoader.getSong())
    return cBox::pick (inClient, pos, change);
  return false;
  }
//}}}
//{{{
bool cSongLoaderBox::prox (bool inClient, cPoint pos) {

  if (mSongLoader.getSong())
    return cBox::prox (inClient, pos);
  else
    return false;
  }
//}}}
//{{{
bool cSongLoaderBox::down (bool right, cPoint pos) {

  (void)right;

  cSong* song = mSongLoader.getSong();
  if (song) {
    //std::shared_lock<std::shared_mutex> lock (song->getSharedMutex());
    pos.y += mRect.top;
    if (pos.y > mDstOverviewTop) {
      mPressedFrameNum = song->getFirstFrameNum() + ((pos.x * song->getTotalFrames()) / getWidth());
      song->setPlayPts (song->getPtsFromFrameNum (int64_t(mPressedFrameNum)));
      mOverviewPressed = true;
      changed();
      }
    else if (pos.y > mDstRangeTop) {
      mPressedFrameNum = song->getPlayFrameNum() + ((pos.x - (getWidth()/2.f)) * mFrameStep / mFrameWidth);
      song->getSelect().start (int64_t(mPressedFrameNum));
      mRangePressed = true;
      changed();
      }
    else
      mPressedFrameNum = double(song->getFrameNumFromPts (int64_t(song->getPlayPts())));
    return true;
    }

  return false;
  }
//}}}
//{{{
bool cSongLoaderBox::move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) {

  (void)right;
  (void)pos;
  (void)pressure;
  (void)timestamp;

  cSong* song = mSongLoader.getSong();
  if (song) {
    //std::shared_lock<std::shared_mutex> lock (song.getSharedMutex());
    if (mOverviewPressed) {
      //song->setPlayPts (song->getPtsFromFrameNum (song->getFirstFrameNum() + int64_t(pos.x * song->getTotalFrames() / getWidth())));
      mPressedFrameNum -= (inc.x / mFrameWidth) * 2.f;
      song->setPlayPts (song->getPtsFromFrameNum (int64_t(mPressedFrameNum)));
      }
    else if (mRangePressed) {
      mPressedFrameNum += (inc.x / mFrameWidth) * mFrameStep;
      song->getSelect().move (int64_t(mPressedFrameNum));
      changed();
      }
    else {
      mPressedFrameNum -= (inc.x / mFrameWidth) * mFrameStep;
      song->setPlayPts (song->getPtsFromFrameNum (int64_t(mPressedFrameNum)));
      }
    }

  return true;
  }
//}}}
//{{{
bool cSongLoaderBox::up (bool right, bool mouseMoved, cPoint pos) {

  (void)right;
  (void)mouseMoved;
  (void)pos;

  //cWidget::up();

  cSong* song = mSongLoader.getSong();
  if (song)
    song->getSelect().end();

  mOverviewPressed = false;
  mRangePressed = false;

  return true;
  }
//}}}
//{{{
bool cSongLoaderBox::wheel (int delta, cPoint pos) {

  (void)delta;
  (void)pos;

  if (getShow())
    setZoom (mZoom - (int)delta);

  return true;
  }
//}}}

//{{{
void cSongLoaderBox::draw() {

  cSong* song = mSongLoader.getSong();
  if (!song)
    return;

  layoutWaveforms();

  { // locked scope
  shared_lock<shared_mutex> lock (song->getSharedMutex());

  // wave left right frames, clip right not left
  int64_t playFrame = song->getPlayFrameNum();
  int64_t leftWaveFrame = playFrame - (((int(getWidth())+mFrameWidth)/2) * mFrameStep) / mFrameWidth;
  int64_t rightWaveFrame = playFrame + (((int(getWidth())+mFrameWidth)/2) * mFrameStep) / mFrameWidth;
  rightWaveFrame = min (rightWaveFrame, song->getLastFrameNum());

  if (song->getNumFrames()) {
    bool mono = (song->getNumChannels() == 1);
    drawWaveform (song, playFrame, leftWaveFrame, rightWaveFrame, mono);
    if (mShowOverview)
      drawOverview (song, playFrame, mono);
    drawFrequencies (song, playFrame);
    }

  drawRange (song, playFrame, leftWaveFrame, rightWaveFrame);
  }

  // draw firstTime left justified
  cRect r (mRect);
  r.top = r.bottom - getBoxHeight();
  drawText (kGray, r, song->getFirstTimeString());

  // draw playTime centre justified large
  cPoint textSize = measureText (song->getPlayTimeString(), cWindow::kLargeMenuFont).x;
  r.left = getCentre().x - (textSize.x/2);
  r.top = r.bottom - getBoxHeight()*2;
  drawText (kWhite, r, song->getPlayTimeString(), cWindow::kLargeMenuFont);

  // draw lastTime right justified
  textSize = measureText (song->getLastTimeString()).x;
  r.left = r.right - textSize.x;
  r.top = r.bottom - getBoxHeight();
  drawText (kGray, r, song->getLastTimeString());
  }
//}}}

// private:
//{{{
void cSongLoaderBox::setZoom (int zoom) {

  mZoom = min (max (zoom, mMinZoom), mMaxZoom);

  // zoomIn expanding frame to mFrameWidth pix
  mFrameWidth = (mZoom < 0) ? -mZoom+1 : 1;

  // zoomOut summing mFrameStep frames per pix
  mFrameStep = (mZoom > 0) ? mZoom+1 : 1;
  }
//}}}
//{{{
void cSongLoaderBox::layoutWaveforms() {

  // check for window size change, refresh any caches dependent on size
  mChanged |= (getSize() != mCacheSize);
  mCacheSize = getSize();

  // heights
  mWaveHeight = 100.f;
  mOverviewHeight = mShowOverview ? 100.f : 0.f;
  mRangeHeight = 8.f;
  mFreqHeight = getHeight() - mRangeHeight - mWaveHeight - mOverviewHeight;

  // y pos
  mDstFreqTop = mRect.top;
  mDstWaveTop = mDstFreqTop + mFreqHeight;
  mDstWaveCentre = mDstWaveTop + (mWaveHeight/2);
  mDstRangeTop = mDstWaveTop + mWaveHeight;
  mDstOverviewTop = mDstRangeTop + mRangeHeight;
  mDstOverviewCentre = mDstOverviewTop + (mOverviewHeight/2);
  }
//}}}

//{{{
void cSongLoaderBox::drawWaveform (cSong* song, int64_t playFrame, int64_t leftFrame, int64_t rightFrame, bool mono) {

  (void)mono;
  array <float,2> values = { 0.f };

  float peakValueScale = mWaveHeight / song->getMaxPeakValue() / 2.f;
  float powerValueScale = mWaveHeight / song->getMaxPowerValue() / 2.f;

  float width = (float)mFrameStep;
  if (mFrameStep == 1) {
    //{{{  draw all peak values
    float left = 0;
    for (int64_t frame = leftFrame; frame < rightFrame; frame += mFrameStep, left += width) {
      cSong::cFrame* framePtr = song->findFrameByFrameNum (frame);
      if (framePtr && framePtr->getPowerValues()) {
        if (framePtr->isSilence()) // draw red silence
          drawRectangleUnclipped (kRed, {left, mDstWaveCentre - 1.f, left  + width, mDstWaveCentre + 1.f});

        // draw frame peak values scaled to maxPeak
        float* peakValuesPtr = framePtr->getPeakValues();
        values[0] = *peakValuesPtr * peakValueScale;
        values[1] = *(peakValuesPtr+1) * peakValueScale;
        drawRectangleUnclipped (kDarkGray, {left, mDstWaveCentre - values[0], left + width, mDstWaveCentre + values[1]});
        }
      }
    }
    //}}}

  float left = mRect.left;
  //float top = mRect.top;
  //{{{  draw powerValues before playFrame, summed if zoomed out
  for (int64_t frame = leftFrame; frame < playFrame; frame += mFrameStep, left += width) {
    cSong::cFrame* framePtr = song->findFrameByFrameNum (frame);
    if (framePtr) {
      if (mFrameStep == 1) {
        // power scaled to maxPeak
        if (framePtr->getPowerValues()) {
         float* powerValuesPtr = framePtr->getPowerValues();
          values[0] = powerValuesPtr[0] * peakValueScale;
          values[1] = powerValuesPtr[1] * peakValueScale;
          }
        }
      else {
        // sum mFrameStep frames, mFrameStep aligned, scaled to maxPower
        values[0] = 0.f;
        values[1] = 0.f;

        int64_t alignedFrame = frame - (frame % mFrameStep);
        int64_t toSumFrame = min (alignedFrame + mFrameStep, rightFrame);
        for (int64_t sumFrame = alignedFrame; sumFrame < toSumFrame; sumFrame++) {
          cSong::cFrame* sumFramePtr = song->findFrameByFrameNum (sumFrame);
          if (sumFramePtr && sumFramePtr->getPowerValues()) {
            float* powerValuesPtr = sumFramePtr->getPowerValues();
            values[0] += powerValuesPtr[0] * powerValueScale;
            values[1] += powerValuesPtr[1] * powerValueScale;
            }
          }

        values[0] /= toSumFrame - alignedFrame + 1;
        values[1] /= toSumFrame - alignedFrame + 1;
        }

      drawRectangleUnclipped (kDarkBlue, {left, mDstWaveCentre - values[0], left + width, mDstWaveCentre + values[1]});
      }
    }
  //}}}
  //{{{  draw powerValues playFrame, no sum
  // power scaled to maxPeak
  cSong::cFrame* framePtr = song->findFrameByFrameNum (playFrame);

  if (framePtr && framePtr->getPowerValues()) {
    drawRectangleUnclipped (framePtr->isSilence() ? kRed : kDimGray, {left, mDstWaveTop, left + width, mDstWaveTop + mWaveHeight});

    // draw play frame power scaled to maxPeak
    float* powerValuesPtr = framePtr->getPowerValues();
    values[0] = powerValuesPtr[0] * peakValueScale;
    values[1] = powerValuesPtr[1] * peakValueScale;
    drawRectangleUnclipped (kWhite, {left, mDstWaveCentre - values[0], left + width, mDstWaveCentre + values[1]});
    }

  left += width;
  //}}}
  //{{{  draw powerValues after playFrame, summed if zoomed out
  for (int64_t frame = playFrame + mFrameStep; frame < rightFrame; frame += mFrameStep, left += width) {
    cSong::cFrame* sumFramePtr = song->findFrameByFrameNum (frame);
    if (sumFramePtr && sumFramePtr->getPowerValues()) {
      if (mFrameStep == 1) {
        // power scaled to maxPeak
        float* powerValuesPtr = sumFramePtr->getPowerValues();
        values[0] = powerValuesPtr[0] * peakValueScale;
        values[1] = powerValuesPtr[1] * peakValueScale;
        }
      else {
        // sum mFrameStep frames, mFrameStep aligned, scaled to maxPower
        values[0] = 0.f;
        values[1] = 0.f;

        int64_t alignedFrame = frame - (frame % mFrameStep);
        int64_t toSumFrame = min (alignedFrame + mFrameStep, rightFrame);
        for (int64_t sumFrame = alignedFrame; sumFrame < toSumFrame; sumFrame++) {
          cSong::cFrame* toSumFramePtr = song->findFrameByFrameNum (sumFrame);
          if (toSumFramePtr && toSumFramePtr->getPowerValues()) {
            float* powerValuesPtr = toSumFramePtr->getPowerValues();
            values[0] += powerValuesPtr[0] * powerValueScale;
            values[1] += powerValuesPtr[1] * powerValueScale;
            }
          }

        values[0] /= toSumFrame - alignedFrame + 1;
        values[1] /= toSumFrame - alignedFrame + 1;
        }

      drawRectangleUnclipped (kLightGray, {left, mDstWaveCentre - values[0], left + width, mDstWaveCentre + values[1]});
      }
    }
  //}}}

  //{{{  copy reversed spectrum column to bitmap, clip high freqs to height
  //int freqSize = min (song->getNumFreqBytes(), (int)mFreqHeight);
  //int freqOffset = song->getNumFreqBytes() > (int)mFreqHeight ? song->getNumFreqBytes() - (int)mFreqHeight : 0;

  // bitmap sampled aligned to mFrameStep, !!! could sum !!! ?? ok if neg frame ???
  //auto alignedFromFrame = fromFrame - (fromFrame % mFrameStep);
  //for (auto frame = alignedFromFrame; frame < toFrame; frame += mFrameStep) {
    //auto framePtr = song->getAudioFramePtr (frame);
    //if (framePtr) {
      //if (framePtr->getFreqLuma()) {
        //uint32_t bitmapIndex = getSrcIndex (frame);
        //D2D1_RECT_U bitmapRectU = { bitmapIndex, 0, bitmapIndex+1, (UINT32)freqSize };
        //mBitmap->CopyFromMemory (&bitmapRectU, framePtr->getFreqLuma() + freqOffset, 1);
        //}
      //}
    //}
  //}}}
  }
//}}}

//{{{
void cSongLoaderBox::drawRange (cSong* song, int64_t playFrame, int64_t leftFrame, int64_t rightFrame) {

  (void)song;
  (void)playFrame;
  (void)leftFrame;
  (void)rightFrame;

  drawRectangleUnclipped (kWhite, {0.f, mDstRangeTop, getWidth(), mDstRangeTop + mRangeHeight});

  //for (auto &item : song->getSelect().getItems()) {
  //  auto firstx = (mSize.y/ 2.f) + (item.getFirstFrame() - playFrame) * mFrameWidth / mFrameStep;
  //  float lastx = item.getMark() ? firstx + 1.f :
  //                                 (getWidth()/2.f) + (item.getLastFrame() - playFrame) * mFrameWidth / mFrameStep;
  //  vg->rect (cPointF(firstx, mDstRangeTop), cPointF(lastx - firstx, mRangeHeight));

  //  auto title = item.getTitle();
  //  if (!title.empty()) {
      //dstRect = { mRect.left + firstx + 2.f, mDstRangeTop + mRangeHeight - mWindow.getTextFormat()->GetFontSize(),
      //            mRect.right, mDstRangeTop + mRangeHeight + 4.f };
      //dc->DrawText (std::wstring (title.begin(), title.end()).data(), (uint32_t)title.size(), mWindow.getTextFormat(),
      //              dstRect, mWindow.getWhiteBrush(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
   //   }
    //}
  }
//}}}
//{{{
void cSongLoaderBox::drawWaveformOverview (cSong* song, int64_t firstFrame, int64_t playFrame, float playFrameX, float valueScale, bool mono) {
// use simple overview cache, invalidate if anything changed

  (void)playFrame;
  (void)playFrameX;
  int64_t lastFrame = song->getLastFrameNum();
  int64_t totalFrames = song->getTotalFrames();

  bool changed = mChanged ||
                 (mOverviewTotalFrames != totalFrames) ||
                 (mOverviewFirstFrame != firstFrame) ||
                 (mOverviewLastFrame != lastFrame) ||
                 (mOverviewValueScale != valueScale);

  mOverviewTotalFrames = totalFrames;
  mOverviewLastFrame = lastFrame;
  mOverviewFirstFrame = firstFrame;
  mOverviewValueScale = valueScale;

  float left = mRect.left;
  float width = 1.f;

  if (!changed) {
    //{{{  draw overview cache, return
    for (uint32_t x = 0; x < static_cast<uint32_t>(getWidth()); x++, left += 1.f)
      drawRectangleUnclipped (kGray, {left, mDstOverviewCentre - mOverviewValuesL[x],
                             left + width, mDstOverviewCentre - mOverviewValuesL[x] + mOverviewValuesR[x]});
    return;
    }
    //}}}

  array <float,2> values;
  for (uint32_t x = 0; x < getWidth(); x++, left += 1.f) {
    // iterate window width
    int64_t frame = firstFrame + ((x * totalFrames) / static_cast<uint32_t>(getWidth()));
    int64_t toFrame = firstFrame + (((x+1) * totalFrames) / static_cast<uint32_t>(getWidth()));
    if (toFrame > lastFrame)
      toFrame = lastFrame+1;

    bool silence = false;
    cSong::cFrame* framePtr = song->findFrameByFrameNum (frame);
    if (framePtr && framePtr->getPowerValues()) {
      //{{{  accumulate frame,silence
      if (framePtr->isSilence())
        silence = true;

      float* powerValues = framePtr->getPowerValues();
      values[0] = powerValues[0];
      values[1] = mono ? 0 : powerValues[1];

      if (frame < toFrame) {
        int numSummedFrames = 1;
        frame++;
        while (frame < toFrame) {
          framePtr = song->findFrameByFrameNum (frame);
          if (framePtr) {
            if (framePtr->isSilence())
              silence = true;
            if (framePtr->getPowerValues()) {
              float* sumPowerValues = framePtr->getPowerValues();
              values[0] += sumPowerValues[0];
              values[1] += mono ? 0 : sumPowerValues[1];
              numSummedFrames++;
              }
            }
          frame++;
          }

        values[0] /= numSummedFrames;
        values[1] /= numSummedFrames;
        }

      values[0] *= valueScale;
      values[1] *= valueScale;

      mOverviewValuesL[x] = values[0];
      mOverviewValuesR[x] = values[0] + values[1];
      }
      //}}}

    if (silence) // draw red silence
      drawRectangleUnclipped (kRed, {left, mDstOverviewCentre - 1.f,
                            left + width, mDstOverviewCentre + 1.f});
    drawRectangleUnclipped (kGray, {left, mDstOverviewCentre - mOverviewValuesL[x],
                           left + width, mDstOverviewCentre - mOverviewValuesL[x] + mOverviewValuesR[x]});
    }
  }
//}}}
//{{{
void cSongLoaderBox::drawOverviewLens (cSong* song, int64_t playFrame, float centreX, float width, bool mono) {
// draw frames centred at playFrame -/+ width in els, centred at centreX

  // cut hole and frame it
  drawRectangleUnclipped (kBlack, {centreX - width, mDstOverviewTop,
                          centreX - width + (width * 2.f), mDstOverviewTop + mOverviewHeight});
  drawBorder (kWhite, {centreX - width, mDstOverviewTop,
                                 centreX - width + (width * 2.f), mDstOverviewTop + mOverviewHeight});

  // calc leftmost frame, clip to valid frame, adjust firstX which may overlap left up to mFrameWidth
  float leftFrame = playFrame - width;
  float firstX = centreX - (playFrame - leftFrame);
  if (leftFrame < 0) {
    firstX += -leftFrame;
    leftFrame = 0;
    }

  int64_t rightFrame = playFrame + static_cast<int64_t>(width);
  rightFrame = min (rightFrame, song->getLastFrameNum());

  // calc lens max power
  float maxPowerValue = 0.f;
  for (int64_t frame = int(leftFrame); frame <= rightFrame; frame++) {
    cSong::cFrame* framePtr = song->findFrameByFrameNum (frame);
    if (framePtr && framePtr->getPowerValues()) {
      float* powerValues = framePtr->getPowerValues();
      maxPowerValue = max (maxPowerValue, powerValues[0]);
      if (!mono)
        maxPowerValue = max (maxPowerValue, powerValues[1]);
      }
    }

  // draw unzoomed waveform, start before playFrame
  float left = firstX;
  float valueScale = mOverviewHeight / 2.f / maxPowerValue;
  for (int64_t frame = int(leftFrame); frame <= rightFrame; frame++) {
    cSong::cFrame* framePtr = song->findFrameByFrameNum (frame);
    if (framePtr && framePtr->getPowerValues()) {
      if (framePtr->isSilence()) // draw red silence
        drawRectangleUnclipped (kRed, {left, mDstOverviewCentre - 2.f, left + 1.f, mDstOverviewCentre + 2.f});
      else {
        float* powerValues = framePtr->getPowerValues();
        float top = mono ? mDstOverviewTop + mOverviewHeight - (powerValues[0] * valueScale * 2.f)
                         : mDstOverviewCentre - (powerValues[0] * valueScale);
        float height = mono ? powerValues[0] * valueScale * 2.f
                            : (powerValues[0] + powerValues[1]) * valueScale;
        drawRectangleUnclipped (kGray, {left, top, left + 1.f, top + height});
        }
      }

    left += 1.f;
    }
  }
//}}}
//{{{
void cSongLoaderBox::drawOverview (cSong* song, int64_t playFrame, bool mono) {

  if (!song->getTotalFrames())
    return;

  int64_t firstFrame = song->getFirstFrameNum();
  float left = mRect.left + (((playFrame - firstFrame) * getWidth()) / song->getTotalFrames());
  float valueScale = mOverviewHeight / 2.f / song->getMaxPowerValue();
  drawWaveformOverview (song, firstFrame, playFrame, left, valueScale, mono);

  if (mOverviewPressed) {
    //{{{  animate on in 5 frames to half height
    if (mOverviewLens < getHeight() / 2.f) {
      mOverviewLens += getHeight() / 2.f / 5.f;
      changed();
      }
    }
    //}}}
  else {
    //{{{  animate off
    if (mOverviewLens > 1.f) {
      mOverviewLens /= 2.f;
      changed();
      }
    else if (mOverviewLens > 0.f) {
      // finish animate
      mOverviewLens = 0.f;
      changed();
      }
    }
    //}}}

  if (mOverviewLens > 0.f) {
    float overviewLensCentreX = left;
    if (overviewLensCentreX - mOverviewLens < 0.f)
      overviewLensCentreX = (float)mOverviewLens;
    else if (overviewLensCentreX + mOverviewLens > getWidth())
      overviewLensCentreX = getWidth() - mOverviewLens;

    drawOverviewLens (song, playFrame, overviewLensCentreX, mOverviewLens-1.f, mono);
    }

  else {
    //  draw playFrame
    cSong::cFrame* framePtr = song->findFrameByFrameNum (playFrame);

    drawRectangleUnclipped (framePtr->isSilence() ? kRed : kDimGray,
                            {left, mDstOverviewTop, left + 1.f, mDstOverviewTop + mOverviewHeight});

    if (framePtr && framePtr->getPowerValues()) {
      float* powerValues = framePtr->getPowerValues();
      float top = (mono ? (mDstOverviewTop + mOverviewHeight - (powerValues[0] * valueScale * 2.f)) :
                          (mDstOverviewCentre - (powerValues[0] * valueScale)));
      float height = mono ? (powerValues[0] * valueScale * 2.f) : ((powerValues[0] + powerValues[1]) * valueScale);
      drawRectangleUnclipped (kWhite, {left, top, left + 1.f, top + height});
      }
    }
  }
//}}}

//{{{
void cSongLoaderBox::drawFrequencies (cSong* song, int64_t playFrame) {

  const float valueScale = 100.f / 255.f;

  float left = mRect.left;
  cSong::cFrame* framePtr = song->findFrameByFrameNum (playFrame);
  if (framePtr && framePtr->getFreqValues()) {
    uint8_t* freqValues = framePtr->getFreqValues();
    for (uint32_t i = 0; (i < song->getNumFreqBytes()) && ((i*2) < getWidth()); i++, left += 2.f)
      drawRectangleUnclipped (kYellow, {left, mRect.bottom - freqValues[i] * valueScale, left + 2.f, mRect.bottom});
    }
  }
//}}}
