// cJpegLoader.cpp - windows only jpeg file decoder
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <string>
#define _USE_MATH_DEFINES
#include <cmath>

// windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "cJpegLoader.h"

#include "../common/date.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#include "../include/jpeglib/jpeglib.h"

#pragma comment(lib,"turbojpeg-static.lib")

using namespace std;
using namespace chrono;
//}}}
//{{{  defines
//{{{  tags x
// tags
#define TAG_INTEROP_INDEX          0x0001
#define TAG_INTEROP_VERSION        0x0002
//}}}
//{{{  tags 1xx
#define TAG_IMAGE_WIDTH            0x0100
#define TAG_IMAGE_LENGTH           0x0101
#define TAG_BITS_PER_SAMPLE        0x0102
#define TAG_COMPRESSION            0x0103
#define TAG_PHOTOMETRIC_INTERP     0x0106
#define TAG_FILL_ORDER             0x010A
#define TAG_DOCUMENT_NAME          0x010D
#define TAG_IMAGE_DESCRIPTION      0x010E
#define TAG_MAKE                   0x010F
#define TAG_MODEL                  0x0110
#define TAG_SRIP_OFFSET            0x0111
#define TAG_ORIENTATION            0x0112
#define TAG_SAMPLES_PER_PIXEL      0x0115
#define TAG_ROWS_PER_STRIP         0x0116
#define TAG_STRIP_BYTE_COUNTS      0x0117
#define TAG_X_RESOLUTION           0x011A
#define TAG_Y_RESOLUTION           0x011B
#define TAG_PLANAR_CONFIGURATION   0x011C
#define TAG_RESOLUTION_UNIT        0x0128
#define TAG_TRANSFER_FUNCTION      0x012D
#define TAG_SOFTWARE               0x0131
#define TAG_DATETIME               0x0132
#define TAG_ARTIST                 0x013B
#define TAG_WHITE_POINT            0x013E
#define TAG_PRIMARY_CHROMATICITIES 0x013F
#define TAG_TRANSFER_RANGE         0x0156
//}}}
//{{{  tags 2xx
#define TAG_JPEG_PROC              0x0200
#define TAG_THUMBNAIL_OFFSET       0x0201
#define TAG_THUMBNAIL_LENGTH       0x0202
#define TAG_Y_CB_CR_COEFFICIENTS   0x0211
#define TAG_Y_CB_CR_SUB_SAMPLING   0x0212
#define TAG_Y_CB_CR_POSITIONING    0x0213
#define TAG_REFERENCE_BLACK_WHITE  0x0214
//}}}
//{{{  tags 1xxx
#define TAG_RELATED_IMAGE_WIDTH    0x1001
#define TAG_RELATED_IMAGE_LENGTH   0x1002
//}}}
//{{{  tags 8xxx
#define TAG_CFA_REPEAT_PATTERN_DIM 0x828D
#define TAG_CFA_PATTERN1           0x828E
#define TAG_BATTERY_LEVEL          0x828F
#define TAG_COPYRIGHT              0x8298
#define TAG_EXPOSURETIME           0x829A
#define TAG_FNUMBER                0x829D
#define TAG_IPTC_NAA               0x83BB
#define TAG_EXIF_OFFSET            0x8769
#define TAG_INTER_COLOR_PROFILE    0x8773
#define TAG_EXPOSURE_PROGRAM       0x8822
#define TAG_SPECTRAL_SENSITIVITY   0x8824
#define TAG_GPSINFO                0x8825
#define TAG_ISO_EQUIVALENT         0x8827
#define TAG_OECF                   0x8828
//}}}
//{{{  tags 9xxx
#define TAG_EXIF_VERSION           0x9000
#define TAG_DATETIME_ORIGINAL      0x9003
#define TAG_DATETIME_DIGITIZED     0x9004
#define TAG_COMPONENTS_CONFIG      0x9101
#define TAG_CPRS_BITS_PER_PIXEL    0x9102
#define TAG_SHUTTERSPEED           0x9201
#define TAG_APERTURE               0x9202
#define TAG_BRIGHTNESS_VALUE       0x9203
#define TAG_EXPOSURE_BIAS          0x9204
#define TAG_MAXAPERTURE            0x9205
#define TAG_SUBJECT_DISTANCE       0x9206
#define TAG_METERING_MODE          0x9207
#define TAG_LIGHT_SOURCE           0x9208
#define TAG_FLASH                  0x9209
#define TAG_FOCALLENGTH            0x920A
#define TAG_SUBJECTAREA            0x9214
#define TAG_MAKER_NOTE             0x927C
#define TAG_USERCOMMENT            0x9286
#define TAG_SUBSEC_TIME            0x9290
#define TAG_SUBSEC_TIME_ORIG       0x9291
#define TAG_SUBSEC_TIME_DIG        0x9292

