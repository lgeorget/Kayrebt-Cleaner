#!/bin/zsh

set -e
set -x

if [[ $# -lt 1 ]]
then
	echo "Usage: $0 <file.dot>"
	exit 1
fi

dotfile=$1
shift

typeset -a transformations
if [[ $# -ge 1 ]]
then
	transformations="$@"
else
	transformations=("taustar" "tauconfluence"  "taudivergence" "strong"\
	                 "safety"  "taucompression" "weaktrace"     "trace")
fi

./cleaner $dotfile
if [[ ${dotfile:e} == "dot" ]]
then
	bcgfile=${dotfile/%dot/bcg}
else
	bcgfile="$dotfile.bcg"
fi

export CADP_CC="/usr/bin/gcc -fno-pie"
for t in $transformations
do
	reducedbcgfile=${bcgfile/%.bcg/_$t.bcg}
	bcg_open $bcgfile reductor -$t $reducedbcgfile
	./grapher $reducedbcgfile
	reduceddotfile=${reducedbcgfile/%bcg/dot}
	dot -Tpng $reduceddotfile > ${reduceddotfile/%dot/png}
done

