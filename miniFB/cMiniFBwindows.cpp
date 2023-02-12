// miniFBwindows.cpp
//{{{  includes
#include <cstdint>

#include "cMiniFB.h"
#include <windowsx.h>

#include "../common/cLog.h"

using namespace std;
//}}}
#ifdef USE_WINTAB
  #include "winTab.h"
  #define PACKETDATA PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_TIME
  #include "pktDef.h"
#endif

namespace {
  #ifdef USE_WINTAB
    //{{{  use wintab
    //{{{
    // Use this enum in conjunction with winTab->Buttons to check for tablet button presses.
    //  e.g. To check for lower pen button press, use:
    //    if (winTab->Buttons & eWinTabButtons_Pen_Lower) {
    //      Lower button is pressed
    enum eWinTabButtons_ {
      eWinTabButtons_Pen_Touch = 1, // Pen touching tablet
      eWinTabButtons_Pen_Lower = 2, // Lower pen button pressed
      eWinTabButtons_Pen_Upper = 4, // Upper pen button pressed
      };
    //}}}
    //{{{  WTfunction addresses
    typedef UINT (WINAPI* WTINFOA) (UINT, UINT, LPVOID);
    typedef HCTX (WINAPI* WTOPENA) (HWND, LPLOGCONTEXTA, BOOL);

    typedef bool (WINAPI* WTGETA) (HCTX, LPLOGCONTEXTA);
    typedef bool (WINAPI* WTSETA) (HCTX, LPLOGCONTEXTA);

    typedef bool (WINAPI* WTCLOSE) (HCTX);
    typedef bool (WINAPI* WTPACKET) (HCTX, UINT, LPVOID);
    typedef bool (WINAPI* WTENABLE) (HCTX, BOOL);
    typedef bool (WINAPI* WTOVERLAP) (HCTX, BOOL);

    typedef bool (WINAPI* WTSAVE) (HCTX, LPVOID);
    typedef bool (WINAPI* WTCONFIG) (HCTX, HWND);
    typedef HCTX (WINAPI* WTRESTORE) (HWND, LPVOID, BOOL);

    typedef bool (WINAPI* WTEXTSET) (HCTX, UINT, LPVOID);
    typedef bool (WINAPI* WTEXTGET) (HCTX, UINT, LPVOID);

    typedef bool (WINAPI* WTQUEUESIZESET) (HCTX, int);
    typedef int  (WINAPI* WTDATAPEEK) (HCTX, UINT, UINT, int, LPVOID, LPINT);
    typedef int  (WINAPI* WTPACKETSGET) (HCTX, int, LPVOID);

    typedef HMGR (WINAPI* WTMGROPEN) (HWND, UINT);
    typedef bool (WINAPI* WTMGRCLOSE) (HMGR);
    typedef HCTX (WINAPI* WTMGRDEFCONTEXT) (HMGR, BOOL);
    typedef HCTX (WINAPI* WTMGRDEFCONTEXTEX) (HMGR, UINT, BOOL);
    //}}}
    //{{{
    struct sWinTabInfo {
      int32_t mPosX;
      int32_t mPosY;
      float mPressure; // Range: 0.0f to 1.0f
      int32_t mButtons;  // Bit field. Use with the eWinTabButtons_ enum.

      DWORD mTime;

      int32_t mRangeX;
      int32_t mRangeY;
      int32_t mMaxPressure;

      HINSTANCE mDll;
      HCTX mContext;

      WTINFOA           mWTInfoA;
      WTOPENA           mWTOpenA;

      WTGETA            mWTGetA;
      WTSETA            mWTSetA;

      WTCLOSE           mWTClose;
      WTPACKET          mWTPacket;
      WTENABLE          mWTEnable;

      WTOVERLAP         mWTOverlap;
      WTSAVE            mWTSave;
      WTCONFIG          mWTConfig;
      WTRESTORE         mWTRestore;

      WTEXTSET          mWTExtSet;
      WTEXTGET          mWTExtGet;

      WTQUEUESIZESET    mWTQueueSizeSet;
      WTDATAPEEK        mWTDataPeek;
      WTPACKETSGET      mWTPacketsGet;

