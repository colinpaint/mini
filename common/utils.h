// utils.h
//{{{  includes
#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include "fmt/format.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
#endif
//}}}

namespace utils {
  //{{{
  inline std::string getTimeString (int64_t value, int daylightSeconds) {

    int64_t subSeconds = (value % 100);
    value /= 100;

    value += daylightSeconds;
    int64_t seconds = value % 60;
    value /= 60;

    int64_t minutes = value % 60;
    value /= 60;

    int64_t hours = value;

    if (hours > 0)
      return fmt::format ("{}:{:02d}:{:02d}:{:02d}", hours, minutes, seconds, subSeconds);
    else if (minutes > 0)
      return fmt::format ("{}:{:02d}:{:02d}", minutes, seconds, subSeconds);
    else
      return fmt::format ("{}:{:02d}", seconds, subSeconds);
    }
  //}}}
  //{{{
  inline std::string getPtsString (int64_t pts) {
  // 90khz int64_t pts - miss out zeros

    if (pts < 0)
      return "--:--:--";
    else {
      pts /= 900;
      uint32_t hs = pts % 100;

      pts /= 100;
      uint32_t secs = pts % 60;

      pts /= 60;
      uint32_t mins = pts % 60;

      pts /= 60;
      uint32_t hours = pts % 60;


      std::string hourMinString = hours ? fmt::format ("{}:{:02}", hours,mins) : fmt::format ("{}",mins);
      return fmt::format ("{}:{:02d}:{:02d}", hourMinString, secs, hs);
      }
    }
  //}}}
  //{{{
  inline std::string getFullPtsString (int64_t pts) {
  // 90khz int64_t pts  - all digits

    if (pts < 0)
      return "--:--:--:--";
    else {
      pts /= 900;
      uint32_t hs = pts % 100;

      pts /= 100;
      uint32_t secs = pts % 60;

      pts /= 60;
      uint32_t mins = pts % 60;

      pts /= 60;
      uint32_t hours = pts % 60;

      return fmt::format ("{:02d}:{:02d}:{:02d}:{:02d}", hours, mins, secs, hs);
      }
    }
  //}}}
  //{{{
  inline std::string getPtsFramesString (int64_t pts, int64_t ptsDuration) {
  // return 90khz int64_t pts - as frames.subframes

    int64_t frames = pts / ptsDuration;
    int64_t subFrames = pts % ptsDuration;
    return fmt::format ("{}.{:04d}", frames,subFrames);
    }
  //}}}

  //{{{
  inline std::string validFileString (const std::string& str, const char* inValidChars) {

    std::string validStr = str;
    #ifdef _WIN32
      for (auto i = 0u; i < strlen(inValidChars); ++i)
        validStr.erase (std::remove (validStr.begin(), validStr.end(), inValidChars[i]), validStr.end());
    #else
      (void)inValidChars;
    #endif

    return validStr;
    }
  //}}}

  //{{{
  inline std::wstring stringToWstring (const std::string& str) {
    return std::wstring (str.begin(), str.end());
    }
  //}}}

  #ifdef _WIN32
    //{{{
    inline std::string wstringToString (const std::wstring& input) {

      int required_characters = WideCharToMultiByte (CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()),
                                                     nullptr, 0, nullptr, nullptr);
      if (required_characters <= 0)
        return {};

      std::string output;
      output.resize (static_cast<size_t>(required_characters));
      WideCharToMultiByte (CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()),
                           output.data(), static_cast<int>(output.size()), nullptr, nullptr);

      return output;
      }
    //}}}
  #endif

  // old stuff
  //{{{
  //inline std::string getTimeDateString (const time_t& time) {

    //tm* localTm = localtime (&time);

    //const char day_name[][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    //const char mon_name[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                 //"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    //return dec(localTm->tm_hour,2,'0') + "." +
           //dec(localTm->tm_min,2,'0') + "." +
           //dec(localTm->tm_sec,2,'0') + " " +
           //day_name[localTm->tm_wday] + " " +
           //dec(localTm->tm_mday) + " " +
           //mon_name[localTm->tm_mon] + " " +
           //dec(1900 + localTm->tm_year);
    //}
  //}}}
  //{{{
  //inline std::string getTimeShortString (const time_t& time) {

    //tm* localTm = localtime (&time);
    //return dec(localTm->tm_hour,2,'0') + "." + dec(localTm->tm_min,2,'0');
    //}
  //}}}
  }
