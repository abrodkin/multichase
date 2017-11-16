#!/bin/bash
KBHUGE=`cat /proc/meminfo | grep Hugepagesize | sed -e 's/.*: *//' -e 's/ .*//'`
NEED_PAGES=$[8*1024*1024/$KBHUGE]
sudo sysctl vm.nr_hugepages=$NEED_PAGES

mkdir -p base
pushd base
/opt/collect.sh > env.txt
#multichase specific
for CMD in "../multichase" "../multichase -m 4m -s 64" "../multichase -m 1g -n 20" \
	"../multichase -m 256k -s 128 -t 2"
do
	res=`sudo $CMD 2>&1`
	echo "$CMD,$res" >> summary.csv
done
#TLP trasher multicore scan
echo "=========" >> summary.csv
echo "numactl --interleave=all -- multichase -H -t\$t -T128 -m4g" >> summary.csv 
echo "=========" >> summary.csv
for t in {1..16} ; do 
	CMD="numactl --interleave=all -- ../multichase -H -t$t -T128 -m4g"
	res=`sudo $CMD`
	echo "$t,$res" >> summary.csv
done
#others

echo "=========" >> summary.csv
echo "fairness all cpus" >> summary.csv 
echo "=========" >> summary.csv
../fairness | perl -pe 's/ +(\d)/,\1/g' >> summary.csv

echo "=========" >> summary.csv
echo "fairness 16 cpus" >> summary.csv 
echo "=========" >> summary.csv
taskset -c 0-15 ../fairness | perl -pe 's/ +(\d)/,\1/g' >> summary.csv

echo "running fairness sweep"
../fairness -s 32 > fairness.log 2>&1

#done with base test

popd
exit

mkdir -p random_access
pushd random_access
/opt/collect.sh > env.txt
for c in simple work:10 parallel2  ; do 
	echo -n "Starting $c radom"
	date
	CN=`echo "$c" | sed -e 's/:/_/g'` 
	for t in {1..64} ; do 
		stride=$[16*$t]
		res=`../multichase -X -c $c -t $t -s $stride`
		echo "$t,$res" >> $CN.csv
	done
done
popd

mkdir -p ordered_access
pushd ordered_access
/opt/collect.sh > env.txt
for c in simple work:10 parallel8  ; do 
	echo -n "Starting $c ordered"
	date
	CN=`echo "$c" | sed -e 's/:/_/g'` 
	for t in {1..64} ; do 
		stride=$[128*$t]
		res=`../multichase -X -c $c -t $t -o -s $stride`
		echo "$t,$res" >> $CN.csv
	done
done
popd

mkdir -p random_access
pushd random_access
/opt/collect.sh > env.txt
for c in simple incr critword:64 critword:5  ; do 
	echo -n "Starting $c radom"
	date
	CN=`echo "$c" | sed -e 's/:/_/g'` 
	for t in {1..64} ; do 
		stride=$[16*$t]
		res=`../multichase -X -c $c -t $t -s $stride -a`
		echo "$t,$res" >> $CN.csv
	done
done
popd

mkdir -p ordered_access
pushd ordered_access
/opt/collect.sh > env.txt
for c in simple incr critword:64 critword:5 parallel8 ; do 
	echo -n "Starting $c ordered"
	date
	CN=`echo "$c" | sed -e 's/:/_/g'` 
	for t in {1..64} ; do 
		stride=$[128*$t]
		res=`../multichase -X -c $c -t $t -o -s $stride -a`
		echo "$t,$res" >> $CN.csv
	done
done
popd

