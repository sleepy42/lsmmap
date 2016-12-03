//===- PFrame.cpp ---------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "PFrame.h"

PFrame::PFrame(uint64_t startaddress)
 : start_address(startaddress), frame_props(0), frame_refcount(0),
   frame_props_valid(false) {
}

uint64_t PFrame::getStartAddress(void) const {
  return start_address;
}

bool PFrame::areFramePropertiesValid(void) const {
  return frame_props_valid;
}

uint64_t PFrame::getRawFrameProperties(void) const {
  return frame_props;
}

uint64_t PFrame::getFrameRefCount(void) const {
  return frame_refcount;
}

void PFrame::setRawFrameProperties(uint64_t new_props, uint64_t new_frame_refcnt,
    bool valid) {
  frame_props = new_props;
  frame_refcount = new_frame_refcnt;
  frame_props_valid = valid;
}

bool PFrame::isLocked(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000001) != 0);
}

bool PFrame::hasError(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000002) != 0);
}

bool PFrame::isReferenced(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000004) != 0);
}

bool PFrame::isUpToDate(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000008) != 0);
}

bool PFrame::isDirty(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000010) != 0);
}

bool PFrame::isInLRU(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000020) != 0);
}

bool PFrame::isInActiveLRU(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000040) != 0);
}

bool PFrame::bySLAB(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000080) != 0);
}

bool PFrame::isWriteback(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000100) != 0);
}

bool PFrame::isReclaim(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000200) != 0);
}

bool PFrame::byBuddy(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000400) != 0);
}

bool PFrame::isMemMapped(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000000800) != 0);
}

bool PFrame::isAnonMapped(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000001000) != 0);
}

bool PFrame::hasSwapCache(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000002000) != 0);
}

bool PFrame::isSwapBacked(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000004000) != 0);
}

bool PFrame::isCompdHead(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000008000) != 0);
}

bool PFrame::isCompdTail(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000010000) != 0);
}

bool PFrame::isHuge(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000020000) != 0);
}

bool PFrame::isUnevictable(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000040000) != 0);
}

bool PFrame::isHWPoison(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000080000) != 0);
}

bool PFrame::isNoFrame(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000100000) != 0);
}

bool PFrame::isKSM(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000200000) != 0);
}

bool PFrame::isTHP(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000400000) != 0);
}

bool PFrame::isBalloon(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000000800000) != 0);
}

bool PFrame::isZeroFrame(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000001000000) != 0);
}

bool PFrame::isIdle(void) const {
  // Since 2.6.25
  return ((frame_props & 0x0000000002000000) != 0);
}