#define TAG_WINXP_TITLE            0x9c9b // Windows XP - not part of exif standard.
#define TAG_WINXP_COMMENT          0x9c9c // Windows XP - not part of exif standard.
#define TAG_WINXP_AUTHOR           0x9c9d // Windows XP - not part of exif standard.
#define TAG_WINXP_KEYWORDS         0x9c9e // Windows XP - not part of exif standard.
#define TAG_WINXP_SUBJECT          0x9c9f // Windows XP - not part of exif standard.
//}}}
//{{{  tags Axxx
#define TAG_FLASH_PIX_VERSION      0xA000
#define TAG_COLOR_SPACE            0xA001
#define TAG_PIXEL_X_DIMENSION      0xA002
#define TAG_PIXEL_Y_DIMENSION      0xA003
#define TAG_RELATED_AUDIO_FILE     0xA004
#define TAG_INTEROP_OFFSET         0xA005
#define TAG_FLASH_ENERGY           0xA20B
#define TAG_SPATIAL_FREQ_RESP      0xA20C
#define TAG_FOCAL_PLANE_XRES       0xA20E
#define TAG_FOCAL_PLANE_YRES       0xA20F
#define TAG_FOCAL_PLANE_UNITS      0xA210
#define TAG_SUBJECT_LOCATION       0xA214
#define TAG_EXPOSURE_INDEX         0xA215
#define TAG_SENSING_METHOD         0xA217
#define TAG_FILE_SOURCE            0xA300
#define TAG_SCENE_TYPE             0xA301
#define TAG_CFA_PATTERN            0xA302
#define TAG_CUSTOM_RENDERED        0xA401
#define TAG_EXPOSURE_MODE          0xA402
#define TAG_WHITEBALANCE           0xA403
#define TAG_DIGITALZOOMRATIO       0xA404
#define TAG_FOCALLENGTH_35MM       0xA405
#define TAG_SCENE_CAPTURE_TYPE     0xA406
#define TAG_GAIN_CONTROL           0xA407
#define TAG_CONTRAST               0xA408
#define TAG_SATURATION             0xA409
#define TAG_SHARPNESS              0xA40A
#define TAG_DISTANCE_RANGE         0xA40C
#define TAG_IMAGE_UNIQUE_ID        0xA420
//}}}

#define FMT_BYTE       1
//#define FMT_STRING     2
#define FMT_USHORT     3
#define FMT_ULONG      4
#define FMT_URATIONAL  5
#define FMT_SBYTE      6
#define FMT_UNDEFINED  7
#define FMT_SSHORT     8
#define FMT_SLONG      9
#define FMT_SRATIONAL 10
#define FMT_SINGLE    11
#define FMT_DOUBLE    12
//}}}
const uint8_t kBytesPerFormat[13] = { 0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8 };

