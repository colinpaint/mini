// jpegWindow.cpp
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <mutex>
#include <thread>

#include "concurrent_vector.h"

// windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <shlguid.h>

#include "../common/date.h"
#include "../common/utils.h"
#include "../common/cLog.h"
#include "../common/resolve.h"
#include "../common/cSemaphore.h"

#include "../gui/cWindow.h"
#include "../gui/basicBoxes.h"
#include "../gui/cLogBox.h"

#include "cJpegLoader.h"

#pragma comment(lib,"common.lib")

using namespace std;
using namespace chrono;
//}}}

//{{{
class cDirectory {
public:
  cDirectory (const string& pathName, const string& name, uint32_t depth)
    : mPathName(pathName), mName(name), mDepth(depth) {}
  virtual ~cDirectory() = default;

  string getName() const { return mName; }
  string getPathName() const { return mPathName; }
  string getFullName() const { return mPathName + "/" + mName; }

  uint32_t getDepth() const { return mDepth; }

  bool getSelected() const { return mSelected; }
  void toggleSelect() { mSelected = !mSelected; }

private:
  const string mPathName;
  const string mName;
  const uint32_t mDepth = 0;

  bool mSelected = false;
  };
//}}}
//{{{
class cImage {
public:
  //{{{
  cImage (const string& fullDirName, const string& filename)
    : mFullDirName(fullDirName), mFilename(filename) {}
  //}}}
  //{{{
  ~cImage() {
    mThumbnailBitmap.release();
    mScaledBitmap.release();
    }
  //}}}

  //{{{  gets
  string getFilename() { return mFilename; }
  string getFullPathName() { return mFullDirName.empty() ? mFilename : (mFullDirName + "/" + mFilename); }

  bool isInfoLoaded() { return mInfoLoaded; }
  bool isThumbnailLoaded() { return mThumbnailLoaded; }
  bool isImageLoaded() { return mImageLoaded; }

  // filesystem info
  string getCreationTimeString() {
    return mCreationTimePoint.time_since_epoch() == seconds::zero() ?
             "" : date::format (kTimeFormatStr, floor<seconds>(mCreationTimePoint)); }

  string getLastAccessTimeString() {
    return mLastAccessTimePoint.time_since_epoch() == seconds::zero() ?
             "" : date::format (kTimeFormatStr, floor<seconds>(mLastAccessTimePoint)); }

  string getLastWriteTimeString() {
    return mLastWriteTimePoint.time_since_epoch() == seconds::zero() ?
             "" : date::format (kTimeFormatStr, floor<seconds>(mLastWriteTimePoint)); }

  // image info, available after loadInfo
  uint32_t getImageWidth() { return mImageWidth; }
  uint32_t getImageHeight() { return mImageHeight; }
  cPoint getImageSize() { return cPoint ((float)mImageWidth, (float)mImageHeight); }

  // exif info
  time_point<system_clock> getExifTimePoint() { return mJpegLoader.getExifTimePoint(); }
  string getExifTimeString() { return date::format (kTimeFormatStr, floor<seconds>(mJpegLoader.getExifTimePoint())); }

  string getMakeString() { return mJpegLoader.getMakeString(); }
  string getModelString() { return mJpegLoader.getModelString(); }
  string getMarkerString() { return mJpegLoader.getMarkerString(); }
  string getGpsString() { return mJpegLoader.getGpsString(); }

  int getOrientation() { return mJpegLoader.getOrientation(); }
  float getAperture() { return mJpegLoader.getAperture(); }
  float getFocalLength() { return mJpegLoader.getFocalLength(); }
  float getExposure() { return mJpegLoader.getExposure(); }

  // thumbail
  uint32_t getThumbnailWidth() { return mJpegLoader.getThumbnailWidth(); }
  uint32_t getThumbnailHeight() { return mJpegLoader.getThumbnailHeight(); }
  cBitmap getThumbnailBitmap() { return mThumbnailBitmap; }

  // image
  float getScale() { return mScale; }
  uint32_t getScaledWidth() { return mScaledWidth; }
  uint32_t getScaledHeight() { return mScaledHeight; }
  cPoint getScaledSize() { return cPoint ((float)mScaledWidth, (float)mScaledHeight); }
  cBitmap getScaledBitmap() { return mScaledBitmap; }

  uint8_t getRed (cPoint pos) { return getColour (pos, 2); }
  uint8_t getGreen (cPoint pos) { return getColour (pos, 1); }
  uint8_t getBlue (cPoint pos) { return getColour (pos, 0); }
  //}}}

