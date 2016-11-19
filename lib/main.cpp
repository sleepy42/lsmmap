#include "CmdOptions.h"
#include "Output.h"
#include "PMemory.h"
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

  #if 0
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
  std::vector<uint64_t> req_frame_no;
  // Collect information about the virtual memory space
  for (Process &cur_proc : processes) {
    cur_proc.populateRanges(cmdopts);
    cur_proc.populatePages(cmdopts);
    // Print found page ranges and pages
    std::clog << "Process: " << cur_proc.getPID() << std::endl;
    std::clog << "Found the following "
              << std::dec << cur_proc.getVPageRanges().size() << " ranges:" << std::endl;
    for (const VPageRange &vpr : cur_proc.getVPageRanges()) {
      std::clog << vpr << std::endl;
      for (const VPage &vp : vpr.getVPages()) {
        // std::clog << "     " << vp << std::endl;
        if (vp.isPresentRAM() == true) {
          req_frame_no.push_back(vp.getFrameNumber());
        }
      }
    }
  }
  #endif

  // First gather information about page ranges and pages
  for (Process &cur_proc : processes) {
    cur_proc.populateRanges(cmdopts);
    cur_proc.populatePages(cmdopts);
  }
  
  // Now gather all required physical frames
  std::vector<uint64_t> reqd_frames;
  for (const Process &cur_proc : processes) {
    for (const VPageRange &cur_vpr : cur_proc.getVPageRanges()) {
      // Skip unmapped ranges as they should not require any valid frames
      if (cur_vpr.getMappingType() == VPageRange::MappingType::Unmapped) {
        continue;
      }

      for (const VPage &cur_vpage : cur_vpr.getVPages()) {
        if (cur_vpage.arePagePropertiesValid() == false) {
          continue;
        }
        // Only present pages map to a valid frame
        if (cur_vpage.isPresentRAM() == false) {
          continue;
        }
        if (cur_vpage.getFrameNumber() == 0) {
          continue;
        }
        // Page maps to a frame so add its frame to the list of required frames
        reqd_frames.push_back(cur_vpage.getFrameNumber());
      }
    }
  }

  // Now gather information about all required frames
  PMemory pmem;
  size_t added_frames = pmem.addPFrames(cmdopts, reqd_frames.begin(),
      reqd_frames.end());

  printResults(cmdopts, std::cout, processes, pmem);
}
