#! /bin/bash

# Creates unfoldings, paths, and behavioral profiles needed by relaxation labeling aligner
#
#  Usage :  ./prepare-data.sh  2> err.log
#
#    (we recommend to redirect stderr since punf is a bit verbose)


BINDIR=`dirname $0`
ROOTDIR=`dirname $BINDIR`

ALLMODELS="$ROOTDIR/data/originals/*/*.pnml"

### Launch processes to extract unfoldings for all models
mkdir -p $ROOTDIR/data/unfoldings
rm -f $ROOTDIR/data/unfoldings/*
for MODEL in $ALLMODELS; do
    echo "PROCESSING UNFOLDING FOR $MODEL"
    name=`basename $MODEL .pnml`
    $BINDIR/punf -f=$MODEL -m=$ROOTDIR/data/unfoldings/$name.bp.pnml &
done
wait     # wait for unfoldings to end, since they are needed below
sleep 1

# Fix weird task names in some models
for m in $ROOTDIR/data/unfoldings/*.bp.pnml; do
    cat $m | sed 's/+complete//g' > $m.tmp
    mv $m.tmp $m
done

ALLUNFOLDINGS="$ROOTDIR/data/unfoldings/*.bp.pnml"

### Launch processes to compute shortest paths for all unfoldings (normal and reconnected)
for MODEL in $ALLUNFOLDINGS; do
    echo "PROCESSING PATHS FOR $MODEL"
    name=`basename $MODEL .bp.pnml`
    $BINDIR/paths $MODEL unfolding true true > $ROOTDIR/data/unfoldings/$name.tt.path &    
    $BINDIR/paths $MODEL unfolding true false > $ROOTDIR/data/unfoldings/$name.tf.path &
done
wait     # wait for paths to end, since they are needed below
sleep 1

### Launch processes to compute behavioral profiles for all unfoldings (normal and reconnected)
for MODEL in $ALLUNFOLDINGS; do
    echo "PROCESSING BPs FOR $MODEL"
    name=`basename $MODEL .bp.pnml`
    $BINDIR/compute-bps $MODEL unfolding true true > $ROOTDIR/data/unfoldings/$name.tt.bp &
    $BINDIR/compute-bps $MODEL unfolding true false > $ROOTDIR/data/unfoldings/$name.tf.bp &
done
wait   # wait for everything to end before closing

