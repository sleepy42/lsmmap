//===- VPage.cpp ----------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#include "VPage.h"

#include <climits>
#include <fcntl.h>
#include <iostream>
#include <iomanip>

VPage::VPage(uint64_t startaddress)
 : page_props(0), page_props_valid(false), start_address(startaddress) {
}

/**
 * \brief Returns the start address of the virtual page.
 */
uint64_t VPage::getStartAddress(void) const {
  return start_address;
}

/**
 * \brief Indicates weather the stored properties are valid or not.
 */
bool VPage::arePagePropertiesValid(void) const {
  return page_props_valid;
}

/**
 * \brief Returns the raw 64bit value page property value.
 */
uint64_t VPage::getRawPageProperties(void) const {
  return page_props;
}

void VPage::setRawPageProperties(uint64_t new_props, bool valid) {
  page_props = new_props;
  page_props_valid = valid;
}

bool VPage::isPresentRAM(void) const {
  // Since 2.6.25
  return ((page_props & 0x8000000000000000) != 0);
}

bool VPage::isPresentSwap(void) const {
  // Since 2.6.25
  return ((page_props & 0x4000000000000000) != 0);
}

bool VPage::isFileMapped(void) const {
  // Since 3.5
  return ((page_props & 0x2000000000000000) != 0);
}

bool VPage::isSoftDirty(void) const {
  // Since 2.6.25
  return ((page_props & 0x0080000000000000) != 0);
}

bool VPage::isExclusive(void) const {
  // Since 4.2
  return ((page_props & 0x0100000000000000) != 0);
}

uint64_t VPage::getFrameNumber(void) const {
  // Since 2.6.25
  return (page_props & 0x007FFFFFFFFFFFFF);
}

uint8_t VPage::getSwapType(void) const {
  // Since 2.6.25
  return static_cast<uint8_t>((page_props & 0x000000000000001F));
}

uint64_t VPage::getSwapOffset(void) const {
  // Since 2.6.25
  return ((page_props & 0x007FFFFFFFFFFFE0) >> 5);
}

std::ostream& operator<<(std::ostream &stream, const VPage &page) {
  // Save format flags
  std::ios_base::fmtflags original_flags = stream.flags();

  const int width_vp_props = 5;
  const int width_vp_pfn = 14;
  const int width_vp_swapty = 2;
  const int width_vp_swapoff = 13;
  const int width_vp_raw = 16;

  // Print the start address of the page
  stream << std::hex << std::uppercase << std::right << std::setfill('0');
  stream << "0x" << std::setw(16) << page.getStartAddress();

  // Print the flags of the page
  stream << " ";
  if (page.arePagePropertiesValid() == true) {
    stream << ((page.isPresentRAM())?'p':'-');
    stream << ((page.isPresentSwap())?'s':'-');
    stream << ((page.isFileMapped())?'f':'-');
    stream << ((page.isExclusive())?'e':'-');
    stream << ((page.isSoftDirty())?'w':'-');
  } else {
    stream << std::left << std::setfill('?');
    stream << std::setw(width_vp_props) << "?";
  }
  // Print frame number and/or swap offset
  stream << " ";
  if (page.arePagePropertiesValid() == true) {
    if (page.isPresentRAM() == true) {
      stream << std::hex << std::uppercase << std::setfill('0') << std::right;
      stream << "0x" << std::setw(width_vp_pfn) << page.getFrameNumber();
    } else {
      stream << std::left << std::setfill('*');
      stream << std::setw(width_vp_pfn+2) << "*";
    }
    stream << " ";
    if (page.isPresentSwap() == true) {
      stream << std::dec << std::setfill('0') << std::right;
      stream << std::setw(width_vp_swapty) << page.getSwapType();
      stream << " ";
      stream << std::hex << std::uppercase << std::setfill('0') << std::right;
      stream << "0x" << std::setw(width_vp_swapoff) << page.getSwapOffset();
    } else {
      stream << std::left << std::setfill('*');
      stream << std::setw(width_vp_swapty) << "*";
      stream << " ";
      stream << std::setw(width_vp_swapoff+2) << "*";
    }
  } else {
    stream << std::setfill('?') << std::left;
    stream << std::setw(width_vp_pfn+2) << "?";
    stream << " ";
    stream << std::setw(width_vp_swapty) << "?";
    stream << " ";
    stream << std::setw(width_vp_swapoff+2) << "?";
  }
  stream << " ";
  // Now print the raw property value
  if (page.arePagePropertiesValid() == true) {
    stream << std::hex << std::uppercase << std::setfill('0') << std::right;
    stream << std::setw(width_vp_raw) << page.getRawPageProperties();
  } else {
    stream << std::setfill('?') << std::right;
    stream << std::setw(width_vp_raw+2) << "?";
  }
  // Restore format flags
  stream.flags(original_flags);
  return stream;
}

