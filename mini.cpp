// mini.cpp
//{{{  includes
#ifdef WINMAIN_ENTRY
  #define NOMINMAX
  #include <windows.h>
#endif

#include "fmt/format.h"
#include "common/basicTypes.h"
#include "common/cLog.h"
#include "common/utils.h"

#include "gui/cWindow.h"
#include "gui/cWindowBox.h"
#include "gui/basicBoxes.h"
#include "gui/cIndexBox.h"
#include "gui/cLogBox.h"
#include "gui/cClockBox.h"
#include "gui/cCalenderBox.h"

#include "paint/cPaintBox.h"
#include "paint/cPaint.h"

#include "song/cSongLoaderBox.h"

#include "tiledMap/cTiledMap.h"
#include "tiledMap/cTiledMapBox.h"
#include "tiledMap/cTiledMapOverviewBox.h"

using namespace std;
using namespace chrono;
//}}}
constexpr unsigned kWidth = 1920;
constexpr unsigned kHeight = 1080-80;
constexpr unsigned kWidthWindow = kWidth*2/3;
constexpr unsigned kHeightWindow = kHeight*2/3;

class cMiniWindow : public cWindow {
public:
  //{{{
  void run (const string& title, const string& fileRoot, const string& tiledMapApiKey,
            milliseconds tickMs, bool fullScreen) {

    if (!createWindow (title, fullScreen ? kWidth : kWidthWindow, fullScreen ? kHeight: kHeightWindow,
                       tickMs, fullScreen)) {
      //{{{  error, return
      cLog::log (LOGERROR, fmt::format ("failed to open miniFB window {}x{}", kWidth, kHeight));
      return;
      }
      //}}}
    cLog::log (LOGINFO, fmt::format ("opened window {}x{}", kWidthWindow, kHeightWindow));

    //{{{  create paint, gui
    mPaint = new cPaint (*this);

    mPaint->addLayer (new cTextureLayer ("burger", cTexture::createLoad(fileRoot),
                                         {getWidth()/2.f, getHeight()/2.f}, 0.5f));
    mPaint->addLayer (new cRectangleLayer ("text background", kDarkBlue, {100.f, 100.f}, {100.f,100.f}));
    mPaint->addLayer (new cTextLayer ("text line 1", kWhite, {150.f, 150.f}, "line 1 of text"));
    mPaint->addLayer (new cTextLayer ("text line 2", kOrange, {150.f, 170.f}, "some more text"));
    mPaint->addLayer (new cTextLayer ("text line 3", kGreenYellow, {150.f, 190.f}, "last line of text"));
    mPaint->addLayer (new cEllipseLayer ("solid circle", kGreen, {200.f, 200.f}, 100.f));
    mPaint->addLayer (new cEllipseLayer ("outline circle", kYellow, {300.f, 300.f}, 100.f, 4.f));

    addBackground (new cPaintBox (*this, 0,0, *mPaint));
    //}}}

    if (!tiledMapApiKey.empty()) {
      //{{{  create tiledMap, layers, threads, gui
      #ifdef _WIN32
        string mapFileRoot = "C:/maps";
      #else
        string mapFileRoot = "/home/pi/maps";
      #endif

      mTiledMap = new cTiledMap (tiledMapApiKey);

      mTiledMap->create ({"uk", 50.10319,60.15456, -7.64133,1.75159}, {50.3444f,-5.1544f}, 14, mapFileRoot,
                         [&]() { changed(); },
                         [&](uint32_t width, uint32_t height, uint8_t* pixels, bool free) -> cTexture {
                         return create (width, height, pixels, free); } );
      mTiledMap->addLayer ({"os",
                            "ecn.t{}.tiles.virtualearth.net", "tiles/r{}?g=5938&lbl=l1&productSet=mmOS&key={}", ".png",
                            true,4000, 12,15});
      mTiledMap->addLayer ({"aerial",
                            "ecn.t{}.tiles.virtualearth.net", "tiles/a{}?g=5939&key={}", ".jpg",
                            true,2048, 10,16}); // 3,18
      mTiledMap->addLayer ({"road",
                            "ecn.t{}.tiles.virtualearth.net", "tiles/r{}?g=5939&mkt=en-GB&shading=hill&key={}", ".png",
                            false,0, 9,17});
      addBelow (new cIndexBox (*this, 5, mTiledMap->getLayerNames(), mTiledMap->getLayerIndex(),
                               [&](uint32_t index) { (void)index; mTiledMap->tilesChanged (true); }));

      mTiledMap->launchThreads (kMapLoadThreads);

      add (new cTiledMapBox (*this, getWidthInBoxes()/2,getHeightInBoxes()-8, *mTiledMap), -getWidthInBoxes()/2,-getHeightInBoxes()+8);
      add (new cTiledMapOverviewBox (*this, 6, 8, *mTiledMap), -6, -8);
      }
      //}}}

    //{{{  create radio songLoader, gui
    function <void (int64_t)> playCallback = [&](int64_t pts) {(void)pts; changed();};

    const vector<string> kRadio3 = {"r3", "a320"};
    mRadioBoxes.push_back (add (new cTextBgndBox (*this, 6,1, "radio3", [&]() {
        //{{{  lambda
        if (mSongLoader) {
          removeBox (mSongLoaderBox);
          delete mSongLoaderBox;
          mSongLoaderBox = nullptr;

          mSongLoader->exit();
          delete mSongLoader;
          mSongLoader = nullptr;
          }

        mSongLoader = new cSongLoader();
        mSongLoader->launchLoad (kRadio3, playCallback);
        mSongLoaderBox = new cSongLoaderBox (*this, getWidthInBoxes()/2,11, *mSongLoader);
        add (mSongLoaderBox, 0,-12);
        for (auto box : mRadioBoxes) box->toTop();
        }
        //}}}
      ), 0,-5));

    const vector<string> kRadio4 = { "r4", "a128" };
    mRadioBoxes.push_back (addBelow (new cTextBgndBox (*this, 6,1, "radio4", [&]() {
      //{{{  lambda
      if (mSongLoader) {
        removeBox(mSongLoaderBox);
        delete mSongLoaderBox;
        mSongLoaderBox = nullptr;

        mSongLoader->exit();
        delete mSongLoader;
        mSongLoader = nullptr;
        }

      mSongLoader = new cSongLoader();
      mSongLoader->launchLoad (kRadio4,playCallback);
      mSongLoaderBox = new cSongLoaderBox (*this, getWidthInBoxes()/2,11, *mSongLoader);
      add (mSongLoaderBox, 0,-12);
      for (auto box : mRadioBoxes) box->toTop();
      } )));
      //}}}

    const vector<string> kRadio6 = { "r6", "a128" };
    mRadioBoxes.push_back (addBelow (new cTextBgndBox (*this, 6,1, "radio6", [&]() {
      //{{{  lambda
      if (mSongLoader) {
        removeBox (mSongLoaderBox);
        delete mSongLoaderBox;
        mSongLoaderBox = nullptr;

        mSongLoader->exit();
        delete mSongLoader;
        mSongLoader = nullptr;
        }

      mSongLoader = new cSongLoader();
      mSongLoader->launchLoad (kRadio6,playCallback);
      mSongLoaderBox = new cSongLoaderBox (*this, getWidthInBoxes()/2,11, *mSongLoader);
      add (mSongLoaderBox, 0,-12);
      for (auto box : mRadioBoxes) box->toTop();
      } )));
      //}}}
    //}}}

    // create clock, calendar boxes
    add (new cClockBox (*this, 6), -7, 1);
    cBox* box = new cCalendarBox (*this);
    add (box, -box->getWidthInBoxes()-8,0);

    #ifdef WINMAIN_ENTRY
      add (new cLogBox (*this));
    #endif
    add (new cWindowBox (*this, 3,1), -3,0);

    uiLoop (false, true, kBlack, kWhite);
    }
  //}}}
protected:
  //{{{
  bool keyDown (int key) {
    switch (key) {
      case KB_KEY_ESCAPE:
        setExit();
        return true;

      case ' ':
        return true;

      default:
        cLog::log (LOGINFO, fmt::format ("key {}", key));
      }

    return false;
    }
  //}}}
private:
  cPaint* mPaint = nullptr;
  cTiledMap* mTiledMap = nullptr;

