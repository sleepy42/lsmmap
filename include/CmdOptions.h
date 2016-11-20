//===- CmdOptions.h -------------------------------------------------------===//
//
// This file contains a class that parses and validates the command line
// options and stores them.
//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_CMDOPTIONS_H_INCLUDE_
#define LSMMAP_CMDOPTIONS_H_INCLUDE_

#include <cstdint>
#include <string>
#include <unistd.h>
#include <vector>

bool isPID(const std::string &str);
bool str2long(const std::string &str, long int *value = nullptr, int base = 0);
bool str2ulong(const std::string &str, unsigned long int *value = nullptr, int base = 0);

class CmdOptions {
public:
  enum class ErrorType {NoError = 0, Option, OptArg, PID};
  enum class ProgMode {Mappings = 0, Pages};
  typedef std::vector<std::string> PID_List_Ty;

  bool parsed_from_cmdl;
  uint64_t cmd_lower_address;
  bool cmd_low_addr_userset;
  uint64_t cmd_upper_address;
  bool cmd_up_addr_userset;
  bool cmd_show_unmapped;
  bool cmd_verbose;
  bool cmd_show_all_pages;
  ProgMode cmd_prog_mode;
  bool cmd_only_vpranges;
  PID_List_Ty cmd_req_pid;

  CmdOptions();
  ErrorType parseFromCommandLine(int argc, char *argv[]);
};

#endif
