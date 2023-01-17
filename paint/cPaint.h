// cPaint.h
#pragma once
#include <cstdint>
#include <string>
#include <deque>

#include "../common/basicTypes.h"
#include "../gui/cWindow.h"

//{{{
class cLayer {
public:
  cLayer (const std::string& name, const cColor& color, cPoint pos) : mName(name), mPos(pos), mColor(color) {}
  virtual ~cLayer() {}

  std::string getName() const { return mName; }
  void setName (const std::string& name) { mName = name; }

  virtual std::string getType() const = 0;
  virtual void addPoint (cPoint pos) = 0;

  virtual bool pick (cPoint pos) = 0;
  virtual void prox (cPoint pos) = 0;
  virtual void proxExit () = 0;
  virtual void down (cPoint pos) = 0;
  virtual void move (cPoint pos, cPoint inc) = 0;
  virtual void up (cPoint pos, bool mouseMoved) = 0;
  virtual void wheel (int delta, cPoint pos) = 0;

  virtual void draw (cWindow& window) = 0;

protected:
  std::string mName;
  cPoint mPos;
  cColor mColor;
  };
//}}}
//{{{
class cPaintLayer : public cLayer {
public:
  cPaintLayer (const std::string& name, const cColor& color, cPoint pos, float width);
  virtual ~cPaintLayer() {}

  virtual std::string getType() const final { return "paint"; }
  virtual void addPoint (cPoint pos) final;

  virtual bool pick (cPoint pos) final;
  virtual void prox (cPoint pos) final;
  virtual void proxExit () final;
  virtual void down (cPoint pos) final;
  virtual void move (cPoint pos, cPoint inc) final;
  virtual void up (cPoint pos, bool mouseMoved) final;
  virtual void wheel (int delta, cPoint pos) final;

  virtual void draw (cWindow& window) final;

protected:
  float mWidth = 1.f;
  std::vector<cPoint> mLine;
  cRect mExtent = {0,0,0,0};
  };
//}}}
//{{{
class cRectangleLayer : public cLayer {
public:
  cRectangleLayer(const std::string& name, const cColor& color, cPoint pos, cPoint length)
    : cLayer(name, color, pos), mLength(length) {}
  virtual ~cRectangleLayer() {}

  virtual std::string getType() const final { return "rectangle"; }
  virtual void addPoint (cPoint pos) final { (void)pos; }

  virtual bool pick (cPoint pos) final;
  virtual void prox (cPoint pos) final;
  virtual void proxExit () final;
  virtual void down (cPoint pos) final;
  virtual void move (cPoint pos, cPoint inc) final;
  virtual void up (cPoint pos, bool mouseMoved) final;
  virtual void wheel (int delta, cPoint pos) final;

  virtual void draw (cWindow& window) final;

protected:
  cPoint mLength;
  };
//}}}
//{{{
class cEllipseLayer : public cLayer {
public:
  cEllipseLayer (const std::string& name, const cColor& color, cPoint pos, float radius, float width = 0.f)
    : cLayer(name, color, pos), mRadius(radius), mWidth(width) {}
  virtual ~cEllipseLayer() {}

  virtual std::string getType() const final { return "circle"; }
  virtual void addPoint (cPoint pos) final { (void)pos; }

  virtual bool pick (cPoint pos) final;
  virtual void prox (cPoint pos) final;
  virtual void proxExit () final;
  virtual void down (cPoint pos) final;
  virtual void move (cPoint pos, cPoint inc) final;
  virtual void up (cPoint pos, bool mouseMoved) final;
  virtual void wheel (int delta, cPoint pos) final;

  virtual void draw (cWindow& window) final;

protected:
  float mRadius;
  float mWidth;
  };
//}}}
//{{{
class cTextLayer : public cLayer {
public:
  cTextLayer (const std::string& name, const cColor& color, cPoint pos, const std::string text)
    : cLayer(name, color, pos), mText(text) {}
  virtual ~cTextLayer() {}

  virtual std::string getType() const final { return "text"; }
  virtual void addPoint (cPoint pos) final { (void)pos; }

  virtual bool pick (cPoint pos) final;
  virtual void prox (cPoint pos) final;
  virtual void proxExit () final;
  virtual void down (cPoint pos) final;
  virtual void move (cPoint pos, cPoint inc) final;
  virtual void up (cPoint pos, bool mouseMoved) final;
  virtual void wheel (int delta, cPoint pos) final;

  virtual void draw (cWindow& window) final;

protected:
  std::string mText;
  cPoint mLength = {0,0};
  };
//}}}
//{{{
class cTextureLayer : public cLayer {
public:
  cTextureLayer (const std::string& name, cTexture texture, cPoint pos, float size, const cColor& color = kBlack)
    : cLayer(name, color,pos), mTexture(texture), mSize(size), mAngle(0) {}
  virtual ~cTextureLayer() {}

  virtual std::string getType() const final { return "texture"; }
  virtual void addPoint (cPoint pos) final { (void)pos; }

  virtual bool pick (cPoint pos) final;
  virtual void prox (cPoint pos) final;
  virtual void proxExit () final;
  virtual void down (cPoint pos) final;
  virtual void move (cPoint pos, cPoint inc) final;
  virtual void up (cPoint pos, bool mouseMoved) final;
  virtual void wheel (int delta, cPoint pos) final;

  virtual void draw (cWindow& window) final;

protected:
  cTexture mTexture;
  float mSize = 1.f;
  float mAngle = 0.f;
  cRect mExtent = {0,0,0,0};
  };
//}}}

class cPaint {
public:
  ~cPaint();

  cLayer* addLayer (cLayer* layer);

  bool pick (cPoint pos);
  bool prox (cPoint pos);
  bool proxExit();
  bool down (cPoint pos);
  bool move (cPoint pos, cPoint inc);
  bool up (cPoint pos, bool mouseMoved);
  bool wheel (int delta, cPoint pos);

  void draw (cWindow& window);

  bool mPainting = true;

private:
  std::deque <cLayer*> mLayers;
  cLayer* mPickedLayer = nullptr;
  };
