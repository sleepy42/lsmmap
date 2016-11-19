//===- CmdOptions.cpp -----------------------------------------------------===//
//
// Short description of the available options
// -l x     Define a lower address to limit the output of ranges.
// -u x     Define an upper address to limit the output of ranges.
//          The -l and -u options form an address range. All detected ranges
//          must "touch" that defined range. Found virtual page ranges will not
//          be cut if they cross any of the bounding addresses defined by -l/-u.
// -n       Show unmapped address regions.
// -v       Be verbose.
// -a       Show all virtual pages and do NOT omit unmapped pages.
//
// Supported Modes:
// -M       Default mode: Show mapping from virtual pages to physical frames.
// -P       Page mode: The user MUST  specify an virtual address interval using
//          the -l and -u option. The mapping for all contained virtual pages
//          will be shown no matter if they are used/mapped or not.
//   Note: The last program mode given will be used. Using multiple different
//   modes is currently NOT detected.
//
// Usage:
// lsmmap [ -l <lower> ] [ -u <upper> ] [ -n ] [ -v ] [ <pid>... ]
//
//===----------------------------------------------------------------------===//

#include "CmdOptions.h"

#include <climits>
#include <cstdlib>
#include <iostream>
#include <limits>

/**
 * \brief Determines if the given string is a valid process id.
 *
 * Determines if the given string \c str is a valid process id. A valid process
 * id is a string that could be access via the /proc directory. So in /proc
 * might be a directory with the name given in \c str. Therefor \c str might
 * only consist of digits (0-9) or be the string "self" (as there is a directory
 * /proc/self that points to the directory of the process that accesses it).
 * Note that this function does *NOT* check wether the /proc/<str> actually
 * exists.
 */
bool isPID(const std::string &str) {
  if (str2ulong(str, nullptr, 10) == true) {
    return true;
  } else if (str.compare("self") == 0) {
    return true;
  } else {
    return false;
  }
}

bool str2long(const std::string &str, long int *value, int base) {
  // Test for empty string
  if (str.empty() == true) {
    return false;
  }
  
  // So we now know, that the string is not empty
  errno = 0;
  const char* const str_cstr = str.c_str();
  char *end = nullptr;
  long int tmp = strtol(str_cstr, &end, base);
  // Check for out-of-range
  if (((tmp == LONG_MAX) && (errno == ERANGE))
   || ((tmp == LONG_MIN) && (errno == ERANGE))) {
    return false;
  }
  // Check if the whole string was parsed
  if (end != str_cstr + str.size()) {
    return false;
  }

  // Everything seems to be ok...
  if (value != nullptr) {
    *value = tmp;
  }
  return true;
}

bool str2ulong(const std::string &str, unsigned long int *value, int base) {
  // Test for empty string
  if (str.empty() == true) {
    return false;
  }
  
  // So we now know, that the string is not empty
  errno = 0;
  const char* const str_cstr = str.c_str();
  char *end = nullptr;
  unsigned long int tmp = strtoul(str_cstr, &end, base);
  // Check for out-of-range
  if ((tmp == ULONG_MAX) && (errno == ERANGE)) {
    return false;
  }
  // Check if the whole string was parsed
  if (end != str_cstr + str.size()) {
    return false;
  }

  // Everything seems to be ok...
  if (value != nullptr) {
    *value = tmp;
  }
  return true;
}

//===- CmdOptions functions -----------------------------------------------===//

CmdOptions::CmdOptions()
 : parsed_from_cmdl(false),
   cmd_lower_address(0), cmd_low_addr_userset(false),
   cmd_upper_address(std::numeric_limits<uint64_t>::max()), cmd_up_addr_userset(false),
   cmd_show_unmapped(false), cmd_verbose(false), cmd_show_all_pages(false),
   cmd_prog_mode(ProgMode::Mappings) {
}

/**
 * \brief Parses the command line options.
 * \param argc The \c argc parameter of the main function.
 * \param argv The \c argv parameter of the main function.
 *
 * The function parses the provided command line options and stores their values
 * in the \c CmdOptions object.
 */
CmdOptions::ErrorType CmdOptions::parseFromCommandLine(int argc, char *argv[]) {
  extern int optind;
  extern char *optarg;

  ErrorType errty = ErrorType::NoError;
  char c;
  while ((c = getopt(argc, argv, "l:u:nvaMP")) != -1) {
    switch(c) {
      case 'a':
        cmd_show_all_pages = true;
        break;
      case 'l':
        if (str2ulong(optarg, &cmd_lower_address, 16) == true) {
          cmd_low_addr_userset = true;
        } else {
          cmd_lower_address = 0;
        }
        break;
      case 'M':
        cmd_prog_mode = ProgMode::Mappings;
        break;
      case 'n':
        cmd_show_unmapped = true;
        break;
      case 'P':
        cmd_prog_mode = ProgMode::Pages;
        break;
      case 'u':
        if (str2ulong(optarg, &cmd_upper_address, 16) == true) {
          cmd_up_addr_userset = true;
        } else {
          cmd_upper_address = std::numeric_limits<uint64_t>::max();
        }
        break;
      case 'v':
        cmd_verbose = true;
        break;
      case '?': case ':':
        errty = ErrorType::Option;
        break;
    }
  }
  // Now parse the remaining options. They represent the requested process ids.
  if (optind < argc) {
    for (int i = optind; i < argc; ++i) {
      std::string cur_pid(argv[i]);
      if (isPID(cur_pid) == true) {
        cmd_req_pid.push_back(cur_pid);
      } else {
        std::cerr << cur_pid << " is not a valid process id!" << std::endl;
        errty = ErrorType::PID;
      }
    }
  } else {
    // If no process ids are given add the self id.
    cmd_req_pid.push_back("self");
  }

  parsed_from_cmdl = true;

  return errty;
}