      WTMGROPEN         mWTMgrOpen;
      WTMGRCLOSE        mWTMgrClose;
      WTMGRDEFCONTEXT   mWTMgrDefContext;
      WTMGRDEFCONTEXTEX mWTMgrDefContextEx;
      };
    //}}}
    sWinTabInfo* gWinTab = nullptr;
    //{{{
    bool winTabLoad (HWND window) {

      gWinTab = (sWinTabInfo*)calloc (1, sizeof(sWinTabInfo));
      if (!gWinTab)
        return false;

      // load wintab32.dll, get function addresses
      gWinTab->mDll = LoadLibraryA ("Wintab32.dll");
      if (!gWinTab->mDll) {
        //{{{  error, return
        cLog::log (LOGERROR, fmt::format ("Wintab32.dll not found"));
        return false;
        }
        //}}}

      //{{{  get WT function addresses
      gWinTab->mWTInfoA = (WTINFOA)GetProcAddress (gWinTab->mDll, "WTInfoA");
      gWinTab->mWTOpenA = (WTOPENA)GetProcAddress (gWinTab->mDll, "WTOpenA");

      gWinTab->mWTGetA = (WTGETA)GetProcAddress (gWinTab->mDll, "WTGetA");
      gWinTab->mWTSetA = (WTSETA)GetProcAddress (gWinTab->mDll, "WTSetA");

      gWinTab->mWTClose = (WTCLOSE)GetProcAddress (gWinTab->mDll, "WTClose");
      gWinTab->mWTPacket = (WTPACKET)GetProcAddress (gWinTab->mDll, "WTPacket");
      gWinTab->mWTEnable = (WTENABLE)GetProcAddress (gWinTab->mDll, "WTEnable");
      gWinTab->mWTOverlap = (WTOVERLAP)GetProcAddress (gWinTab->mDll, "WTOverlap");

      gWinTab->mWTSave = (WTSAVE)GetProcAddress (gWinTab->mDll, "WTSave");
      gWinTab->mWTConfig = (WTCONFIG)GetProcAddress (gWinTab->mDll, "WTConfig");
      gWinTab->mWTRestore = (WTRESTORE)GetProcAddress (gWinTab->mDll, "WTRestore");

      gWinTab->mWTExtSet = (WTEXTSET)GetProcAddress (gWinTab->mDll, "WTExtSet");
      gWinTab->mWTExtGet = (WTEXTGET)GetProcAddress (gWinTab->mDll, "WTExtGet");

      gWinTab->mWTQueueSizeSet = (WTQUEUESIZESET)GetProcAddress (gWinTab->mDll, "WTQueueSizeSet");
      gWinTab->mWTDataPeek = (WTDATAPEEK)GetProcAddress (gWinTab->mDll, "WTDataPeek");
      gWinTab->mWTPacketsGet = (WTPACKETSGET)GetProcAddress (gWinTab->mDll, "WTPacketsGet");

      gWinTab->mWTMgrOpen = (WTMGROPEN)GetProcAddress (gWinTab->mDll, "WTMgrOpen");
      gWinTab->mWTMgrClose = (WTMGRCLOSE)GetProcAddress (gWinTab->mDll, "WTMgrClose");

      gWinTab->mWTMgrDefContext = (WTMGRDEFCONTEXT)GetProcAddress (gWinTab->mDll, "WTMgrDefContext");
      gWinTab->mWTMgrDefContextEx = (WTMGRDEFCONTEXTEX)GetProcAddress (gWinTab->mDll, "WTMgrDefContextEx");                \
      //}}}

      if (!gWinTab->mWTInfoA (0, 0, NULL)) {
        //{{{  error, return
        cLog::log (LOGERROR, fmt::format ("winTab services not available"));
        return false;
        }
        //}}}

      LOGCONTEXTA logContext = {0};
      gWinTab->mWTInfoA (WTI_DEFSYSCTX, 0, &logContext);
      logContext.lcPktData = PACKETDATA;
      logContext.lcOptions |= CXO_MESSAGES | CXO_SYSTEM; // | CXO_PEN;
      logContext.lcPktMode = 0;   // absolute mode
      logContext.lcMoveMask = PACKETDATA;
      logContext.lcBtnUpMask = logContext.lcBtnDnMask;

      logContext.lcOutOrgX = 0;
      logContext.lcOutOrgY = 0;
      logContext.lcOutExtX = GetSystemMetrics (SM_CXSCREEN);
      logContext.lcOutExtY = -GetSystemMetrics (SM_CYSCREEN);

      logContext.lcSysOrgX = 0;
      logContext.lcSysOrgY = 0;
      logContext.lcSysExtX = GetSystemMetrics (SM_CXSCREEN);
      logContext.lcSysExtY = GetSystemMetrics (SM_CYSCREEN);

      gWinTab->mContext = gWinTab->mWTOpenA (window, &logContext, TRUE);
      if (!gWinTab->mContext) {
        //{{{  error, return
        cLog::log (LOGERROR, fmt::format ("winTab failed to open"));
        return false;
        }
        //}}}

      AXIS rangeX = {0};
      gWinTab->mWTInfoA (WTI_DEVICES, DVC_X, &rangeX);
      gWinTab->mRangeX = rangeX.axMax;

      AXIS rangeY = {0};
      gWinTab->mWTInfoA (WTI_DEVICES, DVC_Y, &rangeY);
      gWinTab->mRangeY = rangeY.axMax;

      AXIS pressure = {0};
      gWinTab->mWTInfoA (WTI_DEVICES, DVC_NPRESSURE, &pressure);
      gWinTab->mMaxPressure = pressure.axMax;

      return true;
      }
    //}}}
    //{{{
    void winTabUnload() {

      if (gWinTab->mContext)
        gWinTab->mWTClose (gWinTab->mContext);

      if (gWinTab->mDll)
        FreeLibrary (gWinTab->mDll);

      free (gWinTab);

      gWinTab = nullptr;
      }
    //}}}
    //}}}
  #endif

  int16_t gKeycodes[512] = { 0 };
  //{{{  dpi
  // Copied (and modified) from Windows Kit 10 to avoid setting _WIN32_WINNT to a higher version
  enum mfb_PROCESS_DPI_AWARENESS {
    mfb_PROCESS_DPI_UNAWARE = 0,
    mfb_PROCESS_SYSTEM_DPI_AWARE = 1,
    mfb_PROCESS_PER_MONITOR_DPI_AWARE = 2
    };

  enum mfb_MONITOR_DPI_TYPE {
    mfb_MDT_EFFECTIVE_DPI = 0,
    mfb_MDT_ANGULAR_DPI   = 1,
    mfb_MDT_RAW_DPI       = 2,
    mfb_MDT_DEFAULT       = mfb_MDT_EFFECTIVE_DPI
    };

  #define mfb_DPI_AWARENESS_CONTEXT_UNAWARE              ((HANDLE) -1)
  #define mfb_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((HANDLE) -2)
  #define mfb_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((HANDLE) -3)
  #define mfb_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE) -4)
  #define mfb_DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED    ((HANDLE) -5)

  // user32.dll
  typedef UINT(WINAPI* PFN_GetDpiForWindow)(HWND);
  typedef BOOL(WINAPI* PFN_EnableNonClientDpiScaling)(HWND);
  typedef BOOL(WINAPI* PFN_SetProcessDPIAware)(void);
  typedef BOOL(WINAPI* PFN_SetProcessDpiAwarenessContext)(HANDLE);

  HMODULE gUser32dll = 0;
  PFN_GetDpiForWindow gGetDpiForWindow = 0;
  PFN_EnableNonClientDpiScaling gEnableNonClientDpiScaling = 0;
  PFN_SetProcessDPIAware gSetProcessDPIAware = 0;
  PFN_SetProcessDpiAwarenessContext gSetProcessDpiAwarenessContext = 0;

