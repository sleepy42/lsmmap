#include "CmdOptions.h"
#include "Process.h"

#include <iostream>
#include <limits>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  CmdOptions cmdopts;
  CmdOptions::ErrorType opts_parsed = cmdopts.parseFromCommandLine(argc, argv);
  if ((opts_parsed == CmdOptions::ErrorType::Option)
   || (opts_parsed == CmdOptions::ErrorType::OptArg)) {
    exit(-1);
  }

  // Validate pids and create process objects
  std::vector<Process> processes;
  for (CmdOptions::PID_List_Ty::iterator pid_it = cmdopts.cmd_req_pid.begin(),
      pid_end = cmdopts.cmd_req_pid.end(); pid_it != pid_end; ) {
    if ((str2ulong(*pid_it, nullptr, 10) == false)
     && (pid_it->compare("self") != 0)) {
      // Hmm, invalid iterator erase the pid
      pid_it = cmdopts.cmd_req_pid.erase(pid_it);
      pid_end = cmdopts.cmd_req_pid.end();
      continue;
    }
    // Now try to create the process object. If any of the needed files the
    // CTOR will throw an exception...
    try {
      Process cur_proc(*pid_it);
      processes.push_back(cur_proc);
    } catch(std::invalid_argument inv_arg_exc) {
      std::cerr << "Skipping pid " << *pid_it << ": some needed files "
                << "are not accessible!" << std::endl;
      pid_it = cmdopts.cmd_req_pid.erase(pid_it);
      pid_end = cmdopts.cmd_req_pid.end();
      continue;
    }
    ++pid_it;
  }

  if (processes.size() <= 0) {
    std::cerr << "No pids to process!" << std::endl;
    exit(-1);
  }

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
  for (Process &cur_proc : processes) {
    cur_proc.populateRanges(cmdopts);
    cur_proc.populatePages(cmdopts);
    // Print found page ranges and pages
    std::clog << "Process: " << cur_proc.getPID() << std::endl;
    std::clog << "Found the following "
              << std::dec << cur_proc.getVPageRanges().size() << " ranges:" << std::endl;
    for (const VPageRange &vpr : cur_proc.getVPageRanges()) {
      std::clog << vpr << std::endl;
      /*
      for (const VPage &vp : vpr.getVPages()) {
        std::clog << "     " << vp << std::endl;
      }
      */
    }
  }
}