//{{{
bool cJpegLoader::loadInfo (uint8_t* buf, size_t bufLen) {
// find and read APP1 EXIF marker, return true if thumbnail
// - !!! make more resilient to bytes running out etc !!!
// - !!! update of info strings is barely threadSafe !!!

  mThumbnailOffset = 0;
  mThumbnailLen = 0;

  if (bufLen < 4) {
    //{{{  return false if not enough bytes for first marker
    cLog::log (LOGERROR, "parseHeader not enough bytes ");
    return false;
    }
    //}}}

  uint8_t* soiBuf = buf;
  if (getExifWord (buf, false) != 0xFFD8) {
    //{{{  return false if no SOI marker
    cLog::log (LOGERROR, "parseHeader no soi marker");
    return false;
    }
    //}}}
  buf += 2;

  uint16_t marker = getExifWord (buf, false);
  buf += 2;

  uint16_t markerLength = getExifWord (buf, false);
  buf += 2;

  string markerString = "";
  uint8_t* exifIdbuf = ((marker == 0xFFE1) && (markerLength < bufLen-6)) ? buf : nullptr;
  //{{{  parse markers, SOF marker for imageSize, until SOS, should abort if no more bytes

  while ((marker >= 0xFF00) && (marker != 0xFFDA)) {
    // until invalid marker or SOS
    markerString += getMarkerMnemomic (marker, markerLength);
    if ((marker == 0xFFC0) || (marker == 0xFFC2)) {
      mImageSize.height = getExifWord (buf+1, false);
      mImageSize.width = getExifWord (buf+3, false);
      }
    buf += markerLength-2;

    marker = getExifWord (buf, false);
    buf += 2;

    markerLength = getExifWord (buf, false);
    buf += 2;
    };

  // add terminator marker
  markerString += getMarkerMnemomic (marker, markerLength);
  //}}}

  mMarkerString = markerString;

  if (exifIdbuf) {
    //   check exifId, ignore trailing two nulls
    if (getExifLong (exifIdbuf, false) != 0x45786966) {
      //{{{  return false if not EXIF
      cLog::log (LOGERROR, "loadInfo EXIF00 ident error");
      return false;
      }
      //}}}
    exifIdbuf += 6;
    uint8_t* offsetBasebuf = exifIdbuf;

    bool intelEndian = getExifWord (exifIdbuf, false) == 0x4949;
    exifIdbuf += 2;

    if (getExifWord (exifIdbuf, intelEndian) != 0x002a) {
      //{{{  return false if no 0x002a word
      cLog::log (LOGERROR, "loadInfo EXIF 2a error");
      return false;
      }
      //}}}
    exifIdbuf += 2;

    uint32_t firstOffset = getExifLong (exifIdbuf, intelEndian);
    if (firstOffset != 8) {
      //{{{  return false if unusual firstOffset,
      cLog::log (LOGERROR, fmt::format ("loadInfo firstOffset warning - {}", firstOffset));
      return false;
      }
      //}}}
    exifIdbuf += 4;

    parseExifDirectory (offsetBasebuf, exifIdbuf, intelEndian);

    // adjust mThumbnailOffset to be from soiMarker
    mThumbnailOffset += uint32_t(offsetBasebuf - soiBuf);
    return mThumbnailLen > 0;
    }
  else // no thumbnail
    return false;
  }
//}}}
//{{{
uint8_t* cJpegLoader::loadThumbnail (uint8_t* buf, size_t bufLen, cPoint thumbnailSize) {

  uint8_t* thumbnailBuf = buf;

  //if (mThumbnailOffset) {
    // found thumbnailnail in loadInfo exif
  //  buf += mThumbnailOffset;
  //  bufLen = mThumbnailLen;
  //  }

  if ((thumbnailBuf[0] != 0xFF) || (thumbnailBuf[1] != 0xD8)) {
    //{{{  no SOI marker, return
    cLog::log (LOGERROR, "loadThumbnail - no SOI marker ");
    return 0;
    }
    //}}}

  struct jpeg_error_mgr jerr;
  struct jpeg_decompress_struct cinfo;
  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_decompress (&cinfo);

  jpeg_mem_src (&cinfo, buf, (unsigned long)bufLen);
  jpeg_read_header (&cinfo, true);

  // adjust scale to match thumbnailSize
  while ((cinfo.scale_denom < 8) &&
         ((cinfo.image_width/cinfo.scale_denom > ((uint32_t)thumbnailSize.x * 3 / 2)) ||
           cinfo.image_height/cinfo.scale_denom > ((uint32_t)thumbnailSize.y * 3 / 2)))
    cinfo.scale_denom *= 2;

  cinfo.out_color_space = JCS_EXT_RGBA;
  jpeg_start_decompress (&cinfo);

  uint32_t pitch = cinfo.output_components * cinfo.output_width;
  uint8_t* pixels = (uint8_t*)malloc (pitch * cinfo.output_height);
  while (cinfo.output_scanline < cinfo.output_height) {
    uint8_t* lineArray[1];
    lineArray[0] = pixels + (cinfo.output_scanline * pitch);
    jpeg_read_scanlines (&cinfo, lineArray, 1);
    }

  mThumbnailSize.width = cinfo.output_width;
  mThumbnailSize.height = cinfo.output_height;

  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);

  return pixels;
  }