  //{{{
  void setThumbnailBitmap (cBitmap bitmap) {
    mThumbnailBitmap = bitmap;
    mThumbnailLoaded = true;
    }
  //}}}
  //{{{
  void setImageBitmap (cBitmap bitmap) {
    mScaledBitmap = bitmap;
    mImageLoaded = true;
    }
  //}}}

  //{{{
  uint8_t* loadThumbnail() {

    // get file times from windows
    HANDLE fileHandle = CreateFile (getFullPathName().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    getFileTimes (fileHandle);
    CloseHandle (fileHandle);

    // load 16m
    uint8_t* buf = (uint8_t*)malloc (0x1000000);
    FILE* file = fopen (getFullPathName().c_str(), "rb");
    size_t bufLen = (uint32_t)fread (buf, 1, 0x1000000, file);
    fclose (file);

    // loadInfo
    mJpegLoader.loadInfo (buf, bufLen);
    mInfoLoaded = true;

    // loadThumbnail
    cPoint thumbnailSize;
    uint8_t* pixels = mJpegLoader.loadThumbnail (buf, bufLen, thumbnailSize);
    mImageWidth = mJpegLoader.getImageWidth();
    mImageHeight = mJpegLoader.getImageHeight();

    free (buf);

    return pixels;
    }
  //}}}
  //{{{
  uint8_t* loadImage() {

    uint8_t* buf = (uint8_t*)malloc (0x1000000);
    FILE* file = fopen (getFullPathName().c_str(), "rb");
    size_t bufLen = (uint32_t)fread (buf, 1, 0x1000000, file);
    fclose (file);

    int scale = 1;;

    uint8_t* pixels = mJpegLoader.loadImage (buf, bufLen, scale);
    mImageWidth = mJpegLoader.getImageWidth();
    mImageHeight = mJpegLoader.getImageHeight();
    mScaledWidth = mJpegLoader.getScaledWidth();
    mScaledHeight = mJpegLoader.getScaledHeight();

    return pixels;
    }
  //}}}

private:
  // const
  inline static const string kTimeFormatStr = "%D %T";

  // members
  //{{{
  uint8_t getColour (cPoint pos, int offset) {
  // get colour from pos in fullImage

    //if (mBgraBuf && (pos.x < mSize.width) && (pos.y < mSize.height)) {
    //  offset += ((int(pos.y) * mSize.width) + int(pos.x)) * 4;
    //  return *(mBgraBuf + offset);
    //  }
    //else
      return 0;
    }
  //}}}
  //{{{
  void getFileTimes (HANDLE fileHandle) {

    FILETIME creationTime;
    FILETIME lastAccessTime;
    FILETIME lastWriteTime;
    GetFileTime (fileHandle, &creationTime, &lastAccessTime, &lastWriteTime);

    mCreationTimePoint = getFileTimePoint (creationTime);
    mLastAccessTimePoint = getFileTimePoint (lastAccessTime);
    mLastWriteTimePoint = getFileTimePoint (lastWriteTime);
    }
  //}}}
  //{{{
  system_clock::time_point getFileTimePoint (FILETIME fileTime) {

    // filetime_duration has the same layout as FILETIME; 100ns intervals
    using filetime_duration = duration<int64_t, ratio<1, 10'000'000>>;

    // January 1, 1601 (NT epoch) - January 1, 1970 (Unix epoch):
    constexpr duration<int64_t> nt_to_unix_epoch{INT64_C(-11644473600)};

    const filetime_duration asDuration{static_cast<int64_t>(
      (static_cast<uint64_t>((fileTime).dwHighDateTime) << 32) | (fileTime).dwLowDateTime)};

    const auto withUnixEpoch = asDuration + nt_to_unix_epoch;
    return system_clock::time_point{ duration_cast<system_clock::duration>(withUnixEpoch)};
    }
  //}}}

  // vars
  const string mFullDirName;
  const string mFilename;

  cJpegLoader mJpegLoader;

  bool mInfoLoaded = false;
  time_point<system_clock> mCreationTimePoint;
  time_point<system_clock> mLastAccessTimePoint;
  time_point<system_clock> mLastWriteTimePoint;

  bool mThumbnailLoaded = false;
  uint32_t mImageOffset = 0;
  cBitmap mThumbnailBitmap = nullptr;

  bool mImageLoaded = false;
  float mScale = 1.f;
  uint32_t mImageWidth = 0;
  uint32_t mImageHeight = 0;
  uint32_t mScaledWidth = 0;
  uint32_t mScaledHeight = 0;
  cBitmap mScaledBitmap = nullptr;;
  };
//}}}
//{{{
class cImageSet {
public:
  static constexpr uint32_t kUnselected = 0xFFFFFFFF;

