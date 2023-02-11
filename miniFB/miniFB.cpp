// miniFB.cpp
#include "miniFB.h"

#include <vector>
#include "miniFBinternal.h"
#include "sInfo.h"

short int gKeycodes[512] = { 0 };

//{{{
sOpaqueInfo* open (const char* title, unsigned width, unsigned height) {

  return openEx (title, width, height, 0);
  }
//}}}
//{{{
eUpdateState update (sOpaqueInfo* opaqueInfo, void *buffer) {

  if (!opaqueInfo)
    return STATE_INVALID_WINDOW;

  return updateEx (opaqueInfo, buffer, ((sInfo*)(opaqueInfo))->bufferWidth, ((sInfo*)(opaqueInfo))->bufferHeight);
  }
//}}}
//{{{
void close (sOpaqueInfo* opaqueInfo) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->closed = true;
  }
//}}}

// gets
//{{{
cStub* cStub::getInstance (sOpaqueInfo *opaqueInfo) {

  //{{{
  struct stub_vector {
    std::vector<cStub*> instances;

    stub_vector() = default;
    //{{{
    ~stub_vector() {
      for (cStub* instance : instances)
        delete instance;
      }
    //}}}

    cStub* Get (sOpaqueInfo *opaqueInfo) {
      for(cStub *instance : instances) {
        if(instance->m_opaqueInfo == opaqueInfo) {
          return instance;
          }
        }
      instances.push_back (new cStub);
      instances.back()->m_opaqueInfo = opaqueInfo;
      return instances.back();
      }
    };
  //}}}
  static stub_vector gInstances;

  return gInstances.Get (opaqueInfo);
  }
//}}}
void* getUserData (sOpaqueInfo* opaqueInfo) { return opaqueInfo ? ((sInfo*)(opaqueInfo))->userData : 0; }

//{{{
bool isWindowActive (sOpaqueInfo* opaqueInfo)  {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->isActive : 0; }
//}}}
//{{{
unsigned getWindowWidth (sOpaqueInfo* opaqueInfo)  {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->window_width : 0; }
//}}}
//{{{
unsigned getWindowHeight (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->window_height : 0; }
//}}}

//{{{
int getPointerX (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->pointerPosX : 0; }
//}}}
//{{{
int getPointerY (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->pointerPosY : 0; }
//}}}
//{{{
int getPointerPressure (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->pointerPressure : 0; }
//}}}
//{{{
int64_t getPointerTimestamp (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->pointerTimestamp : 0; }
//}}}

//{{{
float getPointerWheelX (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->pointerWheelX : 0; }
//}}}
//{{{
float getPointerWheelY (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->pointerWheelY : 0; }
//}}}

//{{{
const uint8_t* getPointerButtonBuffer (sOpaqueInfo* opaqueInfo) {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->pointerButtonStatus : 0; }
//}}}
//{{{
const uint8_t* getKeyBuffer (sOpaqueInfo* opaqueInfo)  {
  return opaqueInfo ? ((sInfo*)(opaqueInfo))->keyStatus : 0; }
