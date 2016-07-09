#!/bin/bash

# Computes the "BogoLocs" index for CPUs enumerated in file "graph-2ptr.txt".
# It takes the 8k, 128k, 16M transaction rates for transactions the size of
# two pointers, and reports the performance index for this CPU. The program
# assumes an L1 cache hit rate of 96% and L2 hit rate of 92%. The figures are
# roughly representative of experimental observations. The output index is
# reasonably close to the number of lines of code compiled per second in the
# gcc performance test, hence the name of the index "BogoLocs". As its name
# implies, this index is bogus. But it just gives an idea of what a device
# could be capable of, even if this idea is flawed.

names=( $(head -n 1 data/graph-2ptr.txt) )
l1=( $(grep ^8k data/graph-2ptr.txt) )
l2=( $(grep ^128k data/graph-2ptr.txt) )
l4=( $(grep ^16M data/graph-2ptr.txt) )

# estimated hit ratio (percent) per cache level for a single core
l1hitbase=96
l2hitbase=92

# show title line
cpu=1
echo -n "Cores"
while [ -n "${names[$cpu]}" ]; do
	echo -n " ${names[$cpu]}"
	((cpu++))
done
echo

# and theorical speed per number of cores
for cores in 1 2 4 8; do
	cpu=1
	echo -n "$cores"
	while [ -n "${l1[$cpu]}" ]; do
		# Note that in theory L2's hit ratio decreases with the number
		# of cores, as the part of its size assigned to each core
		# shrinks. But we also have to take into account the cache
		# hit frequency which is much higher on small sizes than on
		# large ones, so the hit ratio is not simply divided by the
		# number of cores.
		l1hit=$l1hitbase
		l2hit=$l2hitbase
		l1miss=$((100 - l1hit))
		l2miss=$((100 - l2hit))

		l2hit=$((100 - l2miss * (cores + 3) / 4))
		l2miss=$((100 - l2hit))

		# OK first, we have to assign correct limits :
		# - L2 transaction rate cannot be higher than RAM transaction
		#   rate times L2's miss rate.
		# - L1 transaction rate cannot be higher than L2's transaction
		#   rate times L1's miss rate divided by the number of cores.
		l1rate=${l1[$cpu]}
		l2rate=${l2[$cpu]}
		l4rate=${l4[$cpu]}

		if [ $((l2rate *  l2miss / 100)) -gt $l4rate ]; then
			#echo -n "${names[$cpu]}: $cores cores l2rate=$l2rate => " >&2
			l2rate=$((l4rate * 100 / l2miss))
			#echo "$l2rate" >&2
		fi

		if [ $((l1rate * cores * l1miss / 100)) -gt $l2rate ]; then
			#echo -n "${names[$cpu]}: $cores cores l1rate=$l1rate => " >&2
			l1rate=$((l2rate * 100 / (cores * l1miss)))
			#echo "$l1rate" >&2
		fi

		# avg access time in ps
		l1time=$((1000000 / ${l1rate}))
		l2time=$((1000000 / ${l2rate}))
		l4time=$((1000000 / ${l4rate}))

		avgtime=$((
			 l1hit * l1time / 100 + \
			 l1miss * l2hit * l2time / 10000 + \
			 l1miss * l2miss * l4time / 10000))

		speed=$((7000000 / avgtime * cores))

		# eliminate known non-existing configurations
		case ${names[$cpu]}:$cores in
			Atom-N26*:[48]) speed=0 ;;
			i5-3320*:[48]) speed=0 ;;
			i7-4790*:8) speed=0 ;;
			i7-6700*:8) speed=0 ;;
			OdroidC2-*:8) speed=0 ;;
			Mirabox-*:[248]) speed=0 ;;
			ClearFog-*:[48]) speed=0 ;;
			T034-*:8) speed=0 ;;
		esac
		echo -n " $speed"
		((cpu++))
	done
	echo
done