//===- VPageRange class ---------------------------------------------------===//
/**
 * \brief Creates a new range of virtual maps.
 * \param firstaddress The address of the first contained virtual map.
 * \param nextaddress The address of the first virtual map not contained anymore.
 */
VPageRange::VPageRange(uint64_t firstaddress, uint64_t nextaddress, long pagesize)
 : map_ty(MappingType::Unmapped), first_address(firstaddress),
   next_address(nextaddress), mapped_file_path(""), map_offset(0),
   page_size(pagesize), perm_canread(TriState::Unknown),
   perm_canwrite(TriState::Unknown), perm_canexec(TriState::Unknown),
   perm_isprivate(TriState::Unknown), range_no(0) {
}

/**
 * \brief Returns the address of the first contained virtual page.
 */
uint64_t VPageRange::getFirstAddress(void) const {
  return first_address;
}

/**
 * \brief Returns the start address the last virtual page that is stil contained
 * \brief in the range if such a page exists.
 */
uint64_t VPageRange::getLastAddress(void) const {
  if (first_address == next_address) {
    return first_address;
  } else {
    return next_address-page_size;
  }
}

/**
 * \brief Returns the address of the first virtual page that is not contained in
 * \brief the range anymore.
 */
uint64_t VPageRange::getNextAddress(void) const {
  return next_address;
}

/**
 * \brief Returns the mapping of the range;
 */
VPageRange::MappingType VPageRange::getMappingType(void) const {
  return map_ty;
}

/**
 * \brief Sets the mapping type of the virtual pages contained in the range.
 * \param newtype The new mapping type.
 */
void VPageRange::setMappingType(VPageRange::MappingType newtype) {
  map_ty = newtype;
}

/**
 * \brief Returns the path to the mapped file.
 * \note The path will only be valid if the range is file mapped.
 */
std::string VPageRange::getMappedFilePath(void) const {
  return mapped_file_path;
}

/**
 * \brief Sets the file mapped by the pages contained in the range.
 * \param path The path to the mapped file.
 */
void VPageRange::setMappedFilePath(const std::string &path) {
  mapped_file_path = path;
}

uint64_t VPageRange::getMappingOffset(void) const {
  return map_offset;
}

void VPageRange::setMappingOffset(uint64_t offset) {
  map_offset = offset;
}

/**
 * \brief Returns the size in bytes of one virtual page contained in the range.
 */
long VPageRange::getPageSize(void) const {
  return page_size;
}

/**
 * \brief Indicates if the contained pages can be read from.
 */
VPageRange::TriState VPageRange::canRead(void) const {
  return perm_canread;
}

void VPageRange::setReadP(VPageRange::TriState canread) {
  perm_canread = canread;
}

/**
 * \brief Indicates if the contained pages can be written to.
 */
VPageRange::TriState VPageRange::canWrite(void) const {
  return perm_canwrite;
}

void VPageRange::setWriteP(VPageRange::TriState canwrite) {
  perm_canwrite = canwrite;
}

/**
 * \brief Indicates if the contained pages can be executed.
 */
VPageRange::TriState VPageRange::canExec(void) const {
  return perm_canexec;
}

void VPageRange::setExecP(VPageRange::TriState canexec) {
  perm_canexec = canexec;
}

/**
 * \brief Indicates if the contained pages are privately mapped.
 */
VPageRange::TriState VPageRange::isPrivate(void) const {
  return perm_isprivate;
}