//}}}
//{{{
const char* getKeyName (eKey key) {

  switch (key) {
    case KB_KEY_SPACE: return "Space";
    case KB_KEY_APOSTROPHE: return "Apostrophe";
    case KB_KEY_COMMA: return "Comma";
    case KB_KEY_MINUS: return "Minus";
    case KB_KEY_PERIOD: return "Period";
    case KB_KEY_SLASH: return "Slash";

    //{{{  numbers
    case KB_KEY_0: return "0";
    case KB_KEY_1: return "1";
    case KB_KEY_2: return "2";
    case KB_KEY_3: return "3";
    case KB_KEY_4: return "4";
    case KB_KEY_5: return "5";
    case KB_KEY_6: return "6";
    case KB_KEY_7: return "7";
    case KB_KEY_8: return "8";
    case KB_KEY_9: return "9";
    //}}}

    case KB_KEY_SEMICOLON: return "Semicolon";
    case KB_KEY_EQUAL: return "Equal";

    //{{{  alpha
    case KB_KEY_A: return "A";
    case KB_KEY_B: return "B";
    case KB_KEY_C: return "C";
    case KB_KEY_D: return "D";
    case KB_KEY_E: return "E";
    case KB_KEY_F: return "F";
    case KB_KEY_G: return "G";
    case KB_KEY_H: return "H";
    case KB_KEY_I: return "I";
    case KB_KEY_J: return "J";
    case KB_KEY_K: return "K";
    case KB_KEY_L: return "L";
    case KB_KEY_M: return "M";
    case KB_KEY_N: return "N";
    case KB_KEY_O: return "O";
    case KB_KEY_P: return "P";
    case KB_KEY_Q: return "Q";
    case KB_KEY_R: return "R";
    case KB_KEY_S: return "S";
    case KB_KEY_T: return "T";
    case KB_KEY_U: return "U";
    case KB_KEY_V: return "V";
    case KB_KEY_W: return "W";
    case KB_KEY_X: return "X";
    case KB_KEY_Y: return "Y";
    case KB_KEY_Z: return "Z";
    //}}}
    //{{{  punctuation
    case KB_KEY_LEFT_BRACKET: return "Left_Bracket";
    case KB_KEY_BACKSLASH: return "Backslash";
    case KB_KEY_RIGHT_BRACKET: return "Right_Bracket";
    case KB_KEY_GRAVE_ACCENT: return "Grave_Accent";
    case KB_KEY_WORLD_1: return "World_1";
    case KB_KEY_WORLD_2: return "World_2";
    case KB_KEY_ESCAPE: return "Escape";
    case KB_KEY_ENTER: return "Enter";
    case KB_KEY_TAB: return "Tab";
    case KB_KEY_BACKSPACE: return "Backspace";
    case KB_KEY_INSERT: return "Insert";
    case KB_KEY_DELETE: return "Delete";
    case KB_KEY_RIGHT: return "Right";
    case KB_KEY_LEFT: return "Left";
    case KB_KEY_DOWN: return "Down";
    case KB_KEY_UP: return "Up";
    case KB_KEY_PAGE_UP: return "Page_Up";
    case KB_KEY_PAGE_DOWN: return "Page_Down";
    case KB_KEY_HOME: return "Home";
    case KB_KEY_END: return "End";
    case KB_KEY_CAPS_LOCK: return "Caps_Lock";
    case KB_KEY_SCROLL_LOCK: return "Scroll_Lock";
    case KB_KEY_NUM_LOCK: return "Num_Lock";
    case KB_KEY_PRINT_SCREEN: return "Print_Screen";
    case KB_KEY_PAUSE: return "Pause";
    //}}}
    //{{{  function
    case KB_KEY_F1: return "F1";
    case KB_KEY_F2: return "F2";
    case KB_KEY_F3: return "F3";
    case KB_KEY_F4: return "F4";
    case KB_KEY_F5: return "F5";
    case KB_KEY_F6: return "F6";
    case KB_KEY_F7: return "F7";
    case KB_KEY_F8: return "F8";
    case KB_KEY_F9: return "F9";
    case KB_KEY_F10: return "F10";
    case KB_KEY_F11: return "F11";
    case KB_KEY_F12: return "F12";
    case KB_KEY_F13: return "F13";
    case KB_KEY_F14: return "F14";
    case KB_KEY_F15: return "F15";
    case KB_KEY_F16: return "F16";
    case KB_KEY_F17: return "F17";
    case KB_KEY_F18: return "F18";
    case KB_KEY_F19: return "F19";
    case KB_KEY_F20: return "F20";
    case KB_KEY_F21: return "F21";
    case KB_KEY_F22: return "F22";
    case KB_KEY_F23: return "F23";
    case KB_KEY_F24: return "F24";
    case KB_KEY_F25: return "F25";
    //}}}
    //{{{  numpad
    case KB_KEY_KP_0: return "KP_0";
    case KB_KEY_KP_1: return "KP_1";
    case KB_KEY_KP_2: return "KP_2";
    case KB_KEY_KP_3: return "KP_3";
    case KB_KEY_KP_4: return "KP_4";
    case KB_KEY_KP_5: return "KP_5";
    case KB_KEY_KP_6: return "KP_6";
    case KB_KEY_KP_7: return "KP_7";
    case KB_KEY_KP_8: return "KP_8";
    case KB_KEY_KP_9: return "KP_9";
    case KB_KEY_KP_DECIMAL: return "KP_Decimal";
    case KB_KEY_KP_DIVIDE: return "KP_Divide";
    case KB_KEY_KP_MULTIPLY: return "KP_Multiply";
    case KB_KEY_KP_SUBTRACT: return "KP_Subtract";
    case KB_KEY_KP_ADD: return "KP_Add";
    case KB_KEY_KP_ENTER: return "KP_Enter";
    case KB_KEY_KP_EQUAL: return "KP_Equal";
    //}}}

    case KB_KEY_LEFT_SHIFT: return "Left_Shift";
    case KB_KEY_LEFT_CONTROL: return "Left_Control";
    case KB_KEY_LEFT_ALT: return "Left_Alt";
    case KB_KEY_LEFT_SUPER: return "Left_Super";

    case KB_KEY_RIGHT_CONTROL: return "Right_Control";
    case KB_KEY_RIGHT_SHIFT: return "Right_Shift";
    case KB_KEY_RIGHT_ALT: return "Right_Alt";
    case KB_KEY_RIGHT_SUPER: return "Right_Super";

    case KB_KEY_MENU: return "Menu";

    case KB_KEY_UNKNOWN: return "Unknown";
    }

  return "Unknown";
  }
