// cTiledMapOvervewBox.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <string>
#include <array>
#include <vector>

//#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX
//#include <windows.h>

#include "../gui/cWindow.h"

#include "../tiledMap/cTiledMap.h"
#include "cTiledMapOverviewBox.h"

using namespace std;
//}}}

// public
//{{{
cTiledMapOverviewBox::cTiledMapOverviewBox (cWindow& window, float width, float height, cTiledMap& map)
  : cBox("tiledMapOverview", window, width, height), mMap(map) {}
//}}}

//{{{
bool cTiledMapOverviewBox::down (bool right, cPoint pos)  {

  (void)right;

  setLatLon (pos);
  return true;
  }
//}}}
//{{{
bool cTiledMapOverviewBox::move (bool right, cPoint pos, cPoint inc) {

  (void)right;
  (void)inc;

  setLatLon (pos);
  return true;
  }
//}}}
//{{{
bool cTiledMapOverviewBox::wheel (int delta, cPoint pos)  {

  (void)pos;

  mMap.incZoom (delta);
  return true;
  }
//}}}

//{{{
void cTiledMapOverviewBox::draw() {
// cTiledMapView dimensions in TileXY cordinates, how do we get them here

  auto viewSize = mMap.getView();

  drawRectangle (kClearBgnd);

  cTileSet* tileSet = mMap.getZoomTileSet (mMap.getZoom());
  float xScale = getWidth() / mMap.getRangeX();
  float yScale = getHeight() / mMap.getRangeY();
  float scale = xScale > yScale ? yScale : xScale;

  float dotWidth = scale / tileSet->getScale();
  cPoint dotSize {dotWidth < 1.f ? 1.f : dotWidth + 1.f};

  // draw tiles
  for (auto& tile : tileSet->getTileMap()) {
    cPoint dot = getTL() + cPoint (tile.second.mScaledX - mMap.getMinTileRangeX(),
                                   tile.second.mScaledY - mMap.getMinTileRangeY()) * scale;
    drawRectangle (tile.second.mTexture.empty() ? kGreen : kDarkGreen, {dot, dot+dotSize});
    }

  // draw emptyTiles
  for (string tile : tileSet->getEmptyTileSet()) {
    uint32_t xTile;
    uint32_t yTile;
    cTiledMap::quadKeyToZoomTileXY (tile, xTile, yTile);
    cPoint dot = getTL() + cPoint (xTile / tileSet->getScale() - mMap.getMinTileRangeX(),
                                   yTile / tileSet->getScale() - mMap.getMinTileRangeY()) * scale;
    drawRectangle (kLightBlue, {dot, dot+dotSize});
    }

  // draw centreGraphic
  int xCentrePix;
  int yCentrePix;
  mMap.getCentrePix (xCentrePix, yCentrePix);

  cPoint dot = getTL() + cPoint ((xCentrePix / kMapTileSize / tileSet->getScale()) - mMap.getMinTileRangeX(),
                                 (yCentrePix / kMapTileSize / tileSet->getScale()) - mMap.getMinTileRangeY()) * scale;
  cPoint xySize = (viewSize / (float)kMapTileSize / tileSet->getScale()) * scale * 0.5f;
  drawRectangle (kWhite, {dot - xySize, dot + xySize});

  // draw text
  uint32_t numTilesX = uint32_t(mMap.getRangeX() * tileSet->getScale());
  uint32_t numTilesY = uint32_t(mMap.getRangeY() * tileSet->getScale());
  drawText (kWhite, {getTL(), getTL() + cPoint(240.f, getBoxHeight())},
            fmt::format ("{} {}x{}", tileSet->getNumTiles(), numTilesX, numTilesY));

  //drawBorder (kWhite, 1);
  }
//}}}

// private
//{{{
void cTiledMapOverviewBox::setLatLon (cPoint pos) {

  float xScale = getWidth() / mMap.getRangeX();
  float yScale = getHeight() / mMap.getRangeY();
  float scale = xScale > yScale ? yScale : xScale;

  mMap.setCentreLatLonFromNormalisedXY ((pos.x / scale) + mMap.getMinTileRangeX(),
                                        (pos.y / scale) + mMap.getMinTileRangeY(), !mWindow.getShift());
  }
//}}}
