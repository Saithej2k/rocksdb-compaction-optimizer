#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace compaction {

constexpr std::size_t kLevelCount = 4;

enum class Strategy {
  kLegacy,
  kPendingFlushAware,
  kPendingFlushHysteresis,
};

struct SchedulerInput {
  std::array<double, kLevelCount> level_bytes{};
  std::array<double, kLevelCount> target_bytes{};
  double pending_flush_bytes = 0.0;
  double pending_flush_weight = 1.0;
  double switch_hysteresis = 0.05;
};

class Scheduler {
 public:
  explicit Scheduler(Strategy strategy);

  std::size_t PickLevel(const SchedulerInput& input);

 private:
  Strategy strategy_;
  bool initialized_ = false;
  std::size_t selected_level_ = 0;
};

struct SimulationConfig {
  std::uint64_t operations = 10'000'000;
  std::uint32_t write_percent = 80;
  std::uint64_t seed = 0x5eed1234ULL;

  double bytes_per_write = 256.0;
  double memtable_flush_bytes = 2.0 * 1024.0 * 1024.0;
  double flush_bytes_per_operation = 420.0;
  double compaction_bytes_per_operation = 245.0;
  std::uint64_t scheduling_quantum = 128;
  std::uint64_t compaction_setup_operations = 40;

  double max_pending_flush_bytes = 5.0 * 1024.0 * 1024.0;
  double l0_hard_limit_bytes = 8.0 * 1024.0 * 1024.0;
  double output_ratio = 0.88;
  double pending_flush_weight = 1.0;
  double switch_hysteresis = 0.05;

  std::array<double, kLevelCount> target_bytes{
      2.0 * 1024.0 * 1024.0,
      32.0 * 1024.0 * 1024.0,
      256.0 * 1024.0 * 1024.0,
      2048.0 * 1024.0 * 1024.0,
  };

  std::array<double, kLevelCount> initial_level_bytes{
      2.0 * 1024.0 * 1024.0,
      30.0 * 1024.0 * 1024.0,
      210.0 * 1024.0 * 1024.0,
      1550.0 * 1024.0 * 1024.0,
  };
};

struct SimulationResult {
  std::string strategy;
  std::uint64_t operations = 0;
  std::uint64_t accepted_writes = 0;
  std::uint64_t stalled_writes = 0;
  std::uint64_t scheduler_switches = 0;
  std::array<std::uint64_t, kLevelCount> level_selections{};
  double final_pending_flush_bytes = 0.0;
  double max_pending_flush_bytes = 0.0;
  double final_l0_bytes = 0.0;

  double StallRatePercent() const;
};

SimulationResult RunSimulation(Strategy strategy,
                               const SimulationConfig& config = {});
std::string StrategyName(Strategy strategy);

}  // namespace compaction
