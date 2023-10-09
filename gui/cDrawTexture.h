// cDrawTexture.h
#pragma once
#include "../common/basicTypes.h"
#include "cTexture.h"

class cDrawAA;
class cDrawTexture : public cTexture {
public:
  // static creates
  static void createStaticResources (float menuTextHeight);

  cDrawTexture() = default;
  cDrawTexture (int32_t width, int32_t height, uPixel* pixels) : cTexture (width, height, pixels) {}
  virtual ~cDrawTexture();

  uint32_t getNumFontChars() const;

  // draws
  void drawGradH (const cColor& colorLeft, const cColor& colorRight, const cRect& rect);
  void drawGradV (const cColor& colorTop, const cColor& colorBottom, const cRect& rect);
  void drawGrad (const cColor& colorTopLeft, const cColor& colorTopRight,
                 const cColor& colorBottomLeft, const cColor& colorBottomRight, const cRect& rect);
  void drawGradRadial (const cColor& color, cPoint centre, cPoint radius);
  void drawEllipse (const cColor& color, cPoint centre, cPoint radius, float width = 0.f);
  void drawLine (const cColor& color, cPoint point1, cPoint point2, float width);

  cPoint measureText (cPoint size, const std::string& text, uint32_t font = 0);
  cPoint drawText (const cColor& color, const cRect& rect, const std::string& text, uint32_t font = 0);
  cPoint drawTextShadow (const cColor& color, const cRect& rect, const std::string& text, uint32_t font = 0);

  // draw edges
  void addEdgeFrom (cPoint point);
  void addEdgeTo (cPoint point);
  void addLine (cPoint point1, cPoint point2, float width = 1.f);
  void addArrowHead (cPoint point1, cPoint point2, float width = 1.f);
  void addTriangle (cPoint point1, cPoint point2, cPoint point3);
  void addEllipse (cPoint centre, cPoint radius, float outlineWidth = 0.f);
  void drawEdges (const cColor& color);

  void drawBorder (const cColor& color, const cRect& rect, float width = 2.f);
  void drawRounded (const cColor& color, const cRect& rect, float radius = 2.f);

private:
  cDrawAA* mDrawAA = nullptr;
  };
