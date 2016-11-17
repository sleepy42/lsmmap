//===- PFrame.cpp ---------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "PFrame.h"

PFrame::PFrame(uint64_t startaddress)
 : start_address(startaddress), frame_props(0), frame_props_valid(false) {
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

void PFrame::setRawFrameProperties(uint64_t new_props, bool valid) {
  frame_props = new_props;
  frame_props_valid = valid;
}