  vector <cBox*> mRadioBoxes;
  cSongLoader* mSongLoader = nullptr;
  cSongLoaderBox* mSongLoaderBox = nullptr;
  };

// main
#ifdef WINMAIN_ENTRY
  //{{{
  int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    int numArgs;
    auto args = CommandLineToArgvW (GetCommandLineW(), &numArgs);

    vector <string> params;
    for (int i = 1; i < numArgs; i++)
      params.push_back (utils::wstringToString (args[i]));
  //}}}
#else
  //{{{
  int main (int numArgs, char* args[]) {

    vector <string> params;

    // comand line args to params
    for (int i = 1; i < numArgs; i++)
      params.push_back (args[i]);
  //}}}
#endif
  // command line options
  eLogLevel logLevel = LOGINFO;
  bool fullScreen = false;
  #ifdef _WIN32
    string fileRoot = "../../piccies/burger.jpg"; // launched in windowsBuild/release/
  #else
    string fileRoot = "../piccies/burger.jpg";    // launched in build/
  #endif
  string tiledMapApiKey;
  //{{{  parse params to command line options
  for (auto it = params.begin(); it < params.end();) {
    if (*it == "log1") { logLevel = LOGINFO1; ++it; }
    else if (*it == "log2") { logLevel = LOGINFO2; ++it; }
    else if (*it == "log3") { logLevel = LOGINFO3; ++it; }
    else if (*it == "full") { fullScreen = true; ++it; }
    else if (*it == "map") { ++it; tiledMapApiKey = *it; ++it; }
    else { fileRoot = *it; ++it; }
    };
  //}}}

  #ifdef WINMAIN_ENTRY
    cLog::init (logLevel, true); // logBox dialog
  #else
    cLog::init (logLevel, false);
  #endif
  cLog::log (LOGNOTICE, fmt::format ("mini"));

  cMiniWindow window;
  window.run ("mini", fileRoot, tiledMapApiKey, 1s, fullScreen);
  return 0;
  }
