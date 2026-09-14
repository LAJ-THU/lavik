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

# Use typed failover commands

## Status

Accepted

Failover state changes use narrow commands for controlled and uncontrolled
begin, uncontrolled Candidate replacement, Promotion Authorization, controlled
abort, controlled degradation, and mode-specific Cutover. Compound commands
validate and mutate bounded copies of the topology, grant, and Operation stores
before publishing them together. This deliberately uses more command tags
than a generic transition setter so illegal combinations of active grant,
fenced term, Operation terminal state, and Cutover semantics cannot be encoded.
