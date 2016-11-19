//===- Process.h ----------------------------------------------------------===//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_PROCESS_H_INCLUDE_
#define LSMMAP_PROCESS_H_INCLUDE_

#include "CmdOptions.h"
#include "VPage.h"

#include <cstdint>
#include <string>
#include <vector>

class Process {
public:
  typedef std::vector<VPageRange> VPR_List_Ty;

private:
  std::string process_id;
  std::string maps_filepath;
  std::string pagemap_filepath;
  VPR_List_Ty vp_ranges;

  bool checkForFiles(void);

public:
  Process(std::string pid);

  const std::string& getPID(void) const;

  std::string getMapsFilePath(void) const;
  std::string getPageMapFilePath(void) const;
  const VPR_List_Ty& getVPageRanges(void) const;

  size_t populateRanges(const CmdOptions &cmd_opts);
  size_t populateRanges(const uint64_t lower_addr, const uint64_t upper_addr);
  size_t populatePages(const CmdOptions &cmd_opts);
};

#endif
