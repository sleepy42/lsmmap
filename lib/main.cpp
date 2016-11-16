#include "CmdOptions.h"
#include "Process.h"

#include <iostream>
#include <limits>
#include <string>

int main(int argc, char *argv[]) {
  CmdOptions cmdopts;
  CmdOptions::ErrorType opts_parsed = cmdopts.parseFromCommandLine(argc, argv);
  if ((opts_parsed == CmdOptions::ErrorType::Option)
   || (opts_parsed == CmdOptions::ErrorType::OptArg)) {
    exit(-1);
  }

  if (cmdopts.cmd_req_pid.size() <= 0) {
    exit(-1);
  }

  Process p("self");
  p.populateRanges(cmdopts);
  p.populatePages(cmdopts);

  std::clog << "Found the following "
            << std::dec << p.getVPageRanges().size() << " ranges:" << std::endl;
  // Print header lines
  std::clog << "no  "
            << " " << "from              " << " " << "to                "
            << " " << "perm"
            << " " << "#  "
            << " " << "offset    "
            << " " << "ty"
            << "    " << "file" << std::endl;
  std::clog << "    "
            << " " << "address           "
            << " " << "flgs"
            << " " << "pfn             "
            << " " << "swapoff        "
            << " " << "swty"
            << " " << "rawprops" << std::endl;
  // Print found page ranges and pages
  for (const VPageRange &vpr : p.getVPageRanges()) {
    std::clog << vpr << std::endl;
    for (const VPage &vp : vpr.getVPages()) {
      std::clog << "     " << vp << std::endl;
    }
  }
}
