#! /bin/bash

### Evaluates fitness and CPU time on the results of an alingment performed by some ProM plugin (A* or RuR)

## Usage:  ./eval-prom.sh < output-file.csv

tee csv.tmp | awk -F',' '/^[0-9]/ {gsub("\"","",$2); gsub(" ","_",$2); split($2,ids,"|"); gsub("\\+complete","",$NF); gsub(" ","_",$NF); for (i in ids) print ids[i],$NF }' | awk '{cost = cost + gsub("\\[M-REAL\\]","[X]",$2) + gsub("\\[L\\]","[X]",$2)} END {print "av.cost =",cost/NR,"("cost"/"NR")"}'

cat csv.tmp | awk -F',' '/^#Reliable cases replayed/ {n=$2} /^Calculation Time/ {t=$2} END {print "total time =", n*t/1000}'
rm csv.tmp
