// cTiledMapBox.cpp
//{{{  includes
#include <cstdint>
#include <string>
#include <array>
#include <vector>

#include "../gui/cWindow.h"

#include "../tiledMap/cTiledMap.h"
#include "cTiledMapBox.h"

using namespace std;
//}}}

//{{{
cTiledMapBox::cTiledMapBox (cWindow& window, float width, float height, cTiledMap& tiledMap)
    : cBox ("tiledMap", window, width, height), mTiledMap(tiledMap) {

  mPin = true;
  }
//}}}

//{{{
bool cTiledMapBox::move (bool right, cPoint pos, cPoint inc, int pressure, int timestamp) {

  (void)right;
  (void)pos;
  (void)pressure;
  (void)timestamp;

  mTiledMap.incPix (int(-inc.x), int(-inc.y), mWindow.getShift());
  return true;
  }
//}}}
//{{{
bool cTiledMapBox::wheel (int delta, cPoint pos)  {

  (void)pos;
  return mTiledMap.incZoom (delta);
  }
//}}}

void cTiledMapBox::draw() {

  int32_t xCentrePix;
  int32_t yCentrePix;
  mTiledMap.getCentrePix (xCentrePix, yCentrePix);

  int xTopLeftPix = xCentrePix - int32_t(getWidth() * 0.5f);
  int yTopLeftPix = yCentrePix - int32_t(getHeight() * 0.5f);

  float xSubTile = - float(xTopLeftPix % kMapTileSize);
  float ySubTile = - float(yTopLeftPix % kMapTileSize);
  cRect dstRect {xSubTile, ySubTile, xSubTile + kMapTileSize, ySubTile + kMapTileSize};

  uint32_t xFirstTile = xTopLeftPix / kMapTileSize;
  uint32_t yFirstTile = yTopLeftPix / kMapTileSize;

  uint32_t yTile = yFirstTile;
  while (dstRect.top < getBottom()) {
    dstRect.left = xSubTile;
    dstRect.right = dstRect.left + kMapTileSize;
    uint32_t xTile = xFirstTile;
    while (dstRect.left < getRight()) {
      // find,draw tileX,tileY
      blit (mTiledMap.getTexture (mTiledMap.getZoom(), xTile, yTile), cRect (getTL() + dstRect.getTL(), getBR()));
      if (mTiledMap.getShowGrid())
        drawBorder (kGreen, dstRect + getTL());
      dstRect.addHorizontal (kMapTileSize);
      xTile++;
      }

    dstRect.addVertical (kMapTileSize);
    yTile++;
    }

  drawTextShadow (kWhite,
                  {getTL(), getTL() + cPoint(400.f, getBoxHeight())},
                  fmt::format ("lat:{:6.4} lon:{:6.4} zoom:{} d:{} e:{} a:{} ",
                               mTiledMap.getCentreLatitude(), mTiledMap.getCentreLongitude(), mTiledMap.getZoom(),
                               mTiledMap.getNumDownloads(), mTiledMap.getNumEmptyDownloads(), mTiledMap.getNumAlreadyQueued()));
  }
