#!/bin/bash -x

./run.sh |& tee log.txt
./gen_mk_table.py log.txt micro | column -t -s "|" -o "|" -R 0
./gen_mk_table.py log.txt GEMM | column -t -s "|" -o "|" -R 0
./gen_mk_table.py log.txt FFT | column -t -s "|" -o "|" -R 0

