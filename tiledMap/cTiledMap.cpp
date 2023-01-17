// cTiledMap.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <algorithm>
#include <string>
#include <array>
#include <vector>
#include <mutex>
#include <thread>
#include <filesystem>

#include "../common/date.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../net/cHttp.h"

#include "cTiledMap.h"

using namespace std;
using namespace chrono;
//}}}

//{{{  cTileSet
//{{{
bool cTileSet::getLoaded (const string& quadKey) {
// return true if file already loaded

  // if emptyTile don't need load
  if (mEmptyTileSet.find (quadKey) != mEmptyTileSet.end())
    return true;

  // return true if found and texture loaded
  auto it = mTileMap.find (quadKey);
  return (it != mTileMap.end()) && !it->second.mTexture.empty();
  }
//}}}
//{{{
cTexture cTileSet::getTexture (const string& quadKey) {

  auto it = mTileMap.find (quadKey);
  return (it == mTileMap.end()) ? cTexture() : it->second.mTexture;
  }
//}}}

//{{{
void cTileSet::addTile (const string& quadKey, cTexture texture, float scaledX, float scaledY) {

  auto it = mTileMap.find (quadKey);
  if (it == mTileMap.end())
    // insert a new tile with texture
    mTileMap.insert (tbb::concurrent_unordered_map <string, cTile>::value_type (quadKey, cTile(texture, scaledX, scaledY)));
  else {
    if (it->second.mTexture.empty())
      // add texture to existing tile with empty texture
      it->second.set (texture, scaledX, scaledY);
    else {
      // !!! not sure how this happens, release the texture resources !!!
      //cLog::log (LOGERROR, fmt::format ("{} cTileSet::addTile already had texture", quadKey));
      texture.release();
      }
    }
  }
//}}}
//{{{
void cTileSet::addEmptyTile (const string& quadKey) {
  mEmptyTileSet.insert (quadKey);
  }
//}}}

//{{{
void cTileSet::releaseTextures() {
// release all textures

  for (auto& tile : mTileMap)
    tile.second.mTexture.release();
  }
//}}}
//{{{
void cTileSet::dumpEmptyFiles (const string& fileName) {

  FILE* writeFile = fopen (fileName.c_str(), "w");

  for (string quadKeyString : mEmptyTileSet) {
    string str = quadKeyString + "\n";
    fwrite (str.c_str(), 1, str.size(), writeFile);
    }

  fclose (writeFile);
  }
//}}}
//}}}
//{{{  cTileMapLayer
//{{{
cTiledMapLayer::cTiledMapLayer (const cLayerSpec& layerSpec) : mLayerSpec(layerSpec) {

  // !!! not sure why tileSet is new'ed - was it faster ? !!!
  for (uint32_t zoom = mLayerSpec.mMinZoom; zoom <= mLayerSpec.mMaxZoom; zoom++)
    mZoomTileSet[zoom] = new cTileSet (zoom);
  }
//}}}

//{{{
void cTiledMapLayer::addTile (const string& quadKey, cRange& tileRange, cTexture texture) {

  uint32_t xTile;
  uint32_t yTile;
  uint32_t zoom = cTiledMap::quadKeyToZoomTileXY (quadKey, xTile, yTile);

  cTileSet* tileSet = mZoomTileSet[zoom];
  float scaledX = xTile / tileSet->getScale();
  float scaledY = yTile / tileSet->getScale();
  tileRange.update (scaledX, scaledY);

  tileSet->addTile (quadKey, texture, scaledX, scaledY);
  }
//}}}
//{{{
void cTiledMapLayer::addEmptyTile (const string& quadKey, cRange& tileRange) {

  uint32_t xTile;
  uint32_t yTile;
  uint32_t zoom = cTiledMap::quadKeyToZoomTileXY (quadKey, xTile, yTile);

  cTileSet* tileSet = mZoomTileSet[zoom];
  float scaledX = xTile / tileSet->getScale();
  float scaledY = yTile / tileSet->getScale();
  tileRange.update (scaledX, scaledY);

  tileSet->addEmptyTile (quadKey);
  }
//}}}
//{{{
void cTiledMapLayer::dumpEmptyFiles (const string& fileRoot) {

  for (uint32_t zoom = mLayerSpec.mMinZoom; zoom <= mLayerSpec.mMaxZoom; zoom++)
    mZoomTileSet[zoom]->dumpEmptyFiles (fmt::format ("{}/{}/{}/empty.txt", fileRoot, mLayerSpec.mName, zoom));
  }