void VPageRange::setPrivateS(VPageRange::TriState isprivate) {
  perm_isprivate = isprivate;
}

/**
 * \brief Returns the number of the page range.
 */
unsigned VPageRange::getVPRangeNumber(void) const {
  return range_no;
}

void VPageRange::setVPRangeNumber(unsigned new_no) {
  range_no = new_no;
}

/**
 * \brief Indicates if the range contains any pages.
 *
 * Returns \c true if the range does not contain any virtual pages. This is the
 * case when first_address == next_address.
 */
bool VPageRange::empty(void) const {
  if (first_address == next_address) {
    return true;
  } else {
    return false;
  }
}

/**
 * \brief Returns the size in bytes of the range.
 */
uint64_t VPageRange::size(void) const {
  return (next_address-first_address);
}

/**
 * \brief Returns the number of virtual pages contained in the range.
 */
uint64_t VPageRange::num(void) const {
  return ((next_address-first_address) / page_size);
}

const VPageRange::VP_List_Ty& VPageRange::getVPages(void) const {
  return v_pages;
}

/**
 * \brief Populates the range with pages.
 * \param fd The file descriptor of the file to read page information from.
 *
 * Populates the page range by creating a \c VPage object for each virtual page
 * contained in the represented page. The page descriptor is read from the file
 * described by \c fd. This function does not close the file.
 */
size_t VPageRange::populatePages(const int fd, const CmdOptions &cmd_opts) {
  // Store format flags for clog
  std::ios_base::fmtflags original_clog_flags = std::clog.flags();
  // Ignore unmapped ranges
  if (map_ty == MappingType::Unmapped) {
    return 0;
  }
  // Check if file descriptor could be valid
  if (fd < 0) {
    return 0;
  }
  // Compute start and end addresses that are aligned to page size
  const uint64_t aligned_low_addr = first_address & (~(page_size - 1));
  uint64_t tmp_addr = next_address & (page_size - 1);
  if (tmp_addr == 0) {
    tmp_addr = next_address;
  } else {
    tmp_addr = (next_address + page_size) & (~(page_size - 1));
  }
  const uint64_t aligned_up_addr = tmp_addr;

  v_pages.clear();
  // Compute the proper first seek position within the pagemap file
  const off_t pm_vpr_offset = (aligned_low_addr / page_size) * (64 / CHAR_BIT);
  if (cmd_opts.cmd_verbose == true) {
    std::clog << "Trying to position reader in pagemap file at offset "
              << std::hex << std::uppercase << "0x" << pm_vpr_offset
              << " for address " << "0x" << aligned_low_addr << std::endl;
  }
  // Now seek to the position
  if (lseek(fd, pm_vpr_offset, SEEK_SET) == -1) {
    std::cerr << "Failed to position in pagemap file." << std::endl;
    perror("lseek:");
    return 0;
  }
  // Create the pages
  for (uint64_t cur_addr = aligned_low_addr; cur_addr < aligned_up_addr;
      cur_addr += page_size) {
    // We do not have to seek again as the read command increments the file
    // offset by the number of bytes read
    uint64_t page_descriptor = 0;
    ssize_t read_bytes = read(fd, &page_descriptor, sizeof(page_descriptor));
    if (read_bytes == -1) {
      std::cerr << "Could not properly read from pagemap file!" << std::endl;
      perror("read:");
      break;
    } else if (read_bytes == sizeof(page_descriptor)) {
      VPage cur_page(cur_addr);
      cur_page.setRawPageProperties(page_descriptor, true);
      v_pages.push_back(cur_page);
    } else {
      VPage cur_page(cur_addr);
      cur_page.setRawPageProperties(0, false);
      v_pages.push_back(cur_page);
    }
    VPage cur_page(cur_addr);
  }

  // Restore format flags of clog
  std::clog.flags(original_clog_flags);
  return v_pages.size();
}

