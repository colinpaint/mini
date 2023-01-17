// cDrawAA.h
#pragma once
//{{{  includes
#include <functional>
#include "../common/basicTypes.h"
#include "../common/cLog.h"
//}}}

class cDrawAA {
public:
  //{{{
  cDrawAA() {

    for (unsigned i = 0; i <= 255; i++)
      mGamma[i] = (uint8_t)(pow(double(i) / 255.0, 1.6) * 255.0);

    initDraw();
    }
  //}}}
  //{{{
  ~cDrawAA() {

    free (mSortedCellList);

    if (mNumCellBlocks) {
      cCell** ptr = mCellBlockList + mNumCellBlocks - 1;
      while (mNumCellBlocks--) {
        free (*ptr);
        ptr--;
        }

      free (mCellBlockList);
      }
    }
  //}}}

  //{{{
  void addEdgeFrom (cPoint point) {

    int32_t x = int32_t(point.x * 256.f);
    int32_t y = int32_t(point.y * 256.f);

    // close shape
    if (!mClosed)
      addEdgeTo (mClosex, mClosey);
    mClosex = x;
    mClosey = y;

    setCurCell (x >> 8, y >> 8);
    mCurx = x;
    mCury = y;
    }
  //}}}
  //{{{
  void addEdgeTo (cPoint point) {
    addEdgeTo (int32_t(point.x * 256.f), int32_t(point.y * 256.f));
    }
  //}}}
  //{{{
  void draw (uint32_t width, uint32_t height, bool fillNonZero,
             const std::function <void (uint8_t*, int32_t, int32_t, uint32_t)>& stampCallback) {

    if (!mNumCells)
      return;

    mWidth = width;
    mHeight = height;
    mStampCallback = stampCallback;

    mScanLine.init (mMinx, mMaxx);

    int32_t coverage = 0;

    const cCell* const* sortedCellList = calcSortedCellList();
    const cCell* cell = *sortedCellList++;
    while (true) {
      int32_t packedYX = cell->getPackedYX();
      int32_t x = cell->getPackedX();
      int32_t y = cell->getPackedY();
      int32_t area = cell->getArea();
      coverage += cell->getCoverage();

      // accumulate all start cells
      while ((cell = *sortedCellList++) != 0) {
        if (cell->getPackedYX() != packedYX)
          break;
        area += cell->getArea();
        coverage += cell->getCoverage();
        }

      if (area) {
        uint8_t alpha = calcAlpha ((coverage << 9) - area, fillNonZero);
        if (alpha) {
          if (mScanLine.isReady (y))
            drawScanLine (mScanLine.getY());
          mScanLine.addSpan (x, y, 1, mGamma[alpha]);
          }
        x++;
        }

      if (!cell)
        break;

      if (cell->getPackedX() > x) {
        uint8_t alpha = calcAlpha (coverage << 9, fillNonZero);
        if (alpha) {
          if (mScanLine.isReady (y))
            drawScanLine (mScanLine.getY());
          mScanLine.addSpan (x, y, cell->getPackedX() - x, mGamma[alpha]);
          }
        }
      }

    if (mScanLine.getNumSpans())
      drawScanLine (mScanLine.getY());

    // clear down for next time
    initDraw();
    }
  //}}}

private:
  //{{{
  class cCell {
  public:
    static int32_t packYX (int32_t x, int32_t y) {
      return (y << 16) | x;
      }

    void set (int32_t x, int32_t y, int32_t c, int32_t a) {
      // !!!! should assert 16 bit range, or implement as 64bit !!!
      mPackedYX = packYX (x,y);
      mCoverage = c;
      mArea = a;
      }

    void setCoverage (int32_t c, int32_t a) {
      mCoverage = c;
      mArea = a;
      }

    void addCoverage (int32_t c, int32_t a) {
      mCoverage += c;
      mArea += a;
      }

    int32_t getPackedYX() const { return mPackedYX; }
    int32_t getPackedX() const { return mPackedYX & 0xFFFF; }
    int32_t getPackedY() const { return mPackedYX >> 16; }

    int32_t getCoverage() const { return mCoverage; }
    int32_t getArea() const { return mArea; }

  private:
    // packedYX is used in to test ==,>,< hence packing order Y:X
    int32_t mPackedYX = 0;
    int32_t mCoverage = 0;
    int32_t mArea = 0;
    };
  //}}}
  //{{{
  class cScanLine {
  public:
    //{{{
    class iterator {
    public:
      iterator (const cScanLine& scanLine) :
        mCoverage(scanLine.mCoverage), mCurCount(scanLine.mCounts), mCurStartPtr(scanLine.mStartPtrs) {}