//}}}
//{{{
uint8_t* cJpegLoader::loadImage (uint8_t* buf, size_t bufLen, int scale) {

  mLoadScale = scale;
  if ((buf[0] != 0xFF) || (buf[1] != 0xD8)) {
    //{{{  no SOI marker, return false
    cLog::log (LOGERROR, "loadFullBitmap no SOI marker - ");
    return 0;
    }
    //}}}

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_decompress (&cinfo);

  jpeg_mem_src (&cinfo, buf, (unsigned long)bufLen);
  jpeg_read_header (&cinfo, true);

  cinfo.scale_denom = scale;
  cinfo.out_color_space = JCS_EXT_RGBA;
  jpeg_start_decompress (&cinfo);

  uint32_t pitch = cinfo.output_components * cinfo.output_width;
  uint8_t* pixels = (uint8_t*)malloc (pitch * cinfo.output_height);
  while (cinfo.output_scanline < cinfo.output_height) {
    uint8_t* lineArray[1];
    lineArray[0] = pixels + (cinfo.output_scanline * pitch);
    jpeg_read_scanlines (&cinfo, lineArray, 1);
    }

  mImageSize.width = cinfo.image_width;
  mImageSize.height = cinfo.image_height;
  mScaledSize.width = cinfo.output_width;
  mScaledSize.height = cinfo.output_height;

  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);

  return pixels;
  }
//}}}

//{{{
//uint8_t* cJpegLoader::compressImage (int& bufLen, float quality) {

  //struct jpeg_compress_struct cinfo;
  //struct jpeg_error_mgr jerr;

  //unsigned char* outbuffer = NULL;
  //unsigned long outsize = 0;
  //JDIMENSION num_scanlines;

  //auto time = system_clock::now();
  //cinfo.err = jpeg_std_error (&jerr);
  //jpeg_create_compress (&cinfo);

  //cinfo.in_color_space = JCS_EXT_BGRX;
  //jpeg_set_defaults (&cinfo);

  ///* Now that we know input colorspace, fix colorspace-dependent defaults */
  //jpeg_default_colorspace (&cinfo);

  ////cinfo.optimize_coding = true;
  //jpeg_set_quality (&cinfo, (int)quality, false);

  ///* Specify data destination for compression */
  //outsize = 1000000;
  //outbuffer = (unsigned char*)malloc (outsize);
  //jpeg_mem_dest (&cinfo, &outbuffer, &outsize);

  ///* Start compressor */
  //cinfo.image_width = getWidth();
  //cinfo.image_height = getHeight();
  //cinfo.input_components = 4;
  //jpeg_start_compress (&cinfo, TRUE);

  //auto bufPtr = mBgraBuf;
  ///* Process data */
  ////while (cinfo.next_scanline < cinfo.image_height) {
  //uint8_t* lineArray[1];
  //while ((int)cinfo.next_scanline < cinfo.image_height) {
    //num_scanlines = 1;
    //lineArray[0] = bufPtr;
    //jpeg_write_scanlines (&cinfo, lineArray, num_scanlines);
    //bufPtr += cinfo.input_components * cinfo.image_width;
    //}

  //bufLen = int(cinfo.dest->next_output_byte - outbuffer);
  //jpeg_finish_compress (&cinfo);
  //jpeg_destroy_compress (&cinfo);

  //auto compressTime = (float)duration_cast<milliseconds>(system_clock::now() - time).count();
  //cLog::log (LOGNOTICE, fmt::format ("compress {} {} took {}ms", outsize, bufLen, compressTime));

  //return outbuffer;
  //}
//}}}

