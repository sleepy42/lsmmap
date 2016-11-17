//===- Process.cpp --------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "Process.h"

#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

Process::Process(std::string pid)
 : process_id(pid), maps_filepath(""), pagemap_filepath("") {
  if (checkForFiles() == false) {
    std::invalid_argument exc("Could not initialize process object. Some files might are inacessible.");
    throw exc;
  }
}

const std::string& Process::getPID(void) const {
  return process_id;
}

std::string Process::getMapsFilePath(void) const {
  return maps_filepath;
}

std::string Process::getPageMapFilePath(void) const {
  return pagemap_filepath;
}

const Process::VPR_List_Ty& Process::getVPageRanges(void) const {
  return vp_ranges;
}

/**
 * \brief Checks if all needed files exist.
 *
 * Checks if all needed files (/proc/id/maps, /proc/id/pagemap) exist and
 * are read-accessible to the user. If so \c true is returned. Else the function
 * returns \c false.
 */
bool Process::checkForFiles(void) {
  // Test if the needed directory exists
  const std::string dir_path("/proc/" + process_id);
  struct stat dir_stat;
  if (stat(dir_path.c_str(), &dir_stat) != 0) {
    return false;
  }
  if (S_ISDIR(dir_stat.st_mode) == false) {
    return false;
  }
  
  // Now set path to the maps file. One would have to make sure that we do run
  // anywhere outside the /proc directory but... maybe later...
  maps_filepath = dir_path + "/maps";
  if (access (maps_filepath.c_str(), F_OK | R_OK) != 0) {
    maps_filepath.clear();
    return false;
  }
  // Now set path to the pagemap file.
  pagemap_filepath = dir_path + "/pagemap";
  if (access (pagemap_filepath.c_str(), F_OK | R_OK) != 0) {
    pagemap_filepath.clear();
    return false;
  }

  return true;
}

/**
 * \brief Populates the process' page ranges.
 *
 * Populates the process' page ranges by creating \c VPageRange objects. Each
 * such object holds information about the page range (such as access
 * permissions or the location in the address space) read from the process'
 * map file. Those ranges can later be populated with single pages. The function
 * returns the number of created page ranges.
 */
