<!--
Copyright (C) 2026 EloqData Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

# Tomb Raider scheduling

Tomb Raider retires tombstones that no surviving disk record needs. Exactly
one of three scheduling modes is active at a time:

- `TOMBRAIDER OFF` disables future rounds. A round already running completes.
- `TOMBRAIDER INTERVAL <milliseconds>` runs one round after each interval. The
  next interval starts only after the previous round completes.
- `TOMBRAIDER DAILY <HH:MM[:SS]>` runs once per day at that time in the
  server's local timezone. If a round overlaps a later scheduled time, that
  occurrence is skipped rather than run concurrently.

`TOMBRAIDER ON` restores the schedule active before `OFF`.
`TOMBRAIDER BLOCK-SLEEP <milliseconds>` changes the pause after each scanned
block and takes effect during the current round. Zero disables the pause.
`TOMBRAIDER STATUS` reports the mode, interval, block pause, daily time,
timezone, and whether a round is running.

Runtime changes are not persisted across restarts. Startup uses
`--tomb-raider-interval-ms` and `--tomb-raider-sleep-ms`; an interval of zero
starts in off mode.
