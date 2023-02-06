#pragma once

struct sWindowData;

bool createGLcontext (sWindowData* windowData);
void destroyGLcontext (sWindowData* windowData);

void initGL (sWindowData* windowData);
void redrawGL (sWindowData* windowData, const void *pixels);
void resizeGL (sWindowData* windowData);
