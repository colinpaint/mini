// cJpegDecoder.h - windows only turbo jpeg decoder
#pragma once
//{{{  includes
#include "../common/basicTypes.h"
#include "../common/date.h"
#include "../common/utils.h"
//#include "../include/jpeglib/jpeglib.h"
//}}}

class cJpegLoader {
public:
  // gets
  cPoint getThumbnailSize() const { return mThumbnailSize; }
  int getThumbnailWidth() const { return mThumbnailSize.width; }
  int getThumbnailHeight() const { return mThumbnailSize.height; }

  cPoint getImageSize() const { return mImageSize; }
  int getImageWidth() const { return mImageSize.width; }
  int getImageHeight() const { return mImageSize.height; }

  int getScale() const { return mLoadScale; }
  cPoint getScaledSize() const { return mScaledSize; }
  int getScaledWidth() const { return mScaledSize.width; }
  int getScaledHeight() const { return mScaledSize.height; }

  std::chrono::time_point<std::chrono::system_clock> getExifTimePoint() const { return mExifTimePoint; }

  std::string getMakeString() const { return mMakeString; }
  std::string getModelString() const { return mModelString; }
  std::string getMarkerString() const { return mMarkerString; }
  std::string getGpsString() const { return mGps ? mGps->getString() : ""; }

  int getOrientation() const { return mOrientation; }
  float getAperture() const { return mAperture; }
  float getFocalLength() const { return mFocalLength; }
  float getExposure() const { return mExposure; }

  // loads
  bool loadInfo (uint8_t* buf, size_t bufLen);
  uint8_t* loadThumbnail (uint8_t* buf, size_t bufLen, cPoint thumbnailSize);
  uint8_t* loadImage (uint8_t* buf, size_t bufLen, int scale);
  //uint8_t* compressImage (int& bufLen, float quality);

private:
  //{{{
  class cGps {
  public:
    cGps() : mVersion(0),
             mLatitudeRef(0),  mLatitudeDeg(0), mLatitudeMin(0), mLatitudeSec(0),
             mLongitudeRef(0), mLongitudeDeg(0), mLongitudeMin(0), mLongitudeSec(0),
             mAltitudeRef(0),  mAltitude(0), mImageDirectionRef(0), mImageDirection(0),
             mHour(0), mMinute(0), mSecond(0) {}

    std::string getString();

    int mVersion;

    char mLatitudeRef;
    float mLatitudeDeg;
    float mLatitudeMin;
    float mLatitudeSec;

    char mLongitudeRef;
    float mLongitudeDeg;
    float mLongitudeMin;
    float mLongitudeSec;

    char mAltitudeRef;
    float mAltitude;

    char mImageDirectionRef;
    float mImageDirection;

    float mHour;
    float mMinute;
    float mSecond;

    std::string mDate;
    std::string mDatum;
    };
  //}}}

  static std::string getMarkerMnemomic (uint16_t marker, uint16_t markerLength);
  static std::chrono::time_point<std::chrono::system_clock> getExifTimePoint (const std::string& isoTimeString);

  void parseExifDirectory (uint8_t* offsetBasePtr, uint8_t* ptr, bool intelEndian);

  // exif info utils
  uint16_t getExifWord (uint8_t* ptr, bool intelEndian);
  uint32_t getExifLong (uint8_t* ptr, bool intelEndian);
  float getExifSignedRational (uint8_t* ptr, bool intelEndian, uint32_t& numerator, uint32_t& denominator);
  std::string getExifString (uint8_t* ptr);
  cGps* getExifGps (uint8_t* ptr, uint8_t* offsetBasePtr, bool intelEndian);

  //{{{  vars
  D2D1_SIZE_U mThumbnailSize = {0,0};
  D2D1_SIZE_U mImageSize = {0,0};

  int mLoadScale = 0;
  D2D1_SIZE_U mScaledSize = {0,0};

  // exif thumbnail jpg
  uint32_t mThumbnailLen = 0;
  uint32_t mThumbnailOffset = 0;

  // exif info
  std::string mMarkerString;
  std::string mMakeString;
  std::string mModelString;
  int mOrientation = 0;
  float mExposure = 0;
  float mFocalLength = 0;
  float mAperture = 0;
  std::chrono::time_point<std::chrono::system_clock> mExifTimePoint;
  cGps* mGps = nullptr;
  //}}}
  };
