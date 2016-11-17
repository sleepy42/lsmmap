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
};

#endif