//}}}
//}}}

// cTiledMap statics
//{{{
string cTiledMap::zoomTileXYtoQuadKey (int zoom, int tileX, int tileY) {
// convert tileXY at mZoom to quadKey string

  string quadKey;

  for (int i = zoom; i > 0; i--) {
    char digit = '0';
    int mask = 1 << (i - 1);
    if ((tileX & mask) != 0)
      digit++;
    if ((tileY & mask) != 0)
       digit += 2;

    quadKey += digit;
    }

  return quadKey;
  }
//}}}
//{{{
uint32_t cTiledMap::quadKeyToZoomTileXY (const string& quadKey, uint32_t& tileX, uint32_t& tileY) {
// get zoom,tileXY from quadkey string

  tileX = 0;
  tileY = 0;
  uint32_t zoom = (uint32_t)quadKey.size();

  for (uint32_t i = zoom; i > 0; i--) {
    uint32_t mask = 1 << (i - 1);
    switch (quadKey[zoom - i]) {
      case '0':
        break;

      case '1':
        tileX |= mask;
        break;

      case '2':
        tileY |= mask;
        break;

      case '3':
        tileX |= mask;
        tileY |= mask;
        break;
      }
    }

  return zoom;
  }
//}}}

// cTiledMap
//{{{
void cTiledMap::create (const cTiledMapSpec& mapSpec, cPoint point, uint32_t zoom, const string& fileRoot,
                   const std::function<void()>& changedCallback,
                   const std::function<cTexture(uint32_t, uint32_t, uint8_t*, bool)>& createCallback) {

  mChangedCallback = changedCallback;
  mCreateCallback = createCallback;

  mMapSpec = mapSpec;

  mCentreLatitude = point.x;
  mCentreLongitude = point.y;
  mZoom = zoom;

  mFileRoot = fileRoot;
  mMapFileRoot = fmt::format ("{}/{}", mFileRoot, mMapSpec.mName);
  }
//}}}
//{{{
void cTiledMap::addLayer (const cLayerSpec& layerSpec) {

  mLayers.push_back (cTiledMapLayer (layerSpec));

  // create layer mapTile directories
  if (layerSpec.mSave)  {
    for (uint32_t zoom = layerSpec.mMinZoom; zoom <= layerSpec.mMaxZoom; zoom++) {
      string mapTileDirectory = fmt::format ("{}/{}/{}", mMapFileRoot, layerSpec.mName, zoom);
      if (filesystem::create_directories (mapTileDirectory))
        cLog::log (LOGINFO, fmt::format ("creating mapTile directory {}", mapTileDirectory));
      else
        cLog::log (LOGINFO, fmt::format ("using existing mapTile directory {}", mapTileDirectory));
      }
    }
  }
//}}}

// gets
//{{{
void cTiledMap::getCentrePix (int& xCentrePix, int& yCentrePix) {

  double x = (mCentreLongitude + 180.) / 360.;
  double sinLatitude = sin(mCentreLatitude * M_PI / 180.);
  double y = 0.5 - log((1. + sinLatitude) / (1. - sinLatitude)) / (4. * M_PI);

  int mapSize = (int)kMapTileSize << mZoom;
  xCentrePix = (int)clip (x * mapSize + 0.5, 0, mapSize - 1);
  yCentrePix = (int)clip (y * mapSize + 0.5, 0, mapSize - 1);
  }
//}}}
//{{{
cTexture cTiledMap::getTexture (uint32_t zoom, int tileX, int tileY) {
// get texture from zoom,tileXY

  string quadKey = zoomTileXYtoQuadKey (zoom, tileX, tileY);
  cTileSet* tileSet = getLayer().mZoomTileSet[zoom];
  return tileSet->getTexture (quadKey);
  }
//}}}
//{{{
vector<string> cTiledMap::getLayerNames() {

  vector<string> result;
  for (auto& layer : mLayers)
    result.push_back (layer.mLayerSpec.mName);

  return result;
  }
//}}}

// sets
//{{{
bool cTiledMap::setZoom (uint32_t zoom) {

  uint32_t prevZoom = mZoom;
  mZoom = max (min (zoom, getLayer().mLayerSpec.mMaxZoom), getLayer().mLayerSpec.mMinZoom);
  if (mZoom == prevZoom)
    return false;

  // changed
  getLayer().mScale = zoom < getLayer().mLayerSpec.mMinZoom ? (1 << (getLayer().mLayerSpec.mMinZoom - zoom)) : 1;
  tilesChanged (true);

  return true;
  }
