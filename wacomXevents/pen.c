// print raw values from a Wacom pen tip to the terminal
// gcc -o pen pen.c -lX11 -lXi
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

//{{{
static void printValuators (Display* display, XIAnyClassInfo** classes, int num_classes) {

  for (int i = 0; i < num_classes; i++) {
    if (classes[i]->type == XIValuatorClass) {
      XIValuatorClassInfo* v = (XIValuatorClassInfo*)classes[i];
      printf ("Valuator %d: '%s'\n", v->number, (v->label) ?  XGetAtomName (display, v->label) : "No label");
      printf ("- Range: %f - %f\n", v->min, v->max);
      printf ("- Resolution: %d units/m\n", v->resolution);
      printf ("- Mode: %s\n", v->mode == XIModeAbsolute ? "absolute": "relative");
      if (v->mode == XIModeAbsolute)
        printf ("- Current value: %f\n", v->value);
      }
    }
  }
//}}}
//{{{
static void deviceInfo (Display *display, int deviceid) {

  int ndevices;
  XIDeviceInfo* dev = XIQueryDevice (display, deviceid, &ndevices);
  printf ("Device Name: '%s' (%d)\n", dev->name, dev->deviceid);
  printValuators (display, dev->classes, dev->num_classes);
}
//}}}
//{{{
static int findStylus (Display *display, int deviceid) {

  int ndevices;
  XIDeviceInfo* info = XIQueryDevice (display, deviceid, &ndevices);

  int stylus = 0;
  int i;
  for (i = 0; i < ndevices; i++) {
    XIDeviceInfo* dev = &info[i];
    if (strstr (dev->name, "stylus")) {
      stylus = dev->deviceid;
      break;
      }
    }

  if (stylus) {
    printf ("Selected Device with ID %i\n", stylus);
    deviceInfo (display, stylus);
    }
  else
    printf ("no tablet connected\n");

  return stylus;
  }
//}}}
//{{{
static Window createWin (Display *dpy) {

  XIEventMask mask;
  int stylus;

  Window win = XCreateSimpleWindow (dpy, DefaultRootWindow(dpy), 0, 0, 1200, 800, 0, 0, WhitePixel(dpy, 0));

  stylus  = findStylus (dpy, XIAllDevices);
  if (stylus)
    mask.deviceid = stylus;
  else
    exit(0);

  mask.mask_len = XIMaskLen (XI_RawMotion);
  mask.mask = calloc (mask.mask_len, sizeof(char));
  memset (mask.mask, 0, mask.mask_len);
  XISetMask (mask.mask, XI_RawMotion);
  XISelectEvents (dpy, DefaultRootWindow(dpy), &mask, 1);
  free (mask.mask);

  XMapWindow (dpy, win);
  XSync (dpy, True);
  return win;
}
//}}}
//{{{
static void printRawMotion (XIRawEvent *event) {

  double* valuator = event->valuators.values;
  printf ("- device:%d event type:%d\n", event->deviceid, event->evtype);

  for (int i = 0; i < event->valuators.mask_len * 8; i++) {
    if (XIMaskIsSet (event->valuators.mask, i)) {
      printf ("  - raw valuator %d:%f\n", i, *valuator);
      valuator++;
      }
    }
  }
//}}}

int main (int argc, char **argv) {

  Display* display = XOpenDisplay (NULL);
  if (!display) {
    fprintf (stderr, "Failed to open display\n");
    return -1;
    }

  int xi_opcode, event, error;
  if (!XQueryExtension (display, "XInputExtension", &xi_opcode, &event, &error)) {
    printf ("X Input extension not available\n");
    return -1;
    }

  // Is XInput 2 available? */
  int major = 2;
  int minor = 0;
  if (XIQueryVersion (display, &major, &minor) == BadRequest) {
    printf ("XInput 2 not available\n");
    return -1;
    }
  printf ("XInput2 %d %d\n", major, minor);

  Window win = createWin (display);

  XEvent ev;
  while (1) {
    XGenericEventCookie* cookie = &ev.xcookie;
    XNextEvent (display, &ev);
    if (cookie->type != GenericEvent ||
        cookie->extension != xi_opcode ||
        !XGetEventData (display, cookie))
      continue;

    if (cookie->evtype == XI_RawMotion)
      printRawMotion (cookie->data);

    XFreeEventData (display, cookie);
    }

  XCloseDisplay (display);
  return 0;
  }