      int next() {
        ++mCurCount;
        ++mCurStartPtr;
        return int(*mCurStartPtr - mCoverage);
        }

      int32_t getNumPix() const { return int32_t(*mCurCount); }
      const uint8_t* getCoverage() const { return *mCurStartPtr; }

    private:
      const uint8_t* mCoverage;
      const uint32_t* mCurCount;
      const uint8_t* const* mCurStartPtr;
      };
    //}}}
    friend class iterator;

    //{{{
    ~cScanLine() {
      free (mCounts);
      free (mStartPtrs);
      free (mCoverage);
      }
    //}}}

    int32_t getY() const { return mLastY; }
    int32_t getBaseX() const { return mMinx;  }
    uint32_t getNumSpans() const { return mNumSpans; }
    int isReady (int32_t y) const { return mNumSpans && (y ^ mLastY); }

    //{{{
    void init (int32_t minx, int32_t maxx) {

      uint32_t maxLen = maxx - minx + 2;
      if (maxLen > mMaxlen) {
        // increase allocations
        mCoverage = (uint8_t*)realloc  (mCoverage, maxLen * sizeof(uint8_t));
        mCounts = (uint32_t*)realloc (mCounts, maxLen * sizeof(uint32_t));
        mStartPtrs = (uint8_t**)realloc (mStartPtrs, maxLen * sizeof(uint8_t*));
        cLog::log (LOGINFO, fmt::format ("drawAA - allocate more scanlines {} previous {}", maxLen, mMaxlen));
        mMaxlen = maxLen;
        }

      mMinx = minx;
      initSpans();
      }
    //}}}

    //{{{
    void initSpans() {

      mNumSpans = 0;
      mCurCount = mCounts;
      mCurStartPtr = mStartPtrs;

      mLastX = 0x7FFF;
      mLastY = 0x7FFF;
      }
    //}}}
    //{{{
    void addSpan (int32_t x, int32_t y, uint32_t num, uint32_t coverage) {

      x -= mMinx;
      memset (mCoverage + x, coverage, num);

      if (x == mLastX + 1)
        (*mCurCount) += num;
      else {
        *++mCurCount = num;
        *++mCurStartPtr = mCoverage + x;
        mNumSpans++;
        }

      mLastX = x + num - 1;
      mLastY = y;
      }
    //}}}

  private:
    uint32_t mNumSpans = 0;

    // allocations
    uint32_t mMaxlen = 0;
    uint8_t* mCoverage = nullptr;
    uint32_t* mCounts = nullptr;
    uint8_t** mStartPtrs = nullptr;

    // cur pointers
    uint32_t* mCurCount = nullptr;
    uint8_t** mCurStartPtr = nullptr;

    int32_t mMinx = 0;
    int32_t mLastX = 0x7FFFFFFF;
    int32_t mLastY = 0x7FFFFFFF;
    };
  //}}}

  //{{{
  void initDraw() {

    mNumCells = 0;
    mCurCell.set (0x7FFFFFFF, 0x7FFFFFFF, 0, 0);
    mSortRequired = true;
    mClosed = true;

    mMinx =  0x7FFFFFFF;
    mMiny =  0x7FFFFFFF;
    mMaxx = -0x7FFFFFFF;
    mMaxy = -0x7FFFFFFF;
    }
  //}}}

  //{{{
  const cCell* const* calcSortedCellList() {

    if (!mClosed) {
      // close shape
      addEdgeTo (mClosex, mClosey);
      mClosed = true;
      }

    // sort first time only
    if (mSortRequired) {
      addCurCell();

      if (!mNumCells) // no cells to sort
        return 0;

      sortCells();
      }

    return mSortedCellList;
    }
  //}}}

