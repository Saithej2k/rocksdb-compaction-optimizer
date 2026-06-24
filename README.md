# Compaction Scheduling Optimizer

An original C++17 simulator for studying level-based compaction scheduling
under write-heavy LSM-tree workloads.

The baseline models a conventional level-score heuristic: each level is
ranked by its current bytes divided by its target size. This deliberately
small model makes scheduling trade-offs reproducible without requiring a full
database build.

## Build and test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/compaction_benchmark
```

The default benchmark runs 10 million deterministic operations at an 80/20
write/read ratio. It reports accepted writes, stalled writes, scheduling
switches, and pending-flush pressure.

## Scope

This repository is a standalone educational simulator inspired by LSM-tree
storage engines. It is not a fork of RocksDB and does not claim upstream
RocksDB contributions.

