// cMiniFB.h - mini frameBuffer, based on https://github.com/emoon/minifb, but hacked to death
#pragma once
//{{{  includes
#include <cstdint>
#include <functional>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
#else
  #include <X11/Xlib.h>
  #include <GL/glx.h>
#endif
//}}}

// enums
//{{{
enum eMiniFlags {
              WF_RESIZABLE          = 0x01,
              WF_FULLSCREEN         = 0x02,
              WF_FULLSCREEN_DESKTOP = 0x04,
              WF_BORDERLESS         = 0x08,
              WF_ALWAYS_ON_TOP      = 0x10 };
//}}}
//{{{
enum eMiniState {
                    STATE_OK             =  0,
                    STATE_EXIT           = -1,
                    STATE_INVALID_WINDOW = -2,
                    STATE_INVALID_BUFFER = -3,
                    STATE_INTERNAL_ERROR = -4 };
//}}}

//{{{
enum eMiniPointerButton {
  MOUSE_BTN_0,
  MOUSE_BTN_1,
  MOUSE_BTN_2,
  MOUSE_BTN_3,
  MOUSE_BTN_4,
  MOUSE_BTN_5,
  MOUSE_BTN_6,
  MOUSE_BTN_7
  };
//}}}
#define MOUSE_LEFT   MOUSE_BTN_1
#define MOUSE_RIGHT  MOUSE_BTN_2
#define MOUSE_MIDDLE MOUSE_BTN_3

//{{{
enum eMiniKeyModifier {
  KB_MOD_SHIFT     = 0x0001,
  KB_MOD_CONTROL   = 0x0002,
  KB_MOD_ALT       = 0x0004,
  KB_MOD_SUPER     = 0x0008,
  KB_MOD_CAPS_LOCK = 0x0010,
  KB_MOD_NUM_LOCK  = 0x0020
  };
//}}}
//{{{
enum eMiniKey {
  KB_KEY_UNKNOWN       = -1,

  KB_KEY_SPACE         = 32,
  KB_KEY_APOSTROPHE    = 39,
  KB_KEY_COMMA         = 44,
  KB_KEY_MINUS         = 45,
  KB_KEY_PERIOD        = 46,
  KB_KEY_SLASH         = 47,
  KB_KEY_0             = 48,
  KB_KEY_1             = 49,
  KB_KEY_2             = 50,
  KB_KEY_3             = 51,
  KB_KEY_4             = 52,
  KB_KEY_5             = 53,
  KB_KEY_6             = 54,
  KB_KEY_7             = 55,
  KB_KEY_8             = 56,
  KB_KEY_9             = 57,
  KB_KEY_SEMICOLON     = 59,
  KB_KEY_EQUAL         = 61,
  KB_KEY_A             = 65,
  KB_KEY_B             = 66,
  KB_KEY_C             = 67,
  KB_KEY_D             = 68,
  KB_KEY_E             = 69,
  KB_KEY_F             = 70,
  KB_KEY_G             = 71,
  KB_KEY_H             = 72,
  KB_KEY_I             = 73,
  KB_KEY_J             = 74,
  KB_KEY_K             = 75,
  KB_KEY_L             = 76,
  KB_KEY_M             = 77,
  KB_KEY_N             = 78,
  KB_KEY_O             = 79,
  KB_KEY_P             = 80,
  KB_KEY_Q             = 81,
  KB_KEY_R             = 82,
  KB_KEY_S             = 83,
  KB_KEY_T             = 84,
  KB_KEY_U             = 85,
  KB_KEY_V             = 86,
  KB_KEY_W             = 87,
  KB_KEY_X             = 88,
  KB_KEY_Y             = 89,
  KB_KEY_Z             = 90,
  KB_KEY_LEFT_BRACKET  = 91,
  KB_KEY_BACKSLASH     = 92,
  KB_KEY_RIGHT_BRACKET = 93,
  KB_KEY_GRAVE_ACCENT  = 96,
  KB_KEY_WORLD_1       = 161,
  KB_KEY_WORLD_2       = 162,

