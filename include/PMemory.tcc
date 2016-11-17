//===- Memory.tcc ---------------------------------------------------------===//
//
// Implementation of template functions
//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_PMEMORY_TCC_INCLUDE_
#define LSMMAP_PMEMORY_TCC_INCLUDE_

template<class It_Ty>
typename std::enable_if<
  std::is_same<typename std::iterator_traits<It_Ty>::value_type, uint64_t>::value &&
  std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<It_Ty>::iterator_category>::value
, size_t>::type
PMemory::addPFrames(It_Ty it_begin, It_Ty it_end) {
  return 1337;
}

#endif