  cImageSet() = default;
  cImageSet (const string& name) : mName(name) {}
  //{{{
  ~cImageSet() {
    for (auto& image : mImages)
      delete (image);
    }
  //}}}

  // gets
  string getName() { return mName; }

  uint32_t size() const { return (uint32_t)mImages.size(); }
  concurrency::concurrent_vector <cImage*>& getImages() { return mImages; }

  cPoint getThumbnailSize() const { return kThumbnailSize; }
  uint32_t getNumRows() const { return (((uint32_t)mImages.size() - 1) / mColumns) + 1; }
  uint32_t getNumColumns() const { return mColumns; }
  cPoint getSrcSize() const  { return cPoint (getNumColumns() * kThumbnailSize.x, getNumRows() * kThumbnailSize.y); }

  //{{{
  uint32_t getBestThumbnailIndex() const {

    uint32_t bestThumbnailImageIndex = kUnselected;

    uint32_t bestMetric = kUnselected;
    uint32_t columns = getNumColumns();
    for (uint32_t i = 0; i < mImages.size(); i++) {
      if (!mImages[i]->isThumbnailLoaded()) {
        uint32_t x = i % columns;
        uint32_t y = i / columns;
        uint32_t metric = x*x + y*y;
        if (metric < bestMetric) {
          bestMetric = metric;
          bestThumbnailImageIndex = i;
          }
        }
      }

    return bestThumbnailImageIndex;
    }
  //}}}
  //{{{
  cRect getRectByIndex (uint32_t index) const {

    uint32_t row = index % mColumns;
    uint32_t column = index / mColumns;
    return cRect (row * kThumbnailSize.x, column * kThumbnailSize.y, (row + 1) * kThumbnailSize.x, (column + 1) * kThumbnailSize.y);
    }
  //}}}
  cImage* getImageByIndex (uint32_t index) const { return (index == kUnselected) ? nullptr : mImages[index]; }
  uint32_t getPickIndex() const { return mPickIndex; }
  uint32_t getSelectIndex() const { return mSelectIndex; }

  concurrency::concurrent_vector<cDirectory>& getDirectories() { return mDirectories; }
  uint32_t getDirectoriesSize() const { return (uint32_t)mDirectories.size(); }
  uint32_t getDirectoryMaxDepth() const { return mDirectoryMaxDepth; }

  // set
  void setName (const string& name) { mName = name; }

  // selects
  //{{{
  cImage* selectImageByIndex (uint32_t index) {
    mSelectIndex = max (0u, min ((uint32_t)mImages.size() - 1, index));
    return mImages[mSelectIndex];
    }
  //}}}
  cImage* selectFirstIndex() { return selectImageByIndex (0); }
  cImage* selectLastIndex() { return selectImageByIndex ((int)mImages.size() - 1); }
  cImage* selectPrevIndex() { return selectImageByIndex (mSelectIndex - 1); }
  cImage* selectNextIndex() { return selectImageByIndex (mSelectIndex + 1); }
  cImage* selectPrevRowIndex() { return selectImageByIndex (mSelectIndex - getNumColumns()); }
  cImage* selectNextRowIndex() { return selectImageByIndex (mSelectIndex + getNumColumns()); }

  //{{{
  void loadThumbnailByIndex (uint32_t index) {
    mImages[index]->loadThumbnail();
    }
  //}}}
  //{{{
  uint32_t pick (cPoint pos) {

    for (uint32_t i = 0; i < mImages.size(); i++) {
      if (getRectByIndex (i).inside (pos)) {
        mPickIndex = i;
        return mPickIndex;
        }
      }

    mPickIndex = -1;
    return mPickIndex;
    }
  //}}}
  //{{{
  void fileScan (const string& parentDirName, const string& dirName, const string& matchName, uint32_t depth,
                 const function<void(const string& filename)>& imageAddedCallback) {
  // recursive filescan

    mDirectoryMaxDepth = max (mDirectoryMaxDepth, depth);

    string indexStr;
    for (uint32_t i = 0; i < depth; i++)
      indexStr += "  ";
    cLog::log (LOGINFO, "fileScan - " + indexStr + dirName);

    string fullDirectoryName = parentDirName.empty() ? dirName : parentDirName + "/" + dirName;
    mDirectories.push_back (cDirectory (parentDirName, dirName, depth));

    string searchStr = fullDirectoryName +  "/*";
    WIN32_FIND_DATA findFileData;
    HANDLE file = FindFirstFileEx (searchStr.c_str(), FindExInfoBasic, &findFileData,
                                   FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
    if (file != INVALID_HANDLE_VALUE) {
      do {
        if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (findFileData.cFileName[0] != '.'))
          // recursive scan of directory
          fileScan (fullDirectoryName, findFileData.cFileName, matchName, depth+1, imageAddedCallback);
        else if (PathMatchSpec (findFileData.cFileName, matchName.c_str())) {
          mImages.push_back (new cImage (fullDirectoryName, findFileData.cFileName));
          imageAddedCallback (findFileData.cFileName);
          }
        } while (FindNextFile (file, &findFileData));

      FindClose (file);
      }

    mColumns = (uint32_t)sqrt (mImages.size()) * 11/12 + 1;
    }
  //}}}


private:
  const cPoint kThumbnailSize = cPoint(160.f, 120.f);

