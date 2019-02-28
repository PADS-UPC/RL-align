#$ -S /bin/bash

# Run a particular RL configuration on the test set

# Usage:
#     ./test.sh date configuration
#
# E.g.:   ./test.sh 2019-02-01  60.60.-50.-20.-100


DATE=$1
PARAMS=$2

order=`echo $PARAMS | cut -d. -f1`
paral=`echo $PARAMS | cut -d. -f2`
cross=`echo $PARAMS | cut -d. -f3`
dummy=`echo $PARAMS | cut -d. -f4`
excl=`echo $PARAMS | cut -d. -f5`

cd $(dirname $0)
rm -f data/results/test/stats.$DATE.$order.$paral.$cross.$dummy.$excl

TRAIN="data/alignments/M*[13579].gold data/alignments/pr[ACEG]*.gold"
TEST="data/alignments/M*[02468].gold data/alignments/pr[BDF]*.gold"

# create configuration file
cat config/config-base \
    | sed 's/DummyCompatibility/DummyCompatibility '$dummy'/' \
    | sed 's/ExclusiveCompatibility/ExclusiveCompatibility '$excl'/' \
    | sed 's/CrossCompatibility/CrossCompatibility '$cross'/' \
    | sed 's/OrderCompatibility/OrderCompatibility '$order'/' \
    | sed 's/ParallelCompatibility/ParallelCompatibility '$paral'/' \
          > config/config.$order.$paral.$cross.$dummy.$excl.cfg

# execute on test set
bin/execute.sh config/config.$order.$paral.$cross.$dummy.$excl.cfg test "$TEST"
echo -e "\n=========== "$order.$paral.$cross.$dummy.$excl >> data/results/test/stats.$DATE.$order.$paral.$cross.$dummy.$excl

# evaluate results
bin/eval.py data/alignments data/results/output-test.$order.$paral.$cross.$dummy.$excl >> data/results/test/stats.$DATE.$order.$paral.$cross.$dummy.$excl

rm -rf config/config.$order.$paral.$cross.$dummy.$excl.cfg
