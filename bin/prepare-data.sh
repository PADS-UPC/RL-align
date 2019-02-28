#! /bin/bash

BINDIR=`dirname $0`
DATADIR=`dirname $BINDIR`/data
JBPT=`dirname $BINDIR`/jbpt
CLASSPATH=$BINDIR:$JBPT/jbpt-bp-0.3.1.jar:$JBPT/jbpt-core-0.3.1.jar:$JBPT/jbpt-petri-0.3.1.jar

####### Extract gold alignments #####

for x in $DATADIR/alignments/ProM/*.csv; do    
    name=`basename $x .csv`
    echo "Extracting gold alignments from $x" >&2
    cat $x | grep instance | awk -F',' '{gsub("\"","",$2); split($2,ids,"|"); gsub("+complete","",$NF); for (i in ids) print ids[i],$NF }' | sort > $DATADIR/alignments/$name.gold    
done

for x in $DATADIR/alignments/ILPSDP/*.csv; do
    name=`basename $x .csv`     
    echo "Extracting gold alignments from $x" >&2
    cat $DATADIR/alignments/ILPSDP/$name.csv | grep L/M | awk -F, '{gsub("\"","",$1); gsub("\\|","",$1); printf "instance_%s ",$1; gsub("\\|$","",$NF); gsub("\\[M\\]","[M-REAL]",$NF); print $NF}' | sort > $DATADIR/alignments/$name.gold
done

####### Fix weird task names in some models #####

for x in $DATADIR/alignments/*.gold; do    
    name=`basename $x .gold`
    cat $DATADIR/unfoldings/$name.bp.pnml | sed 's/+complete//g' > $x.tmp
    mv $x.tmp $DATADIR/unfoldings/$name.bp.pnml
done


####### Compute BPs #####
for x in $DATADIR/alignments/*.gold; do    
    name=`basename $x .gold`
    echo "Computing BPs for $name" >&2
    $BINDIR/dump $DATADIR/unfoldings/$name.bp.pnml unfolding true true | java ComputeBP > $DATADIR/unfoldings/$name.tt.bp &
    $BINDIR/dump $DATADIR/unfoldings/$name.bp.pnml unfolding true false | java ComputeBP > $DATADIR/unfoldings/$name.tf.bp &

    # Alternativament usar el programa del JosepS (hauria de donar els mateixos BP)
    #java -jar $BINDIR/compute-bp.jar $DATADIR/unfoldings/$name.bp.pnml false true true | sort > $DATADIR/unfoldings/$name.tt.bp
    #java -jar $BINDIR/compute-bp.jar $DATADIR/unfoldings/$name.bp.pnml false true false | sort > $DATADIR/unfoldings/$name.tf.bp
done
wait

####### Compute paths and distances #####

for x in $DATADIR/alignments/*.gold; do    
    name=`basename $x .gold`
    echo "Computing paths for $name" >&2
    $BINDIR/paths $DATADIR/unfoldings/$name.bp.pnml unfolding true true > $DATADIR/unfoldings/$name.tt.path &
done
wait