  // vars
  string mName;

  concurrency::concurrent_vector<cDirectory> mDirectories;
  concurrency::concurrent_vector <cImage*> mImages;

  uint32_t mPickIndex = kUnselected;
  uint32_t mSelectIndex = kUnselected;

  uint32_t mDirectoryMaxDepth = 0;
  uint32_t mColumns = 1;
  };
//}}}

//{{{
class cImageSetBox : public cWindow::cBox {
public:
  //{{{
  cImageSetBox (cWindow* window, float height, cImageSet& imageSet,
                const function<void (cImage* image)>& callback = [](cImage*) noexcept {})
      : cBox("imageSetBox", window, 0,0), mImageSet(imageSet), mCallback(callback) {

    mPin = true;

    mScale = height * getBoxHeight() / mImageSet.getSrcSize().y;

    setSize ((mImageSet.getSrcSize() * mScale) + cPoint(0.f, getBoxHeight()));
    }
  //}}}
  virtual ~cImageSetBox() = default;

  //{{{
  virtual bool onProx (bool inClient, cPoint pos) final {

    if (inClient) {
      pos -= cPoint (0.f, getBoxHeight());
      if (pos.y < 0.f) {
        cLog::log (LOGINFO, "picked title");
        return true;
        }

      if (mWindow->getControl() || mWindow->getShift()) {
        if (!mLastLocked) {
          mFirstLockedPos = pos;
          mLastLocked = true;
          }
        pos.y = mFirstLockedPos.y;
        }
      else
        mLastLocked = false;

      uint32_t pickIndex = mImageSet.pick (pos / mScale);
      if (pickIndex != cImageSet::kUnselected) {
        mImageSet.loadThumbnailByIndex (pickIndex);
        return true;
        }
      }

    return true;
    }
  //}}}
  //{{{
  virtual bool onMove (bool right, cPoint pos, cPoint inc) final {

    if (mRect.left + inc.x < 0)
      inc.x = -mRect.left;
    if (mRect.top + inc.y < 0)
      inc.y = -mRect.top;

    setPos (getTL() + inc);

    return true;
    }
  //}}}
  //{{{
  virtual bool onUp (bool right, bool mouseMoved, cPoint pos) final {

    if (!mouseMoved) {
      if (mImageSet.getPickIndex() != cImageSet::kUnselected) {
        // hit thumbnail
        mCallback (mImageSet.selectImageByIndex (mImageSet.getPickIndex()));
        return true;
        }
      else {
        // select image ????
        cRect r(0,0, 40.f, 0);
        for (cDirectory& directory : mImageSet.getDirectories()) {
          r.bottom = r.top + getBoxHeight();
          if (r.inside (pos)) {
            directory.toggleSelect();
            return true;
            }
          r.top = r.bottom;
          }
        }
      }

    return false;
    }
  //}}}
  //{{{
  virtual bool onWheel (int delta, cPoint pos) final {

    mScale *= (delta > 0.f) ? 1.05f : 1.f / 1.05f;
    setSize ((mImageSet.getSrcSize() * mScale) + cPoint(0.f, getBoxHeight()));

    return true;
    }
  //}}}

  //{{{
  virtual void onDraw() final {

    cRect r (mRect);
    r.bottom = r.top + getBoxHeight();
    drawTextRectangle (kTextGray, kBoxGray, r, mImageSet.getName());

    cPoint tl = getTL() + cPoint (0.f, getBoxHeight());

    // draw thumbnails
    for (uint32_t i = 0; i < mImageSet.getImages().size(); i++) {
      cImage* image = mImageSet.getImages()[i];
      if (image->isThumbnailLoaded())
        blit (image->getThumbnailBitmap(), (mImageSet.getRectByIndex (i) * mScale) + tl);
      }

    // draw selected
    if (mImageSet.getSelectIndex() != cImageSet::kUnselected)
      drawBorder (kWhite, (mImageSet.getRectByIndex (mImageSet.getSelectIndex()) * mScale) + tl);

    // draw prox
    if (mImageSet.getPickIndex() != cImageSet::kUnselected)
      drawBorder (kYellow, (mImageSet.getRectByIndex (mImageSet.getPickIndex()) * mScale) + tl);

    // draw border
    //drawRectangleOutline (kWhite);
    }
  //}}}

private:
  cImageSet& mImageSet;
  const function <void (cImage* image)> mCallback;

