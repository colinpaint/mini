// cD2dWindow.cpp - Direct2d cWindow - !!!! could have multiple windows if I split the true static info from the window !!!
//{{{  includes
#define _CRT_SECURE_NO_WARNINGS

// std C++
#include <cstdint>
#include <algorithm>
#include <array>
#include <thread>

// windows
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <DXGI1_2.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "cWindow.h"
#include "../common/utils.h"
#include "../common/cLog.h"

#pragma comment(lib,"d3d11")
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"shlwapi.lib")

using namespace std;
using namespace chrono;
//}}}
namespace {
  //{{{  D3D11 device resources
  ID3D11Device* gD3dDevice = nullptr;
  ID3D11Device1* gD3dDevice1 = nullptr;

  // D3D11 device contexts
  ID3D11DeviceContext* gD3dContext = nullptr;
  ID3D11DeviceContext1* gD3dContext1 = nullptr;
  //}}}
  //{{{  DXGI resources
  // DXGI device
  IDXGIDevice1* gDxgiDevice1 = nullptr;

  // sized swapChain resources
  IDXGISwapChain1* gSwapChain = nullptr;
  //}}}
  //{{{  D2D resources
  ID2D1Bitmap1* gD2dTargetBitmap = nullptr;

  // ID2D1 device context
  ID2D1DeviceContext* gD2dDeviceContext = nullptr;

  // DirectWrite factory
  IDWriteFactory* gDWriteFactory = nullptr;
  //}}}

  cColor gLastColor = kWhite;
  ID2D1SolidColorBrush* mSolidColorBrush = nullptr;
  //{{{
  ID2D1SolidColorBrush* setColor (const cColor& color) {

    mSolidColorBrush->SetColor (D2D1_COLOR_F(color.r, color.g, color.b, color.a));
    return mSolidColorBrush;
    }
  //}}}
  array<IDWriteTextFormat*, cWindow::kNumFonts> gFonts;
  //{{{  window resources
  cWindow* gWindow = nullptr;

  HWND gHWND = 0;
  RECT gScreenRect = {0};
  DWORD gScreenStyle = 0;
  //}}}
  //{{{
  LRESULT CALLBACK WndProc (HWND hWnd, unsigned int msg, WPARAM wparam, LPARAM lparam) {

    return gWindow->wndProc (hWnd, msg, wparam, lparam);
    }
  //}}}

  cPoint gClientSize;
  //{{{
  bool createDevice() {
  // create DX11 API device object, get D3Dcontext
  // !!!!! remember recreate case on device removed !!!!! - TODO }

    const D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };

    D3D_FEATURE_LEVEL featureLevel;
    if (D3D11CreateDevice (nullptr,                          // specify null to use the default adapter
                           D3D_DRIVER_TYPE_HARDWARE,
                           0,
                           D3D11_CREATE_DEVICE_BGRA_SUPPORT, // optionally set debug and Direct2D compatibility flags
                           featureLevels, ARRAYSIZE(featureLevels), // list of feature levels this app can support
                           D3D11_SDK_VERSION,
                           &gD3dDevice,                      // returns the Direct3D device created
                           &featureLevel,                    // returns feature level of device created
                           &gD3dContext) != S_OK) {          // returns the device immediate context
      //{{{  error, return
      cLog::log (LOGERROR, "D3D11CreateDevice failed");
      return false;
      }
      //}}}

    cLog::log (LOGNOTICE, fmt::format ("D3D11CreateDevice - featureLevel {}.{}",
                                       (featureLevel >> 12) & 0xF, (featureLevel >> 8) & 0xF));

    // query gD3dDevice1 from gD3dDevice
    if (gD3dDevice->QueryInterface (IID_PPV_ARGS (&gD3dDevice1)) != S_OK) {
      //{{{  error, return
      cLog::log (LOGERROR, "query d3dDevice1 failed");
      return false;
      };
      //}}}

    // query dxgiDevice from gD3dDevice
    IDXGIDevice* dxgiDevice = nullptr;
    if (gD3dDevice->QueryInterface (IID_PPV_ARGS (&dxgiDevice)) != S_OK) {
      //{{{  error, return
      cLog::log (LOGERROR, "query dxgiDevice failed");
      return false;
      };
      //}}}

