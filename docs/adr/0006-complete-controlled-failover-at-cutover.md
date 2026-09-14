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

# Complete controlled failover at committed cutover

## Status

Accepted

An operator's Controlled Failover completes successfully when Raft commits the
Cutover after the Candidate has reported Promotion Preparation, rather than
waiting for a later serving heartbeat. A Candidate failure after that point is
a new Owner failure handled by an independent Uncontrolled Failover. The
controlled deadline is absolute and committed across Meta Leader changes, and
new writes fail fast with `TRYAGAIN` while the reversible Controlled Pause is
active.
