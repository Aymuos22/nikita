#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
make all
./build/social_cli demo results/demo
./build/social_cli route data/sample 0 11 dijkstra
./build/social_cli route data/sample 0 11 bellman
./build/social_cli match kmp 'community community campaign' community
./build/social_bench results/benchmarks.csv
