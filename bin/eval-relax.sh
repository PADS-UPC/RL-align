#! /bin/bash

### Evaluates fitness and CPU time on the results of an alingment performed by relaxation labelling (bin/align program)

## Usage:  ./eval-relax.sh < output-file
##         cat output-file | ./eval-relax.sh
##         cat output-file1 output-file2 .. output-fileN | ./eval-relax.sh

awk '{gsub("\\[M-REAL\\]tau","[M-INVI]tau",$2); x = gsub("\\[M-REAL\\]","[X]",$2) + gsub("\\[L\\]","[X]",$2); cost = cost+x; time = time+$4} END {print "av.cost =",cost/NR,"("cost"/"NR")"; print "total time =", time} '