// private static
//{{{
string cJpegLoader::getMarkerMnemomic (uint16_t marker, uint16_t markerLength) {

  switch (marker) {
    case 0xFFC0: return fmt::format ("sof0:{} ", markerLength);
    case 0xFFC2: return fmt::format ("sof2:{} ", markerLength);
    case 0xFFC4: return fmt::format ("dht:{} ", markerLength);
    case 0xFFD8: return fmt::format ("soi:{} ", markerLength);
    case 0xFFDA: return fmt::format ("sos:{} ", markerLength);
    case 0xFFDB: return fmt::format ("dqt:{} ", markerLength);
    case 0xFFDD: return fmt::format ("dri:{} ", markerLength);
    case 0xFFE0: return fmt::format ("app0:{} ", markerLength);
    case 0xFFE1: return fmt::format ("app1:{} ", markerLength);
    case 0xFFE2: return fmt::format ("app2:{} ", markerLength);
    case 0xFFE3: return fmt::format ("app3:{} ", markerLength);
    case 0xFFE4: return fmt::format ("app4:{} ", markerLength);
    case 0xFFE5: return fmt::format ("app5:{} ", markerLength);
    case 0xFFE6: return fmt::format ("app6:{} ", markerLength);
    case 0xFFE7: return fmt::format ("app7:{} ", markerLength);
    case 0xFFE8: return fmt::format ("app8:{} ", markerLength);
    case 0xFFE9: return fmt::format ("app9:{} ", markerLength);
    case 0xFFEA: return fmt::format ("app10:{} ", markerLength);
    case 0xFFEB: return fmt::format ("app11:{} ", markerLength);
    case 0xFFEC: return fmt::format ("app12:{} ", markerLength);
    case 0xFFED: return fmt::format ("app13:{} ", markerLength);
    case 0xFFEE: return fmt::format ("app14:{} ", markerLength);
    case 0xFFFE: return fmt::format ("com:{} ", markerLength);
    default: return fmt::format ("{:4x}:{} ", marker, markerLength);
    }
  }
//}}}
//{{{
time_point<system_clock> cJpegLoader::getExifTimePoint (const string& isoTimeString) {
// parse ISO time format from string

  time_point<system_clock> timePoint;
  istringstream inputStream (isoTimeString);
  inputStream >> date::parse ("%Y:%m:%d %T", timePoint);
  return timePoint;
  }
//}}}

// private member
//{{{
void cJpegLoader::parseExifDirectory (uint8_t* offsetBasePtr, uint8_t* ptr, bool intelEndian) {

  uint16_t numDirectoryEntries = getExifWord (ptr, intelEndian);
  ptr += 2;

  for (auto entry = 0; entry < numDirectoryEntries; entry++) {
    uint16_t tag = getExifWord (ptr, intelEndian);
    uint16_t format = getExifWord (ptr+2, intelEndian);
    uint32_t components = getExifLong (ptr+4, intelEndian);
    uint32_t offset = getExifLong (ptr+8, intelEndian);
    uint32_t bytes = components * kBytesPerFormat[format];
    uint8_t* valuePtr = (bytes <= 4) ? ptr+8 : offsetBasePtr + offset;
    ptr += 12;

    uint32_t numerator;
    uint32_t denominator;
    switch (tag) {
      case TAG_EXIF_OFFSET:
        parseExifDirectory (offsetBasePtr, offsetBasePtr + offset, intelEndian); break;

      case TAG_ORIENTATION:
        mOrientation = offset; break;
      case TAG_APERTURE:
        mAperture = exp(float(getExifSignedRational (valuePtr, intelEndian, numerator, denominator) * log(2) * 0.5)); break;
      case TAG_FOCALLENGTH:
        mFocalLength = getExifSignedRational (valuePtr, intelEndian, numerator, denominator); break;
      case TAG_EXPOSURETIME:
        mExposure = getExifSignedRational (valuePtr, intelEndian, numerator, denominator); break;
      case TAG_MAKE:
        if (mMakeString.empty()) mMakeString = getExifString (valuePtr); break;
      case TAG_MODEL:
        if (mModelString.empty()) mModelString = getExifString (valuePtr); break;

      case TAG_DATETIME:
      case TAG_DATETIME_ORIGINAL:
      case TAG_DATETIME_DIGITIZED:
        mExifTimePoint = getExifTimePoint (getExifString (valuePtr)); break;

      case TAG_THUMBNAIL_OFFSET:
        mThumbnailOffset = offset; break;
      case TAG_THUMBNAIL_LENGTH:
        mThumbnailLen = offset; break;

      case TAG_GPSINFO:
        mGps = getExifGps (offsetBasePtr + offset, offsetBasePtr, intelEndian); break;
      //case TAG_MAXAPERTURE:
      //  printf ("TAG_MAXAPERTURE\n"); break;
      //case TAG_SHUTTERSPEED:
      //  printf ("TAG_SHUTTERSPEED\n"); break;
      default:;
      //  printf ("TAG %x\n", tag);
      }
    }

  auto extraDirectoryOffset = getExifLong (ptr, intelEndian);
  if (extraDirectoryOffset > 4)
    parseExifDirectory (offsetBasePtr, offsetBasePtr + extraDirectoryOffset, intelEndian);
  }
