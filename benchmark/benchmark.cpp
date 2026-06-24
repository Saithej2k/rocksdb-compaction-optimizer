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
  const auto hysteresis = compaction::RunSimulation(
      compaction::Strategy::kPendingFlushHysteresis);

  PrintResult(legacy);
  std::cout << '\n';
  PrintResult(pending_aware);
  std::cout << '\n';
  PrintResult(hysteresis);

  const auto reduction = [](std::uint64_t baseline, std::uint64_t improved) {
    return baseline == 0
               ? 0.0
               : 100.0 * (static_cast<double>(baseline) -
                          static_cast<double>(improved)) /
                     static_cast<double>(baseline);
  };
  std::cout << "\npending_aware_stall_reduction_percent="
            << reduction(legacy.stalled_writes, pending_aware.stalled_writes)
            << '\n'
            << "hysteresis_stall_reduction_percent="
            << reduction(legacy.stalled_writes, hysteresis.stalled_writes)
            << '\n'
            << "hysteresis_incremental_reduction_percent="
            << reduction(pending_aware.stalled_writes,
                         hysteresis.stalled_writes)
            << '\n';
}
