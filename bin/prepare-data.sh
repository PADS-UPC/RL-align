#! /bin/bash

# Creates unfoldings, paths, and behavioral profiles needed by relaxation labeling aligner
#
#  Usage :  ./prepare-data.sh  2> err.log
#
#    (we recommend to redirect stderr since punf is a bit verbose)


BINDIR=`dirname $0`
ROOTDIR=`dirname $BINDIR`

ALLMODELS="$ROOTDIR/data/originals/*/*.pnml"

rm -f prepare-data.times

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

for MODEL in $ALLUNFOLDINGS; do
    echo "PROCESSING $MODEL"
    name=`basename $MODEL .bp.pnml`

    ### compute shortest paths for both unfoldings (normal and reconnected)
    echo "      PATHS"
    /usr/bin/time -f '%U' -o $name.path1 $BINDIR/paths $MODEL unfolding true true > $ROOTDIR/data/unfoldings/$name.tt.path &    
    /usr/bin/time -f '%U' -o $name.path2 $BINDIR/paths $MODEL unfolding true false > $ROOTDIR/data/unfoldings/$name.tf.path &

    wait     # wait for paths to end, since they are needed by BPs below
    sleep 1

    ### compute behavioural profiles for both unfoldings (normal and reconnected)
    echo "      BPs"
    /usr/bin/time -f '%U' -o $name.bp1 $BINDIR/compute-bps $MODEL unfolding true true > $ROOTDIR/data/unfoldings/$name.tt.bp &
    /usr/bin/time -f '%U' -o $name.bp2 $BINDIR/compute-bps $MODEL unfolding true false > $ROOTDIR/data/unfoldings/$name.tf.bp &

    wait     # wait for paths to end, since they are needed by BPs below
    sleep 1

    rm -f $name.time
    cat $name.path1 $name.path2 | awk '{s+=$1} END {print "PATHS",s}' >> $name.time
    cat $name.bp1 $name.bp2 | awk '{s+=$1} END {print "BPS",s}' >> $name.time
    echo $name | cat - $name.time | awk 'NR>1 {printf(" ");} {printf("%s",$0);} END {printf("\n");}' >> prepare-data.times
    rm $name.path[12] $name.bp[12] $name.time
done