size_t Process::populateRanges(const CmdOptions &cmd_opts) {
  // Store the format flags of the clog stream
  std::ios_base::fmtflags original_clog_flags = std::clog.flags();
  // Open the stream to the /proc/pid/maps file
  std::ifstream maps_file(maps_filepath, std::ios_base::in);
  if (maps_file.is_open() == false) {
    std::cerr << "Could not open maps file for process " << process_id
              << " (stream is not open)!" << std::endl;
    return 0;
  }
  if (maps_file.good() == false) {
    std::cerr << "Error occured while opening maps file for process "
              << process_id << " (stream is not good)!" << std::endl;
    return 0;
  }

  if (cmd_opts.cmd_verbose == true) {
    std::clog << "Opened maps file for process " << process_id << std::endl;
    std::clog << "Searching for page ranges in " << std::uppercase
              << std::hex << std::setfill('0') << "0x" << std::setw(16)
              << cmd_opts.cmd_lower_address << " to "
              << std::hex << std::setfill('0') << "0x" << std::setw(16)
              << cmd_opts.cmd_upper_address << std::endl;
  }
  // Determine page size and compute according mask
  const long proc_pagesize = sysconf(_SC_PAGESIZE);
  const long proc_pageoffset_mask = proc_pagesize - 1;

  // This vector will temporarily store the page ranges
  std::vector<VPageRange> tmp_ranges;
  // Now read all lines from the maps file
  for (unsigned cur_range_no = 0;
      ((maps_file.good() == true) && (maps_file.eof() == false));
      ++cur_range_no) {
    uint64_t cur_lower = 0, cur_upper = 0;
    // Read the lower address of the current range
    maps_file >> std::hex >> cur_lower;
    // Ignore the seperating -
    maps_file.ignore();
    // Read the upper address of the current range
    maps_file >> std::hex >> cur_upper;
    if (maps_file.good() == false) break;

    // Some sanity checks
    if (cur_lower > cur_upper) {
      if (cmd_opts.cmd_verbose == true) {
        std::clog << "Skipping invalid range " << std::dec << cur_range_no
                  << " (lower address > upper address)!" << std::endl;
      }
      // Ignore the remainder of the current line and continue with next one
      maps_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }

    // Test if the current range is wanted
    if (((cmd_opts.cmd_low_addr_userset == true) && (cur_upper <= cmd_opts.cmd_lower_address))
     || ((cmd_opts.cmd_up_addr_userset == true) && (cur_lower >= cmd_opts.cmd_upper_address))) {
      // The current range is not included in the requested range
      if (cmd_opts.cmd_verbose == true) {
        std::clog << "Skipping range " << std::dec << cur_range_no << " ("
                  << std::hex << std::setfill('0') << "0x" << std::setw(16) << cur_lower
                  << "-"
                  << std::hex << std::setfill('0') << "0x" << std::setw(16) << cur_upper
                  << ") as not in requested range!" << std::endl;
      }
      // Ignore the remainder of the current line and continue with next one
      maps_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    if (maps_file.good() == false) break;

    // Test if the boundaries of the ranges are aligned to the pagesize
    if ((cur_lower & proc_pageoffset_mask) != 0) {
      std::cerr << "Skipping range " << std::dec << cur_range_no << ": "
                << "lower address is not aligned to pagesize!" << std::endl;
      // Ignore the remainder of the current line and continue with next one
      maps_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    if ((cur_upper & proc_pageoffset_mask) != 0) {
      std::cerr << "Skipping range " << std::dec << cur_range_no << ": "
                << "upper address is not aligned to pagesize!" << std::endl;
      // Ignore the remainder of the current line and continue with next one
      maps_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }
    if (((cur_upper - cur_lower) & proc_pageoffset_mask) != 0) {
      std::cerr << "Skipping range " << std::dec << cur_range_no << ": "
                << "number of contained pages is not an integer!" << std::endl;
      // Ignore the remainder of the current line and continue with next one
      maps_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;
    }

    // The range seems to be valid so create an object
    VPageRange cur_range(cur_lower, cur_upper, proc_pagesize);

    // Set range number
    cur_range.setVPRangeNumber(cur_range_no);

    // Now extract the permissions for the contained pages
    char cur_perm = '?';
    // Check the read-flag
    maps_file >> cur_perm;
    if ((cur_perm == 'r') || (cur_perm == 'R')) {
      cur_range.setReadP(VPageRange::TriState::True);
    } else if (cur_perm == '-') {
      cur_range.setReadP(VPageRange::TriState::False);
    } else {
      cur_range.setReadP(VPageRange::TriState::Unknown);
    }
    if (maps_file.good() == false) break;
    // Check the write-flag
    maps_file >> cur_perm;
    if ((cur_perm == 'w') || (cur_perm == 'W')) {
      cur_range.setWriteP(VPageRange::TriState::True);
    } else if (cur_perm == '-') {
      cur_range.setWriteP(VPageRange::TriState::False);
    } else {
      cur_range.setWriteP(VPageRange::TriState::Unknown);
    }
    if (maps_file.good() == false) break;
    // Check the execute-flag
    maps_file >> cur_perm;
    if ((cur_perm == 'x') || (cur_perm == 'X')) {
      cur_range.setExecP(VPageRange::TriState::True);
    } else if (cur_perm == '-') {
      cur_range.setExecP(VPageRange::TriState::False);
    } else {
      cur_range.setExecP(VPageRange::TriState::Unknown);
    }
    if (maps_file.good() == false) break;
    // Check the private-flag
    maps_file >> cur_perm;
    if ((cur_perm == 'p') || (cur_perm == 'P')) {
      cur_range.setPrivateS(VPageRange::TriState::True);
    } else if ((cur_perm == 's') || (cur_perm == 'S')) {
      cur_range.setPrivateS(VPageRange::TriState::False);
    } else {
      cur_range.setPrivateS(VPageRange::TriState::Unknown);
    }
    if (maps_file.good() == false) break;

    // Now the offset
    uint64_t cur_offset = 0;
    maps_file >> cur_offset;
    cur_range.setMappingOffset(cur_offset);
    if (maps_file.good() == false) break;

    // Now go to the inode number
    uint64_t inode = 0;
    // First skip deice id
    maps_file >> std::hex >> inode;
    maps_file.ignore();
    maps_file >> std::hex >> inode;
    if (maps_file.good() == false) break;
    // Now the inode
    maps_file >> std::dec >> inode;
    if (inode == 0) {
      cur_range.setMappingType(VPageRange::MappingType::Anonymous);
    } else {
      cur_range.setMappingType(VPageRange::MappingType::Filemapping);
    }
    if (maps_file.good() == false) break;

    // Now read the file if needed
    if (inode != 0) {
      std::string cur_mappedfile;
      maps_file >> cur_mappedfile;
      cur_range.setMappedFilePath(cur_mappedfile);
    }

    // Now ignore the remainder of the line
    maps_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // The object seems to be valid else we would have jumped out of the loop
    // would have continued
    tmp_ranges.push_back(cur_range);
  } // End of for-loop iterating over lines in maps file
  if (maps_file.eof() == false) {
    // Something went wrong...
    std::cerr << "Error occured while reading virtual page ranges for process "
              << process_id << " (stream is not good and not eof)!" << std::endl;
  }
  // We do not need the file anymore...
  maps_file.close();

  // There should not be nothing else bail out early
  if (tmp_ranges.size() == 0) {
    return vp_ranges.size();
  }

  // Now sort the ranges
  std::sort(tmp_ranges.begin(), tmp_ranges.end(),
      [](const VPageRange lhs, const VPageRange rhs){
        if (lhs.getFirstAddress() < rhs.getFirstAddress()) {
          return true;
        } else {
          return false;
        }
      });

  // Now put those ranges into the member vector and add unmapped regions if wished
  vp_ranges.clear();
  if (cmd_opts.cmd_show_unmapped == false) {
    vp_ranges.reserve(tmp_ranges.size());
  } else {
    vp_ranges.reserve(tmp_ranges.size() + (tmp_ranges.size() - 1) + 2);
  }
  // cur_address_pos will point to an address that seperates the range that was
  // already processed from the one that we will still have to process:
  // all ranges starting below cur_address_pos are already processed and addrd
  // to the member vector including possibly unmapped regions. All ranges
  // starting above cur_address_pos will still have to be processed
  uint64_t cur_address_pos = 0;
  if (cmd_opts.cmd_show_unmapped == true) {
    if (cmd_opts.cmd_low_addr_userset == true) {
      cur_address_pos = cmd_opts.cmd_lower_address & (~(proc_pagesize-1));
    } else {
      cur_address_pos = tmp_ranges.front().getFirstAddress();
    }
  }
  // We can now rely on the ordering within the vector as we sorted the ranges
  // in tmp_ranges according to their lower_address in ascending order
  for (size_t i = 0, e = tmp_ranges.size(); i < e; ++i) {
    VPageRange &cur_vp_range = tmp_ranges[i];
    // Skip ranges that are not requested...
    if (((cmd_opts.cmd_low_addr_userset == true)
        && (cur_vp_range.getNextAddress() <= cmd_opts.cmd_lower_address))
     || ((cmd_opts.cmd_up_addr_userset == true)
        && (cur_vp_range.getFirstAddress() >= cmd_opts.cmd_upper_address))) {
      continue;
    }
    // Test if ranges overlap (should not occur) and skip them
    if (vp_ranges.size() > 0) {
      if (cur_vp_range.getFirstAddress() < vp_ranges.back().getNextAddress()) {
        std::cerr << "Skipping overlapping range " << cur_vp_range.getVPRangeNumber()
                  << " (preceeding range: " << vp_ranges.back().getVPRangeNumber()
                  << std::endl;
        continue;
      }
    }

    if (cmd_opts.cmd_show_unmapped == true) {
      // Now check if a unmapped region should be inserted
      if (cur_vp_range.getFirstAddress() > cur_address_pos) {
        VPageRange cur_unmapped_vpr(cur_address_pos,
            cur_vp_range.getFirstAddress(), proc_pagesize);
        cur_unmapped_vpr.setMappingType(VPageRange::MappingType::Unmapped);
        vp_ranges.push_back(cur_unmapped_vpr);
        cur_address_pos = cur_unmapped_vpr.getNextAddress();
      }
    }

    vp_ranges.push_back(cur_vp_range);
    cur_address_pos = cur_vp_range.getNextAddress();
  }
  if (cmd_opts.cmd_show_unmapped == true) {
    // Now we might need to add a last unmapped region. We only do that if
    // if the user specified an upper address
    if ((cmd_opts.cmd_up_addr_userset == true)
     && (cur_address_pos < cmd_opts.cmd_upper_address)) {
      // The following line might produce an overflow but at the moment it
      // is intended
      uint64_t new_upper = (cmd_opts.cmd_upper_address + proc_pagesize) & (~(proc_pagesize-1));
      VPageRange cur_unmapped_vpr(cur_address_pos, new_upper, proc_pagesize);
      cur_unmapped_vpr.setMappingType(VPageRange::MappingType::Unmapped);
      vp_ranges.push_back(cur_unmapped_vpr);
    }
  }

  // Restore flags for clog stream
  std::clog.flags(original_clog_flags);
  return vp_ranges.size();
}

/**
 * \brief Populates the ranges by creating \c VPage objects.
 *
 * Populates the page ranges by creating \c VPage objects that are inserted
 * into their range. Each page will store further information read from the
 * process' pagemap file. The number of created page objects is returned.
 */
size_t Process::populatePages(const CmdOptions &cmd_opts) {
  // Store the format flags for clog
  std::ios_base::fmtflags original_clog_flags = std::clog.flags();
  // Open the file. We will do this on a very basic level...
  // The file contains an 64bit entry for each virtual page. So that value can
  // be perfectly stored in an uint64_t.
  int pagemap_file_fd = open(pagemap_filepath.c_str(), O_RDONLY);
  if (pagemap_file_fd == -1) {
    std::cerr << "Could not open pagemap file " << pagemap_filepath << std::endl;
    perror("open:");
    return 0;
  }
  if (cmd_opts.cmd_verbose == true) {
    std::clog << "Opened pagemap file for process " << process_id << std::endl;
  }
  // Now populate all ranges
  size_t num_pages = 0;
  for (VPageRange &cur_vp_range : vp_ranges) {
    // Skip unmapped ranges
    if (cur_vp_range.getMappingType() == VPageRange::MappingType::Unmapped) {
      continue;
    }
    // We want to make sure to close the file although an exception was thrown
    try {
      size_t cur_created_pages = cur_vp_range.populatePages(pagemap_file_fd, cmd_opts);
      num_pages = num_pages + cur_created_pages;
    } catch(...) {
      close(pagemap_file_fd);
      throw;
    }
  }
  close(pagemap_file_fd);
  
  // Restore format flags of clog
  std::clog.flags(original_clog_flags);
  return num_pages;
}
