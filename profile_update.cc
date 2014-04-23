// Updates the AutoFDO profile using the module info and debug info stored
// in the new binary.

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "base/common.h"
#include "base/logging.h"
#include "addr2line.h"
#include "gcov.h"
#include "profile_reader.h"
#include "profile_writer.h"
#include "symbol_map.h"
#include "module_grouper.h"

DEFINE_string(input, "fbdata.afdo",
              "Input file name of the old profile.");
DEFINE_string(output, "fbdata.afdo",
              "Output file name of the new profile.");
DEFINE_string(binary, "",
              "New binary that has updated module and debug info (built with "
              "-gmlt and -frecord-compilation-info-in-elf).");

using autofdo::SymbolMap;
using autofdo::ModuleGrouper;
using autofdo::Addr2line;
using autofdo::AutoFDOProfileReader;
using autofdo::AutoFDOProfileWriter;

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  SymbolMap symbol_map;
  std::unique_ptr<Addr2line> addr2line(Addr2line::Create(FLAGS_binary));

  AutoFDOProfileReader reader(&symbol_map, NULL);
  reader.ReadFromFile(FLAGS_input);
  symbol_map.UpdateSymbolMap(FLAGS_binary, addr2line.get());

  ModuleGrouper *grouper = ModuleGrouper::GroupModule(
      FLAGS_binary, GCOV_ELF_SECTION_NAME, &symbol_map);

  AutoFDOProfileWriter writer(symbol_map,
      grouper->module_map(), FLAGS_gcov_version);
  if (!writer.WriteToFile(FLAGS_output)) {
    LOG(FATAL) << "Error writing to " << FLAGS_output;
  }
  return 0;
}
