//===- Memory.tcc ---------------------------------------------------------===//
//
// Implementation of template functions
//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_PMEMORY_TCC_INCLUDE_
#define LSMMAP_PMEMORY_TCC_INCLUDE_

#include <climits>
#include <fcntl.h>
#include <iostream>

template<class It_Ty>
typename std::enable_if<
  std::is_same<typename std::iterator_traits<It_Ty>::value_type, uint64_t>::value &&
  std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<It_Ty>::iterator_category>::value
, size_t>::type
PMemory::addPFrames(const CmdOptions &cmd_opts, It_Ty it_begin, It_Ty it_end) {
  const std::string frameflags_file("/proc/kpageflags");
  const std::string framerefcnt_file("/proc/kpagecount");
  // Store format flags of clog
  std::ios_base::fmtflags original_clog_flags = std::clog.flags();

  const int frameflags_file_fd = open(frameflags_file.c_str(), O_RDONLY);
  if (frameflags_file_fd == -1) {
    std::cerr << "Could not open frameflags file " << frameflags_file << std::endl;
    perror("open:");
    return 0;
  }
  if (cmd_opts.cmd_verbose == true) {
    std::clog << "Opened frameflags file." << std::endl;
  }
  const int framerefcnt_file_fd = open(framerefcnt_file.c_str(), O_RDONLY);
  if (framerefcnt_file_fd == -1) {
    std::cerr << "Could not open frame refcount file " << framerefcnt_file << std::endl;
    return false;
  }
  if (cmd_opts.cmd_verbose == true) {
    std::clog << "Opened frame refcount file." << std::endl;
  }
  size_t added_frames = 0;
  while (it_begin != it_end) {
    try {
      if (addPFrame(*it_begin, frameflags_file_fd, framerefcnt_file_fd) == true) {
        ++added_frames;
      }
    } catch(...) {
      close(frameflags_file_fd);
      close(framerefcnt_file_fd);
      throw;
    }
    ++it_begin;
  }
  close(frameflags_file_fd);
  close(framerefcnt_file_fd);

  // Restore clog flags
  std::clog.flags(original_clog_flags);
  return added_frames;
}

#endif
