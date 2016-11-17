//===- PFrame.h -----------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_PFRAME_H_INCLUDE_
#define LSMMAP_PFRAME_H_INCLUDE_

#include <cstdint>

class PFrame {
private:
  uint64_t start_address;
  uint64_t frame_props;
  bool frame_props_valid;

public:
  PFrame(uint64_t startaddress);

  uint64_t getStartAddress(void) const;
  bool areFramePropertiesValid(void) const;
  uint64_t getRawFrameProperties(void) const;
  void setRawFrameProperties(uint64_t new_props, bool valid);

  bool isLocked(void) const;      // Bit 0
  bool hasError(void) const;      // Bit 1
  bool isReferenced(void) const;  // Bit 2
  bool isUpToDate(void) const;    // Bit 3
  bool isDirty(void) const;       // Bit 4
  bool isInLRU(void) const;       // Bit 5
  bool isInActiveLRU(void) const; // Bit 6
  bool bySLAB(void) const;        // Bit 7
  bool isWriteback(void) const;   // Bit 8
  bool isReclaim(void) const;     // Bit 9
  bool byBuddy(void) const;       // Bit 10
  bool isMemMapped(void) const;   // Bit 11
  bool isAnonMapped(void) const;  // Bit 12
  bool hasSwapCache(void) const;  // Bit 13
  bool isSwapBacked(void) const;  // Bit 14
  bool isCompdHead(void) const;   // Bit 15
  bool isCompdTail(void) const;   // Bit 16
  bool isHuge(void) const;        // Bit 17
  bool isUnevictable(void) const; // Bit 18
  bool isHWPoison(void) const;    // Bit 19
  bool isNoFrame(void) const;     // Bit 20
  bool isKSM(void) const;         // Bit 21
  bool isTHP(void) const;         // Bit 22
  bool isBalloon(void) const;     // Bit 23
  bool isZeroFrame(void) const;   // Bit 24
  bool isIdle(void) const;        // Bit 25
};

#endif