  // shcore.dll
  typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(mfb_PROCESS_DPI_AWARENESS);
  typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, mfb_MONITOR_DPI_TYPE, UINT *, UINT *);

  HMODULE gShCoreDll = 0;
  PFN_GetDpiForMonitor gGetDpiForMonitor = 0;
  PFN_SetProcessDpiAwareness gSetProcessDpiAwareness = 0;

  //{{{
  // NOT Thread safe. Just convenient (Don't do this at home guys)
  char* getErrorMessage() {

    static char buffer[256];

    buffer[0] = 0;
    FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL,  // Not used with FORMAT_MESSAGE_FROM_SYSTEM
                   GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buffer,
                   sizeof(buffer),
                   NULL);
    return buffer;
    }
  //}}}

  //{{{
  void dpiAware() {

    if (gSetProcessDpiAwarenessContext) {
      if (!gSetProcessDpiAwarenessContext (mfb_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        uint32_t error = GetLastError();
        if (error == ERROR_INVALID_PARAMETER) {
          error = NO_ERROR;
          if (!gSetProcessDpiAwarenessContext (mfb_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
            error = GetLastError();
          }
        if (error != NO_ERROR)
          cLog::log (LOGERROR, fmt::format ("SetProcessDpiAwarenessContext failed {}", getErrorMessage()));
        }
      }

    else if (gSetProcessDpiAwareness) {
      if (gSetProcessDpiAwareness(mfb_PROCESS_PER_MONITOR_DPI_AWARE) != S_OK)
        cLog::log (LOGERROR, fmt::format ("SetProcessDpiAwareness failed {}", getErrorMessage()));
      }

    else if (gSetProcessDPIAware) {
      if (!gSetProcessDPIAware())
        cLog::log (LOGERROR, fmt::format ("SetProcessDPIAware failed {}", getErrorMessage()));
      }

    }
  //}}}
  //{{{
  void getWindowsMonitorScale (HWND hWnd, float* scale_x, float* scale_y) {

    UINT x, y;

    if (gGetDpiForMonitor) {
      HMONITOR monitor = MonitorFromWindow (hWnd, MONITOR_DEFAULTTONEAREST);
      gGetDpiForMonitor (monitor, mfb_MDT_EFFECTIVE_DPI, &x, &y);
      }

    else {
      const HDC dc = GetDC (hWnd);
      x = GetDeviceCaps (dc, LOGPIXELSX);
      y = GetDeviceCaps (dc, LOGPIXELSY);
      ReleaseDC (NULL, dc);
      }

    if (scale_x) {
      *scale_x = x / (float)USER_DEFAULT_SCREEN_DPI; // 96dpi
      if (*scale_x == 0)
        *scale_x = 1;
      }

    if (scale_y) {
      *scale_y = y / (float)USER_DEFAULT_SCREEN_DPI;  // 96dpi
      if (*scale_y == 0)
        *scale_y = 1;
      }
    }
  //}}}

  //{{{
  void loadFunctions() {

    if (!gUser32dll) {
      gUser32dll = LoadLibraryA ("user32.dll");
      if (gUser32dll) {
        gSetProcessDPIAware = (PFN_SetProcessDPIAware)GetProcAddress (gUser32dll, "SetProcessDPIAware");
        gSetProcessDpiAwarenessContext = (PFN_SetProcessDpiAwarenessContext)GetProcAddress (gUser32dll, "SetProcessDpiAwarenessContext");
        gGetDpiForWindow = (PFN_GetDpiForWindow)GetProcAddress (gUser32dll, "GetDpiForWindow");
        gEnableNonClientDpiScaling = (PFN_EnableNonClientDpiScaling)GetProcAddress (gUser32dll, "EnableNonClientDpiScaling");
        }
      }

    if (gShCoreDll) {
      gShCoreDll = LoadLibraryA ("shcore.dll");
      if (!gShCoreDll) {
        gSetProcessDpiAwareness = (PFN_SetProcessDpiAwareness)GetProcAddress (gShCoreDll, "SetProcessDpiAwareness");
        gGetDpiForMonitor = (PFN_GetDpiForMonitor)GetProcAddress (gShCoreDll, "GetDpiForMonitor");
        }
      }
    }
  //}}}
  //}}}
  //{{{
  uint32_t translateMod() {

    uint32_t mods = 0;

    if (GetKeyState(VK_SHIFT) & 0x8000)
      mods |= KB_MOD_SHIFT;

    if (GetKeyState(VK_CONTROL) & 0x8000)
      mods |= KB_MOD_CONTROL;

    if (GetKeyState(VK_MENU) & 0x8000)
      mods |= KB_MOD_ALT;

    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
      mods |= KB_MOD_SUPER;

    if (GetKeyState(VK_CAPITAL) & 1)
      mods |= KB_MOD_CAPS_LOCK;

    if (GetKeyState(VK_NUMLOCK) & 1)
      mods |= KB_MOD_NUM_LOCK;

    return mods;
    }
  //}}}
  //{{{
  eKey translateKey (unsigned int wParam, unsigned long lParam) {

    if (wParam == VK_CONTROL) {
      if (lParam & 0x01000000)
        return KB_KEY_RIGHT_CONTROL;

      DWORD time = GetMessageTime();

      MSG next;
      if (PeekMessageW (&next, 0x0, 0, 0, PM_NOREMOVE))
        if (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN ||
            next.message == WM_KEYUP || next.message == WM_SYSKEYUP)
          if (next.wParam == VK_MENU && (next.lParam & 0x01000000) && next.time == time)
            return KB_KEY_UNKNOWN;

      return KB_KEY_LEFT_CONTROL;
      }

    if (wParam == VK_PROCESSKEY)
      return KB_KEY_UNKNOWN;

    return (eKey)gKeycodes[HIWORD(lParam) & 0x1FF];
    }
  //}}}
  //{{{
  LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    cMiniFB* miniFB = (cMiniFB*)GetWindowLongPtr (hWnd, GWLP_USERDATA);
    switch (message) {
      //{{{
      case WM_NCCREATE:
        if (gEnableNonClientDpiScaling)
          gEnableNonClientDpiScaling (hWnd);

        return DefWindowProc (hWnd, message, wParam, lParam);
      //}}}
      //{{{
      case WM_SIZE:
        if (miniFB) {
          if (wParam == SIZE_MINIMIZED)
            return 0;

          float scale_x, scale_y;
          getWindowsMonitorScale (hWnd, &scale_x, &scale_y);
          miniFB->window_width = GET_X_LPARAM(lParam);
          miniFB->window_height =  GET_Y_LPARAM(lParam);
          miniFB->resizeDst (miniFB->window_width, miniFB->window_height);

          miniFB->resizeGL();
          if (miniFB->window_width && miniFB->window_height) {
            miniFB->windowScaledWidth  = (uint32_t)(miniFB->window_width  / scale_x);
            miniFB->windowScaledHeight = (uint32_t)(miniFB->window_height / scale_y);
            if (miniFB->resizeFunc)
              miniFB->resizeFunc (miniFB);
            }
          }

        break;
      //}}}

      //{{{
      case WM_CLOSE:
        if (miniFB) {
          bool destroy = false;

          // Obtain a confirmation of close
          if (!miniFB->closeFunc || miniFB->closeFunc (miniFB))
            destroy = true;

          if (destroy) {
            miniFB->closed = true;
            if (miniFB)
              DestroyWindow (miniFB->window);
            }
          }

        break;
      //}}}
      //{{{
      case WM_DESTROY:
        if (miniFB)
          miniFB->closed = true;

        break;
      //}}}

      //{{{
      case WM_SETFOCUS:
        if (miniFB) {
          miniFB->isActive = true;
          if (miniFB->activeFunc)
            miniFB->activeFunc (miniFB);
          }

        break;
      //}}}
      //{{{
      case WM_KILLFOCUS:
        if (miniFB) {
          miniFB->isActive = false;
          if (miniFB->activeFunc)
            miniFB->activeFunc (miniFB);
          }

        break;
      //}}}

      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      //{{{
      case WM_KEYUP:
        if (miniFB) {
          eKey keyCode = translateKey ((unsigned int)wParam, (unsigned long)lParam);
          miniFB->isPressed = !((lParam >> 31) & 1);
          miniFB->modifierKeys = translateMod();

          if (keyCode == KB_KEY_UNKNOWN)
            return 0;

          miniFB->keyCode = keyCode;
          miniFB->keyStatus[keyCode] = (uint8_t)miniFB->isPressed;
          if (miniFB->keyFunc)
            miniFB->keyFunc (miniFB);
          }

        break;
      //}}}

      case WM_CHAR:
      //{{{
      case WM_SYSCHAR:
        {
        static WCHAR highSurrogate = 0;

        if (miniFB) {
          if (wParam >= 0xd800 && wParam <= 0xdbff)
            highSurrogate = (WCHAR) wParam;
          else {
            miniFB->codepoint = 0;
            if (wParam >= 0xdc00 && wParam <= 0xdfff) {
              if (highSurrogate != 0) {
                miniFB->codepoint += (highSurrogate - 0xd800) << 10;
                miniFB->codepoint += (WCHAR) wParam - 0xdc00;
                miniFB->codepoint += 0x10000;
                }
              }
            else
              miniFB->codepoint = (WCHAR) wParam;

            highSurrogate = 0;
            if (miniFB->charFunc)
              miniFB->charFunc (miniFB);
            }
          }
        }
        break;
      //}}}

      #ifdef USE_WINTAB
        //{{{
        case WT_PACKET:
          if ((HCTX)lParam == gWinTab->mContext) {
            PACKET packet = {0};
            if (gWinTab->mWTPacket (gWinTab->mContext, (UINT)wParam, &packet)) {
              POINT point = { 0 };
              point.x = packet.pkX;
              point.y = packet.pkY;
              ScreenToClient (hWnd, &point);

              gWinTab->mPosX = point.x;
              gWinTab->mPosY = point.y;
              gWinTab->mPressure = (float)packet.pkNormalPressure / (float)gWinTab->mMaxPressure;
              gWinTab->mButtons = packet.pkButtons;
              gWinTab->mTime = packet.pkTime;

              cLog::log (LOGminiFB, fmt::format ("WT_PACKET press:{} time:{}", gWinTab->mPressure, gWinTab->mTime));
              }
            else
              cLog::log (LOGERROR, fmt::format ("WT_PACKET no packet"));
            }
          else
            cLog::log (LOGERROR, fmt::format ("WT_PACKET wrong context"));

          break;
        //}}}
        //{{{
        case WT_PROXIMITY:
          if (lParam & 0xFFFF)
            cLog::log (LOGINFO, fmt::format ("WT_PROXIMITY in {:x}", lParam));
          else
            cLog::log (LOGINFO, fmt::format ("WT_PROXIMITY out {:x}", lParam));

          break;
        //}}}
      #endif

      //{{{
      case WM_POINTERENTER:
        miniFB->pointerInside = true;
        if (miniFB->enterFunc)
          miniFB->enterFunc (miniFB);

        break;
      //}}}
      //{{{
      case WM_POINTERLEAVE:
        miniFB->pointerInside = false;
        if (miniFB->enterFunc)
          miniFB->enterFunc (miniFB);

        break;
      //}}}
      //{{{
      case WM_POINTERDOWN:
        if (miniFB) {
          POINTER_INFO pointerInfo;
          if (GetPointerInfo (GET_POINTERID_WPARAM (wParam), &pointerInfo)) {
            if (pointerInfo.pointerType == PT_MOUSE) {
              cLog::log (LOGINFO, fmt::format ("mouseDown"));
              }
            else if (pointerInfo.pointerType == PT_PEN) {
              cLog::log (LOGINFO, fmt::format ("penDown"));
              }
            else // unused PT_TOUCH, PT_TOUCHPAD
              cLog::log (LOGERROR, fmt::format ("pointerDown - unknown type:{}", pointerInfo.pointerType));

            miniFB->pointerButtonStatus[MOUSE_BTN_1] = 1;
            miniFB->modifierKeys = translateMod();
            miniFB->isDown = 1;
            if (miniFB->buttonFunc)
              miniFB->buttonFunc (miniFB);
            }
          else
            cLog::log (LOGERROR, fmt::format ("pointerDown - no miniFB"));
          }

        break;
      //}}}
      //{{{
      case WM_POINTERUP:
        if (miniFB) {
          POINTER_INFO pointerInfo;
          if (GetPointerInfo (GET_POINTERID_WPARAM (wParam), &pointerInfo)) {
            if (pointerInfo.pointerType == PT_MOUSE) {
              cLog::log (LOGINFO, fmt::format ("mouseUp"));
              }
            else if (pointerInfo.pointerType == PT_PEN) {
              cLog::log (LOGINFO, fmt::format ("penUp"));
              }
            else // unused PT_TOUCH, PT_TOUCHPAD
              cLog::log (LOGERROR, fmt::format ("pointerUp - unknown type:{}", pointerInfo.pointerType));

            miniFB->modifierKeys = translateMod();
            miniFB->pointerButtonStatus[MOUSE_BTN_1] = 0;
            miniFB->isDown = 0;
            if (miniFB->buttonFunc)
              miniFB->buttonFunc (miniFB);
            }
          else
            cLog::log (LOGERROR, fmt::format ("pointerUp - no info"));
          }

        break;
      //}}}
      //{{{
      case WM_POINTERUPDATE:
        //{{{  pointer info
        //POINTER_INFO
        //POINTER_INPUT_TYPE         pointerType;
        //UINT32                     pointerId;
        //UINT32                     frameId;
        //POINTER_FLAGS              pointerFlags;
        //HANDLE                     sourceDevice;
        //HWND                       hwndTarget;
        //POINT                      ptPixelLocation;
        //POINT                      ptHimetricLocation;
        //POINT                      ptPixelLocationRaw;
        //POINT                      ptHimetricLocationRaw;
        //DWORD                      dwTime;
        //UINT32                     historyCount;
        //INT32                      InputData;
        //DWORD                      dwKeyStates;
        //UINT64                     PerformanceCount;
        //POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;

        ////POINTER_INPUT
          //PT_POINTER = 1,
          //PT_TOUCH = 2,
          //PT_PEN = 3,
          //PT_MOUSE = 4,
          //PT_TOUCHPAD = 5

        ////POINTER_FLAG
        //POINTER_FLAG_NONE 0x00000000
        //POINTER_FLAG_NEW 0x00000001
        //POINTER_FLAG_INRANGE 0x00000002
        //POINTER_FLAG_INCONTACT 0x00000004
        //POINTER_FLAG_FIRSTBUTTON 0x00000010
        //POINTER_FLAG_SECONDBUTTON 0x00000020
        //POINTER_FLAG_THIRDBUTTON 0x00000040
        //POINTER_FLAG_FOURTHBUTTON 0x00000080
        //POINTER_FLAG_FIFTHBUTTON 0x00000100
        //POINTER_FLAG_PRIMARY 0x00002000
        //POINTER_FLAG_CONFIDENCE 0x000004000
        //POINTER_FLAG_CANCELED 0x000008000
        //POINTER_FLAG_DOWN 0x00010000
        //POINTER_FLAG_UPDATE 0x00020000
        //POINTER_FLAG_UP 0x00040000
        //POINTER_FLAG_WHEEL 0x00080000
        //POINTER_FLAG_HWHEEL 0x00100000
        //POINTER_FLAG_CAPTURECHANGED 0x00200000
        //POINTER_FLAG_HASTRANSFORM 0x00400000

        ////POINTER_BUTTON_CHANGE_TYPE {
          //POINTER_CHANGE_NONE,
          //POINTER_CHANGE_FIRSTBUTTON_DOWN,
          //POINTER_CHANGE_FIRSTBUTTON_UP,
          //POINTER_CHANGE_SECONDBUTTON_DOWN,
          //POINTER_CHANGE_SECONDBUTTON_UP,
          //POINTER_CHANGE_THIRDBUTTON_DOWN,
          //POINTER_CHANGE_THIRDBUTTON_UP,
          //POINTER_CHANGE_FOURTHBUTTON_DOWN,
          //POINTER_CHANGE_FOURTHBUTTON_UP,
          //POINTER_CHANGE_FIFTHBUTTON_DOWN,
          //POINTER_CHANGE_FIFTHBUTTON_UP
        //}}}
        //{{{  pointerPenInfo
        //BOOL GetPointerPenInfo ([in]UINT32 pointerId, [out] POINTER_PEN_INFO* penInfo)

        ////POINTER_PEN_INFO
          //POINTER_INFO pointerInfo;
          //PEN_FLAGS    penFlags;
          //PEN_MASK     penMask;
          //UINT32       pressure;
          //UINT32       rotation;
          //INT32        tiltX;
          //INT32        tiltY;
        //}}}
        if (miniFB) {
          POINTER_INFO pointerInfo;
          if (GetPointerInfo (GET_POINTERID_WPARAM (wParam), &pointerInfo)) {
            if (pointerInfo.pointerType == PT_MOUSE) {
              //cLog::log (LOGINFO, fmt::format ("pointerUpdate mouse type:{} flags:{:x} time:{}",
              //                                 pointerInfo.pointerType, pointerInfo.pointerFlags, pointerInfo.dwTime));
              miniFB->pointerInside = true;

              POINT clientPos = pointerInfo.ptPixelLocation;
              ScreenToClient (hWnd, &clientPos);
              miniFB->pointerPosX = clientPos.x;
              miniFB->pointerPosY = clientPos.y;
              miniFB->pointerPressure = miniFB->pointerButtonStatus[MOUSE_BTN_1] * 1024;
              miniFB->pointerTimestamp = 0;
              if (miniFB->moveFunc)
                miniFB->moveFunc (miniFB);
              }
            else if (pointerInfo.pointerType == PT_PEN) {
              POINTER_PEN_INFO pointerPenInfos[10];
              uint32_t entriesCount = 10;
              if (GetPointerPenInfoHistory (GET_POINTERID_WPARAM (wParam), &entriesCount, pointerPenInfos)) {
                miniFB->pointerInside = true;
                for (uint32_t i = entriesCount; i > 0; i--) {
                  miniFB->pointerTimestamp = pointerPenInfos[i-1].pointerInfo.dwTime;
                  ScreenToClient (hWnd, &pointerPenInfos[i-1].pointerInfo.ptPixelLocation);
                  miniFB->pointerPosX = pointerPenInfos[i-1].pointerInfo.ptPixelLocation.x;
                  miniFB->pointerPosY = pointerPenInfos[i-1].pointerInfo.ptPixelLocation.y;
                  miniFB->pointerPressure = pointerPenInfos[i-1].pressure;
                  miniFB->pointerTiltX = 0;
                  miniFB->pointerTiltY = 0;
                  if (miniFB->moveFunc)
                    miniFB->moveFunc (miniFB);
                  }
                }
              }
            else
              cLog::log (LOGERROR, fmt::format ("pointerUpdate - unknown type:{}", pointerInfo.pointerType));

            }
          else
            cLog::log (LOGERROR, fmt::format ("pointerUpdate - no info"));
          }

        break;
      //}}}
      //{{{
      case WM_POINTERWHEEL:
        if (miniFB) {
          cLog::log (LOGINFO, fmt::format ("pointerWheel"));
          miniFB->pointerWheelX = 0;
          miniFB->pointerWheelY = (SHORT)HIWORD(wParam) / (float)WHEEL_DELTA;
          if (miniFB->wheelFunc)
            miniFB->wheelFunc (miniFB);
          }
        else
          cLog::log (LOGERROR, fmt::format ("pointerWheel - no info"));

        break;
      //}}}

      //{{{
      //case 0x02E4://WM_GETDPISCALEDSIZE: {
      //  SIZE* size = (SIZE*)lParam;
      //  WORD dpi = (wParam);
      //  return true;
      //  break;
      //  }
      //}}}
      //{{{
      //case WM_DPICHANGED: //
      //  const float xscale = HIWORD(wParam);
      //  const float yscale = LOWORD(wParam);
      //  break;
      //}}}
      case WM_POINTERACTIVATE:
      case WM_POINTERCAPTURECHANGED:
      case WM_POINTERDEVICECHANGE:
      case WM_POINTERDEVICEINRANGE:
      case WM_POINTERDEVICEOUTOFRANGE:
      case WM_TOUCHHITTESTING:
      case WM_INPUT:
      case WM_NCPOINTERUPDATE:
      case WM_NCPOINTERUP:
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_XBUTTONUP:
      case WM_LBUTTONDOWN:
      case WM_LBUTTONDBLCLK:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONDBLCLK:
      case WM_MOUSEMOVE:
      case WM_MOUSELEAVE:
      case WM_MOUSEWHEEL:
      case WM_MOUSEHWHEEL:
      default:
        return DefWindowProc (hWnd, message, wParam, lParam);
      }

    return 0;
    }
  //}}}
  }

// interface
//{{{
cMiniFB* cMiniFB::create (const char* title, unsigned width, unsigned height, unsigned flags) {

  RECT rect = { 0 };
  int x = 0;
  int y = 0;

  loadFunctions();
  dpiAware();

  cMiniFB* miniFB = new cMiniFB();

  miniFB->initKeycodes();
  miniFB->bufferWidth  = width;
  miniFB->bufferHeight = height;
  miniFB->bufferStride = width * 4;

  long s_window_style = WS_POPUP | WS_SYSMENU | WS_CAPTION;
  s_window_style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
  if (flags & WF_FULLSCREEN) {
    //{{{  fullscreen
    flags = WF_FULLSCREEN;  // Remove all other flags
    rect.right  = GetSystemMetrics (SM_CXSCREEN);
    rect.bottom = GetSystemMetrics (SM_CYSCREEN);
    s_window_style = WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

    DEVMODE settings = { 0 };
    EnumDisplaySettings (0, 0, &settings);
    settings.dmPelsWidth  = GetSystemMetrics(SM_CXSCREEN);
    settings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
    settings.dmBitsPerPel = 32;
    settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    if (ChangeDisplaySettings (&settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
      flags = WF_FULLSCREEN_DESKTOP;
    }
    //}}}

  if (flags & WF_BORDERLESS)
    s_window_style = WS_POPUP;
  if (flags & WF_RESIZABLE)
    s_window_style |= WS_MAXIMIZEBOX | WS_SIZEBOX;

  if (flags & WF_FULLSCREEN_DESKTOP) {
    //{{{  desktop
    s_window_style = WS_OVERLAPPEDWINDOW;

    width  = GetSystemMetrics (SM_CXFULLSCREEN);
    height = GetSystemMetrics (SM_CYFULLSCREEN);

    rect.right  = width;
    rect.bottom = height;
    AdjustWindowRect (&rect, s_window_style, 0);
    if (rect.left < 0) {
      width += rect.left * 2;
      rect.right += rect.left;
      rect.left = 0;
      }
    if (rect.bottom > (LONG) height) {
      height -= (rect.bottom - height);
      rect.bottom += (rect.bottom - height);
      rect.top = 0;
      }
    }
    //}}}
  else if (!(flags & WF_FULLSCREEN)) {
    //{{{  desktop fullscreen
    float scale_x, scale_y;
    miniFB->getMonitorScale (&scale_x, &scale_y);

    rect.right  = (LONG) (width  * scale_x);
    rect.bottom = (LONG) (height * scale_y);
    AdjustWindowRect(&rect, s_window_style, 0);

    rect.right  -= rect.left;
    rect.bottom -= rect.top;
    x = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) / 2;
    }
    //}}}

  miniFB->wc.style         = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
  miniFB->wc.lpfnWndProc   = WndProc;
  miniFB->wc.hCursor       = LoadCursor(0, IDC_ARROW);
  miniFB->wc.lpszClassName = title;
  RegisterClass (&miniFB->wc);

  miniFB->calcDstFactor (width, height);

  miniFB->window_width  = rect.right;
  miniFB->window_height = rect.bottom;

  miniFB->window = CreateWindowEx (0,
                                           title, title,
                                           s_window_style,
                                           x, y,
                                           miniFB->window_width, miniFB->window_height,
                                           0, 0, 0, 0);
  if (!miniFB->window) {
    //{{{  error, return
    free (miniFB);
    return 0x0;
    }
    //}}}

  SetWindowLongPtr (miniFB->window, GWLP_USERDATA, (LONG_PTR) miniFB);

  if (flags & WF_ALWAYS_ON_TOP)
    SetWindowPos (miniFB->window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  ShowWindow (miniFB->window, SW_NORMAL);

  miniFB->hdc = GetDC (miniFB->window);

  miniFB->createGLcontext();

  cLog::log (LOGINFO, "using windows OpenGL");

  #ifdef USE_WINTAB
    // enable winTab, mainly for WT_PROXIMITY, get WT_PACKET
    if (!winTabLoad (miniFB->window))
      cLog::log (LOGERROR, fmt::format ("winTab load failed"));
  #endif

  // enable WM_POINTER wndProc messaging, to tell mouse from pen
  EnableMouseInPointer (true);

  miniFB->isInitialized = true;
  return miniFB;
  }
//}}}

//{{{
eUpdateState cMiniFB::updateEx (void* buffer, unsigned width, unsigned height) {

  if (closed) {
    freeResources();
    return STATE_EXIT;
    }

  if (!buffer)
    return STATE_INVALID_BUFFER;

  draw_buffer = buffer;
  bufferWidth = width;
  bufferStride = width * 4;
  bufferHeight = height;
  redrawGL (buffer);

  return STATE_OK;
  }
//}}}
//{{{
eUpdateState cMiniFB::updateEvents() {

  if (closed) {
    freeResources();
    return STATE_EXIT;
    }

  MSG msg;
  while (!closed && PeekMessage (&msg, window, 0, 0, PM_REMOVE)) {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
    }

  return STATE_OK;
  }
//}}}

//{{{
void cMiniFB::getMonitorScale (float* scale_x, float* scale_y) {
  getWindowsMonitorScale (window, scale_x, scale_y);
  }
//}}}
//{{{
bool cMiniFB::setViewport (unsigned offset_x, unsigned offset_y, unsigned width, unsigned height) {

  if (offset_x + width > window_width)
    return false;

  if (offset_y + height > window_height)
    return false;

  float scale_x, scale_y;
  getWindowsMonitorScale (window, &scale_x, &scale_y);

  dst_offset_x = (uint32_t)(offset_x * scale_x);
  dst_offset_y = (uint32_t)(offset_y * scale_y);

  dst_width = (uint32_t)(width  * scale_x);
  dst_height = (uint32_t)(height * scale_y);

  calcDstFactor (window_width, window_height);

  return true;
  }
//}}}

// private
//{{{
void cMiniFB::initKeycodes() {

  if (gKeycodes[0x00B] != KB_KEY_0) {
    //{{{  alpha numeric
    gKeycodes[0x00B] = KB_KEY_0;
    gKeycodes[0x002] = KB_KEY_1;
    gKeycodes[0x003] = KB_KEY_2;
    gKeycodes[0x004] = KB_KEY_3;
    gKeycodes[0x005] = KB_KEY_4;
    gKeycodes[0x006] = KB_KEY_5;
    gKeycodes[0x007] = KB_KEY_6;
    gKeycodes[0x008] = KB_KEY_7;
    gKeycodes[0x009] = KB_KEY_8;
    gKeycodes[0x00A] = KB_KEY_9;
    gKeycodes[0x01E] = KB_KEY_A;
    gKeycodes[0x030] = KB_KEY_B;
    gKeycodes[0x02E] = KB_KEY_C;
    gKeycodes[0x020] = KB_KEY_D;
    gKeycodes[0x012] = KB_KEY_E;
    gKeycodes[0x021] = KB_KEY_F;
    gKeycodes[0x022] = KB_KEY_G;
    gKeycodes[0x023] = KB_KEY_H;
    gKeycodes[0x017] = KB_KEY_I;
    gKeycodes[0x024] = KB_KEY_J;
    gKeycodes[0x025] = KB_KEY_K;
    gKeycodes[0x026] = KB_KEY_L;
    gKeycodes[0x032] = KB_KEY_M;
    gKeycodes[0x031] = KB_KEY_N;
    gKeycodes[0x018] = KB_KEY_O;
    gKeycodes[0x019] = KB_KEY_P;
    gKeycodes[0x010] = KB_KEY_Q;
    gKeycodes[0x013] = KB_KEY_R;
    gKeycodes[0x01F] = KB_KEY_S;
    gKeycodes[0x014] = KB_KEY_T;
    gKeycodes[0x016] = KB_KEY_U;
    gKeycodes[0x02F] = KB_KEY_V;
    gKeycodes[0x011] = KB_KEY_W;
    gKeycodes[0x02D] = KB_KEY_X;
    gKeycodes[0x015] = KB_KEY_Y;
    gKeycodes[0x02C] = KB_KEY_Z;
    //}}}
    //{{{  punctuation
    gKeycodes[0x028] = KB_KEY_APOSTROPHE;
    gKeycodes[0x02B] = KB_KEY_BACKSLASH;
    gKeycodes[0x033] = KB_KEY_COMMA;
    gKeycodes[0x00D] = KB_KEY_EQUAL;
    gKeycodes[0x029] = KB_KEY_GRAVE_ACCENT;
    gKeycodes[0x01A] = KB_KEY_LEFT_BRACKET;
    gKeycodes[0x00C] = KB_KEY_MINUS;
    gKeycodes[0x034] = KB_KEY_PERIOD;
    gKeycodes[0x01B] = KB_KEY_RIGHT_BRACKET;
    gKeycodes[0x027] = KB_KEY_SEMICOLON;
    gKeycodes[0x035] = KB_KEY_SLASH;
    gKeycodes[0x056] = KB_KEY_WORLD_2;
    //}}}
    //{{{  other
    gKeycodes[0x00E] = KB_KEY_BACKSPACE;
    gKeycodes[0x153] = KB_KEY_DELETE;
    gKeycodes[0x14F] = KB_KEY_END;
    gKeycodes[0x01C] = KB_KEY_ENTER;
    gKeycodes[0x001] = KB_KEY_ESCAPE;
    gKeycodes[0x147] = KB_KEY_HOME;
    gKeycodes[0x152] = KB_KEY_INSERT;
    gKeycodes[0x15D] = KB_KEY_MENU;
    gKeycodes[0x151] = KB_KEY_PAGE_DOWN;
    gKeycodes[0x149] = KB_KEY_PAGE_UP;
    gKeycodes[0x045] = KB_KEY_PAUSE;
    gKeycodes[0x146] = KB_KEY_PAUSE;
    gKeycodes[0x039] = KB_KEY_SPACE;
    gKeycodes[0x00F] = KB_KEY_TAB;
    gKeycodes[0x03A] = KB_KEY_CAPS_LOCK;
    gKeycodes[0x145] = KB_KEY_NUM_LOCK;
    gKeycodes[0x046] = KB_KEY_SCROLL_LOCK;
    gKeycodes[0x03B] = KB_KEY_F1;
    gKeycodes[0x03C] = KB_KEY_F2;
    gKeycodes[0x03D] = KB_KEY_F3;
    gKeycodes[0x03E] = KB_KEY_F4;
    gKeycodes[0x03F] = KB_KEY_F5;
    gKeycodes[0x040] = KB_KEY_F6;
    gKeycodes[0x041] = KB_KEY_F7;
    gKeycodes[0x042] = KB_KEY_F8;
    gKeycodes[0x043] = KB_KEY_F9;
    gKeycodes[0x044] = KB_KEY_F10;
    gKeycodes[0x057] = KB_KEY_F11;
    gKeycodes[0x058] = KB_KEY_F12;
    gKeycodes[0x064] = KB_KEY_F13;
    gKeycodes[0x065] = KB_KEY_F14;
    gKeycodes[0x066] = KB_KEY_F15;
    gKeycodes[0x067] = KB_KEY_F16;
    gKeycodes[0x068] = KB_KEY_F17;
    gKeycodes[0x069] = KB_KEY_F18;
    gKeycodes[0x06A] = KB_KEY_F19;
    gKeycodes[0x06B] = KB_KEY_F20;
    gKeycodes[0x06C] = KB_KEY_F21;
    gKeycodes[0x06D] = KB_KEY_F22;
    gKeycodes[0x06E] = KB_KEY_F23;
    gKeycodes[0x076] = KB_KEY_F24;
    gKeycodes[0x038] = KB_KEY_LEFT_ALT;
    gKeycodes[0x01D] = KB_KEY_LEFT_CONTROL;
    gKeycodes[0x02A] = KB_KEY_LEFT_SHIFT;
    gKeycodes[0x15B] = KB_KEY_LEFT_SUPER;
    gKeycodes[0x137] = KB_KEY_PRINT_SCREEN;
    gKeycodes[0x138] = KB_KEY_RIGHT_ALT;
    gKeycodes[0x11D] = KB_KEY_RIGHT_CONTROL;
    gKeycodes[0x036] = KB_KEY_RIGHT_SHIFT;
    gKeycodes[0x15C] = KB_KEY_RIGHT_SUPER;
    gKeycodes[0x150] = KB_KEY_DOWN;
    gKeycodes[0x14B] = KB_KEY_LEFT;
    gKeycodes[0x14D] = KB_KEY_RIGHT;
    gKeycodes[0x148] = KB_KEY_UP;
    //}}}
    //{{{  numpad
    gKeycodes[0x052] = KB_KEY_KP_0;
    gKeycodes[0x04F] = KB_KEY_KP_1;
    gKeycodes[0x050] = KB_KEY_KP_2;
    gKeycodes[0x051] = KB_KEY_KP_3;
    gKeycodes[0x04B] = KB_KEY_KP_4;
    gKeycodes[0x04C] = KB_KEY_KP_5;
    gKeycodes[0x04D] = KB_KEY_KP_6;
    gKeycodes[0x047] = KB_KEY_KP_7;
    gKeycodes[0x048] = KB_KEY_KP_8;
    gKeycodes[0x049] = KB_KEY_KP_9;
    gKeycodes[0x04E] = KB_KEY_KP_ADD;
    gKeycodes[0x053] = KB_KEY_KP_DECIMAL;
    gKeycodes[0x135] = KB_KEY_KP_DIVIDE;
    gKeycodes[0x11C] = KB_KEY_KP_ENTER;
    gKeycodes[0x037] = KB_KEY_KP_MULTIPLY;
    gKeycodes[0x04A] = KB_KEY_KP_SUBTRACT;
    //}}}
    }
  }
//}}}
//{{{
void cMiniFB::freeResources() {

  destroyGLcontext();

  if (window && hdc) {
    ReleaseDC (window, hdc);
    DestroyWindow (window);
    }

  window = 0;
  hdc = 0;

  draw_buffer = 0x0;
  closed = true;

  #ifdef USE_WINTAB
    winTabUnload();
  #endif
  }
//}}}
