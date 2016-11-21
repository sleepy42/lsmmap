//===- Output.cpp ---------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "Output.h"
#include "VPage.h"

#include <iomanip>

// Some length for producing output (addresses are counted without the 0x)
static const int out_width_range_no = 4;
static const int out_width_range_addresses = 16;
static const int out_width_range_perms = 4;
static const int out_width_range_mapty = 2;
static const int out_width_range_nopages = 3;
static const int out_width_range_offset = 8;
static const int out_width_range_filesep = 4;
static const int out_width_page_indent = out_width_range_no+1;
static const int out_width_page_startaddr = 16;
static const int out_width_page_props = 5;
static const int out_width_page_maptosign = 4;
static const int out_width_page_swapty = 2;
static const int out_width_page_swapoff = 13;
static const int out_width_frame_startaddr = 12;

char getTristateChar(const VPageRange::TriState &val, char TrueC,
    char FalseC, char UnknownC) {
  if (val == VPageRange::TriState::True) {
    return TrueC;
  } else if (val == VPageRange::TriState::False) {
    return FalseC;
  } else {
    return UnknownC;
  }
}

char getBoolChar(const bool val, char TrueC, char FalseC) {
  if (val == true) {
    return TrueC;
  } else {
    return FalseC;
  }
}

void printHelpMessage(std::ostream &stream) {
  stream << "lsmmap - version 0.1" << std::endl << std::endl;
  stream << "lsmmap lists the mapping from virtual page address to "
         << "its " << std::endl
         << "physical frame address for the specified processes." << std::endl;
  stream << std::endl;
  stream << "Usage:" << std::endl
         << "lsmmap [OPTIONS] [PROCESSIDs]" << std::endl;
  stream << std::endl;
  stream << "OPTIONS:" << std::endl;
  stream << "  -a     Show mapping for all virtual pages and do not " << std::endl
         << "         omit unmapped pages." << std::endl;
  stream << "  -h     Print this help message." << std::endl;
  stream << "  -l x   Use x as lower address and limit the list of " << std::endl
         << "         mappings to all pages and ranges that have a " << std::endl
         << "         higher addresses." << std::endl;
  stream << "  -M     Use mappings mode. Virtual page ranges will be " << std::endl
         << "         read from /proc and for each mapped page the " << std::endl
         << "         corresponding physical frame will be determined " << std::endl
         << "         (if available). The mapping of ranges and pages " << std::endl
         << "         will be printed." << std::endl;
  stream << "  -n     Do also list virtual page ranges that are " << std::endl
         << "         not mapped (for those ranges no mapping " << std::endl
         << "         for single pages will be shown)." << std::endl;
  stream << "  -P     Use pages-only mode. In this mode the -l and -u " << std::endl
         << "         options MUST be specified by the user! Then all " << std::endl
         << "         the mapping for all virtual pages within that " << std::endl
         << "         interval will be shown (no matter if the pages " << std::endl
         << "         are actually mapped or not)." << std::endl;
  stream << "  -r     Do only list the page ranges and omit the " << std::endl
         << "         mapping for each single page." << std::endl;
  stream << "  -u x   Use x as upper address and limit the list of " << std::endl
         << "         mappings to all pages and ranges that have a " << std::endl
         << "         lower address." << std::endl;
  stream << std::endl;
  stream << "PROCESSIDs:" << std::endl;
  stream << "  The ids of the processes whose address spaces should " << std::endl
         << "  be examined. Several process ids can be given. A " << std::endl
         << "  seperate list of mappings will printed for each given " << std::endl
         << "  process. For each process there must be an accessible " << std::endl
         << "  directory (with the pid as its name) in /proc. Wihtin " << std::endl
         << "  that directory the accessible files \"maps\" and \"pagemap\" " << std::endl
         << "  must exist. A process id is a positive integer value. " << std::endl
         << "  The special pid \"self\" is allowed and refers to the " << std::endl
         << "  lsmmap process itself. It is also used as default if " << std::endl
         << "  no pid is given at all." << std::endl
         << "  Furthermore the file /proc/kpageflags must be " << std::endl
         << "  accessible." << std::endl << std::endl
         << "  To make sure that the shown addresses and flags are " << std::endl
         << "  correct the program should be run as root." << std::endl;
}