  float mScale = 1.f;

  bool mLastLocked = false;
  cPoint mFirstLockedPos;
  };
//}}}
//{{{
class cImageSetDirectoryBox : public cWindow::cBox {
public:
  //{{{
  cImageSetDirectoryBox (cWindow* window, float width, float height, cImageSet& imageSet)
      : cBox("imageSetBox", window, width, height), mImageSet(imageSet) {
    mPin = true;
    }
  //}}}
  virtual ~cImageSetDirectoryBox() = default;

  virtual void onDraw() final {
    cRect r (mRect);
    r.bottom = r.top + getBoxHeight();
    for (cDirectory& directory : mImageSet.getDirectories()) {
      string indentString;
      for (uint32_t i = 0; i < directory.getDepth(); i++)
        indentString += " -";
      drawText (directory.getSelected() ? kYellow : kWhite, r, indentString + directory.getName());
      r.top += getBoxHeight();
      r.bottom += getBoxHeight();
      }
    }

private:
  cImageSet& mImageSet;
  };
//}}}
//{{{
class cImageBox : public cWindow::cBox {
public:
  //{{{
  cImageBox (cWindow* window, float width, float height) : cBox("imageBox", window, width,height) {
    mPin = true;
    }
  //}}}
  virtual ~cImageBox() = default;

  //{{{
  void setImage (cImage* image) {

    uint8_t* pixels = image->loadImage();
    image->setImageBitmap (mWindow->createBitmap (image->getScaledWidth(), image->getScaledHeight(), pixels, true));
    uint32_t scale = 1 + uint32_t(image->getImageSize().x / image->getScaledSize().x);

    if (!image->isImageLoaded() || (scale != image->getScale()))
      cLog::log (LOGNOTICE, fmt::format ("loadImage new:{} old:{}", scale, image->getScale()));
      //mImage->loadImage (mWindow->getDc(), scale);

    float srcScaleX = getSize().x / image->getScaledSize().x;
    float srcScaleY = getSize().y / image->getScaledSize().y;
    float bestScale = (srcScaleX < srcScaleY) ? srcScaleX : srcScaleY;

    cLog::log (LOGINFO, fmt::format ("setImage {},{} {},{}:{}",
                                     getSrcSize (image).x, getSrcSize (image).y, srcScaleX, srcScaleY, bestScale));

    mScale = bestScale;
    setSize (getSrcSize (image).x * mScale, getSrcSize (image).y * mScale);

    mImage = image;
    }
  //}}}
  //{{{
  void changeImage (cImage* image) {

    cImage* temp = mImage;
    mImage = nullptr;

    if (temp)
      delete temp;

    if (image)
      setImage (image);
    }
  //}}}