//}}}

// exif info read utils
//{{{
uint16_t cJpegLoader::getExifWord (uint8_t* ptr, bool intelEndian) {

  return *(ptr + (intelEndian ? 1 : 0)) << 8 |  *(ptr + (intelEndian ? 0 : 1));
  }
//}}}
//{{{
uint32_t cJpegLoader::getExifLong (uint8_t* ptr, bool intelEndian) {

  return getExifWord (ptr + (intelEndian ? 2 : 0), intelEndian) << 16 |
         getExifWord (ptr + (intelEndian ? 0 : 2), intelEndian);
  }
//}}}
//{{{
float cJpegLoader::getExifSignedRational (uint8_t* ptr, bool intelEndian, uint32_t& numerator, uint32_t& denominator) {

  numerator = getExifLong (ptr, intelEndian);
  denominator = getExifLong (ptr+4, intelEndian);

  if (denominator == 0)
    return 0;
  else
    return (float)numerator / denominator;
  }
//}}}
//{{{
string cJpegLoader::getExifString (uint8_t* ptr) {
  return string ((char*)ptr);
  }
//}}}
//{{{
cJpegLoader::cGps* cJpegLoader::getExifGps (uint8_t* ptr, uint8_t* offsetBasePtr, bool intelEndian) {

  cGps* gps = new cGps();

  uint16_t numDirectoryEntries = getExifWord (ptr, intelEndian);
  ptr += 2;

  for (uint16_t entry = 0; entry < numDirectoryEntries; entry++) {
    uint16_t tag = getExifWord (ptr, intelEndian);
    uint16_t format = getExifWord (ptr+2, intelEndian);
    uint32_t components = getExifLong (ptr+4, intelEndian);
    uint32_t offset = getExifLong (ptr+8, intelEndian);
    uint32_t bytes = components * kBytesPerFormat[format];
    uint8_t* valuePtr = (bytes <= 4) ? ptr+8 : offsetBasePtr + offset;
    ptr += 12;

    uint32_t numerator;
    uint32_t denominator;
    switch (tag) {
      //{{{
      case 0x00:  // version
        gps->mVersion = getExifLong (valuePtr, intelEndian);
        break;
      //}}}
      //{{{
      case 0x01:  // latitude ref
        gps->mLatitudeRef = valuePtr[0];
        break;
      //}}}
      //{{{
      case 0x02:  // latitude
        gps->mLatitudeDeg = getExifSignedRational (valuePtr, intelEndian, numerator, denominator);
        gps->mLatitudeMin = getExifSignedRational (valuePtr + 8, intelEndian, numerator, denominator);
        gps->mLatitudeSec = getExifSignedRational (valuePtr + 16, intelEndian, numerator, denominator);
        break;
      //}}}
      //{{{
      case 0x03:  // longitude ref
        gps->mLongitudeRef = valuePtr[0];
        break;
      //}}}
      //{{{
      case 0x04:  // longitude
        gps->mLongitudeDeg = getExifSignedRational (valuePtr, intelEndian, numerator, denominator);
        gps->mLongitudeMin = getExifSignedRational (valuePtr + 8, intelEndian, numerator, denominator);
        gps->mLongitudeSec = getExifSignedRational (valuePtr + 16, intelEndian, numerator, denominator);
        break;
      //}}}
      //{{{
      case 0x05:  // altitude ref
        gps->mAltitudeRef = valuePtr[0];
        break;
      //}}}
      //{{{
      case 0x06:  // altitude
        gps->mAltitude = getExifSignedRational (valuePtr, intelEndian, numerator, denominator);
        break;
      //}}}
      //{{{
      case 0x07:  // timeStamp
        gps->mHour = getExifSignedRational (valuePtr, intelEndian, numerator, denominator),
        gps->mMinute = getExifSignedRational (valuePtr + 8, intelEndian, numerator, denominator),
        gps->mSecond = getExifSignedRational (valuePtr + 16, intelEndian, numerator, denominator);
        break;
      //}}}
      //{{{
      case 0x08:  // satellites
        printf ("TAG_gps_Satellites %s \n", getExifString (valuePtr).c_str());
        break;
      //}}}
      //{{{
      case 0x0B:  // dop
        printf ("TAG_gps_DOP %f\n", getExifSignedRational (valuePtr, intelEndian, numerator, denominator));
        break;
      //}}}
      //{{{
      case 0x10:  // direction ref
        gps->mImageDirectionRef = valuePtr[0];
        break;
      //}}}
      //{{{
      case 0x11:  // direction
        gps->mImageDirection = getExifSignedRational (valuePtr, intelEndian, numerator, denominator);
        break;
      //}}}
      //{{{
      case 0x12:  // datum
        gps->mDatum = getExifString (valuePtr);
        break;
      //}}}
      //{{{
      case 0x1D:  // date
        gps->mDate = getExifString (valuePtr);
        break;
      //}}}
      //{{{
      case 0x1B:
        //printf ("TAG_gps_ProcessingMethod\n");
        break;
      //}}}
      //{{{
      case 0x1C:
        //printf ("TAG_gps_AreaInformation\n");
        break;
      //}}}
      case 0x09: printf ("TAG_gps_status\n"); break;
      case 0x0A: printf ("TAG_gps_MeasureMode\n"); break;
      case 0x0C: printf ("TAG_gps_SpeedRef\n"); break;
      case 0x0D: printf ("TAG_gps_Speed\n"); break;
      case 0x0E: printf ("TAG_gps_TrackRef\n"); break;
      case 0x0F: printf ("TAG_gps_Track\n"); break;
      case 0x13: printf ("TAG_gps_DestLatitudeRef\n"); break;
      case 0x14: printf ("TAG_gps_DestLatitude\n"); break;
      case 0x15: printf ("TAG_gps_DestLongitudeRef\n"); break;
      case 0x16: printf ("TAG_gps_DestLongitude\n"); break;
      case 0x17: printf ("TAG_gps_DestBearingRef\n"); break;
      case 0x18: printf ("TAG_gps_DestBearing\n"); break;
      case 0x19: printf ("TAG_gps_DestDistanceRef\n"); break;
      case 0x1A: printf ("TAG_gps_DestDistance\n"); break;
      case 0x1E: printf ("TAG_gps_Differential\n"); break;
      default: printf ("unknown gps tag %x\n", tag); break;
      }
    }

  return gps;
  }
//}}}

// get cGps as string
//{{{
string cJpegLoader::cGps::getString() {

  string gpsString;

  if (!mDatum.empty())
    gpsString = mDatum + " ";

  if (mLatitudeDeg || mLatitudeMin || mLatitudeSec)
    gpsString += fmt::format ("{} {} {} {} ", mLatitudeDeg, &mLatitudeRef, mLatitudeMin, mLatitudeSec);

  if (mLongitudeDeg || mLongitudeMin || mLongitudeSec)
    gpsString += fmt::format ("{} {} {} {} ", mLongitudeDeg, &mLongitudeRef, mLongitudeMin, mLongitudeSec);

  if (mAltitude)
    gpsString += fmt::format ("alt:{}:{} ", mAltitudeRef, mAltitude);

  if (mImageDirection)
    gpsString += fmt::format ("dir:{}:{} ", &mImageDirectionRef, mImageDirection);

  if (!mDatum.empty())
    gpsString += fmt::format ("{} ", mDate);

  if (mHour || mMinute || mSecond)
    gpsString += fmt::format ("{}:{}:{}", mHour, mMinute, mSecond);

  return gpsString;
  }
//}}}