void printPageRangeHeadline(const CmdOptions &cmd_opts, std::ostream &stream) {
  stream << std::setfill(' ') << std::left;
  stream << std::setw(out_width_range_no) << "no";
  stream << " ";
  stream << std::setw(out_width_range_addresses+2) << "from";
  stream << " ";
  stream << std::setw(out_width_range_addresses+2) << "to";
  stream << " ";
  stream << std::setw(out_width_range_perms) << "perm";
  stream << " ";
  stream << std::setw(out_width_range_mapty) << "ty";
  stream << " ";
  stream << std::setw(out_width_range_nopages) << "#";
  stream << " ";
  stream << std::setw(out_width_range_offset+2) << "offset";
  stream << std::setw(out_width_range_filesep) << " ";
  stream << "file";
  stream << std::endl;
}

void printResults(const CmdOptions &cmd_opts, std::ostream &stream,
    const std::vector<Process> &processes, const PMemory &pmem) {

  // Store the format flags
  std::ios_base::fmtflags original_fmt_flags = stream.flags();

  // First the headlines should be printed
  printPageRangeHeadline(cmd_opts, stream);

  // Now print one line for each process
  for (const Process &cur_proc : processes) {
    stream << "Process: " << cur_proc.getPID() << std::endl;

    // Now print the page ranges
    for (const VPageRange &cur_vpr : cur_proc.getVPageRanges()) {
      // First the range number in dec
      if ((cur_vpr.getMappingType() == VPageRange::MappingType::Unmapped)
       || (cur_vpr.getMappingType() == VPageRange::MappingType::Mixed)) {
        stream << std::setfill('*') << std::left;
        stream << std::setw(out_width_range_no) << "*";
      } else {
        stream << std::dec << std::setfill('0') << std::right;
        stream << std::setw(out_width_range_no) << cur_vpr.getVPRangeNumber();
      }

      // Then the address range
      stream << " ";
      stream << std::hex << std::uppercase << std::right << std::setfill('0');
      stream << "0x";
      stream << std::setw(out_width_range_addresses) << cur_vpr.getFirstAddress();
      stream << "-0x";
      stream << std::setw(out_width_range_addresses) << cur_vpr.getNextAddress();

      // Now the permissions
      stream << " ";
      if (cur_vpr.getMappingType() == VPageRange::MappingType::Unmapped) {
        stream << std::setfill('*') << std::left;
        stream << std::setw(out_width_range_perms) << "*";
      } else if (cur_vpr.getMappingType() == VPageRange::MappingType::Mixed) {
        stream << std::setfill('?') << std::left;
        stream << std::setw(out_width_range_perms) << "?";
      } else {
        stream << getTristateChar(cur_vpr.canRead(), 'r');
        stream << getTristateChar(cur_vpr.canWrite(), 'w');
        stream << getTristateChar(cur_vpr.canExec(), 'x');
        stream << getTristateChar(cur_vpr.isPrivate(), 'p', 's');
      }

      // Print the mapping type
      stream << " ";
      if (cur_vpr.getMappingType() == VPageRange::MappingType::Unmapped) {
        stream << "n-";
      } else if (cur_vpr.getMappingType() == VPageRange::MappingType::Anonymous) {
        stream << "-a";
      } else if (cur_vpr.getMappingType() == VPageRange::MappingType::Filemapping) {
        stream << "-f";
      } else if (cur_vpr.getMappingType() == VPageRange::MappingType::Mixed) {
        stream << "mu";
      } else {
        stream << "??";
      }

      // Now the number of pages and offset
      if ((cur_vpr.getMappingType() == VPageRange::MappingType::Anonymous)
       || (cur_vpr.getMappingType() == VPageRange::MappingType::Filemapping)) {
        // First print the number of contained pages...
        stream << " ";
        stream << std::dec << std::setfill('0') << std::right;
        stream << std::setw(out_width_range_nopages) << cur_vpr.num();
        // ... and then the offset
        stream << " ";
        stream << std::hex << std::setfill('0') << std::right;
        stream << "0x" << std::setw(out_width_range_offset) << cur_vpr.getMappingOffset();
      } else {
        // As we do not know any offset we can use more space for the number
        // of contained pages
        const int cur_out_width_nopages =
            out_width_range_nopages + 1 + 2 + out_width_range_offset;
        stream << " ";
        stream << std::dec << std::setfill(' ') << std::left;
        stream << std::setw(cur_out_width_nopages) << cur_vpr.num();
      }

      // Now print the mapped file or the "[null]" indicator
      if (cur_vpr.getMappingType() == VPageRange::MappingType::Filemapping) {
        stream << std::setfill(' ') << std::left << std::setw(out_width_range_filesep) << " ";
        stream << cur_vpr.getMappedFilePath();
      } else if (cur_vpr.getMappingType() == VPageRange::MappingType::Unmapped) {
        stream << std::setfill(' ') << std::left << std::setw(out_width_range_filesep) << " ";
        stream << "[null]";
      }
      stream << std::endl;

      // Now print the mapping for each page
      if (cmd_opts.cmd_only_vpranges == false) {
        if ((cur_vpr.getMappingType() == VPageRange::MappingType::Anonymous)
         || (cur_vpr.getMappingType() == VPageRange::MappingType::Filemapping)
         || (cur_vpr.getMappingType() == VPageRange::MappingType::Mixed)) {
          unsigned no_omitted_pages = 0;
          for (const VPage &cur_vpage : cur_vpr.getVPages()) {
            if (cur_vpage.arePagePropertiesValid() == false) {
              ++no_omitted_pages;
              continue;
            }
            const bool isPageUsed = cur_vpage.arePagePropertiesValid()
                                  && (cur_vpage.isPresentRAM()
                                   || cur_vpage.isPresentSwap()
                                   || (cur_vpage.getFrameNumber() != 0));
            if (cmd_opts.cmd_show_all_pages == false) {
              if (isPageUsed == false) {
                // Skip the current page as it is not used
                ++no_omitted_pages;
                continue;
              }
              // Test if any pages were skipped
              if (no_omitted_pages > 0) {
                // Print the indention for each page
                stream << std::setfill(' ') << std::left
                       << std::setw(out_width_page_indent) << " ";
                // Now the note that pages were skipped
                stream << "[" << std::dec << no_omitted_pages
                       << " page" << ((no_omitted_pages != 1)?"s":"")
                       << " omitted]" << std::endl;
                no_omitted_pages = 0;
              }
            }
            // Now print the page details
            // First some indention
            stream << std::setfill(' ') << std::left
                   << std::setw(out_width_page_indent) << " ";
            // First entry is the start address
            stream << std::hex << std::uppercase << std::setfill('0') << std::right;
            stream << "0x" << std::setw(out_width_page_startaddr) << cur_vpage.getStartAddress();
            // Now some page propertiers
            stream << " ";
            stream << getBoolChar(cur_vpage.isPresentRAM(), 'p');
            stream << getBoolChar(cur_vpage.isPresentSwap(), 's');
            stream << getBoolChar(cur_vpage.isFileMapped(), 'f');
            stream << getBoolChar(cur_vpage.isExclusive(), 'e');
            stream << getBoolChar(cur_vpage.isSoftDirty(), 'd');
            // Now the location the page is mapped to
            stream << " -> ";
            if (cur_vpage.isPresentRAM() == true) {
              // The current page is present in RAM
              const PMemory::PF_Map_Ty::const_iterator cur_pframe_iter = 
                  pmem.getPFrameMap().find(cur_vpage.getFrameNumber());
              if (cur_pframe_iter == pmem.getPFrameMap().end()) {
                stream << "[null]" << std::endl;
                continue;
              }
              // Next fetch the corresponding frame
              const PFrame &cur_pframe = cur_pframe_iter->second;
              if (cur_pframe.areFramePropertiesValid() == false) {
                stream << "frameno:0x";
                stream << std::hex << std::uppercase << std::setfill('0') << std::left;
                stream << cur_vpage.getFrameNumber();
                continue;
              }
              // Now print the start address of the frame
              stream << std::hex << std::uppercase << std::setfill('0') << std::right;
              stream << "0x" << std::setw(out_width_frame_startaddr) << cur_pframe.getStartAddress();
              // Now print the frame properties
              stream << " ";
              stream << getBoolChar(cur_pframe.isLocked(), 'l');
              stream << getBoolChar(cur_pframe.hasError(), 'e');
              stream << getBoolChar(cur_pframe.isReferenced(), 'r');
              stream << getBoolChar(cur_pframe.isUpToDate(), 'u');
              stream << getBoolChar(cur_pframe.isDirty(), 'd');
              stream << getBoolChar(cur_pframe.isInLRU(), 'l');
              stream << getBoolChar(cur_pframe.isInActiveLRU(), 'a');
              stream << getBoolChar(cur_pframe.bySLAB(), 's');

              stream << getBoolChar(cur_pframe.isWriteback(), 'w');
              stream << getBoolChar(cur_pframe.isReclaim(), 'r');
              stream << getBoolChar(cur_pframe.byBuddy(), 'b');
              stream << getBoolChar(cur_pframe.isMemMapped(), 'm');
              stream << getBoolChar(cur_pframe.isAnonMapped(), 'a');
              stream << getBoolChar(cur_pframe.hasSwapCache(), 's');
              stream << getBoolChar(cur_pframe.isSwapBacked(), 's');
              stream << getBoolChar(cur_pframe.isCompdHead(), 'c');

              stream << getBoolChar(cur_pframe.isCompdTail(), 'c');
              stream << getBoolChar(cur_pframe.isHuge(), 'h');
              stream << getBoolChar(cur_pframe.isUnevictable(), 'u');
              stream << getBoolChar(cur_pframe.isHWPoison(), 'p');
              stream << getBoolChar(cur_pframe.isNoFrame(), 'n');
              stream << getBoolChar(cur_pframe.isKSM(), 'k');
              stream << getBoolChar(cur_pframe.isTHP(), 't');
              stream << getBoolChar(cur_pframe.isBalloon(), 'b');

              stream << getBoolChar(cur_pframe.isZeroFrame(), 'z');
              stream << getBoolChar(cur_pframe.isIdle(), 'i');
            } else if (cur_vpage.isPresentSwap() == true) {
              // The current page is swapped
              stream << "swap:";
              stream << std::dec << std::setfill('0') << std::left;
              stream << cur_vpage.getSwapType();
              stream << std::hex << std::uppercase << std::setfill('0') << std::left;
              stream << "@0x" << cur_vpage.getSwapOffset();
            } else if (cur_vpage.getFrameNumber() != 0) {
              // Page has frame number not 0
              stream << "frameno:0x";
              stream << std::hex << std::uppercase << std::setfill('0') << std::left;
              stream << cur_vpage.getFrameNumber();
            } else {
              // Page seems not to be mapped
              stream << "[null]";
            }
            stream << std::endl;
          } // End of page loop
          // Test if any pages were skipped at the end of the range
          if (no_omitted_pages > 0) {
            if ((cur_vpr.num() > 0) && (no_omitted_pages == cur_vpr.num())) {
              stream << std::setfill(' ') << std::left
                     << std::setw(out_width_page_indent) << " ";
              stream << "[all pages omitted]" << std::endl;
            } else {
              stream << std::setfill(' ') << std::left
                     << std::setw(out_width_page_indent) << " ";
              stream << "[" << std::dec << no_omitted_pages
                     << " page" << ((no_omitted_pages != 1)?"s":"")
                     << " omitted]" << std::endl;
              no_omitted_pages = 0;
            }
          }
        }
      } else {
        if (cmd_opts.cmd_prog_mode == CmdOptions::ProgMode::Pages) {
          stream << "Omit option \"-r\" to show mappings for each page." << std::endl;
        }
      }
    } // End of pagerange loop
  } // End of process loop

  // Restore format flags
  stream.flags(original_fmt_flags);
}