  //{{{
  virtual bool onMove (bool right, cPoint pos, cPoint inc) final {

    if (mRect.left + inc.x < 0)
      inc.x = -mRect.left;
    if (mRect.top + inc.y < 0)
      inc.y = -mRect.top;
    setPos (getTL() + inc);

    return true;
    }
  //}}}
  //{{{
  virtual bool onWheel (int delta, cPoint pos) final {

    mScale *= (delta > 0.f) ? 1.05f : 1.f / 1.05f;
    setSize (getSrcSize (mImage) * mScale);

    return true;
    }
  //}}}
  //{{{
  virtual bool onProx (bool inClient, cPoint pos) final {

    mPos = pos;
    mSamplePos = pos;
    mWindow->changed();

    return cBox::onProx (inClient, pos);
    }
  //}}}
  //{{{
  virtual void onDraw() final {

    if (mImage && mImage->isImageLoaded()) {
      if (!mImage->getScaledBitmap().empty()) {
        blit (mImage->getScaledBitmap());
        drawBorder (kWhite);
        }
      return;

      cRect r = cRect (mRect.getTR(), mRect.getTR() + cPoint(240.f, getBoxHeight()));
      //{{{  draw pen pos
      drawTextRectangle (kTextGray, kBoxGray, r, fmt::format ("{} {}", mPos.x, mPos.y));
      r.top += getBoxHeight();
      r.bottom += getBoxHeight();
      //}}}
      //{{{  draw samplePos
      drawTextRectangle (kTextGray, kBoxGray, r, fmt::format ("{} {}", mSamplePos.x, mSamplePos.y));
      r.top += getBoxHeight();
      r.bottom += getBoxHeight();
      //}}}
      if (!mImage->getScaledBitmap().empty()) {
        //{{{  draw pick colour
        int red = mImage->getRed (mSamplePos);
        int green = mImage->getGreen (mSamplePos);
        int blue = mImage->getBlue (mSamplePos);

        //mBrush->SetColor (D2D1::ColorF(red /255.f, green /255.f, blue /255.f, 1.f));
        drawTextRectangle (kTextGray, kBoxGray, r,
                           fmt::format ("r:{} g:{} b:{}",red, green, blue));
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
        //}}}
      }
    }
  //}}}

private:
  //{{{
  cPoint getSrcSize (cImage* image) {
    return image->isImageLoaded() ? image->getScaledSize() : getSize();
    }
  //}}}

  cImage* mImage = nullptr;
  float mScale = 1.f;

  cPoint mPos;
  cPoint mSamplePos;
  };
//}}}
//{{{
class cImageInfoBox : public cWindow::cBox {
public:
  //{{{
  cImageInfoBox (cWindow* window, float width, float height, cImageSet& imageSet)
      : cBox("imageSetInfoBox", window, width, height), mImageSet(imageSet) {
    mPin = true;
    }
  //}}}
  virtual ~cImageInfoBox() = default;

