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

# Emit sourceable startup settings from this process's CPU allowance. The plan
# keeps whole physical cores together and reserves roughly one quarter of them
# for IRQs and ordinary host work. It is a reproducible starting point; callers
# must validate its throughput and latency on their own workload.
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: scripts/plan_startup_tuning.sh [--reserve-cores N] [--nic NAME]

Print shell assignments for a Lavik CPU plan. Source the output or use it in a
service unit. --nic selects CPUs on the NIC's NUMA node when known;
it does not change host IRQ affinity.
EOF
}

reserve_cores=''
nic=''
while (($#)); do
  case "$1" in
    --reserve-cores)
      reserve_cores=${2:?--reserve-cores needs a number}
      shift 2
      ;;
    --nic)
      nic=${2:?--nic needs an interface name}
      shift 2
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      printf 'Unknown argument: %s\n' "$1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

[[ -z $reserve_cores || $reserve_cores =~ ^[0-9]+$ ]] || {
  printf '%s\n' '--reserve-cores must be a non-negative integer' >&2
  exit 2
}

if [[ -z $nic ]] && command -v ip >/dev/null; then
  nic=$(ip -o route show default 2>/dev/null |
    awk 'NR == 1 { for (i = 1; i <= NF; ++i) if ($i == "dev") { print $(i + 1); exit } }')
fi

nic_numa='unknown'
if [[ -n $nic ]]; then
  [[ -d /sys/class/net/$nic ]] || { printf 'Unknown network interface: %s\n' "$nic" >&2; exit 2; }
  nic_numa=$(cat "/sys/class/net/$nic/device/numa_node" 2>/dev/null || printf 'unknown')
fi

expand_cpu_list() {
  local item start end cpu
  IFS=',' read -r -a items <<<"$1"
  for item in "${items[@]}"; do
    if [[ $item == *-* ]]; then
      start=${item%-*}
      end=${item#*-}
      for ((cpu = start; cpu <= end; ++cpu)); do printf '%s\n' "$cpu"; done
    else
      printf '%s\n' "$item"
    fi
  done
}

compress_cpus() {
  local -a values=("$@")
  local result='' first=${values[0]} previous=${values[0]} value
  for value in "${values[@]:1}"; do
    if ((value == previous + 1)); then
      previous=$value
      continue
    fi
    [[ -n $result ]] && result+=','
    if ((first == previous)); then result+=$first; else result+="$first-$previous"; fi
    first=$value
    previous=$value
  done
  [[ -n $result ]] && result+=','
  if ((first == previous)); then result+=$first; else result+="$first-$previous"; fi
  printf '%s\n' "$result"
}

allowed_text=$(awk '/^Cpus_allowed_list:/ { print $2 }' /proc/self/status)
[[ -n $allowed_text ]] || { printf '%s\n' 'Cannot read the current CPU allowance' >&2; exit 1; }
mapfile -t allowed_cpus < <(expand_cpu_list "$allowed_text")
declare -A allowed=()
for cpu in "${allowed_cpus[@]}"; do allowed[$cpu]=1; done

# lscpu's parsable format keeps core identity stable across sockets. When a
# NIC has a NUMA node, restrict workers to it to avoid remote NIC access.
declare -A core_cpus=()
declare -A core_order=()
while IFS=',' read -r cpu core socket node online; do
  [[ $cpu =~ ^[0-9]+$ && $online == Y && -n ${allowed[$cpu]:-} ]] || continue
  [[ $nic_numa =~ ^[0-9]+$ && $node != "$nic_numa" ]] && continue
  key="$socket:$core"
  core_cpus[$key]+=" $cpu"
  [[ -n ${core_order[$key]:-} ]] || core_order[$key]=$cpu
done < <(lscpu -p=CPU,CORE,SOCKET,NODE,ONLINE | awk -F, '$1 !~ /^#/ { print }')

mapfile -t cores < <(for key in "${!core_order[@]}"; do
  printf '%s %s\n' "${core_order[$key]}" "$key"
done | sort -n | awk '{ print $2 }')
(( ${#cores[@]} > 0 )) || { printf '%s\n' 'No online CPUs are available to this process' >&2; exit 1; }

if [[ -z $reserve_cores ]]; then
  reserve_cores=$(( (${#cores[@]} + 3) / 4 ))
fi
# A one-core allowance cannot reserve CPU capacity and still serve requests.
if ((reserve_cores >= ${#cores[@]})); then reserve_cores=$((${#cores[@]} - 1)); fi
if ((reserve_cores < 0)); then reserve_cores=0; fi

worker_cpus=()
irq_cpus=()
split=$(( ${#cores[@]} - reserve_cores ))
for index in "${!cores[@]}"; do
  key=${cores[$index]}
  read -r -a siblings <<<"${core_cpus[$key]}"
  if ((index < split)); then worker_cpus+=("${siblings[@]}"); else irq_cpus+=("${siblings[@]}"); fi
done
mapfile -t worker_cpus < <(printf '%s\n' "${worker_cpus[@]}" | sort -n)
mapfile -t irq_cpus < <(printf '%s\n' "${irq_cpus[@]}" | sort -n)

worker_set=$(compress_cpus "${worker_cpus[@]}")
irq_set=''
if ((${#irq_cpus[@]})); then irq_set=$(compress_cpus "${irq_cpus[@]}"); fi
thread_count=${#worker_cpus[@]}
mem_available_kib=$(awk '/^MemAvailable:/ { print $2 }' /proc/meminfo)
# Reserve at most 25% of currently available memory for registered buffers,
# capped at the default 256 MiB per worker and floored at 64 MiB.
buffer_mib=$((mem_available_kib / 1024 / 4 / thread_count))
if ((buffer_mib > 256)); then buffer_mib=256; fi
if ((buffer_mib < 64)); then buffer_mib=64; fi

cat <<EOF
# Generated from this process's allowed CPUs: $allowed_text
# Physical cores: ${#cores[@]}; reserved for IRQs and host work: $reserve_cores
# NIC: ${nic:-not specified}; NIC NUMA node: $nic_numa
LAVIK_CPUSET='$worker_set'
LAVIK_THREADS=$thread_count
LAVIK_IRQ_CPUSET='$irq_set'
LAVIK_REGISTERED_BUFFER_MB_PER_WORKER=$buffer_mib
LAVIK_BUSY_POLL_US=20
LAVIK_FLUSH_MAX_MS=100
EOF
