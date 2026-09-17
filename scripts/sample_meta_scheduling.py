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

"""Sample actual Meta thread runtime, runqueue wait, and host pressure."""
import json
import os
import pathlib
import sys
import time

binary = pathlib.Path(sys.argv[1]).resolve()
with open(sys.argv[2], "w", buffering=1) as out:
    while True:
        sample = {"wall_ns": time.time_ns(), "monotonic_ns": time.monotonic_ns(), "tasks": []}
        for entry in os.scandir("/proc"):
            if not entry.name.isdigit():
                continue
            process = pathlib.Path(entry.path)
            try:
                if (process / "exe").resolve() != binary:
                    continue
                for task in (process / "task").iterdir():
                    stat = (task / "stat").read_text().rsplit(")", 1)[1].split()
                    scheduling = [int(n) for n in (task / "schedstat").read_text().split()]
                    sample["tasks"].append({"pid": int(entry.name), "tid": int(task.name),
                        "state": stat[0], "run_ns": scheduling[0],
                        "runqueue_wait_ns": scheduling[1], "timeslices": scheduling[2]})
            except (OSError, ValueError):
                continue
        for name in ("cpu", "io", "memory"):
            sample[name + "_pressure"] = pathlib.Path("/proc/pressure/" + name).read_text()
        out.write(json.dumps(sample) + "\n")
        time.sleep(0.25)