//}}}

// sets
//{{{
void setUserData (sOpaqueInfo* opaqueInfo, void* user_data) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->userData = user_data;
  }
//}}}
//{{{
bool setViewportBestFit (sOpaqueInfo* opaqueInfo, unsigned old_width, unsigned old_height) {

  if (opaqueInfo) {
    unsigned new_width  = ((sInfo*)(opaqueInfo))->window_width;
    unsigned new_height = ((sInfo*)(opaqueInfo))->window_height;

    float scale_x = new_width  / (float) old_width;
    float scale_y = new_height / (float) old_height;
    if (scale_x >= scale_y)
      scale_x = scale_y;
    else
      scale_y = scale_x;

    unsigned finalWidth  = (unsigned)((old_width  * scale_x) + 0.5f);
    unsigned finalHeight = (unsigned)((old_height * scale_y) + 0.5f);

    unsigned offset_x = (new_width  - finalWidth)  >> 1;
    unsigned offset_y = (new_height - finalHeight) >> 1;

    getMonitorScale (opaqueInfo, &scale_x, &scale_y);
    return setViewport (opaqueInfo, (unsigned)(offset_x / scale_x), (unsigned)(offset_y / scale_y),
                                   (unsigned)(finalWidth / scale_x), (unsigned)(finalHeight / scale_y));
    }

  return false;
  }
//}}}

//{{{
void keyDefault (sOpaqueInfo* opaqueInfo) {

  if (((sInfo*)(opaqueInfo))->keyCode == KB_KEY_ESCAPE) {
    if (!((sInfo*)(opaqueInfo))->closeFunc ||
         ((sInfo*)(opaqueInfo))->closeFunc ((sOpaqueInfo*)opaqueInfo))
      ((sInfo*)(opaqueInfo))->closed = true;
    }
  }
//}}}

// set callbacks
//{{{
void setActiveCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->activeFunc = callback;
  }
