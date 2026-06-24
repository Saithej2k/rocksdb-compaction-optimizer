# Compaction Scheduling Optimizer

An original C++17 simulator for studying level-based compaction scheduling
under write-heavy LSM-tree workloads.

The simulator includes two scheduling strategies:

- `legacy`: rank each level by current bytes divided by target size.
- `pending-flush-aware`: add queued flush bytes to L0's score so compaction
  reacts before newly flushed data reaches the hard limit.
- `pending-flush-hysteresis`: retain the selected level until another score is
  materially higher, avoiding costly back-and-forth scheduling.

This deliberately small model makes scheduling trade-offs reproducible
without requiring a full database build.

## Build and test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/compaction_benchmark
```

The default benchmark runs 10 million deterministic operations at an 80/20
write/read ratio. It reports accepted writes, stalled writes, scheduling
switches, pending-flush pressure, and the stall reduction relative to legacy
scoring.

### Reference result

Release build on the default deterministic workload:

| Strategy | Stalled writes | Stall rate |
| --- | ---: | ---: |
| Legacy level score | 3,744,841 | 46.812% |
| Pending-flush-aware | 2,916,293 | 36.455% |
| Pending flush + hysteresis | 2,810,376 | 35.131% |

Pending-flush-aware scoring reduces stalled writes by **22.125%**. Adding 5%
score hysteresis raises the total reduction to **24.953%** and cuts scheduler
switches from 6,965 to 1,269 compared with pending-aware scoring alone.

The model includes a 40-operation setup cost when switching compaction levels,
representing task setup and iterator initialization. Pending-aware scoring
starts L0 work earlier, while hysteresis avoids repeatedly paying that setup
cost for marginal score changes.

These are synthetic simulator results, not RocksDB production measurements.

## Scope

This repository is a standalone educational simulator inspired by LSM-tree
storage engines. It is not a fork of RocksDB and does not claim upstream
RocksDB contributions.
