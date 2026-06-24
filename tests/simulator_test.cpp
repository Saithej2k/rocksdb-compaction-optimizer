#include "compaction/simulator.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

void Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
  }
}

}  // namespace

int main() {
  compaction::Scheduler scheduler(compaction::Strategy::kLegacy);
  compaction::SchedulerInput input;
  input.level_bytes = {80.0, 120.0, 20.0, 10.0};
  input.target_bytes = {100.0, 100.0, 100.0, 100.0};
  input.pending_flush_bytes = 1000.0;

  Expect(scheduler.PickLevel(input) == 1,
         "legacy scoring should select the largest current level score");

  compaction::Scheduler pending_aware(
      compaction::Strategy::kPendingFlushAware);
  input.pending_flush_bytes = 50.0;
  Expect(pending_aware.PickLevel(input) == 0,
         "pending-aware scoring should include queued flush bytes in L0");

  input.pending_flush_bytes = 0.0;
  Expect(pending_aware.PickLevel(input) == scheduler.PickLevel(input),
         "pending-aware scoring should match legacy without pending bytes");

  compaction::SimulationConfig small_config;
  small_config.operations = 50'000;
  const auto first =
      compaction::RunSimulation(compaction::Strategy::kLegacy, small_config);
  const auto second =
      compaction::RunSimulation(compaction::Strategy::kLegacy, small_config);

  Expect(first.accepted_writes == second.accepted_writes,
         "simulation should be deterministic");
  Expect(first.stalled_writes == second.stalled_writes,
         "stall count should be deterministic");
  Expect(std::isfinite(first.StallRatePercent()),
         "stall rate should be finite");

  const auto optimized = compaction::RunSimulation(
      compaction::Strategy::kPendingFlushAware, small_config);
  Expect(optimized.stalled_writes <= first.stalled_writes,
         "pending-aware scoring should not increase write stalls");

  std::cout << "all tests passed\n";
}