//}}}
//{{{
void setResizeCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->resizeFunc = callback;
  }
//}}}
//{{{
void setCloseCallback  (sOpaqueInfo* opaqueInfo, closeFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->closeFunc = callback;
  }
//}}}
//{{{
void setKeyCallback    (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->keyFunc = callback;
  }
//}}}
//{{{
void setCharCallback   (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->charFunc = callback;
  }
//}}}
//{{{
void setButtonCallback (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->buttonFunc = callback;
  }
//}}}
//{{{
void setMoveCallback   (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->moveFunc = callback;
  }
//}}}
//{{{
void setWheelCallback  (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->wheelFunc = callback;
  }
//}}}
//{{{
void setEnterCallback  (sOpaqueInfo* opaqueInfo, infoFuncType callback) {

  if (opaqueInfo)
    ((sInfo*)(opaqueInfo))->enterFunc = callback;
  }
//}}}

// set callback lamdas
//{{{
void setActiveCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mActiveFunc = std::bind (func, _1);
  setActiveCallback (opaqueInfo, cStub::activeStub);
  }
//}}}
//{{{
void setResizeCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mResizeFunc = std::bind(func, _1);
  setResizeCallback(opaqueInfo, cStub::resizeStub);
  }
//}}}
//{{{
void setCloseCallback  (std::function <bool (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mCloseFunc = std::bind(func, _1);
  setCloseCallback(opaqueInfo, cStub::closeStub);
  }
//}}}
//{{{
void setKeyCallback    (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo *opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mKeyFunc = std::bind (func, _1);
  setKeyCallback (opaqueInfo, cStub::keyStub);
  }
//}}}
//{{{
void setCharCallback   (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mCharFunc = std::bind (func, _1);
  setCharCallback (opaqueInfo, cStub::charStub);
  }
//}}}
//{{{
void setButtonCallback (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo *opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mButtonFunc = std::bind (func, _1);
  setButtonCallback (opaqueInfo, cStub::buttonStub);
  }
//}}}
//{{{
void setMoveCallback   (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo* opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mMoveFunc = std::bind (func, _1);
  setMoveCallback (opaqueInfo, cStub::moveStub);
  }
//}}}
//{{{
void setWheelCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo *opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mWheelFunc = std::bind (func, _1);
  setWheelCallback (opaqueInfo, cStub::wheelStub);
  }
//}}}
//{{{
void setEnterCallback  (std::function <void (sOpaqueInfo*)> func, sOpaqueInfo *opaqueInfo) {

  using namespace std::placeholders;

  cStub::getInstance (opaqueInfo)->mEnterFunc = std::bind (func, _1);
  setEnterCallback (opaqueInfo, cStub::enterStub);
  }
//}}}

// callback stubs
//{{{
void cStub::activeStub (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mActiveFunc (opaqueInfo);
  }
//}}}
//{{{
void cStub::resizeStub (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mResizeFunc (opaqueInfo);
  }
//}}}
//{{{
bool cStub::closeStub  (sOpaqueInfo* opaqueInfo) {
  return cStub::getInstance (opaqueInfo)->mCloseFunc (opaqueInfo);
  }
//}}}
//{{{
void cStub::keyStub    (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mKeyFunc(opaqueInfo);
  }
//}}}
//{{{
void cStub::charStub   (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mCharFunc(opaqueInfo);
  }
//}}}
//{{{
void cStub::buttonStub (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mButtonFunc(opaqueInfo);
  }
//}}}
//{{{
void cStub::moveStub   (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mMoveFunc(opaqueInfo);
  }
//}}}
//{{{
void cStub::wheelStub  (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mWheelFunc(opaqueInfo);
  }
//}}}
//{{{
void cStub::enterStub  (sOpaqueInfo* opaqueInfo) {
  cStub::getInstance (opaqueInfo)->mEnterFunc(opaqueInfo);
  }
//}}}
