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

# Separate uncontrolled execution from failure detection

## Status

Accepted

The shared failover foundation includes the complete Uncontrolled Executor,
including fencing, an empty Candidate slot, replacement, preparation, and
Cutover. Automatic SUSPECT detection and policy only decide when to begin that
executor. This lets a controlled transition degrade safely and lets operators
exercise recovery without coupling its correctness to the later automatic
detector.