//}}}
//{{{
bool cTiledMap::incZoom (int inc) {

  return setZoom (mZoom + inc);
  }
//}}}

//{{{
void cTiledMap::setCentreLatLonFromNormalisedXY (double x, double y, bool flushQueue) {
  setCentreLatLon (90. - (360. * atan (exp ((y - 0.5) * 2*M_PI)) / M_PI), 360. * (x - 0.5), flushQueue);
  }
//}}}
//{{{
void cTiledMap::incPix (int xIncPix, int yIncPix, bool flushQueue) {
// must be simpler way to do this, pix inc as LatLon inc

  // latLon to pix
  int xCentrePix;
  int yCentrePix;
  getCentrePix (xCentrePix, yCentrePix);

  // inc in pix
  xCentrePix += xIncPix;
  yCentrePix += yIncPix;

  // pix back to latLon
  int mapSize = (int)kMapTileSize << mZoom;
  double x = (clip (xCentrePix, 0, mapSize - 1) / mapSize) - 0.5;
  double y = 0.5 - (clip (yCentrePix, 0, mapSize - 1) / mapSize);

  double lat = 90. - 360. * atan (exp (-y * 2. * M_PI)) / M_PI;
  double lon = 360. * x;
  setCentreLatLon (lat, lon, flushQueue);
  }
//}}}

//{{{
void cTiledMap::cycleLayers() {
  mLayerIndex = (mLayerIndex + 1) % mLayers.size();
  tilesChanged (true);
  }
//}}}
//{{{
void cTiledMap::cycleGrid() {
  mShowGrid = !mShowGrid;
  tilesChanged (true);
  }
//}}}

//{{{
void cTiledMap::releaseTextures() {

  for (auto& layer : mLayers)
    for (uint32_t zoom = layer.mLayerSpec.mMinZoom; zoom <= layer.mLayerSpec.mMaxZoom; zoom++)
      layer.mZoomTileSet[zoom]->releaseTextures();

  tilesChanged (true);
  }
//}}}
//{{{
void cTiledMap::dumpEmptyFiles() {
  for (auto& layer : mLayers)
    layer.dumpEmptyFiles (mMapFileRoot);
  }
//}}}

//{{{
void cTiledMap::launchThreads (uint32_t numThreads) {

  // fileScan thread
  thread ([=, this]() {
    cLog::setThreadName ("file");
    fileScan();
    cLog::log (LOGNOTICE, "exit");
    }).detach();

  // loadTiles threads
  for (uint32_t threadIndex = 0; threadIndex < numThreads; threadIndex++)
    thread ([=, this]() {
      cLog::setThreadName (fmt::format ("ld:{}", threadIndex));
      loadTiles (threadIndex);
      cLog::log (LOGERROR, "exit");
      }).detach();
  };
//}}}

//{{{
void cTiledMap::tilesChanged (bool flushQueue) {

  uint32_t flushCount = flushQueue ? queueFlush() : 0;

  int32_t xCentrePix;
  int32_t yCentrePix;
  getCentrePix (xCentrePix, yCentrePix);

  uint32_t xTile = xCentrePix / kMapTileSize;
  uint32_t yTile = yCentrePix / kMapTileSize;

  // load window, cheap and cheerful load order, seems a bit crude
  uint32_t queueCount = 0;
  int32_t xWindow = int32_t(ceil(getViewWidth() / (float)kMapTileSize / 2.f)) + 1;
  int32_t yWindow = int32_t(ceil(getViewHeight() / (float)kMapTileSize / 2.f)) + 1;
  int32_t window = xWindow > yWindow ? xWindow : yWindow;
  for (int32_t w = 0; w <= window; w++)
    for (int32_t y = -w; y <= w; y++)
        for (int x = -w; x <= w; x++)
          if (((abs(x) == w) || (abs(y) == w)) &&              // outside edge
              ((abs(x) <= xWindow) && (abs(y) <= yWindow)) &&  // limited to x and y window
              ((abs(x) != xWindow || (abs(y) != yWindow))))    // miss visible +1 corners
            if (queueLoad (xTile+x, yTile+y))
              queueCount++;

  if (flushCount || queueCount)
    cLog::log (LOGINFO, fmt::format ("tilesChanged {}x{} {} flushed {} queued",
                                     xWindow, yWindow, flushCount, queueCount));
  if (queueCount)
    mLoadSem.notifyAll();
  if (mChangedCallback)
    mChangedCallback();
  }