    // query gD3dContext1 from gD3dContext
    if (gD3dContext->QueryInterface (IID_PPV_ARGS (&gD3dContext1)) != S_OK) {
      //{{{  error, return
      cLog::log (LOGERROR, "query d3dContext1 failed");
      return false;
      };
      //}}}

    // query gDxgiDevice1 from gD3dDevice1
    if (gD3dDevice1->QueryInterface (IID_PPV_ARGS (&gDxgiDevice1)) != S_OK) {
      //{{{  error, return
      cLog::log (LOGERROR, "query dxgiDevice1 failed");
      return false;
      };
      //}}}

    //{{{  create mD2dDeviceContext from d2dDevice, d2dDevice created from d2dFactory1 and dxgiDevice
    // create ID2D1Factory1* d2dFactory1
    ID2D1Factory1* d2dFactory1;
    #ifdef _DEBUG
      D2D1CreateFactory (D2D1_FACTORY_TYPE_MULTI_THREADED, { D2D1_DEBUG_LEVEL_INFORMATION }, &d2dFactory1);
    #else
      D2D1CreateFactory (D2D1_FACTORY_TYPE_MULTI_THREADED, { D2D1_DEBUG_LEVEL_NONE }, &d2dFactory1);
    #endif

    //  create ID2D1Device* d2dDevice from d2dFactory1,dxgiDevice
    ID2D1Device* d2dDevice;
    d2dFactory1->CreateDevice (dxgiDevice, &d2dDevice);
    dxgiDevice->Release();
    d2dFactory1->Release();

    // create ID2D1DeviceContext* mD2dDeviceContext from d2dDevice
    d2dDevice->CreateDeviceContext (D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &gD2dDeviceContext);
    //}}}
    gD2dDeviceContext->CreateSolidColorBrush (D2D1::ColorF (D2D1::ColorF::Black), &mSolidColorBrush);

