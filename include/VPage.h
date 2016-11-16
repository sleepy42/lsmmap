//===- VPage.h ------------------------------------------------------------===//
//
// This file contains the classes to maintain the virtual page tables of one
// process.
//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_VPAGE_H_INCLUDE_
#define LSMMAP_VPAGE_H_INCLUDE_

#include "CmdOptions.h"

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

/**
 * This class represents a single virtual page in the address space of a
 * process and stores all the flags available.
 */
class VPage {
private:
  uint64_t page_props;
  bool page_props_valid;
  uint64_t start_address;

public:
  VPage(uint64_t startaddress);

  uint64_t getStartAddress(void) const;

  bool arePagePropertiesValid(void) const;
  uint64_t getRawPageProperties(void) const;
  void setRawPageProperties(uint64_t new_props, bool valid);
  bool isPresentRAM(void) const;
  bool isPresentSwap(void) const;
  bool isFileMapped(void) const;
  bool isSoftDirty(void) const;
  uint64_t getFrameNumber(void) const;
  uint8_t getSwapType(void) const;
  uint64_t getSwapOffset(void) const;
};
std::ostream& operator<<(std::ostream &stream, const VPage &page);

/**
 * This class represents a range of virtual pages. It stores the start address
 * (the address of the first page in the range), the address of the last
 * contained page and the address of the first page not contained anymore.
 * Furthermore permission flags and other information are contained.
 */
class VPageRange {
public:
  enum class MappingType {Unmapped = 0, Anonymous, Filemapping, Mixed};
  enum class TriState {Unknown = 0, True, False};
  typedef std::vector<VPage> VP_List_Ty;

private:
  MappingType map_ty;
  uint64_t first_address;
  uint64_t next_address;
  std::string mapped_file_path;
  uint64_t map_offset;
  long page_size;
  TriState perm_canread;
  TriState perm_canwrite;
  TriState perm_canexec;
  TriState perm_isprivate;
  unsigned range_no;
  VP_List_Ty v_pages;

public:
  VPageRange(uint64_t firstaddress, uint64_t nextaddress, long pagesize);

  uint64_t getFirstAddress(void) const;
  uint64_t getLastAddress(void) const;
  uint64_t getNextAddress(void) const;
  MappingType getMappingType(void) const;
  void setMappingType(MappingType newtype);
  std::string getMappedFilePath(void) const;
  void setMappedFilePath(const std::string &path);
  uint64_t getMappingOffset(void) const;
  void setMappingOffset(uint64_t offset);
  long getPageSize(void) const;
  TriState canRead(void) const;
  void setReadP(TriState canread);
  TriState canWrite(void) const;
  void setWriteP(TriState canwrite);
  TriState canExec(void) const;
  void setExecP(TriState canexec);
  TriState isPrivate(void) const;
  void setPrivateS(TriState isprivate);
  unsigned getVPRangeNumber(void) const;
  void setVPRangeNumber (unsigned new_no);
  const VP_List_Ty& getVPages(void) const;

  bool empty(void) const;
  uint64_t size(void) const;
  uint64_t num(void) const;

  size_t populatePages(const int fd, const CmdOptions &cmd_opts);
};
std::ostream& operator<<(std::ostream &stream, const VPageRange &vp_range);

#endif
