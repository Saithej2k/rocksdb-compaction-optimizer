#include "compaction/simulator.h"

#include <iomanip>
#include <iostream>

namespace {

void PrintResult(const compaction::SimulationResult& result) {
  std::cout << std::fixed << std::setprecision(3)
            << "strategy=" << result.strategy << '\n'
            << "operations=" << result.operations << '\n'
            << "accepted_writes=" << result.accepted_writes << '\n'
            << "stalled_writes=" << result.stalled_writes << '\n'
            << "stall_rate_percent=" << result.StallRatePercent() << '\n'
            << "scheduler_switches=" << result.scheduler_switches << '\n'
            << "max_pending_flush_mib="
            << result.max_pending_flush_bytes / (1024.0 * 1024.0) << '\n';
}

}  // namespace

int main() {
  std::cout << "workload=10000000_operations_80w20r\n\n";

  const auto legacy =
      compaction::RunSimulation(compaction::Strategy::kLegacy);
  const auto pending_aware =
      compaction::RunSimulation(compaction::Strategy::kPendingFlushAware);

  PrintResult(legacy);
  std::cout << '\n';
  PrintResult(pending_aware);

  const double reduction = legacy.stalled_writes == 0
                               ? 0.0
                               : 100.0 *
                                     (static_cast<double>(
                                          legacy.stalled_writes) -
                                      static_cast<double>(
                                          pending_aware.stalled_writes)) /
                                     static_cast<double>(
                                         legacy.stalled_writes);
  std::cout << "\nstall_reduction_percent=" << reduction << '\n';
}