    // create mDWriteFactory, to make textFormat fonts and textLayout drawText
    DWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&gDWriteFactory));
    //{{{  create proportionalSpaced kMenuFont
    gDWriteFactory->CreateTextFormat (L"FreeSans", NULL,
                                      DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                      cWindow::getBoxHeight() * 0.8f, L"en-us",
                                      &gFonts[cWindow::kMenuFont]);

    gFonts[cWindow::kMenuFont]->SetWordWrapping (DWRITE_WORD_WRAPPING_EMERGENCY_BREAK);
    //}}}
    //{{{  create proportionalSpaced kLargeMenuFont
    gDWriteFactory->CreateTextFormat (L"FreeSans", NULL,
                                      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                      cWindow::getBoxHeight() * 1.8f, L"en-us",
                                      &gFonts[cWindow::kLargeMenuFont]);

    gFonts[cWindow::kLargeMenuFont]->SetWordWrapping (DWRITE_WORD_WRAPPING_EMERGENCY_BREAK);
    //}}}
    //{{{  create monoSpaced kConsoleFont
    gDWriteFactory->CreateTextFormat (L"Consolas", NULL,
                                      DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                      cWindow::getConsoleHeight(), L"en-us",
                                      &gFonts[cWindow::kConsoleFont]);
    //D2D1_DRAW_TEXT_OPTIONS_CLIP);
    //}}}
    //{{{  create kSymbolFont
    gDWriteFactory->CreateTextFormat (L"Marlett", NULL,
                                      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                      cWindow::getBoxHeight(), L"en-us",
                                      &gFonts[cWindow::kSymbolFont]);

    gFonts[cWindow::kSymbolFont]->SetTextAlignment (DWRITE_TEXT_ALIGNMENT_TRAILING);
    //}}}

    return true;
    }
  //}}}
  //{{{
  void createSized() {

    RECT rect;
    GetClientRect (gHWND, &rect);
    cLog::log (LOGINFO, fmt::format ("createSized {} {} {} {}",
                                     rect.left, rect.top, rect.right, rect.bottom));

    gClientSize.x = float(rect.right - rect.left);
    gClientSize.y = float(rect.bottom - rect.top);

    if (!gSwapChain) {
      //{{{  create swapChain and buffers
      // get IDXGIAdapter* dxgiAdapter from mDxgiDevice1
      IDXGIAdapter* dxgiAdapter;
      gDxgiDevice1->GetAdapter (&dxgiAdapter);

      // get IDXGIFactory2* dxgiFactory2 from dxgiAdapter
      IDXGIFactory2* dxgiFactory2;
      dxgiAdapter->GetParent (IID_PPV_ARGS (&dxgiFactory2));

      //  create swapChain
      DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
      dxgiSwapChainDesc.Width = 0;                                     // use automatic sizing
      dxgiSwapChainDesc.Height = 0;
      dxgiSwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // this is the most common swapchain format
      dxgiSwapChainDesc.Stereo = false;
      dxgiSwapChainDesc.SampleDesc.Count = 1;                          // don't use multi-sampling
      dxgiSwapChainDesc.SampleDesc.Quality = 0;
      dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      dxgiSwapChainDesc.BufferCount = 2;                               // use double buffering to enable flip
      dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
      dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
      dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
      dxgiSwapChainDesc.Flags = 0;

      dxgiFactory2->CreateSwapChainForHwnd (gD3dDevice1, gHWND, &dxgiSwapChainDesc, nullptr, nullptr, &gSwapChain);
      gDxgiDevice1->SetMaximumFrameLatency (1);
      }
      //}}}
    else {
      // resize buffers
      HRESULT hresult = gSwapChain->ResizeBuffers (0, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
      if (hresult == DXGI_ERROR_DEVICE_REMOVED) {
        gSwapChain = nullptr;
        // !!!!!!!!!!!!!! watch out !!!!!!!!!!!!!!!!
        cLog::log (LOGERROR, "createSized - device removed");
        createDevice();
        return;
        }
      }

    // can get backBuffer
    //ID3D11Texture2D* backBuffer;
    //mSwapChain->GetBuffer (0, IID_PPV_ARGS (&backBuffer));

    // get dxgiBackbuffer from swapchain, create mD2dTargetBitmap for it
    IDXGISurface* dxgiBackBuffer;
    gSwapChain->GetBuffer (0, IID_PPV_ARGS (&dxgiBackBuffer));

    D2D1_BITMAP_PROPERTIES1 bitmapProperties
      { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE, 0,0,
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, NULL };
    gD2dDeviceContext->CreateBitmapFromDxgiSurface (dxgiBackBuffer, &bitmapProperties, &gD2dTargetBitmap);

    // set Direct2D render target.
    gD2dDeviceContext->SetTarget (gD2dTargetBitmap);

    // grayscale text anti-aliasing
    gD2dDeviceContext->SetTextAntialiasMode (D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    }
  //}}}
  }

// static createBitmap can be called by nonGUI thread, typically a loader
//{{{
cBitmap cWindow::createBitmap (uint32_t width, uint32_t height, void* pixels, bool freePixels) {

  ID2D1Bitmap* bitmap;
  gD2dDeviceContext->CreateBitmap ({width, height},
                                   {DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE, 0,0},
                                   &bitmap);
  if (bitmap) {
    D2D1_RECT_U rect(D2D1::RectU (0, 0, width, height));
    bitmap->CopyFromMemory (&rect, pixels, width*4);
    }
  else
    cLog::log (LOGERROR, fmt::format ("createBitmap {}x{} failed", width, height));

  if (freePixels)
    free (pixels);

  return cBitmap (bitmap);
  }
//}}}

// create
//{{{
cWindow::~cWindow() {

   if (gD3dContext)
     gD3dContext->Release();
   if (gD3dContext1)
     gD3dContext1->Release();
   if (gD3dDevice)
     gD3dDevice->Release();
   if (gD3dDevice1)
     gD3dDevice1->Release();

   if (gDxgiDevice1)
     gDxgiDevice1->Release();
   if (gSwapChain)
     gSwapChain->Release();

   if (gD2dTargetBitmap)
     gD2dTargetBitmap->Release();
   if (gD2dDeviceContext)
     gD2dDeviceContext->Release();
   if (gDWriteFactory)
     gDWriteFactory->Release();
  }
//}}}
//{{{
bool cWindow::createWindow (const string& title, uint32_t width, uint32_t height, chrono::milliseconds tickMs, bool fullScreen) {

  if (!createDevice())
    return false;

  gWindow = this;

  TIME_ZONE_INFORMATION timeZoneInfo;
  if (GetTimeZoneInformation (&timeZoneInfo) == TIME_ZONE_ID_DAYLIGHT)
    mDayLightSeconds = -timeZoneInfo.DaylightBias * 60;

  //{{{
  WNDCLASSEX wndclass = { sizeof (WNDCLASSEX),
                          CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
                          WndProc,
                          0, 0,
                          GetModuleHandle (0),
                          LoadIcon (0,IDI_APPLICATION),
                          LoadCursor (0,IDC_ARROW),
                          NULL,
                          0,
                          "windowClass",
                          LoadIcon (0,IDI_APPLICATION) };
  //}}}
  if (RegisterClassEx (&wndclass))
    //{{{
    gHWND = CreateWindowEx (0,
                            "windowClass",
                            title.data(),
                            WS_OVERLAPPEDWINDOW,
                            20, 20, width+4, height+32,
                            0, 0,
                            GetModuleHandle(0), 0);
    //}}}

  if (gHWND) {
    //SetWindowLong (hWnd, GWL_STYLE, 0);
    if (fullScreen)
      toggleFullScreen();
    ShowWindow (gHWND, SW_SHOWDEFAULT);
    UpdateWindow (gHWND);

    createSized();

    if (tickMs != 0ms) {
      //{{{  launch tick clock thread
      thread ([=]() {
        cLog::setThreadName ("tick");

        while (!mExit) {
          this_thread::sleep_for (tickMs);
          changed();
          }
        }).detach();
      }
      //}}}

    //{{{  launch renderThread
    thread ([=]() {
      // render lambda
      cLog::setThreadName ("rend");

      while (!gD2dTargetBitmap) {
        // wait for target bitmap creation
        this_thread::sleep_for (10ms);
        cLog::log (LOGINFO, "waiting for target bitmap creation");
        }

      while (gD2dDeviceContext && !mExit) {
        if (!mChanged)
          this_thread::sleep_for (10ms);
        else {
          system_clock::time_point timePoint = system_clock::now();

          gD2dDeviceContext->BeginDraw();
          clear (kBlack);
          for (auto& box : mBoxes)
            if (box->getShow())
              box->onDraw();
          gD2dDeviceContext->EndDraw();

          mRenderUs = duration_cast<microseconds>(system_clock::now() - timePoint).count();
          mChanged = false;

          if (gSwapChain->Present (1, 0) == DXGI_ERROR_DEVICE_REMOVED) {
            gSwapChain = nullptr;
            createDevice();
            cLog::log (LOGERROR, "device removed - createDevice fails to deallocate previous resources");
            }
          }
        }

      cLog::log (LOGINFO, "exit");
      mExitDone = true;
      }).detach();
    //}}}
    }

  return true;
  }
//}}}

// gets
float cWindow::getWidth() const { return gClientSize.x; }
float cWindow::getHeight() const { return gClientSize.y; }
cPoint cWindow::getSize() const { return gClientSize; }

//{{{
system_clock::time_point cWindow::getNow() const {
  return system_clock::now();
  }
//}}}
//{{{
system_clock::time_point cWindow::getNowDaylight() const {
  return getNow() + seconds (mDayLightSeconds);
  }
//}}}

// draw
//{{{
void cWindow::clear (const cColor& color) {
// !!! only color is black for now !!!
  gD2dDeviceContext->Clear (D2D1::ColorF(D2D1::ColorF::Black));
  }
//}}}
//{{{
void cWindow::drawRectangle (const cColor& color, const cRect& rect) {
  gD2dDeviceContext->FillRectangle (rect, setColor (color));
  }
//}}}
//{{{
void cWindow::drawBorder (const cColor& color, const cRect& rect) {
  gD2dDeviceContext->DrawRectangle (rect, setColor (color), getOutlineWidth());
  }
//}}}
//{{{
void cWindow::drawBorder (const cColor& color, const cRect& rect, float width) {
  gD2dDeviceContext->DrawRectangle (rect, setColor (color), width);
  }
//}}}
//{{{
void cWindow::drawRounded (const cColor& color, const cRect& rect) {
  gD2dDeviceContext->FillRoundedRectangle (D2D1::RoundedRect (rect, getRoundRadius(), getRoundRadius()), setColor (color));
  }
//}}}
//{{{
void cWindow::drawRounded (const cColor& color, const cRect& rect, float radius) {
  gD2dDeviceContext->FillRoundedRectangle (D2D1::RoundedRect (rect, radius, radius), setColor (color));
  }
//}}}
//{{{
void cWindow::drawEllipse (const cColor& color, cPoint centre, cPoint radius, float width) {
  if (width > 0.f)
    gD2dDeviceContext->DrawEllipse (D2D1::Ellipse (centre, radius.x, radius.y), setColor (color), width);
  else
    gD2dDeviceContext->FillEllipse (D2D1::Ellipse (centre, radius.x, radius.y), setColor (color));
  }
//}}}
//{{{
void cWindow::drawLine (const cColor& color, cPoint point1, cPoint point2, float width) {
  gD2dDeviceContext->DrawLine (point1, point2, setColor (color), width);
  }
//}}}

// text
//{{{
cPoint cWindow::measureText (cPoint size, const string& text, uint32_t font) {

  IDWriteTextLayout* textLayout;
  gDWriteFactory->CreateTextLayout (utils::stringToWstring (text).data(), (uint32_t)text.size(),
                                    gFonts[font], size.x, size.y, &textLayout);
  if (!textLayout) // clipped
    return cPoint();

  struct DWRITE_TEXT_METRICS textMetrics;
  textLayout->GetMetrics (&textMetrics);
  textLayout->Release();

  //cLog::log (LOGINFO, fmt::format ("{} left:{} top:{} width:{}:{} height:{} lw:{} lh:{} max:{} lines:{}",
  //  text,
  //  textMetrics.left, textMetrics.top,
  //  textMetrics.width, textMetrics.widthIncludingTrailingWhitespace, textMetrics.height,
  //  textMetrics.layoutWidth, textMetrics.layoutHeight,
  //  textMetrics.maxBidiReorderingDepth, textMetrics.lineCount));

  return cPoint (textMetrics.width, textMetrics.height);
  }
//}}}
//{{{
cPoint cWindow::drawText (const cColor& color, const cRect& rect, const string& text, uint32_t font) {

  IDWriteTextLayout* textLayout;
  gDWriteFactory->CreateTextLayout (utils::stringToWstring (text).data(), (uint32_t)text.size(),
                                    gFonts[font], rect.getWidth(), rect.getHeight(), &textLayout);
  if (!textLayout) // clipped
    return cPoint();

  struct DWRITE_TEXT_METRICS textMetrics;
  textLayout->GetMetrics (&textMetrics);

  gD2dDeviceContext->DrawTextLayout (rect.getTL(), textLayout, setColor (color));
  textLayout->Release();

  return cPoint (textMetrics.width, textMetrics.height);
  }
//}}}
//{{{
cPoint cWindow::drawTextShadow (const cColor& color, const cRect& rect, const string& text, uint32_t font) {

  IDWriteTextLayout* textLayout;
  gDWriteFactory->CreateTextLayout (utils::stringToWstring (text).data(), (uint32_t)text.size(),
                                    gFonts[font], rect.getWidth(), rect.getHeight(), &textLayout);
  if (!textLayout) // clipped
    return cPoint();

  struct DWRITE_TEXT_METRICS textMetrics;
  textLayout->GetMetrics (&textMetrics);

  gD2dDeviceContext->DrawTextLayout (rect.getTL() + cPoint(2,2), textLayout, setColor (kBlack));
  gD2dDeviceContext->DrawTextLayout (rect.getTL(), textLayout, setColor (color));

  textLayout->Release();

  return cPoint (textMetrics.width, textMetrics.height);
  }
//}}}

// wstring text
//{{{
cPoint cWindow::measureText (cPoint size, const wstring& text, uint32_t font) {

  IDWriteTextLayout* textLayout;
  gDWriteFactory->CreateTextLayout (text.data(), (uint32_t)text.size(),
                                    gFonts[font], size.x, size.y, &textLayout);
  if (!textLayout) // clipped
    return cPoint();

  struct DWRITE_TEXT_METRICS textMetrics;
  textLayout->GetMetrics (&textMetrics);
  textLayout->Release();

  return cPoint (textMetrics.width, textMetrics.height);
  }
//}}}
//{{{
cPoint cWindow::drawText (const cColor& color, const cRect& rect, const wstring& text, uint32_t font) {

  IDWriteTextLayout* textLayout;
  gDWriteFactory->CreateTextLayout (text.data(), (uint32_t)text.size(),
                                    gFonts[font], rect.getWidth(), rect.getHeight(), &textLayout);
  if (!textLayout) // clipped
    return cPoint();

  struct DWRITE_TEXT_METRICS textMetrics;

  textLayout->GetMetrics (&textMetrics);
  gD2dDeviceContext->DrawTextLayout (rect.getTL(), textLayout, setColor (color));

  textLayout->Release();

  return cPoint (textMetrics.width, textMetrics.height);
  }
//}}}
//{{{
cPoint cWindow::drawTextShadow (const cColor& color, const cRect& rect, const wstring& text, uint32_t font) {

  IDWriteTextLayout* textLayout;
  gDWriteFactory->CreateTextLayout (text.data(), (uint32_t)text.size(),
                                    gFonts[font], rect.getWidth(), rect.getHeight(), &textLayout);
  if (!textLayout) // clipped
    return cPoint();

  struct DWRITE_TEXT_METRICS textMetrics;
  textLayout->GetMetrics (&textMetrics);

  // draw shadow, offset
  gD2dDeviceContext->DrawTextLayout (rect.getTL() + cPoint(2,2), textLayout, setColor (kBlack));

  // draw color
  gD2dDeviceContext->DrawTextLayout (rect.getTL(), textLayout, setColor (color));

  textLayout->Release();

  return cPoint (textMetrics.width, textMetrics.height);
  }
//}}}

// bitmap 
//{{{
void cWindow::blit (cBitmap bitmap, const cRect& dstRect) {
  if (bitmap.empty())
    gD2dDeviceContext->FillRectangle (dstRect, setColor (bitmap.getEmptyColor()));
  else
    gD2dDeviceContext->DrawBitmap ((ID2D1Bitmap*)(bitmap.getBitmap()), dstRect, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
  }
//}}}

// window actions
//{{{
void cWindow::resized() {

  RECT rect;
  GetClientRect (gHWND, &rect);

  if (gD2dDeviceContext &&
     (cPoint (float(rect.right - rect.left), float(rect.bottom - rect.top)) != gClientSize)) {

    cLog::log (LOGINFO, fmt::format ("cWindow::resized {} {} {} {}", rect.left, rect.top, rect.right, rect.bottom));

    gD2dDeviceContext->SetTarget (nullptr);
    if (gD2dTargetBitmap)
      gD2dTargetBitmap->Release();
    gD2dTargetBitmap = nullptr;
    createSized();

    resize();
    changed();
    }
  }
//}}}
//{{{
void cWindow::toggleFullScreen() {

  mFullScreen = !mFullScreen;

  if (mFullScreen) {
    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);

    HMONITOR hMonitor = MonitorFromWindow (gHWND, MONITOR_DEFAULTTONEAREST);
    GetMonitorInfo (hMonitor, &monitorInfo);

    WINDOWINFO wndInfo;
    wndInfo.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo (gHWND, &wndInfo);

    gScreenRect = wndInfo.rcWindow;
    gScreenStyle = wndInfo.dwStyle;

    SetWindowLong (gHWND, GWL_STYLE, WS_POPUP);
    SetWindowPos (gHWND, HWND_NOTOPMOST,
                  monitorInfo.rcMonitor.left , monitorInfo.rcMonitor.top,
                  abs(monitorInfo.rcMonitor.left - monitorInfo.rcMonitor.right),
                  abs(monitorInfo.rcMonitor.top - monitorInfo.rcMonitor.bottom),
                  SWP_SHOWWINDOW);
    }
  else {
    AdjustWindowRectEx (&gScreenRect, 0, 0, 0);
    SetWindowLong (gHWND, GWL_STYLE, gScreenStyle);
    SetWindowPos (gHWND, HWND_NOTOPMOST,
                  gScreenRect.left , gScreenRect.top ,
                  abs(gScreenRect.right - gScreenRect.left), abs(gScreenRect.bottom - gScreenRect.top),
                  SWP_SHOWWINDOW);
    }

  }
//}}}
bool cWindow::updateBoxes() { return true; }
bool cWindow::updateScreen() { return true; }
bool cWindow::waitSync() { return true; }

//{{{
LRESULT cWindow::wndProc (HWND hWnd, unsigned int msg, WPARAM wparam, LPARAM lparam) {

  switch (msg) {
    //{{{
    case WM_KEYDOWN:
      mKeyDown++;

      if (wparam == 0x10)
        mShiftKeyDown = true;
      if (wparam == 0x11)
        mControlKeyDown = true;

      if (onKey ((int)wparam))
        PostQuitMessage (0) ;

      keyChanged();
      return 0;
    //}}}
    //{{{
    case WM_KEYUP:
      mKeyDown--;

      if (wparam == 0x10)
        mShiftKeyDown = false;
      if (wparam == 0x11)
        mControlKeyDown = false;
      onKeyUp ((int)wparam);

      keyChanged();
      return 0;
    //}}}

    case WM_LBUTTONDOWN:
    //{{{
    case WM_RBUTTONDOWN: {

      if (!mMousePress)
        SetCapture (hWnd);

      mMousePress = true;
      mMouseMoved = false;

      mMousePressPos = cPoint (LOWORD(lparam), HIWORD(lparam));
      mMousePressRight = (msg == WM_RBUTTONDOWN);
      mMouseLastPos = mMousePressPos;

      mMousePressUsed = mouseDown (mMousePressRight, mMousePressPos);
      if (mMousePressUsed)
        changed();

      cursorChanged();
      return 0;
      }
    //}}}

    case WM_LBUTTONUP:
    //{{{
    case WM_RBUTTONUP: {

      mMouseLastPos = cPoint (LOWORD(lparam), HIWORD(lparam));
      if (mouseUp (mMousePressRight, mMouseMoved, mMouseLastPos))
        changed();

      if (mMousePress)
        ReleaseCapture();
      mMousePress = false;
      mMousePressUsed = false;

      cursorChanged();
      return 0;
      }
    //}}}

    //{{{
    case WM_MOUSEMOVE: {

      mMousePos = cPoint (LOWORD(lparam), HIWORD(lparam));

      if (mMousePress) {
        mMouseMoved = true;
        if (mouseMove (mMousePressRight, mMousePos, mMousePos - mMouseLastPos))
          changed();
        mMouseLastPos = mMousePos;
        }
      else if (mouseProx (true, mMousePos))
        changed();

      if (!mMouseTracking) {
        TRACKMOUSEEVENT trackMouseEvent;
        trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
        trackMouseEvent.dwFlags = TME_LEAVE;
        trackMouseEvent.hwndTrack = hWnd;
        if (TrackMouseEvent (&trackMouseEvent))
          mMouseTracking = true;
        }

      cursorChanged();
      return 0;
      }
    //}}}
    //{{{
    case WM_MOUSEWHEEL: {
      if (mouseWheel (GET_WHEEL_DELTA_WPARAM(wparam) / 120, mMousePos))
        changed();

      cursorChanged();
      return 0;
      }
    //}}}
    //{{{
    case WM_MOUSELEAVE: {

      if (mouseProx (false, cPoint()))
        changed();

      mMouseTracking =  false;
      return 0;
      }
    //}}}

    //{{{
    case WM_SIZE: {
      resized();
      return 0;
      }
    //}}}
    //{{{
    case WM_DESTROY: {
      PostQuitMessage(0) ;
      return 0;
      }
    //}}}

    case WM_TIMER:
      //{{{  cursor hide logic
      if (mCursorDown > 0) {
        mCursorDown--;
        if (!mCursorOn) {
          mCursorOn = true;
          ShowCursor (mCursorOn);
          }
        }
      else if (mCursorOn) {
        mCursorOn = false;
        ShowCursor (mCursorOn);
        }
      return 0;
      //}}}

    case WM_SYSCOMMAND:
      switch (wparam) {
        //{{{
        case SC_MAXIMIZE:
          cLog::log (LOGINFO, "SC_MAXIMIZE");
          toggleFullScreen();
          break;
        //}}}
        //{{{
        case SC_SCREENSAVE:
          cLog::log (LOGINFO, "SC_SCREENSAVE");
          return 0;
        //}}}
        //{{{
        case SC_MONITORPOWER:
          cLog::log (LOGINFO, "SC_MONITORPOWER");
          return 0;
        //}}}
        default: ; // not handled, fall thru
        }
    }

  return DefWindowProc (hWnd, msg, wparam, lparam);
  }
//}}}

// protected
//{{{
void cWindow::messagePump() {

  UINT_PTR timer = SetTimer (gHWND, 0x1234, 1000/25, NULL);

  MSG msg;
  while (!mExit && GetMessage (&msg, NULL, 0, 0)) {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
    }

  KillTimer (NULL, timer);
  }
//}}}
