// cTiledMap.h
//{{{  includes
#pragma once
#include <cstdint>
#include <algorithm>
#include <string>
#include <array>
#include <vector>
#include <functional>

#include "oneapi/tbb/concurrent_queue.h"
#include "oneapi/tbb/concurrent_unordered_map.h"
#include "oneapi/tbb/concurrent_unordered_set.h"

// common
#include "../common/basicTypes.h"
#include "../common/cSemaphore.h"
#include "../gui/cTexture.h"
//}}}
constexpr uint32_t kMapMaxZoom = 17;
constexpr uint32_t kMapTileSize = 256;
constexpr uint32_t kMapMaxPngSize = 200000;
constexpr uint32_t kMapLoadThreads = 4;

//{{{
struct cTile {
  cTile (cTexture texture, float scaledX, float scaledY) :
    mTexture(texture), mScaledX(scaledX), mScaledY(scaledY) {}

  void set (cTexture texture, float scaledX, float scaledY) {
    mTexture = texture;
    mScaledX = scaledX;
    mScaledY = scaledY;
    }

  cTexture mTexture;
  float mScaledX;
  float mScaledY;
  };
//}}}
//{{{
struct cRange {

  void update (float x, float y) {
    mMinX = std::min (mMinX, x);
    mMaxX = std::max (mMaxX, x);
    mMinY = std::min (mMinY, y);
    mMaxY = std::max (mMaxY, y);
    }

  float mMinX = 1.f;
  float mMaxX = 0.f;
  float mMinY = 1.f;
  float mMaxY = 0.f;
  };
//}}}
//{{{
class cTileSet {
public:
  cTileSet (uint32_t zoom) : mScale (float(1 << zoom)) {}
  virtual~ cTileSet() = default;

  uint32_t getNumTiles() const { return (uint32_t)mTileMap.size(); }
  uint32_t getNumEmptyTiles() const { return (uint32_t)mEmptyTileSet.size(); }

  float getScale() const { return mScale; }
  tbb::concurrent_unordered_map <std::string, cTile>& getTileMap() { return mTileMap; }
  tbb::concurrent_unordered_set <std::string>& getEmptyTileSet() { return mEmptyTileSet; }

  bool getLoaded (const std::string& quadKey);
  cTexture getTexture (const std::string& quadKey);

  void addTile (const std::string& quadKey, cTexture texture, float scaledX, float scaledY);
  void addEmptyTile (const std::string& quadKey);

  void releaseTextures();
  void dumpEmptyFiles (const std::string& fileName);

private:
  const float mScale;
  tbb::concurrent_unordered_map <std::string, cTile> mTileMap;
  tbb::concurrent_unordered_set <std::string> mEmptyTileSet;
  };
//}}}
//{{{
struct cLayerSpec {
  cLayerSpec (const std::string& name,
              const std::string& host, const std::string& path,
              const std::string& extension, bool save, int minSize,
              uint32_t minZoom, uint32_t maxZoom) :
    mName(name),
    mHost(host), mPath(path),
    mExtension(extension), mSave(save), mMinSize(minSize),
    mMinZoom(minZoom), mMaxZoom(maxZoom) {}

  std::string mName;

  std::string mHost;
  std::string mPath;

  std::string mExtension;
  bool mSave;
  int mMinSize;

  uint32_t mMinZoom;
  uint32_t mMaxZoom;
  };
//}}}
//{{{
class cTiledMapLayer {
public:
  cTiledMapLayer(const cLayerSpec& layerSpec);
  virtual ~cTiledMapLayer() = default; // could delete zoomTileSet

  void addTile (const std::string& quadKey, cRange& tileRange, cTexture texture);
  void addEmptyTile (const std::string& quadKey, cRange& tileRange);
  void dumpEmptyFiles (const std::string& fileRoot);

  // vars
  const cLayerSpec mLayerSpec;

  uint32_t mScale = 1;
  std::array <cTileSet*, kMapMaxZoom+1> mZoomTileSet = { nullptr };
  };
//}}}
//{{{
struct cTiledMapSpec {
  cTiledMapSpec() {}