  //{{{
  void addEdgeTo (int32_t x, int32_t y) {

    if (mSortRequired && ((mCurx ^ x) | (mCury ^ y))) {
      int32_t c = mCurx >> 8;
      if (c < mMinx)
        mMinx = c;
      ++c;
      if (c > mMaxx)
        mMaxx = c;

      c = x >> 8;
      if (c < mMinx)
        mMinx = c;
      ++c;
      if (c > mMaxx)
        mMaxx = c;

      addLine (mCurx, mCury, x, y);
      mCurx = x;
      mCury = y;

      mClosed = false;
      }
    }
  //}}}
  //{{{
  void addCurCell() {

    if (mCurCell.getArea() | mCurCell.getCoverage()) {
      if (!(mNumCells % kNumCellsPerBlock)) {
        // need another cellBlock
        uint32_t block = mNumCells / kNumCellsPerBlock;
        if (block >= mNumCellBlocks) {
          // allocate another cellBlock
          mCellBlockList = (cCell**)realloc (mCellBlockList, (mNumCellBlocks + 1) * sizeof(cCell*));
          mCellBlockList[mNumCellBlocks] = (cCell*)malloc (kNumCellsPerBlock * sizeof(cCell));
          mNumCellBlocks++;
          cLog::log (LOGINFO, fmt::format ("drawAA - allocate another cellBlock {}", mNumCellBlocks));
          }
        mCurCellPtr = mCellBlockList[block];
        }

      *mCurCellPtr++ = mCurCell;
      mNumCells++;
      }
    }
  //}}}
  //{{{
  void setCurCell (int32_t x, int32_t y) {

    if (mCurCell.getPackedYX() != cCell::packYX (x,y)) {
      addCurCell();
      mCurCell.set (x, y, 0, 0);
      }
   }
  //}}}
  //{{{
  void swapCells (cCell** a, cCell** b) {

    cCell* temp = *a;
    *a = *b;
    *b = temp;
    }
  //}}}
  //{{{
  void sortCells() {

    //cLog::log (LOGINFO, fmt::format ("sortCells numCells:{} numSortedCells:{}", mNumCells, mMaxNumSortedCells));

    mSortRequired = false;

    if (!mNumCells) // no cells to sort
      return;

    // allocate mSortedCellList, a contiguous vector of cCell pointers
    if (mNumCells > mMaxNumSortedCells) {
      mSortedCellList = (cCell**)realloc (mSortedCellList, (mNumCells + 1) * sizeof(cCell*));
      cLog::log (LOGINFO, fmt::format ("drawAA - allocate more sortedCellList {} previous {}", mNumCells, mMaxNumSortedCells));
      mMaxNumSortedCells = mNumCells;
      }

    // copy cell pointers to mSortedCellList
    cCell** blockPtr = mCellBlockList;
    cCell** sortedPtr = mSortedCellList;
    uint32_t numBlocks = mNumCells / kNumCellsPerBlock;
    while (numBlocks--) {
      // add whole block of cells to mSortedCellList
      cCell* cellPtr = *blockPtr++;
      uint32_t cellInBlock = kNumCellsPerBlock;
      while (cellInBlock--)
        *sortedPtr++ = cellPtr++;
      }

    // add partial last block of cells to mSortedCellList
    cCell* cellPtr = *blockPtr++;
    uint32_t cellInBlock = mNumCells % kNumCellsPerBlock;
    while (cellInBlock--)
      *sortedPtr++ = cellPtr++;

    // terminate mSortedCellList with nullptr
    mSortedCellList[mNumCells] = nullptr;

    // sort them
    quickSortCells (mSortedCellList, mNumCells);
    }
  //}}}
  //{{{
  void quickSortCells (cCell** sortedCellList, uint32_t numCells) {

    cCell** stack[80];

    cCell*** top = stack;
    cCell** base = sortedCellList;
    cCell** limit = sortedCellList + numCells;

    while (true) {
      uint32_t len = int(limit - base);
      if (len <= 9) {
        //{{{  subarray small, use simple insertion sort
        cCell** j = base;
        cCell** i = j + 1;

        for (; i < limit; j = i, i++)
          for (; (*(j+1))->getPackedYX() < (*j)->getPackedYX(); j--) {
            swapCells (j + 1, j);
            if (j == base)
              break;
            }

        if (top > stack) {
          top  -= 2;
          base  = top[0];
          limit = top[1];
          }
        else
          break;
        }
        //}}}
      else {
        // qsort, use base + len/2 as the pivot
        cCell** pivot = base + len / 2;
        swapCells (base, pivot);

        cCell** i = base + 1;
        cCell** j = limit - 1;

        // ensure that *i <= *base <= *j
        if ((*j)->getPackedYX() < (*i)->getPackedYX())
          swapCells (i, j);
        if ((*base)->getPackedYX() < (*i)->getPackedYX())
          swapCells (base, i);
        if ((*j)->getPackedYX() < (*base)->getPackedYX())
          swapCells (base, j);

        while (true) {
          do {
            i++;
            } while ((*i)->getPackedYX() < (*base)->getPackedYX());
          do {
            j--;
            } while ((*base)->getPackedYX() < (*j)->getPackedYX());
          if (i > j)
            break;
          swapCells (i, j);
          }
        swapCells (base, j);

        // push the largest subarray
        if (j - base > limit - i) {
          top[0] = base;
          top[1] = j;
          base   = i;
          }
        else {
          top[0] = i;
          top[1] = limit;
          limit  = j;
          }
        top += 2;
        }
      }
    }
  //}}}

