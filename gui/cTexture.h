// cTexture.h
#pragma once
#include "../common/basicTypes.h"

//{{{
class cBaseTexture {
public:
  static void* allocate (size_t bytes);
  static void deAllocate (void* allocation);

  cBaseTexture() = default;
  cBaseTexture (int32_t width, int32_t height, void* pixels) : mPixels(pixels), mWidth(width), mHeight(height) {}
  virtual ~cBaseTexture() = default;

  virtual void createPixels (int32_t width, int32_t height) = 0;
  virtual void release() = 0;

  // gets
  int32_t getWidth() const { return mWidth; }
  int32_t getHeight() const { return mHeight; }
  cPoint getSize() const { return cPoint(mWidth, mHeight); }

  virtual bool empty() const { return !mPixels; }

  // sets
  virtual void setWidth (int32_t width) { (void)width; }
  virtual void setHeight (int32_t height) { (void)height; }

protected:
  void* mPixels = nullptr;
  int32_t mWidth = 0;
  int32_t mHeight = 0;
  };
//}}}
//{{{
class cAlphaTexture : public cBaseTexture {
public:
  // static create
  static cAlphaTexture create (uint32_t width, uint32_t height, void* pixels, bool freePixels = false);

  cAlphaTexture() = default;
  cAlphaTexture (int32_t width, int32_t height, void* pixels) : cBaseTexture (width, height, pixels) {}
  virtual ~cAlphaTexture();

  virtual void createPixels (int32_t width, int32_t height);
  virtual void release();

  // gets
  uint8_t* getPixels() { return (uint8_t*)mPixels; }
  uint8_t getPixel (cPoint point) { return *getPixels (point); }
  uint8_t* getPixels (cPoint point) { return getPixels (point.getYInt32(), point.getXInt32()); }
  uint8_t* getPixels (int32_t x, int32_t y) { return empty() ? nullptr : getPixels() + (y * mWidth) + x; }

  // draws
  void clear (uint8_t alpha = 0);
  void drawRectangle (uint8_t alpha, const cRect& rect);
  void drawRectangleUnclipped (uint8_t alpha, const cRect& rect);

  void drawPixel (uint8_t alpha, cPoint point) { setPixel (alpha, point); }

protected:
  uint8_t getPixel (int32_t x, int32_t y) { return *getPixels (x,y); }

  void setPixel (uint8_t pixel, int32_t x, int32_t y) { *getPixels (x,y) = pixel; }
  void setPixel (uint8_t pixel, cPoint point) { *getPixels (point) = pixel; }
  };
//}}}
//{{{
class cTexture : public cBaseTexture {
public:
  //{{{
  union uPixel {
    uPixel() {}
    uPixel (uint32_t pixelValue) : pixel (pixelValue) {}
    //{{{
    uPixel (const cColor& color) {
      rgba.b = uint8_t(0xFF * color.b);
      rgba.g = uint8_t(0xFF * color.g);
      rgba.r = uint8_t(0xFF * color.r);
      rgba.a = uint8_t(0xFF * color.a);
      }
    //}}}
    //{{{
    uPixel (float r, float g, float b, float a) {
      rgba.b = uint8_t(0xFF * b);
      rgba.g = uint8_t(0xFF * g);
      rgba.r = uint8_t(0xFF * r);
      rgba.a = uint8_t(0xFF * a);
      }
    //}}}
    //{{{
    uPixel (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
      rgba.b = b;
      rgba.g = g;
      rgba.r = r;
      rgba.a = a;
      }
    //}}}

    uint32_t pixel;
    struct sRGBAtype {
      uint8_t r;
      uint8_t g;
      uint8_t b;
      uint8_t a;
      } rgba;
    };
  //}}}

  // static creates
  static cTexture createLoad (const std::string& name);
  static cTexture createDecode (uint8_t* buffer, uint32_t bufferSize);
  static cTexture create (uint32_t width, uint32_t height, void* pixels, bool freePixels = false);

  cTexture() = default;
  cTexture (int32_t width, int32_t height, uPixel* pixels) : cBaseTexture (width, height, (void*)pixels) {}
  virtual ~cTexture();

  virtual void createPixels (int32_t width, int32_t height);
  virtual void release();

  // gets
  uPixel* getPixels() { return (uPixel*)mPixels; }
  uPixel getPixel (cPoint point) { return *getPixels (point); }
  uPixel* getPixels (cPoint point) { return getPixels (point.getYInt32(), point.getXInt32()); }
  uPixel* getPixels (int32_t x, int32_t y) { return empty() ? nullptr : getPixels() + (y * mWidth) + x; }

  // draws
  void clear (const cColor& color = kBlack);
  void drawRectangle (const cColor& color, const cRect& rect);
  void drawRectangleUnclipped (const cColor& color, const cRect& rect);

  void drawPixel (const cColor& color, cPoint point) { setPixel (uPixel (color), point); }

  void blit (cTexture srcTexture, const cRect& dstRect);
  void blit (cTexture srcTexture, const cRect& dstRect, const cRect& clip);
  void blitSize (cTexture srcTexture, const cRect& dstRect);
  void blitAffine (cTexture srcTexture, const cRect& dstRect, const cMatrix3x2& matrix);
  void blitAffine (cTexture srcTexture, const cRect& dstRect, float size, float angle, float x, float y);

  void stamp (const cColor& color, cAlphaTexture& alphaTexture, const cPoint& point);

protected:
  uPixel getPixel (int32_t x, int32_t y) { return *getPixels (x,y); }

  void setPixel (uPixel pixel, int32_t x, int32_t y) { *getPixels (x,y) = pixel; }
  void setPixel (uPixel pixel, cPoint point) { *getPixels (point) = pixel; }
  };
//}}}

#ifdef BUILD_D2D
  //{{{
  class cD2dTexture : public cTexture {
    //{{{
    static cBitmap create (uint32_t width, uint32_t height, void* pixels, bool freePixels = false) {
      cTexture::create (width, height, pixels, freePixele);
      }
    //}}}

    cD2dTexture (void* bitmap) : mBitmap(bitmap) {}
    virtual ~cD2dTexture () = default;

    //{{{
    virtual void release() final {

      if (mBitmap) {
        // null bitmap before we release it
        ID2D1Bitmap* temp = (ID2D1Bitmap*)mBitmap;
         mBitmap = nullptr;
        temp->Release();
        }

      cTexture::release();
      }
    //}}}

    virtual bool empty() const final { return !mBitmap && cTexture::empty(); }

    // !!!! type it here !!!!
    void* getBitmap() const { return mBitmap; };

  protected:
    // !!!! type it here !!!!
    void* mBitmap = nullptr;
    };
  //}}}
#endif
