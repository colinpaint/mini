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
  cLayer (cWindow& window, const std::string& name, const cColor& color, cPoint pos)
    : mWindow(window), mName(name), mPos(pos), mColor(color) {}
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

  virtual void draw() = 0;

protected:
  cWindow& mWindow;
  std::string mName;
  cPoint mPos = {0,0};
  cColor mColor = kRed;
  bool mProx = false;
  };
//}}}
//{{{
class cPaintLayer : public cLayer {
public:
  cPaintLayer (cWindow& window, const std::string& name, const cColor& color, cPoint pos, float width);
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

  virtual void draw() final;

protected:
  float mWidth = 1.f;
  std::vector<cPoint> mLine;
  cRect mExtent = {0,0,0,0};
  };
//}}}
//{{{
class cStrokeLayer : public cLayer {
public:
  cStrokeLayer (cWindow& window, const std::string& name, const cColor& color, cPoint pos, float width);
  virtual ~cStrokeLayer() {}

  virtual std::string getType() const final { return "stroke"; }
  virtual void addPoint (cPoint pos) final;

  virtual bool pick (cPoint pos) final;
  virtual void prox (cPoint pos) final;
  virtual void proxExit () final;
  virtual void down (cPoint pos) final;
  virtual void move (cPoint pos, cPoint inc) final;
  virtual void up (cPoint pos, bool mouseMoved) final;
  virtual void wheel (int delta, cPoint pos) final;

  virtual void draw() final;

protected:
  float mWidth = 1.f;
  std::vector<cPoint> mLine;
  cRect mExtent = {0,0,0,0};
  };
//}}}
//{{{
class cRectangleLayer : public cLayer {
public:
  cRectangleLayer(cWindow& window, const std::string& name, const cColor& color, cPoint pos, cPoint length)
    : cLayer(window, name, color, pos), mLength(length) {}
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

  virtual void draw() final;

protected:
  cPoint mLength;
  };
//}}}
//{{{
class cEllipseLayer : public cLayer {
public:
  cEllipseLayer (cWindow& window, const std::string& name, const cColor& color, cPoint pos, float radius, float width = 0.f)
    : cLayer(window, name, color, pos), mRadius(radius), mWidth(width) {}
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

  virtual void draw() final;

protected:
  float mRadius = 0;
  float mWidth = 0;
  };
//}}}
//{{{
class cTextLayer : public cLayer {
public:
  cTextLayer (cWindow& window, const std::string& name, const cColor& color, cPoint pos, const std::string text)
    : cLayer(window, name, color, pos), mText(text) {}
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

  virtual void draw() final;

protected:
  std::string mText;
  cPoint mLength = {0,0};
  };
//}}}
//{{{
class cTextureLayer : public cLayer {
public:
  cTextureLayer (cWindow& window, const std::string& name, cTexture texture,
                 cPoint pos, float size, const cColor& color = kBlack)
    : cLayer(window, name, color,pos), mTexture(texture), mSize(size), mAngle(0) {}
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

  virtual void draw() final;

protected:
  cTexture mTexture;
  float mSize = 1.f;
  float mAngle = 0.f;
  cRect mExtent = {0,0,0,0};
  };
//}}}

class cPaint {
public:
  cPaint (cWindow& window);
  ~cPaint();

  cLayer* addLayer (cLayer* layer);

  bool pick (cPoint pos);
  bool prox (cPoint pos);
  bool proxExit();
  bool down (cPoint pos);
  bool move (cPoint pos, cPoint inc);
  bool up (cPoint pos, bool mouseMoved);
  bool wheel (int delta, cPoint pos);

  void draw();

  bool mPainting = true;
  bool mStroking = false;

private:
  cWindow& mWindow;
  std::deque <cLayer*> mLayers;
  cLayer* mPickedLayer = nullptr;
  };
