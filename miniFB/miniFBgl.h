#pragma once
#include "windowData.h"

bool createGLcontext (SWindowData* windowData);
void destroyGLcontext (SWindowData* windowData);

void initGL (SWindowData* windowData);
void redrawGL (SWindowData* windowData, const void *pixels);
void resizeGL (SWindowData* windowData);