//}}}

// private
//{{{
double cTiledMap::clip (double n, double minValue, double maxValue) {
  return min (max (n, minValue), maxValue);
  }
//}}}
//{{{
void cTiledMap::setCentreLatLon (double lat, double lon, bool flushQueue) {

  mCentreLatitude = max (min ((float)lat, mMapSpec.mMaxLat), mMapSpec.mMinLat);
  mCentreLongitude = max (min ((float)lon, mMapSpec.mMaxLon), mMapSpec.mMinLon);
  mCentreLatitude = max (min ((float)lat, mMapSpec.mMaxLat), mMapSpec.mMinLat);
  mCentreLongitude = max (min ((float)lon, mMapSpec.mMaxLon), mMapSpec.mMinLon);

  tilesChanged (flushQueue);
  }
//}}}

//{{{
bool cTiledMap::queueLoad (int xTile, int yTile) {

  string quadKey = zoomTileXYtoQuadKey (mZoom, xTile, yTile);
  if (getZoomTileSet (mZoom)->getLoaded (quadKey))
    return false;

  // !!!! is this ok, doesn't have to be 100% accurate !!!
  for (tbb::concurrent_queue<string>::const_iterator i(mLoadQueue.unsafe_begin()); i != mLoadQueue.unsafe_end(); ++i) {
    if (*i == quadKey) {
      //cLog::log (LOGINFO, fmt::format ("{} already queued", *i));
      mNumAlreadyQueued++;
      return false;
      }
    }

  // load it
  mLoadQueue.push (quadKey);
  return true;
  }
//}}}
//{{{
uint32_t cTiledMap::queueFlush() {

  uint32_t flushCount = 0;

  string quadKey;
  while (mLoadQueue.try_pop (quadKey))
    flushCount++;

  return flushCount;
  }
//}}}

//{{{
void cTiledMap::fileScan() {

  for (auto& layer : mLayers) {
    int numFiles = 0;
    for (uint32_t zoom = layer.mLayerSpec.mMinZoom; zoom <= layer.mLayerSpec.mMaxZoom; zoom++) {
      string searchStr = fmt::format ("{}/{}/{}/", mMapFileRoot, layer.mLayerSpec.mName, zoom);

      string path = searchStr;
      filesystem::path srcPath = path;
      if (filesystem::is_directory (srcPath)) {
        int numLayerFiles = 0;
        for (const filesystem::directory_entry& entry : filesystem::directory_iterator (srcPath)) {
          if (entry.path().extension().string() == layer.mLayerSpec.mExtension) {
            // add unloaded tile
            layer.addTile (entry.path().stem().string(), mTileRange, cTexture());
            numLayerFiles++;
            }
          }

        tilesChanged (true);
        cLog::log (LOGINFO, fmt::format ("{} has {} tiles", searchStr, numLayerFiles));
        numFiles += numLayerFiles;
        }
      }

    // !!! put this in the normal scan above !!!
    int numEmptyTiles = 0;
    for (uint32_t zoom = layer.mLayerSpec.mMinZoom; zoom <= layer.mLayerSpec.mMaxZoom; zoom++) {
      string fileName = fmt::format ("{}/{}/{}/empty.txt", mMapFileRoot, layer.mLayerSpec.mName, zoom);
      FILE* readFile = fopen (fileName.c_str(), "r");
      if (readFile) {
        string quadKey;
        while (true) {
          char ch;
          size_t numRead = fread (&ch, 1, 1, readFile);
          if (!numRead)
            break;
          if (ch == '\n') {
            numEmptyTiles++;
            layer.addEmptyTile (quadKey, mTileRange);
            quadKey = "";
            }
          else
            quadKey += ch;
          }
        fclose (readFile);
        }
      }
    cLog::log (LOGINFO, fmt::format ("{} tiles:{} empty:{}", layer.mLayerSpec.mName, numFiles, numEmptyTiles));
    }
  }
