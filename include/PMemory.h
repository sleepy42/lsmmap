//===- PMemory.h ----------------------------------------------------------===//
//
// This file contains the PMemory class that represents the physical memory.
//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_PMEMORY_H_INCLUDE_
#define LSMMAP_PMEMORY_H_INCLUDE_

#include "CmdOptions.h"
#include "PFrame.h"

#include <cstdint>
#include <iterator>
#include <map>
#include <type_traits>

class PMemory {
private:
  std::map<uint64_t, PFrame> p_frames;
  uint64_t frame_size;

protected:
  bool addPFrame(uint64_t frame_no, const int flags_fd);

public:
  PMemory(void);

  template<class It_Ty>
  typename std::enable_if<
    std::is_same<typename std::iterator_traits<It_Ty>::value_type, uint64_t>::value &&
    std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<It_Ty>::iterator_category>::value
  , size_t>::type
  addPFrames(It_Ty it_begin, It_Ty it_end);
  bool addPFrame(const CmdOptions &cmd_opts, uint64_t frame_no);
};

#include "PMemory.tcc"

#endif