  //{{{
  virtual void onDraw() final {

    // draw panel bgnd
    drawRectangle (kBoxGray, mRect);

    cRect r (mRect);
    r.left += getBorderWidth();

    cImage* image = mImageSet.getImageByIndex (mImageSet.getPickIndex());
    if (image) {
      //{{{  draw fullFileName text
      drawText (kTextGray, r, image->getFullPathName());
      r.top += getBoxHeight();
      r.bottom += getBoxHeight();
      //}}}
      //{{{  draw imageSize text
      drawText (kTextGray, r, fmt::format ("{}x{}", image->getImageSize().x, image->getImageSize().y));
      r.top += getBoxHeight();
      r.bottom += getBoxHeight();
      //}}}
      //{{{  draw exifTime text
      if (!image->getExifTimeString().empty()) {
        drawText (kTextGray, r, image->getExifTimeString());
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
      //}}}
      //{{{  draw make model text
      if (!image->getMakeString().empty() || !image->getModelString().empty()) {
        drawText (kTextGray, r, fmt::format ("{} {}", image->getMakeString(), image->getModelString()));
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
      //}}}
      //{{{  draw orientation text
      if (image->getOrientation()) {
        drawText (kTextGray, r, fmt::format ("orientation {}", image->getOrientation()));
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
      //}}}
      //{{{  draw focalLength text
      if (image->getFocalLength() > 0) {
        drawText (kTextGray, r, fmt::format ("focal length {}", image->getFocalLength()));
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
      //}}}
      //{{{  draw exposure text
      if (image->getExposure() > 0) {
        drawText (kTextGray, r, fmt::format ("exposure {}", image->getExposure()));
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
      //}}}
      //{{{  draw aperture text
      if (image->getAperture() > 0) {
        drawText (kTextGray, r, fmt::format ("aperture {}", image->getAperture()));
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
      //}}}
      //{{{  draw GPSinfo text
      if (!image->getGpsString().empty()) {
        drawText (kTextGray, r, fmt::format ("{}", image->getGpsString()));
        r.top += getBoxHeight();
        r.bottom += getBoxHeight();
        }
      //}}}

      // draw thumbnailBitmap
      if (image->isThumbnailLoaded()) {
        cRect thumbnailRect (mRect);
        thumbnailRect.addBorder (getBorderWidth());
        thumbnailRect.left = thumbnailRect.right - mImageSet.getThumbnailSize().x;
        thumbnailRect.bottom = thumbnailRect.top + mImageSet.getThumbnailSize().y;
        blit (image->getThumbnailBitmap(), thumbnailRect);
        }

      if (!image->getExifTimeString().empty()) {
        // draw exif time
        float radius = (getHeight()/2.f) - getBoxHeight();
        cPoint centre = {mRect.right - mImageSet.getThumbnailSize().x - getBoxHeight() - radius , mRect.top + getBoxHeight() + radius};
        //{{{  draw clock
        auto datePoint = floor<date::days>(image->getExifTimePoint());
        auto timeOfDay = date::make_time(duration_cast<seconds>(image->getExifTimePoint() - datePoint));

        float hoursRadius = radius * 0.6f;
        int64_t hours = timeOfDay.hours().count() + (timeOfDay.minutes().count() / 60);
        float hoursAngle = (1.f - (hours / 6.f)) * kPi;
        drawLine (kWhite, centre, centre + cPoint(hoursRadius * sin (hoursAngle),
                                                  hoursRadius * cos (hoursAngle)), getOutlineWidth());

        float minutesRadius = radius * 0.75f;
        int64_t minutes = timeOfDay.minutes().count() + (timeOfDay.seconds().count() / 60);
        float minutesAngle = (1.f - (minutes/30.f)) * kPi;
        drawLine (kWhite, centre, centre + cPoint (minutesRadius * sin (minutesAngle),
                                                   minutesRadius * cos (minutesAngle)), getOutlineWidth());

        float secondsRadius = radius * 0.85f;
        int64_t seconds = timeOfDay.seconds().count();
        float secondsAngle = (1.0f - (seconds /30.0f)) * kPi;
        drawLine (kRed, centre, centre + cPoint (secondsRadius * sin (secondsAngle),
                                                 secondsRadius * cos (secondsAngle)), getOutlineWidth());
        drawEllipse (kWhite, centre, radius, getOutlineWidth());
        //}}}

        const float kDayWidth = getBoxHeight() * 1.5f;
        cRect calendarRect = {centre.x - radius - getBoxHeight() - 7*kDayWidth, mRect.top,
                              centre.x - radius - getBoxHeight(), mRect.bottom};
        //{{{  draw calendar
        // draw bgnd
        drawRounded (kBlack, calendarRect, mWindow->getRoundRadius());

        // rectangle for clendar text
        calendarRect.addBorder (mWindow->getRoundRadius());
        float calendarLeft = calendarRect.left;

        // draw month, left justified, top row
        date::year_month_day yearMonthDay = date::year_month_day {datePoint};
        date::year_month yearMonth = yearMonthDay.year() / date::month { yearMonthDay.month() };
        drawText (kWhite, calendarRect, date::format ("%B", yearMonth));

        // draw year, right justified, top row
        string yearString = date::format ("%Y", yearMonth);
        calendarRect.left = calendarRect.right - measureText (yearString).x;
        drawText (kWhite, calendarRect, yearString);
        calendarRect.top += getBoxHeight();

        // draw daysOfWeek, 2nd row
        date::day today = yearMonthDay.day();
        date::weekday weekDayToday = date::weekday { yearMonth / today };
        date::weekday titleWeekDay = date::sun;

        calendarRect.left = calendarLeft;
        do {
          string dayString = date::format ("%a", titleWeekDay);
          dayString.resize (2);
          drawText ((weekDayToday == titleWeekDay) ?  kWhite : kGray, calendarRect, dayString);
          calendarRect.left += kDayWidth;
          } while (++titleWeekDay != date::sun);
        calendarRect.top += getBoxHeight();

        // draw lines of days, skip leading space
        date::weekday weekDay = date::weekday{yearMonth / 1};
        using date::operator""_d;
        date::day curDay = 1_d;
        date::day lastDayOfMonth = (yearMonth / date::last).day();

        int line = 1;
        calendarRect.left = calendarLeft + ((weekDay - date::sun).count() * kDayWidth);
        while (curDay <= lastDayOfMonth) {
          // iterate days of week
          drawText (today == curDay ? kWhite : kGray, calendarRect, date::format ("%e", curDay));
          if (++weekDay == date::sun) {
            // line 6 folds back to first
            line++;
            calendarRect.top += (line == 6) ? -4 * getBoxHeight() : getBoxHeight();
            calendarRect.left = calendarLeft;
            }
          else
            calendarRect.left += kDayWidth;
          ++curDay;
          };
        //}}}
        }
      }
    }
  //}}}

private:
  cImageSet& mImageSet;
  };
//}}}

// cApp
class cJpegWindow : public cWindow {
public:
  //{{{
  void run (const string& title, const string& fileRoot, chrono::milliseconds tickMs, bool fullscreen) {

    createWindow (title, 1920/2, 1080/2, tickMs, fullscreen);

    add (new cClockBox (this, 6), -7,-7);
    cBox* box = new cCalendarBox (this);
    add (box, -box->getSizeInBoxes() - cPoint(7,0));

    // scan files
    mImageSet.setName (fileRoot);
    mImageSet.fileScan ("", fileRoot, "*.jpg", 0, [&](const string& filename) { changed(); });

    if (mImageSet.getDirectoriesSize() > 1)
      add (new cImageSetDirectoryBox (this, 10,getHeightInBoxes()/2, mImageSet));

    if (mImageSet.size() > 0) {
      add (new cImageSetBox (this, getHeightInBoxes()/2, mImageSet, [&](cImage* image) { selectImage (image); }));

      float infoWidth = (getWidth() < 1920/2) ? (getWidthInBoxes() - (7*1.5f) - 7) : getWidthInBoxes()/2;
      add (new cImageInfoBox (this, infoWidth, 8, mImageSet), 0,-8);
      }

    add (new cLogBox (this));
    add (new cUint64Box (this, 3,1, "{}us", getRenderUs()), -3, -1)->setSelfSize();
    add (new cWindowBox (this, 3,1), -3,0);

    // load thumbnails threads
    for (uint32_t threadNum = 0; threadNum < kThumbnailThreads; threadNum++)
      thread ([=]() {
        // thread lambda
        cLog::setThreadName (fmt::format ("td {}", threadNum));
        while (true) {
          uint32_t thumbnailIndex = mImageSet.getBestThumbnailIndex();
          if (thumbnailIndex == cImageSet::kUnselected)
            break;
          cImage* image = mImageSet.getImageByIndex (thumbnailIndex);
          image->setThumbnailBitmap (createBitmap (image->getThumbnailWidth(), image->getThumbnailHeight(), image->loadThumbnail(), true));
          changed();
          }
        cLog::log (LOGNOTICE, "exit");
        }).detach();

    messagePump();
    };
  //}}}

protected:
  //{{{
  bool onKey (int key) {

    switch (key) {
      case 0x1B: return true; // escape abort

      case 0x10: changed(); break; // shift
      case 0x11: break; // control

      case  ' ': break; // space bar

      // navigation
      case 0x21: break; // page up
      case 0x22: break; // page down
      case 0x23: if (mImageBox) selectImage (mImageSet.selectLastIndex()); break;    // end
      case 0x24: if (mImageBox) selectImage (mImageSet.selectFirstIndex()); break;   // home
      case 0x25: if (mImageBox) selectImage (mImageSet.selectPrevIndex()); break;    // left arrow
      case 0x26: if (mImageBox) selectImage (mImageSet.selectPrevRowIndex()); break; // up arrow
      case 0x27: if (mImageBox) selectImage (mImageSet.selectNextIndex()); break;    // right arrow
      case 0x28: if (mImageBox) selectImage (mImageSet.selectNextRowIndex()); break; // down arrow

      case 'F':  toggleFullScreen(); break;

      case 0x00: break;
      default: cLog::log (LOGERROR, "unused key %x", key);
      }

    return false;
    }
  //}}}
  //{{{
  bool onKeyUp (int key) {

    switch (key) {
      case 0x10: changed(); break; // shift
      default: break;
      }

    return false;
    }
  //}}}

private:
  static constexpr uint32_t kThumbnailThreads = 2;
  //{{{
  void selectImage (cImage* image) {

    if (!mImageBox)  {
      mImageBox = new cImageBox (this, getWidthInBoxes()/2, getHeightInBoxes()/2);
      add (mImageBox, getWidthInBoxes()/2,0);
      }

    mImageBox->setImage (image);
    changed();
    };
  //}}}

  cImageSet mImageSet;
  cImageBox* mImageBox = nullptr;
  };

// main
//{{{
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

  int numArgs;
  auto args = CommandLineToArgvW (GetCommandLineW(), &numArgs);
  vector <string> params;
  for (int i = 1; i < numArgs; i++)
    params.push_back (utils::wstringToString (args[i]));

  eLogLevel logLevel = LOGINFO;
  bool fullscreen = false;
  string rootDirName = "D:/pics/cloud/life";
  for (auto& param : params) {
    if (param == "log1") { logLevel = LOGINFO1; }
    else if (param == "log2") { logLevel = LOGINFO2; }
    else if (param == "log3") { logLevel = LOGINFO3; }
    else if (param == "full") { fullscreen = true; }
    else { rootDirName = param; }
    }

  if (rootDirName.find (".lnk") <= rootDirName.size()) {
    string fullName;
    if (resolveShortcut (rootDirName, fullName)) {
      rootDirName = fullName;
      cLog::log (LOGINFO, fmt::format ("jpegWindow resolved link to {}", rootDirName));
      }
    }

  cLog::init (logLevel, true);
  cLog::log (LOGNOTICE, "player");

  cJpegWindow jpegWindow;
  jpegWindow.run ("jpegWindow", rootDirName, 1s, fullscreen);
  }
//}}}
