//===- PMemory.cpp --------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "PMemory.h"

#include <climits>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

PMemory::PMemory(void)
 : frame_size(0) {
  frame_size = sysconf(_SC_PAGESIZE);
}

/**
 * \brief Tries to add a new frame object to memory.
 * \param frame_no The frame number of the new frame.
 * \param flags_fd The file descriptor of the file to get the frame flags from.
 *
 * Tries to add a new frame object to the memory. If there is already a frame
 * with the given number that frame remains unchanged and \c false is returned.
 * If no such frame exists a frame is created. Its start address is computed
 * using the frame number and the frame size. The frame flags are read from the
 * file described by the file descriptor \c flags_fd (usually the
 * \c /proc/kpageflags file). If everything is fine \c true is returned. In case
 * of an error the function returns \c false.
 */
bool PMemory::addPFrame(uint64_t frame_no, const int flags_fd) {
  if (flags_fd < 0) {
    return false;
  }
  // Frame number must not be aligned as it is NOT the start address
  // Check if a frame with the given number already exists
  if (p_frames.count(frame_no) > 0) {
    return false;
  }

  // Now compute the proper seek position within the kpageflags file
  const off_t ff_offset = frame_no * (64 / CHAR_BIT);
  if (lseek(flags_fd, ff_offset, SEEK_SET) == -1) {
    std::cerr << "Failed to position in frameflags file." << std::endl;
    perror("lseek:");
    return false;
  }
  // Now read the frame flags
  uint64_t frame_flags = 0;
  ssize_t read_bytes = read(flags_fd, &frame_flags, sizeof(frame_flags));
  if (read_bytes == -1) {
    std::cerr << "Could not properly from frameflags file!" << std::endl;
    perror("read:");
    return false;
  } else if (read_bytes == sizeof(frame_flags)) {
    PFrame cur_frame(frame_no * frame_size);
    cur_frame.setRawFrameProperties(frame_flags, true);
    p_frames.insert(std::make_pair(frame_no, cur_frame));
    return true;
  }

  return false;
}

/**
 * \brief Adds the frame with the given number to the memory.
 * \param frame_no The number (not address) of the required frame.
 * \note This function is mainly intended for debugging purposes.
 *
 * Tries to add the add frame with the given number (not address) to the memory
 * and read its status flags from the /proc/kpageflags file. If a frame with
 * the given number already exists the existing frame is not changed. The
 * function returns \c true if a new frame was created and added. If an error
 * occured or a frame with the given frame number already exists \c false is
 * returned.
 */
bool PMemory::addPFrame(const CmdOptions &cmd_opts, uint64_t frame_no) {
  const std::string frameflags_file("/proc/kpageflags");
  // Store format flags of clog
  std::ios_base::fmtflags original_clog_flags = std::clog.flags();
  int frameflags_file_fd = open(frameflags_file.c_str(), O_RDONLY);
  if (frameflags_file_fd == -1) {
    std::cerr << "Could not open frameflags file " << frameflags_file << std::endl;
    perror("open:");
    return false;
  }
  if (cmd_opts.cmd_verbose == true) {
    std::clog << "Opened frameflags file." << std::endl;
  }
  bool added_frame = false;
  try {
    added_frame = addPFrame(frame_no, frameflags_file_fd);
  } catch(...) {
    close(frameflags_file_fd);
    throw;
  }
  close(frameflags_file_fd);

  // Restore clog flags
  std::clog.flags(original_clog_flags);
  return added_frame;
}
