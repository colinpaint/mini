// cSongLoaderBox.h
#pragma once
//{{{  includes
#include <cstdint>
#include <array>

#include "../common/basicTypes.h"
#include "../gui/cWindow.h"

#include "../song/cSong.h"
#include "../song/cSongLoader.h"
//}}}

class cSongLoaderBox : public cWindow::cBox {
public:
  cSongLoaderBox (cWindow& window, float width, float height, cSongLoader& songLoader);
  virtual ~cSongLoaderBox() = default;

  virtual bool pick (bool inClient, cPoint pos, bool& change) final;
  virtual bool prox (bool inClient, cPoint pos) final;
  virtual bool down (bool right, cPoint pos) final;
  virtual bool move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) final;
  virtual bool up (bool right, bool mouseMoved, cPoint pos) final;
  virtual bool wheel (int delta, cPoint pos) final;
  virtual void draw() final;

private:
  void setZoom (int zoom);
  void layoutWaveforms();

  void drawWaveform (cSong* song, int64_t playFrame, int64_t leftFrame, int64_t rightFrame, bool mono);

  void drawWaveformOverview (cSong* song, int64_t firstFrame, int64_t playFrame, float playFrameX, float valueScale, bool mono);
  void drawOverviewLens (cSong* song, int64_t playFrame, float centreX, float width, bool mono);
  void drawOverview (cSong* song, int64_t playFrame, bool mono);

  void drawFrequencies (cSong* song, int64_t playFrame);
  void drawRange (cSong* song, int64_t playFrame, int64_t leftFrame, int64_t rightFrame);

  // vars
  cSongLoader& mSongLoader;
  bool mShowOverview = true;

  bool mChanged = false;
  cPoint mCacheSize = {0.f};

  //float mMove = 0;
  //bool mMoved = false;
  //float mPressInc = 0;

  double mPressedFrameNum = 0;
  bool mOverviewPressed = false;
  bool mRangePressed = false;

  // zoom - 0 unity, > 0 zoomOut framesPerPix, < 0 zoomIn pixPerFrame
  int mZoom = 0;
  int mMinZoom = -8;
  int mMaxZoom = 8;
  int mFrameWidth = 1;
  int mFrameStep = 1;

  float mOverviewLens = 0.f;

  // layout
  float mFreqHeight = 0.f;
  float mWaveHeight = 0.f;
  float mRangeHeight = 0.f;
  float mOverviewHeight = 0.f;

  float mDstFreqTop = 0.f;
  float mDstWaveTop = 0.f;
  float mDstRangeTop = 0.f;
  float mDstOverviewTop = 0.f;

  float mDstWaveCentre = 0.f;
  float mDstOverviewCentre = 0.f;

  // mOverview cache
  std::array <float,3000> mOverviewValuesL = { 0.f };
  std::array <float,3000> mOverviewValuesR = { 0.f };
  int64_t mOverviewTotalFrames = 0;
  int64_t mOverviewLastFrame = 0;
  int64_t mOverviewFirstFrame = 0;
  float mOverviewValueScale = 0.f;
  };