std::ostream& operator<<(std::ostream &stream, const VPageRange &vp_range) {
  // Save format flags
  std::ios_base::fmtflags original_flags = stream.flags();

  const int width_pr_no = 4;
  const int width_addresses = 16;
  const int width_perms = 4;      // Do not change!!!
  const int width_num_pages = 3;
  const int width_offset = 8;

  if (vp_range.getMappingType() == VPageRange::MappingType::Unmapped) {
    // Print the pagerange number
    stream << std::setfill('*') << std::setw(width_pr_no) << "*";
    // Print the addresses
    stream << " ";
    stream << std::hex << std::uppercase << std::setfill('0');
    stream << "0x" << std::setw(width_addresses) << vp_range.getFirstAddress();
    stream << "-";
    stream << "0x" << std::setw(width_addresses) << vp_range.getNextAddress();
    // Now print the permissions
    stream << " ";
    stream << std::setfill('*') << std::setw(width_perms) << "*";
    // Now print the number of contained pages
    stream << " ";
    stream << std::dec << std::setfill(' ') << std::left;
    stream << std::setw(width_num_pages + 1 + 2 + width_offset) << vp_range.num();
    // Now print the mapping type
    stream << " ";
    stream << "n-";
    // Now print the [null] indicator
    stream << "    [null]";
  } else if (vp_range.getMappingType() == VPageRange::MappingType::Mixed) {
    // Print the pagerange number
    stream << std::setfill('*') << std::setw(width_pr_no) << "*";
    // Print the addresses
    stream << " ";
    stream << std::hex << std::uppercase << std::setfill('0');
    stream << "0x" << std::setw(width_addresses) << vp_range.getFirstAddress();
    stream << "-";
    stream << "0x" << std::setw(width_addresses) << vp_range.getNextAddress();
    // Now print the permissions
    stream << " ";
    stream << std::setfill('?') << std::setw(width_perms) << "?";
    // Now print the number of contained pages
    stream << " ";
    stream << std::dec << std::setfill(' ') << std::left;
    stream << std::setw(width_num_pages + 1 + 2 + width_offset) << vp_range.num();
    // Now print the mapping type
    stream << " ";
    stream << "mu";
  } else {
    // Print the pagerange number
    stream << std::dec << std::setfill('0');
    stream << std::setw(width_pr_no) << vp_range.getVPRangeNumber();
    // Print the addresses
    stream << " ";
    stream << std::hex << std::uppercase << std::setfill('0');
    stream << "0x" << std::setw(width_addresses) << vp_range.getFirstAddress();
    stream << "-";
    stream << "0x" << std::setw(width_addresses) << vp_range.getNextAddress();
    // Now print the permissions
    stream << " ";
    switch (vp_range.canRead()) {
      case VPageRange::TriState::True:
        stream << "r";
        break;
      case VPageRange::TriState::False:
        stream << "-";
        break;
      default:
        stream << "?";
        break;
    }
    switch (vp_range.canWrite()) {
      case VPageRange::TriState::True:
        stream << "w";
        break;
      case VPageRange::TriState::False:
        stream << "-";
        break;
      default:
        stream << "?";
        break;
    }
    switch (vp_range.canExec()) {
      case VPageRange::TriState::True:
        stream << "x";
        break;
      case VPageRange::TriState::False:
        stream << "-";
        break;
      default:
        stream << "?";
        break;
    }
    switch (vp_range.isPrivate()) {
      case VPageRange::TriState::True:
        stream << "p";
        break;
      case VPageRange::TriState::False:
        stream << "s";
        break;
      default:
        stream << "?";
        break;
    }
    // Now print the number of contained pages
    stream << " ";
    stream << std::dec << std::setfill('0');
    stream << std::setw(width_num_pages) << vp_range.num();
    // Now print the offset
    stream << " ";
    stream << std::hex << std::uppercase << std::setfill('0');
    stream << "0x" << std::setw(width_offset) << vp_range.getMappingOffset();
    // Now print the mapping type
    stream << " ";
    if (vp_range.getMappingType() == VPageRange::MappingType::Anonymous) {
      stream << "-a";
    } else if (vp_range.getMappingType() == VPageRange::MappingType::Filemapping) {
      stream << "-f";
    }
    // Now print a possibly needed file
    if (vp_range.getMappingType() == VPageRange::MappingType::Filemapping) {
      stream << "    ";
      stream << vp_range.getMappedFilePath();
    }
  }

  // Restore format flags
  stream.flags(original_flags);
  return stream;
}
