#pragma once

class sInfo;

bool createGLcontext (sInfo* info);
void destroyGLcontext (sInfo* info);

void initGL (sInfo* info);
void redrawGL (sInfo* info, const void *pixels);
void resizeGL (sInfo* info);
