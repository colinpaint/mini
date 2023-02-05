// miniFB.h - main interface to miniFB
#pragma once
#include <cstdint>
#include "miniFBenums.h"

//{{{
#ifdef __cplusplus
  extern "C" {
#endif
//}}}
#define MFB_RGB(r,g,b) (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | ((uint32_t)b)

struct mfb_window* mfbOpen (const char* title, unsigned width, unsigned height);
struct mfb_window* mfbOpenEx (const char* title, unsigned width, unsigned height, unsigned flags);

mfb_update_state mfbUpdate (struct mfb_window* window, void* buffer);
mfb_update_state mfbUpdateEx (struct mfb_window* window, void* buffer, unsigned width, unsigned height);
mfb_update_state mfbUpdateEvents (struct mfb_window* window);

void mfbClose (struct mfb_window* window);

// gets
bool mfbIsWindowActive (struct mfb_window* window);
unsigned mfbGetWindowWidth (struct mfb_window* window);
unsigned mfbGetWindowHeight (struct mfb_window* window);

int mfbGetMouseX (struct mfb_window* window);          // Last mouse pos X
int mfbGetMouseY (struct mfb_window* window);          // Last mouse pos Y
int mfbGetMousePressure (struct mfb_window* window);   // Last mouse pressure
int64_t mfbGetMouseTimestamp (struct mfb_window* window);  // Last mouse timestamp

float mfbGetMouseScrollX (struct mfb_window* window); // Mouse wheel X as a sum. When you call this function it resets.
float mfbGetMouseScrollY (struct mfb_window* window); // Mouse wheel Y as a sum. When you call this function it resets.

const uint8_t* mfbGetMouseButtonBuffer (struct mfb_window* window); // One byte for every button. Press (1), Release 0. (up to 8 buttons)
const uint8_t* mfbGetKeyBuffer (struct mfb_window* window);          // One byte for every key. Press (1), Release 0.
const char* mfbGetKeyName (mfb_key key);

void mfbGetMonitorScale (struct mfb_window* window, float* scale_x, float* scale_y);

// sets
void mfb_set_user_data (struct mfb_window* window, void* user_data);
void* mfb_get_user_data (struct mfb_window* window);

bool mfbSetViewport (struct mfb_window* window, unsigned offset_x, unsigned offset_y, unsigned width, unsigned height);
bool mfbSetViewportBestFit (struct mfb_window* window, unsigned old_width, unsigned old_height);

// fps
void mfb_set_target_fps (uint32_t fps);
unsigned mfb_get_target_fps();
bool mfbWaitSync (struct mfb_window* window);

// callbacks
void mfb_set_activeCallback (struct mfb_window* window, mfb_active_func callback);
void mfb_set_resizeCallback (struct mfb_window* window, mfb_resize_func callback);
void mfb_set_closeCallback (struct mfb_window* window, mfb_close_func callback);

void mfb_set_keyboardCallback (struct mfb_window* window, mfb_keyboard_func callback);
void mfb_set_char_inputCallback (struct mfb_window* window, mfb_char_input_func callback);

void mfb_set_mouse_buttonCallback (struct mfb_window* window, mfb_mouse_button_func callback);
void mfb_set_mouse_moveCallback (struct mfb_window* window, mfb_mouse_move_func callback);
void mfb_set_mouse_scrollCallback (struct mfb_window* window, mfb_mouse_scroll_func callback);

// timer
struct mfb_timer* mfb_timer_create();
void mfb_timer_destroy (struct mfb_timer* tmr);
void  mfb_timer_reset (struct mfb_timer* tmr);
double mfb_timer_now( struct mfb_timer* tmr);
double mfb_timer_delta (struct mfb_timer* tmr);
double mfb_timer_get_frequency();
double mfb_timer_get_resolution();
//{{{
#ifdef __cplusplus
  }
#endif
//}}}
