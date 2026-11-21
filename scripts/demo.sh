#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
make all
./build/social_cli demo results/demo
./build/social_cli route data/sample 0 11 dijkstra
./build/social_cli route data/sample 0 11 bellman
./build/social_cli match kmp 'community community campaign' community
./build/social_cli workbench data/sample 35
./build/social_bench results/benchmarks.csv
python3 scripts/compare_scenarios.py
