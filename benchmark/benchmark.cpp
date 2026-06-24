#include "compaction/simulator.h"

#include <iomanip>
#include <iostream>

int main() {
  const auto result =
      compaction::RunSimulation(compaction::Strategy::kLegacy);

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