  KB_KEY_ESCAPE        = 256,
  KB_KEY_ENTER         = 257,
  KB_KEY_TAB           = 258,
  KB_KEY_BACKSPACE     = 259,
  KB_KEY_INSERT        = 260,
  KB_KEY_DELETE        = 261,
  KB_KEY_RIGHT         = 262,
  KB_KEY_LEFT          = 263,
  KB_KEY_DOWN          = 264,
  KB_KEY_UP            = 265,
  KB_KEY_PAGE_UP       = 266,
  KB_KEY_PAGE_DOWN     = 267,
  KB_KEY_HOME          = 268,
  KB_KEY_END           = 269,
  KB_KEY_CAPS_LOCK     = 280,
  KB_KEY_SCROLL_LOCK   = 281,
  KB_KEY_NUM_LOCK      = 282,
  KB_KEY_PRINT_SCREEN  = 283,
  KB_KEY_PAUSE         = 284,
  KB_KEY_F1            = 290,
  KB_KEY_F2            = 291,
  KB_KEY_F3            = 292,
  KB_KEY_F4            = 293,
  KB_KEY_F5            = 294,
  KB_KEY_F6            = 295,
  KB_KEY_F7            = 296,
  KB_KEY_F8            = 297,
  KB_KEY_F9            = 298,
  KB_KEY_F10           = 299,
  KB_KEY_F11           = 300,
  KB_KEY_F12           = 301,
  KB_KEY_F13           = 302,
  KB_KEY_F14           = 303,
  KB_KEY_F15           = 304,
  KB_KEY_F16           = 305,
  KB_KEY_F17           = 306,
  KB_KEY_F18           = 307,
  KB_KEY_F19           = 308,
  KB_KEY_F20           = 309,
  KB_KEY_F21           = 310,
  KB_KEY_F22           = 311,
  KB_KEY_F23           = 312,
  KB_KEY_F24           = 313,
  KB_KEY_F25           = 314,
  KB_KEY_KP_0          = 320,
  KB_KEY_KP_1          = 321,
  KB_KEY_KP_2          = 322,
  KB_KEY_KP_3          = 323,
  KB_KEY_KP_4          = 324,
  KB_KEY_KP_5          = 325,
  KB_KEY_KP_6          = 326,
  KB_KEY_KP_7          = 327,
  KB_KEY_KP_8          = 328,
  KB_KEY_KP_9          = 329,
  KB_KEY_KP_DECIMAL    = 330,
  KB_KEY_KP_DIVIDE     = 331,
  KB_KEY_KP_MULTIPLY   = 332,
  KB_KEY_KP_SUBTRACT   = 333,
  KB_KEY_KP_ADD        = 334,
  KB_KEY_KP_ENTER      = 335,
  KB_KEY_KP_EQUAL      = 336,
  KB_KEY_LEFT_SHIFT    = 340,
  KB_KEY_LEFT_CONTROL  = 341,
  KB_KEY_LEFT_ALT      = 342,
  KB_KEY_LEFT_SUPER    = 343,
  KB_KEY_RIGHT_SHIFT   = 344,
  KB_KEY_RIGHT_CONTROL = 345,
  KB_KEY_RIGHT_ALT     = 346,
  KB_KEY_RIGHT_SUPER   = 347,
  KB_KEY_MENU          = 348
  };
//}}}
#define KB_KEY_LAST KB_KEY_MENU

//{{{
class cMiniFB {
public:
  // static
  static cMiniFB* create (const char* title, uint32_t width, uint32_t height, uint32_t flags);
  static const char* getKeyName (eMiniKey key);

  eMiniState update (void* buffer);
  eMiniState updateEvents();
  void close() { closed = true; }

  // gets
  bool isWindowActive() const { return isActive; }
  unsigned getWindowWidth() const { return windowWidth; }
  unsigned getWindowHeight() const { return windowHeight; }

  int64_t getPointerTimestamp() const { return pointerTimestamp; }
  const uint8_t* getPointerButtonBuffer() const { return pointerButtonStatus; }
  int getPointerX() const { return pointerPosX; }
  int getPointerY() const { return pointerPosY; }
  int getPointerPressure() const { return pointerPressure; }
  float getPointerWheelX() { return pointerWheelX; }
  float getPointerWheelY() { return pointerWheelY; }

  const uint8_t* getKeyBuffer() { return keyStatus; }

  void* getUserData() { return userData; }
  void getMonitorScale (float* scale_x, float* scale_y);

  // sets
  void setUserData (void* user_data) { userData = user_data; }
  bool setViewport (uint32_t offset_x, uint32_t offset_y, uint32_t width, uint32_t height);
  bool setViewportBestFit (uint32_t oldWidth, uint32_t oldHeight);
  //{{{  set C style callbacks
  void setActiveCallback (void(*callback)(cMiniFB* miniFB));
  void setResizeCallback (void(*callback)(cMiniFB* miniFB));
  void setCloseCallback  (bool(*callback)(cMiniFB* miniFB));
  void setKeyCallback    (void(*callback)(cMiniFB* miniFB));
  void setCharCallback   (void(*callback)(cMiniFB* miniFB));
  void setButtonCallback (void(*callback)(cMiniFB* miniFB));
  void setMoveCallback   (void(*callback)(cMiniFB* miniFB));
  void setWheelCallback  (void(*callback)(cMiniFB* miniFB));
  void setEnterCallback  (void(*callback)(cMiniFB* miniFB));
  //}}}
  //{{{  set function style callbacks
  void setActiveFunc (std::function <void (cMiniFB*)> func);
  void setResizeFunc (std::function <void (cMiniFB*)> func);
  void setCloseFunc  (std::function <bool (cMiniFB*)> func);
  void setKeyFunc    (std::function <void (cMiniFB*)> func);
  void setCharFunc   (std::function <void (cMiniFB*)> func);
  void setButtonFunc (std::function <void (cMiniFB*)> func);
  void setMoveFunc   (std::function <void (cMiniFB*)> func);
  void setWheelFunc  (std::function <void (cMiniFB*)> func);
  void setEnterFunc  (std::function <void (cMiniFB*)> func);
  //}}}

