#! /bin/bash



# usage:
#        plot.sh (original|unfolding) filename.pnml

BINDIR=$(dirname $0)

python $BINDIR/draw_PN.py $1 $2

BNAME=$(basename $2 .bp.pnml)
mv plot.png.pdf $BNAME.$1.pdf
rm plot.png Graph_net_moves_color.dot
