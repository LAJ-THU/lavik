#!/usr/bin/env python3
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

"""Temporary runner-side measurement; no artificial I/O delay is injected."""
import json
import pathlib
import re
import shlex
import sys
import time

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "tests/meta_integration"))
import harness as H

binary = pathlib.Path(sys.argv[1]).resolve()
out = pathlib.Path(sys.argv[2]).resolve()
out.mkdir(parents=True, exist_ok=True)
H.set_tag("actual-sync-timing")
wrapper = out / "traced-meta"
wrapper.write_text("#!/bin/bash\n" +
    "trace_prefix=" + shlex.quote(str(out / "sync-trace")) + "\n" +
    'exec strace -D -f --seccomp-bpf -ttt -T -yy -e trace=fsync,fdatasync '
    '-o "$trace_prefix.$$" -- ' + shlex.quote(str(binary)) + ' "$@"\n')
wrapper.chmod(0o755)
results = []
for traced in (False, True):
    for repeat in range(2):
        name = ("traced" if traced else "baseline") + "-" + str(repeat)
        workdir = out / name
        workdir.mkdir(exist_ok=True)
        # Omit all timing flags to exercise the executable's production defaults.
        nodes = H.make_nodes(str(wrapper if traced else binary), str(workdir), 4,
                            args=["--snapshot-distance", "100000", "--reserved-log-items", "0"])
        started = time.monotonic()
        result = {"case": name}
        try:
            leader = H.bootstrap_cluster(nodes[:3])
            history = H.CommittedHistory()
            H.propose_ops(leader, 0, 20, history=history)
            nodes[3].start()
            H.join_and_verify(leader, nodes[3])
            H.propose_ops(leader, 20, 20, history=history)
            history.check(nodes)
            result["status"] = "passed"
        except Exception as error:
            result.update(status="failed", error=str(error))
        finally:
            for node in nodes:
                node.force_kill()
        result["elapsed_seconds"] = round(time.monotonic() - started, 3)
        results.append(result)
        print(json.dumps(result), flush=True)
# Tracees have exited; allow the detached tracers to finish their final writes.
time.sleep(0.2)
rows = []
for path in out.glob("sync-trace.*"):
    for line in path.read_text(errors="replace").splitlines():
        match = re.search(r"\b(fdatasync|fsync)\(.*?\)\s+=.*?<([0-9.]+)>$", line)
        if match:
            rows.append({"call": match.group(1), "milliseconds": float(match.group(2))*1000,
                         "trace": path.name, "line": line})
stats = {}
for call in ("fsync", "fdatasync"):
    values = sorted(row["milliseconds"] for row in rows if row["call"] == call)
    if values:
        stats[call] = {"count": len(values),
            **{f"p{pct}_ms": values[min(len(values)-1, int((len(values)-1)*pct/100))]
               for pct in (50, 95, 99)}, "max_ms": values[-1]}
report = {"cases": results, "sync_stats": stats,
          "slowest": sorted(rows, key=lambda row: row["milliseconds"], reverse=True)[:20]}
(out/"summary.json").write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps(report, indent=2), flush=True)
if not rows:
    raise SystemExit("No sync timing records captured")
