#!/usr/bin/env bash
# Copyright (C) 2026 EloqData Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

if [[ ${KEYLANE_CLUSTER_HARDWARE_OPT_IN:-0} != 1 ]]; then
  echo "cluster hardware tests skipped: set KEYLANE_CLUSTER_HARDWARE_OPT_IN=1"
  exit 77
fi

device=${KEYLANE_CLUSTER_SCRATCH_DEVICE:-}
if [[ -z $device ]]; then
  echo "KEYLANE_CLUSTER_SCRATCH_DEVICE is required after hardware opt-in" >&2
  exit 1
fi
if [[ $device != /dev/* || ! -b $device ]]; then
  echo "scratch device must be an existing absolute block device: $device" >&2
  exit 1
fi
if findmnt --noheadings --source "$device" >/dev/null 2>&1; then
  echo "refusing mounted scratch device: $device" >&2
  exit 1
fi

root_source=$(findmnt --noheadings --output SOURCE / | head -n 1)
if [[ $root_source == "$device" || $root_source == "$device"* ]]; then
  echo "refusing root filesystem device: $device" >&2
  exit 1
fi

echo "cluster hardware safety gate accepted $device"