//}}}
//{{{
void cTiledMap::loadTiles (uint32_t threadIndex) {

  // !!! should assert maxSize !!!!
  uint8_t* fileBuf = new uint8_t [kMapMaxPngSize];

  cHttp http;
  http.initialise();

  while (true) {
    string quadKey;
    if (mLoadQueue.try_pop (quadKey)) {
      cTiledMapLayer& layer = getLayer();
      string fileName = fmt::format ("{}/{}/{}/{}{}",
                                     mMapFileRoot, layer.mLayerSpec.mName, quadKey.size(), quadKey, layer.mLayerSpec.mExtension);

      auto startTime = system_clock::now();
      FILE* file = fopen (fileName.c_str(), "rb");
      auto findTime = system_clock::now();
      int64_t findUs = duration_cast<microseconds>(findTime - startTime).count();

      if (file) {
        //{{{  file found, loadDecode to texture
        uint32_t fileBufLen = (uint32_t)fread (fileBuf, 1, 200000, file);
        fclose (file);
        auto loadTime = system_clock::now();

        cTexture texture = cTexture::createDecode (fileBuf, fileBufLen);
        if (!texture.empty()) {
          int64_t loadUs = duration_cast<microseconds>(loadTime - findTime).count();
          int64_t decodeUs = duration_cast<microseconds>(system_clock::now() - loadTime).count();
          cLog::log (LOGINFO, fmt::format ("{} size:{:6} find:{:4}us load:{:4}us decode:{:4}us",
                                           quadKey, fileBufLen, findUs, loadUs, decodeUs));

          // add loaded tile
          layer.addTile (quadKey, mTileRange, texture);
          if (mChangedCallback)
            mChangedCallback();
          }
        else
          cLog::log (LOGERROR, fmt::format ("{} size: {:6} load failed", quadKey, fileBufLen));
        }
        //}}}
      else {
        //{{{  file notFound, download, save, decode to texture
        string hostName = fmt::vformat (layer.mLayerSpec.mHost, fmt::make_format_args (threadIndex));
        string pathName = fmt::vformat (layer.mLayerSpec.mPath, fmt::make_format_args (quadKey, mApiKey));

        int response = http.get (hostName, pathName);
        if (response == 200) {
          //  download ok
          auto downTime = system_clock::now();
          int64_t downUs = duration_cast<milliseconds>(downTime - findTime).count();

          if (layer.mLayerSpec.mSave && http.getContentSize() < layer.mLayerSpec.mMinSize) {
            // too small to save, add to emptyTile set
            int64_t loadTime = duration_cast<microseconds>(system_clock::now() - startTime).count();
            cLog::log (LOGINFO2, fmt::format ("{} empty  {}us", quadKey, loadTime));

            mNumEmptyDownloads++;
            layer.addEmptyTile (quadKey, mTileRange);
            }

          else {
            mNumDownloads++;

            uint8_t* httpPngBuf = http.getContent();
            uint32_t httpPngBufLen = http.getContentSize();

            cTexture texture = cTexture::createDecode (httpPngBuf, httpPngBufLen);
            if (!texture.empty()) {
              auto decodeTime = system_clock::now();
              int64_t decodeUs = duration_cast<microseconds>(decodeTime - downTime).count();

              // decode ok
              if (layer.mLayerSpec.mSave) {
                //{{{  loaded and save
                FILE* writeFile = fopen (fileName.c_str(), "wb");
                if (writeFile) {
                  fwrite (httpPngBuf, 1, httpPngBufLen, writeFile);
                  fclose (writeFile);

                  auto writeTime = system_clock::now();
                  int64_t writeUs = duration_cast<microseconds>(writeTime - decodeTime).count();
                  cLog::log (LOGINFO, fmt::format ("{} size:{:6} find:{:4}us down:{:4}ms decode:{:4}us write:{:5}us",
                                                   quadKey, httpPngBufLen, findUs, downUs, decodeUs, writeUs));
                  // add loaded and saved tile
                  layer.addTile (quadKey, mTileRange, texture);
                  }
                else
                  cLog::log (LOGERROR, fmt::format ("download - failed to save {}", quadKey));
                }
                //}}}
              else {
                //{{{  loaded but don't save
                cLog::log (LOGINFO, fmt::format ("{} size:{:6} find:{:4}us down:{:4}ms decode:{:4}us",
                                                 quadKey, httpPngBufLen, findUs, downUs, decodeUs));
                // add loaded and unsaved tile
                layer.addTile (quadKey, mTileRange, texture);
                }
                //}}}
              }
            else
              cLog::log (LOGERROR, fmt::format ("{} size:{:6} download decode failed", quadKey, httpPngBufLen));

            if (mChangedCallback)
              mChangedCallback();
            }
          }
        }
        //}}}
      }
    else
      mLoadSem.wait();
    }

  delete[] fileBuf;
  }
//}}}