  #ifdef _WIN32
    LRESULT CALLBACK processMessage (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  #endif

  //{{{  vars
  void(*activeFunc)(cMiniFB* miniFB) = nullptr;
  void(*resizeFunc)(cMiniFB* miniFB) = nullptr;
  bool(*closeFunc)(cMiniFB* miniFB) = nullptr;
  void(*keyFunc)(cMiniFB* miniFB) = nullptr;
  void(*charFunc)(cMiniFB* miniFB) = nullptr;
  void(*buttonFunc)(cMiniFB* miniFB) = nullptr;
  void(*moveFunc)(cMiniFB* miniFB) = nullptr;
  void(*wheelFunc)(cMiniFB* miniFB) = nullptr;
  void(*enterFunc)(cMiniFB* miniFB) = nullptr;

  uint32_t windowWidth = 0;
  uint32_t windowHeight = 0;
  uint32_t windowScaledWidth = 0;
  uint32_t windowScaledHeight = 0;

  uint32_t dstOffsetX = 0;
  uint32_t dstOffsetY = 0;
  uint32_t dstWidth = 0;
  uint32_t dstHeight = 0;

  float    factorX = 0;
  float    factorY = 0;
  float    factorWidth = 0;
  float    factorHeight = 0;

  void*    drawBuffer = nullptr;
  uint32_t bufferWidth = 0;
  uint32_t bufferHeight = 0;
  uint32_t bufferStride = 0;

  uint32_t isPressed = 0;
  bool     isActive = false;
  bool     isInitialized = false;
  bool     isDown = false;
  bool     pointerInside = false;
  bool     closed = false;

  uint32_t codepoint = 0;
  eMiniKey keyCode = eMiniKey(0);
  uint8_t  keyStatus[512] = {0};
  uint32_t modifierKeys = 0;

  int32_t  pointerTimestamp = 0;
  uint8_t  pointerButtonStatus[8] = {0};
  int32_t  pointerPosX = 0;
  int32_t  pointerPosY = 0;
  int32_t  pointerPressure = 0;
  int32_t  pointerTiltX = 0;
  int32_t  pointerTiltY = 0;
  float    pointerWheelX = 0;
  float    pointerWheelY = 0;

  uint32_t textureId;

  #ifdef _WIN32
    HWND       window = 0;
    WNDCLASS   wc;
    HDC        hdc = 0;
    HGLRC      hGLRC = 0;
  #else
    Window     window = 0;
    Display*   display = 0;
    int        screen = 0;
    GC         gc = 0;
    GLXContext context = 0;
  #endif
  //}}}

private:
  #ifndef _WIN32
    void processEvent (XEvent* event);
  #endif

  bool init (const char* title, uint32_t width, uint32_t height, uint32_t flags);
  void initKeycodes();
  void freeResources();

  bool createGLcontext();
  void initGL();
  void resizeGL();
  void redrawGL (const void* pixels);
  void destroyGLcontext();

  void resizeDst (uint32_t width, uint32_t height);
  void calcDstFactor (uint32_t width, uint32_t height);

  void* userData = nullptr;
  };
//}}}
//{{{
class cMiniCallbackStub {
public:
  cMiniCallbackStub() {}

  // statics
  static cMiniCallbackStub* getInstance (cMiniFB* miniFB);

  static void activeStub (cMiniFB* miniFB);
  static void resizeStub (cMiniFB* miniFB);
  static bool closeStub  (cMiniFB* miniFB);
  static void keyStub    (cMiniFB* miniFB);
  static void charStub   (cMiniFB* miniFB);
  static void buttonStub (cMiniFB* miniFB);
  static void moveStub   (cMiniFB* miniFB);
  static void wheelStub  (cMiniFB* miniFB);
  static void enterStub  (cMiniFB* miniFB);

  // vars
  cMiniFB* mMiniFB = nullptr;

  std::function <void (cMiniFB* miniFB)> mActiveFunc;
  std::function <void (cMiniFB* miniFB)> mResizeFunc;
  std::function <bool (cMiniFB* miniFB)> mCloseFunc;
  std::function <void (cMiniFB* miniFB)> mKeyFunc;
  std::function <void (cMiniFB* miniFB)> mCharFunc;
  std::function <void (cMiniFB* miniFB)> mButtonFunc;
  std::function <void (cMiniFB* miniFB)> mMoveFunc;
  std::function <void (cMiniFB* miniFB)> mWheelFunc;
  std::function <void (cMiniFB* miniFB)> mEnterFunc;
  };
//}}}
