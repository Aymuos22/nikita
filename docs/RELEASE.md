# Release v1.0.0

The initial local release includes all 36 syllabus algorithm variants, the network analytics CLI, reusable static library, sample data, reproducible benchmark runner, test suite and documentation. It is a C++17 student-project implementation with the candidate, instance-size and measurement limitations described in the README.

From a fresh checkout:

```sh
make -j4
make test
scripts/demo.sh
```

A release tag identifies the verified local source state. No remote repository, issue tracker, publication or external deployment is configured. Generated demo reports can be recreated with `make demo`; benchmark evidence is versioned. Named student ownership, human peer reviews, and a recorded presentation remain student-provided evidence, tracked in `docs/OWNERSHIP.md`.
