#!/usr/bin/env python3
# Copyright (C) 2026 EloqData Inc.
# SPDX-License-Identifier: Apache-2.0
"""Plot the storage-tier report using its published five-minute measurements.

Requires matplotlib. Run from any directory; --preview optionally writes a PNG.
The separate Azure Managed Redis experiment uses a different protocol and is
excluded. Colors and patterns follow the Redis/Valkey comparison charts.
"""

import argparse
from pathlib import Path
import re

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter


REPORT = Path(__file__).resolve().parents[1] / "keylane-vs-dragonfly-tiering-2026-08-11" / "README.md"
WORKLOADS = ("Read-only GET", "1:1 read/write", "Write-only SET")
# Keep the three Lavik backends distinct; do not select a different backend
# per workload. Fields: source label, legend label, color, hatch.
PRODUCTS = (
    ("Lavik SPDK", "Lavik · SPDK", "#1473E6", "///"),
    ("Lavik io_uring (two raw block devices)", "Lavik · raw io_uring", "#438FEC", "///"),
    ("Lavik io_uring (two XFS files)", "Lavik · XFS io_uring", "#8CB9EF", "///"),
    ("Microsoft Garnet Storage Tier", "Garnet Storage Tier", "#C84C8A", "xx"),
    ("Dragonfly Tiered Storage", "Dragonfly Tiered Storage", "#E97827", ".."),
    ("Pika", "Pika", "#26958B", "++"),
    ("Apache Kvrocks", "Apache Kvrocks", "#8263B3", "\\\\"),
    ("Tendis", "Tendis", "#AD8C37", "--"),
    ("KeyDB On Flash", "KeyDB On Flash", "#748294", "oo"),
)


def read_results():
    results = {}
    for line in REPORT.read_text().splitlines():
        cells = [cell.strip() for cell in line.split("|")[1:-1]]
        if len(cells) == 5 and cells[0] in WORKLOADS:
            results[cells[0], cells[1]] = float(cells[2].replace(",", ""))
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preview", type=Path, help="Optional PNG output path")
    args = parser.parse_args()
    results = read_results()
    plt.rcParams.update({
        "font.family": "DejaVu Sans", "font.size": 11,
        "svg.fonttype": "none", "svg.hashsalt": "lavik-tiering-2026-08-12",
        "hatch.linewidth": 0.6,
    })
    fig, ax = plt.subplots(figsize=(16, 8.6))
    fig.patch.set_facecolor("white")
    fig.subplots_adjust(left=0.08, right=0.98, top=0.65, bottom=0.21)
    fig.text(0.044, 0.935, "Lavik versus Redis-compatible storage tiers",
             fontsize=25, weight="bold", color="#17202A")
    fig.text(0.044, 0.89,
             "Azure Standard_L16s_v3 · Two NVMe drives · 200 million keys · 1–4 KB values",
             fontsize=13, color="#566573")
    fig.text(0.044, 0.85,
             "80 connections · 300 seconds per workload · Unlimited request rate · Higher is better",
             fontsize=12, color="#566573")
    bar_width = 0.078
    for index, (source, label, color, hatch) in enumerate(PRODUCTS):
        positions = [group + (index - 4) * bar_width for group in range(3)]
        values = [results[workload, source] for workload in WORKLOADS]
        bars = ax.bar(positions, values, width=bar_width * 0.89, label=label,
                      color=color, hatch=hatch, edgecolor="white", linewidth=0.4)
        ax.bar_label(bars, labels=[f"{v / 1000:.1f}k" for v in values],
                     padding=4, fontsize=8, color="#34495E")
    ax.set_ylim(0, 450000)
    ax.set_yticks(range(0, 400001, 100000))
    ax.yaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value / 1000:g}k"))
    ax.set_xticks(range(3), ("GET · read-only", "GET + SET · 1:1", "SET · write-only"))
    ax.set_ylabel("Operations per second", labelpad=12, weight="bold", color="#17202A")
    ax.grid(axis="y", color="#DDE3E8")
    ax.set_axisbelow(True)
    ax.tick_params(axis="both", length=0, pad=10, labelcolor="#34495E")
    for name, spine in ax.spines.items():
        spine.set_visible(name in ("left", "bottom"))
        spine.set_color("#7B8794")
    handles, labels = ax.get_legend_handles_labels()
    order = (0, 3, 6, 1, 4, 7, 2, 5, 8)
    fig.legend([handles[i] for i in order], [labels[i] for i in order],
               loc="upper center", bbox_to_anchor=(0.54, 0.815), ncol=3,
               frameon=False, fontsize=11, handlelength=2.5, columnspacing=3)
    fig.text(0.044, 0.12, "Lavik benchmark · August 12, 2026 rerun · Zero-baseline QPS axis",
             fontsize=11, color="#566573")
    fig.text(0.044, 0.075,
             "Cache, warmup and persistence settings differ; see the report. Uniform access is unfavorable to KeyDB's hot-tier design.",
             fontsize=10, color="#566573")
    output = Path(__file__).with_name("tiering-throughput.svg")
    fig.savefig(output, metadata={"Date": None, "Title": "Lavik versus Redis-compatible storage tiers"})
    # Normalize path whitespace to keep the generated asset compact and clean
    # under the repository's diff checks, without altering drawing geometry.
    svg = re.sub(r'\bd="([^"]*)"', lambda match: 'd="' + ' '.join(match[1].split()) + '"', output.read_text())
    output.write_text('\n'.join(line.rstrip() for line in svg.splitlines()) + '\n')
    if args.preview:
        fig.savefig(args.preview, dpi=120)
    plt.close(fig)


if __name__ == "__main__":
    main()
