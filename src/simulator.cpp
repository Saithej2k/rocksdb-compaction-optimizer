#include "compaction/simulator.h"

#include <algorithm>
#include <cmath>

namespace compaction {
namespace {

class XorShift64 {
 public:
  explicit XorShift64(std::uint64_t seed) : state_(seed == 0 ? 1 : seed) {}

  std::uint64_t Next() {
    state_ ^= state_ << 13;
    state_ ^= state_ >> 7;
    state_ ^= state_ << 17;
    return state_;
  }

 private:
  std::uint64_t state_;
};

}  // namespace

Scheduler::Scheduler(Strategy strategy) : strategy_(strategy) {}

std::size_t Scheduler::PickLevel(const SchedulerInput& input) {
  std::array<double, kLevelCount> scores{};
  std::size_t candidate = 0;
  double best_score = -1.0;
  for (std::size_t level = 0; level < kLevelCount; ++level) {
    double bytes = input.level_bytes[level];
    if (strategy_ != Strategy::kLegacy && level == 0) {
      bytes += input.pending_flush_bytes * input.pending_flush_weight;
    }
    scores[level] = bytes / input.target_bytes[level];
    if (scores[level] > best_score) {
      candidate = level;
      best_score = scores[level];
    }
  }

  if (strategy_ != Strategy::kPendingFlushHysteresis) {
    return candidate;
  }

  if (!initialized_) {
    initialized_ = true;
    selected_level_ = candidate;
    return selected_level_;
  }

  const double current_score = scores[selected_level_];
  const bool current_level_drained =
      input.level_bytes[selected_level_] <
      input.target_bytes[selected_level_] * 0.01;
  const bool challenger_is_materially_higher =
      best_score > current_score * (1.0 + input.switch_hysteresis);

  if (candidate != selected_level_ &&
      (current_level_drained || challenger_is_materially_higher)) {
    selected_level_ = candidate;
  }
  return selected_level_;
}

double SimulationResult::StallRatePercent() const {
  const auto total_writes = accepted_writes + stalled_writes;
  if (total_writes == 0) {
    return 0.0;
  }
  return 100.0 * static_cast<double>(stalled_writes) /
         static_cast<double>(total_writes);
}

std::string StrategyName(Strategy strategy) {
  switch (strategy) {
    case Strategy::kLegacy:
      return "legacy";
    case Strategy::kPendingFlushAware:
      return "pending-flush-aware";
    case Strategy::kPendingFlushHysteresis:
      return "pending-flush-hysteresis";
  }
  return "unknown";
}

SimulationResult RunSimulation(Strategy strategy,
                               const SimulationConfig& config) {
  Scheduler scheduler(strategy);
  XorShift64 random(config.seed);
  auto level_bytes = config.initial_level_bytes;
  double memtable_bytes = 0.0;
  double pending_flush_bytes = 0.0;
  std::size_t selected_level = 0;
  std::size_t previous_level = 0;
  std::uint64_t setup_remaining = config.compaction_setup_operations;

  SimulationResult result;
  result.strategy = StrategyName(strategy);
  result.operations = config.operations;

  for (std::uint64_t operation = 0; operation < config.operations;
       ++operation) {
    const bool is_write = random.Next() % 100 < config.write_percent;
    const bool write_stalled =
        pending_flush_bytes >= config.max_pending_flush_bytes ||
        level_bytes[0] >= config.l0_hard_limit_bytes;

    if (is_write) {
      if (write_stalled) {
        ++result.stalled_writes;
      } else {
        ++result.accepted_writes;
        memtable_bytes += config.bytes_per_write;
      }
    }

    if (memtable_bytes >= config.memtable_flush_bytes) {
      const double flushes =
          std::floor(memtable_bytes / config.memtable_flush_bytes);
      const double bytes = flushes * config.memtable_flush_bytes;
      memtable_bytes -= bytes;
      pending_flush_bytes += bytes;
    }

    const double l0_room =
        std::max(0.0, config.l0_hard_limit_bytes - level_bytes[0]);
    const double flushed =
        std::min({pending_flush_bytes, config.flush_bytes_per_operation,
                  l0_room});
    pending_flush_bytes -= flushed;
    level_bytes[0] += flushed;

    if (operation % config.scheduling_quantum == 0) {
      SchedulerInput input;
      input.level_bytes = level_bytes;
      input.target_bytes = config.target_bytes;
      input.pending_flush_bytes = pending_flush_bytes;
      input.pending_flush_weight = config.pending_flush_weight;
      input.switch_hysteresis = config.switch_hysteresis;
      selected_level = scheduler.PickLevel(input);
      ++result.level_selections[selected_level];
      if (operation != 0 && selected_level != previous_level) {
        ++result.scheduler_switches;
        setup_remaining = config.compaction_setup_operations;
      }
      previous_level = selected_level;
    }

    if (setup_remaining > 0) {
      --setup_remaining;
    } else {
      const double compacted =
          std::min(level_bytes[selected_level],
                   config.compaction_bytes_per_operation);
      level_bytes[selected_level] -= compacted;
      if (selected_level + 1 < kLevelCount) {
        level_bytes[selected_level + 1] += compacted * config.output_ratio;
      }
    }

    result.max_pending_flush_bytes =
        std::max(result.max_pending_flush_bytes, pending_flush_bytes);
  }

  result.final_pending_flush_bytes = pending_flush_bytes;
  result.final_l0_bytes = level_bytes[0];
  return result;
}

}  // namespace compaction
