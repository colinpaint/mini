// sInfo - non opaque sOpaqueInfo
#pragma once

struct sInfo {
  void* specificInfo;
  void* userData;

  infoFuncType  activeFunc;
  infoFuncType  resizeFunc;
  closeFuncType closeFunc;
  infoFuncType  keyFunc;
  infoFuncType  charFunc;
  infoFuncType  buttonFunc;
  infoFuncType  moveFunc;
  infoFuncType  wheelFunc;
  infoFuncType  enterFunc;

  uint32_t window_width;
  uint32_t window_height;
  uint32_t windowScaledWidth;
  uint32_t windowScaledHeight;

  uint32_t dst_offset_x;
  uint32_t dst_offset_y;
  uint32_t dst_width;
  uint32_t dst_height;

  float    factor_x;
  float    factor_y;
  float    factor_width;
  float    factor_height;

  void*    draw_buffer;
  uint32_t bufferWidth;
  uint32_t bufferHeight;
  uint32_t bufferStride;

  int32_t  pointerTimestamp;
  uint8_t  pointerButtonStatus[8];
  int32_t  pointerPosX;
  int32_t  pointerPosY;
  int32_t  pointerPressure;
  int32_t  pointerTiltX;
  int32_t  pointerTiltY;
  float    pointerWheelX;
  float    pointerWheelY;

  uint32_t isPressed;
  bool     isActive;
  bool     isInitialized;
  bool     isDown;
  bool     pointerInside;
  bool     closed;

  uint32_t codepoint;
  eKey     keyCode;
  uint8_t  keyStatus[512];
  uint32_t modifierKeys;

  uint32_t textureId;
  sMiniFBtimer* timer;
  };