  cTiledMapSpec (const std::string& name, float minLat, float maxLat, float minLon, float maxLon) :
    mName(name), mMinLat(minLat), mMaxLat(maxLat), mMinLon(minLon), mMaxLon(maxLon) {}

  std::string mName;
  float mMinLat = 0.f;
  float mMaxLat = 0.f;
  float mMinLon = 0.f;
  float mMaxLon = 0.f;
  };
//}}}

class cTiledMap {
public:
  cTiledMap (const std::string& apiKey) : mApiKey(apiKey) {}
  virtual ~cTiledMap() = default; // delete layers ?

  void create (const cTiledMapSpec& mapSpec, cPoint point, uint32_t zoom, const std::string& fileRoot,
               const std::function<void()>& changedCallback,
               const std::function<cTexture(uint32_t, uint32_t, uint8_t*, bool)>& createCallback);

  void addLayer (const cLayerSpec& layerSpec);

  static std::string zoomTileXYtoQuadKey (int zoom, int tileX, int tileY);
  static uint32_t quadKeyToZoomTileXY (const std::string& quadKey, uint32_t& tileX, uint32_t& tileY);

  //{{{  gets
  double getCentreLatitude() { return mCentreLatitude; }
  double getCentreLongitude() { return mCentreLongitude; }
  void getCentrePix (int& xCentrePix, int& yCentrePix);

  uint32_t getZoom() { return mZoom; }
  uint32_t& getLayerIndex() { return mLayerIndex; }
  cTiledMapLayer& getLayer() { return mLayers[getLayerIndex()]; }
  cTileSet* getZoomTileSet (uint32_t zoom) { return getLayer().mZoomTileSet[zoom]; }

  float getMinTileRangeX() { return mTileRange.mMinX; }
  float getMinTileRangeY() { return mTileRange.mMinY; }
  float getRangeX() { return mTileRange.mMaxX - mTileRange.mMinX; }
  float getRangeY() { return mTileRange.mMaxY - mTileRange.mMinY; }

  cTexture getTexture (uint32_t zoom, int tileX, int tileY);

  cPoint getView() { return cPoint (mViewWidth, mViewHeight); }
  int getViewWidth() { return mViewWidth; }
  int getViewHeight() { return mViewHeight; }

  int getNumDownloads() { return mNumDownloads; }
  int getNumEmptyDownloads() { return mNumEmptyDownloads; }
  int getNumAlreadyQueued() { return mNumAlreadyQueued; }

  std::vector<std::string> getLayerNames();

  bool getShowGrid() { return mShowGrid; }
  //}}}
  //{{{  sets
  bool setZoom (uint32_t zoom);
  bool incZoom (int inc);

  void setCentreLatLonFromNormalisedXY (double x, double y, bool flushQueue);
  void incPix (int xIncPix, int yIncPix, bool flushQueue);

  void cycleLayers();
  void cycleGrid();
  //}}}

  void releaseTextures();
  void dumpEmptyFiles();

  void launchThreads (uint32_t numThreads);

  void tilesChanged (bool flushQueue);

private:
  double clip (double n, double minValue, double maxValue);
  void setCentreLatLon (double lat, double lon, bool flushQueue);

  bool queueLoad (int32_t xTile, int32_t yTile);
  uint32_t queueFlush();

  void fileScan();
  void loadTiles (uint32_t threadIndex);

  // private vars
  cTiledMapSpec mMapSpec;
  std::function <void()> mChangedCallback = nullptr;
  std::function <cTexture(uint32_t, uint32_t, uint8_t*, bool)> mCreateCallback = nullptr;

  uint32_t mLayerIndex = 0;
  std::vector <cTiledMapLayer> mLayers;

  uint32_t mZoom = 0;
  double mCentreLatitude = 0.;
  double mCentreLongitude = 0.;
  cRange mTileRange;
  bool mShowGrid = false;

  int mViewWidth = 1920;
  int mViewHeight = 1080;
  std::string mFileRoot;
  std::string mMapFileRoot;
  std::string mApiKey;

  cSemaphore mLoadSem;
  tbb::concurrent_queue <std::string> mLoadQueue;
  uint32_t mNumDownloads = 0;
  uint32_t mNumEmptyDownloads = 0;
  uint32_t mNumAlreadyQueued = 0;
  };
