//===- Output.h -----------------------------------------------------------===//
//
// Contains functions for printing results to some output stream.
//
//===----------------------------------------------------------------------===//

#ifndef LSMMAP_OUTPUT_H_INCLUDE_
#define LSMMAP_OUTPUT_H_INCLUDE_

#include <iterator>
#include <ostream>
#include <type_traits>
#include <vector>

#include "Process.h"
#include "PMemory.h"

inline char getTristateChar(const VPageRange::TriState &val, char TrueC,
    char FalseC = '-', char UnknownC = '?');
inline char getBoolChar(const bool val, char TrueC, char FalseC = '-');
void printResults(const CmdOptions &cmd_opts, std::ostream &stream,
    std::vector<Process> &processes, PMemory &pmem);
void printPageRangeHeadline(const CmdOptions &cmd_opts, std::ostream &stream);


#endif