  //{{{
  void addLine (int32_t x1, int32_t y1, int32_t x2, int32_t y2) {

    // split into pix and subPix
    int32_t yPix1 = y1 >> 8;
    int32_t yPix2 = y2 >> 8;
    int32_t ySubPix1 = y1 & 0xFF;
    int32_t ySubPix2 = y2 & 0xFF;

    // min max y
    mMiny = std::min (mMiny, yPix1);
    mMaxy = std::max (mMaxy, yPix1+1);
    mMiny = std::min (mMiny, yPix2);
    mMaxy = std::min (mMaxy, yPix2+1);

    if (yPix1 == yPix2) {
      //{{{  all on a single cScanLine, return
      addScanLine (yPix1, x1, ySubPix1, x2, ySubPix2);
      return;
      }
      //}}}

    int32_t incr = 1;
    int32_t dy = y2 - y1;
    int32_t dx = x2 - x1;
    if (dx == 0) {
      //{{{  simple vertical line, return
      // we have to calculate start and end cell
      // the common values of the area and coverage for all cells of the line.
      // We know exactly there's only one cell, so, we don't have to call drawScanLine().
      int32_t ex = x1 >> 8;
      int32_t two_fx = (x1 - (ex << 8)) << 1;
      int32_t first = 0x100;
      if (dy < 0) {
        first = 0;
        incr  = -1;
        }

      int32_t delta = first - ySubPix1;
      mCurCell.addCoverage (delta, two_fx * delta);

      yPix1 += incr;
      setCurCell (ex, yPix1);

      delta = first + first - 0x100;
      int32_t area = two_fx * delta;
      while (yPix1 != yPix2) {
        mCurCell.setCoverage (delta, area);
        yPix1 += incr;
        setCurCell (ex, yPix1);
        }

      delta = ySubPix2 - 0x100 + first;
      mCurCell.addCoverage (delta, two_fx * delta);
      return;
      }
      //}}}

    // several scanLines
    int32_t p  = (0x100 - ySubPix1) * dx;
    int32_t first = 0x100;
    if (dy < 0) {
      p = ySubPix1 * dx;
      first = 0;
      incr = -1;
      dy = -dy;
      }

    int32_t delta = p / dy;
    int32_t mod = p % dy;
    if (mod < 0) {
      delta--;
      mod += dy;
      }

    int32_t xFrom = x1 + delta;
    addScanLine (yPix1, x1, ySubPix1, xFrom, first);

    yPix1 += incr;
    setCurCell (xFrom >> 8, yPix1);

    if (yPix1 != yPix2) {
      //{{{  iterate y
      p = 0x100 * dx;
      int lift  = p / dy;
      int rem   = p % dy;
      if (rem < 0) {
        lift--;
        rem += dy;
        }
      mod -= dy;
      while (yPix1 != yPix2) {
        delta = lift;
        mod  += rem;
        if (mod >= 0) {
          mod -= dy;
          delta++;
          }

        int xTo = xFrom + delta;
        addScanLine (yPix1, xFrom, 0x100 - first, xTo, first);
        xFrom = xTo;

        yPix1 += incr;
        setCurCell (xFrom >> 8, yPix1);
        }
      }
      //}}}

    addScanLine (yPix1, xFrom, 0x100 - first, x2, ySubPix2);
    }
  //}}}
  //{{{
  void addScanLine (int32_t yScanLine, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {

    // split into pix and subPix
    int32_t xPix1 = x1 >> 8;
    int32_t xPix2 = x2 >> 8;
    int32_t xSubPix1 = x1 & 0xFF;
    int32_t xSubPix2 = x2 & 0xFF;

    if (y1 == y2) {
      //{{{  same y, trivial case, happens often, return
      setCurCell (xPix2, yScanLine);
      return;
      }
      //}}}

    if (xPix1 == xPix2) {
      //{{{  single cell, return
      int delta = y2 - y1;
      mCurCell.addCoverage (delta, (xSubPix1 + xSubPix2) * delta);
      return;
      }
      //}}}

    // draw run of adjacent cells on same scanLine
    int32_t p = (0x100 - xSubPix1) * (y2 - y1);
    int32_t first = 0x100;
    int32_t incr = 1;
    int32_t dx = x2 - x1;
    if (dx < 0) {
      p = xSubPix1 * (y2 - y1);
      first = 0;
      incr = -1;
      dx = -dx;
      }

    int32_t delta = p / dx;
    int32_t mod = p % dx;
    if (mod < 0) {
      delta--;
      mod += dx;
      }

    mCurCell.addCoverage (delta, (xSubPix1 + first) * delta);

    xPix1 += incr;
    setCurCell (xPix1, yScanLine);
    y1  += delta;
    if (xPix1 != xPix2) {
      p = 0x100 * (y2 - y1 + delta);
      int lift = p / dx;
      int rem = p % dx;
      if (rem < 0) {
        lift--;
        rem += dx;
        }

      mod -= dx;
      while (xPix1 != xPix2) {
        delta = lift;
        mod  += rem;
        if (mod >= 0) {
          mod -= dx;
          delta++;
          }

        mCurCell.addCoverage (delta, (0x100) * delta);
        y1 += delta;
        xPix1 += incr;
        setCurCell (xPix1, yScanLine);
        }
      }

    delta = y2 - y1;
    mCurCell.addCoverage (delta, (xSubPix2 + 0x100 - first) * delta);
    }
  //}}}

  //{{{
  uint8_t calcAlpha (int32_t area, bool fillNonZero) {
  // !!! don't understand this !!!

    int32_t coverage = area >> 9;
    if (coverage < 0)
      coverage = -coverage;

    if (!fillNonZero) {
      coverage &= 0x1FF;
      if (coverage > 0x100)
        coverage = 0x200 - coverage;
      }

    if (coverage > 0xFF)
      return 0xFF;

    return (uint8_t)coverage;
    }
  //}}}
  //{{{
  void drawScanLine (int32_t y) {

    // clip top
    if (y < 0) {
      //{{{  top clipped, return
      mScanLine.initSpans();
      return;
      }
      //}}}

    // clip bottom
    if (y >= mHeight) {
      //{{{  bottom clipped, return
      mScanLine.initSpans();
      return;
      }
      //}}}

    int32_t baseX = mScanLine.getBaseX();
    uint32_t numSpans = mScanLine.getNumSpans();
    cScanLine::iterator span (mScanLine);
    do {
      int32_t x = baseX + span.next();
      uint8_t* coverage = (uint8_t*)span.getCoverage();

      // clip left  !!! not right !!!
      int32_t numPix = span.getNumPix();
      if (x < 0) {
        numPix += x;
        if (numPix <= 0)
          continue;
        coverage -= x;
        x = 0;
        }

      // clip right
      if (x + numPix >= mWidth) {
        numPix = mWidth - x;
        if (numPix <= 0)
          continue;
        }

      mStampCallback (coverage, x, y, numPix);
      } while (--numSpans);

    mScanLine.initSpans();
    }
  //}}}

  //  vars
  int32_t mWidth = 0;
  int32_t mHeight = 0;
  std::function <void (uint8_t*, int32_t, int32_t, uint32_t)> mStampCallback = nullptr;

  cScanLine mScanLine;
  uint8_t mGamma[256];

  // cells allocated in blocks for contiguous allocation
  static constexpr uint32_t kNumCellsPerBlock = 2048;
  uint32_t mNumCellBlocks = 0;
  cCell** mCellBlockList = nullptr;

  uint32_t mNumCells = 0;
  cCell mCurCell;
  cCell* mCurCellPtr = nullptr;

  bool mSortRequired = false;
  uint32_t mMaxNumSortedCells = 0;
  cCell** mSortedCellList = nullptr;

  int32_t mCurx = 0;
  int32_t mCury = 0;
  int32_t mClosex = 0;
  int32_t mClosey = 0;
  bool mClosed = false;

  int32_t mMinx = 0;
  int32_t mMiny = 0;
  int32_t mMaxx = 0;
  int32_t mMaxy = 0;
  };
