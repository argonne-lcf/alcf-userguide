#!/usr/bin/env python3

import sys
import re

with open(sys.argv[1]) as f:
    data=f.readlines()

def str_unit(value, unit):
    if value >= 1000:
        return f"{value/1000:.0f} {unit.replace('G','T')}"
    else:
        return f"{value:.0f} {unit}"

def finds(target):
    for line in data:
        if line.startswith(target):
            *_, value, unit = line.split()
            yield (float(value), unit)

def yield_row(target):
    for target in target:
        try:
            tile, *_, node = list(finds(target))
        except ValueError:
            continue
        scaling = node[0] / tile[0]
        tile, node = [str_unit(v,u) for v,u in [tile,node]]
        yield(f"| {target} | {tile} | {node} | {scaling:.1f} |")


if sys.argv[2] == "micro":
    targets = ["Single Precision Peak Flops", "Double Precision Peak Flops",
               "Memory Bandwidth (triad)",
               "PCIe Unidirectional Bandwidth (H2D)", "PCIe Unidirectional Bandwidth (D2H)", 
               "PCIe Bidirectional Bandwidth",
               "Tile2Tile Unidirectional Bandwidth", "Tile2Tile Bidirectional Bandwidth",
               "GPU2GPU Unidirectional Bandwidth", "GPU2GPU Bidirectional Bandwidth"]

elif sys.argv[2] == "GEMM":
    targets = ["DGEMM","SGEMM","HGEMM","BF16GEMM","TF32GEMM","I8GEMM"]
elif sys.argv[2] == "FFT":
    targets = ["Single-precision FFT C2C 1D", "Single-precision FFT C2C 2D"]

print ("| | One Tile | Full Node | Scaling |")
print ("|---|-----------:|-----------:|----:|")
for row in yield_row(targets):
    print (row)


