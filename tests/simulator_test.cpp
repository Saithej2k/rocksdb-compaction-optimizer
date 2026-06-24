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

  std::cout << "all tests passed\n";
}

