#! /bin/bash

BINDIR=`dirname $0`
DATADIR=`dirname $BINDIR`/data
JBPT=`dirname $BINDIR`/jbpt
export CLASSPATH=$BINDIR:$JBPT/jbpt-bp-0.3.1.jar:$JBPT/jbpt-core-0.3.1.jar:$JBPT/jbpt-petri-0.3.1.jar

####### Compute BPs #####
for x in $DATADIR/unfoldings/*.bp.pnml; do    
    name=`basename $x .bp.pnml`
    echo "Computing BPs for $name" >&2
    $BINDIR/dump $x unfolding true true | java ComputeBP > $DATADIR/unfoldings/$name.tt.bp &
    $BINDIR/dump $x unfolding true false | java ComputeBP > $DATADIR/unfoldings/$name.tf.bp &
done
wait

####### Compute paths and distances #####

for x in $DATADIR/unfoldings/*.bp.pnml; do    
    name=`basename $x .bp.pnml`
    echo "Computing paths for $name" >&2
    $BINDIR/paths $x unfolding true true > $DATADIR/unfoldings/$name.tt.path &
done
wait
