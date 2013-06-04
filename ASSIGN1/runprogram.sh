#! /bin/bash

# usage: A1_linux_donut_loop.sh  number_of_runs_integer
# so from the shell prompt:  $ A1_linux_donut_loop.sh 10 
# will run the proddonuts program and the consdonuts
# programs (5 of them) 10 times and report how many
# of the 10 runs ended in deadlock

if [ -z $1 ]
then
echo "USAGE: A1_linux_donut_loop.sh  number_of_runs_integer"
echo "Try the command again"
echo " "
fi

echo " "
echo "The configuration is for $1 LOOPS"
echo " "

rm c1 c2 c3 c4 c5 prod_out.txt 2>&1

local1=$1
lpcnt=1
count=0
while test $local1 -ne 0
do
echo "___________________________________________________________"
echo " "
echo "Working on LOOP $lpcnt"
echo " "
echo "Starting Producer"
./myproducer >>  prod_out.txt & 
sleep 1
./myconsumer 1 >> c1 & ./myconsumer 2 >> c2 & ./myconsumer 3 >> c3 & ./myconsumer 4 >> c4 & ./myconsumer 5 >> c5 &

sleep 5
pid=`ps | grep -v 'grep' | grep 'myproducer' | cut -c1-6`
echo $pid
if ps | grep -v 'grep' | grep -q 'myconsumer'
then
echo " "
echo "DEADLOCK DETECTED ON LOOP $lpcnt"
echo " "
count=`expr $count + 1`
else
echo "LOOP $lpcnt COMPLETED SUCCESSFULLY"
echo " "
fi
echo ">>>>>>>> NOW KILLING THE PRODUCER PROCESS PID: $pid"
kill $pid
local1=`expr $local1 - 1`
lpcnt=`expr $lpcnt + 1`
sleep 5
done
#echo ">>>>>>>> NOW KILLING THE PRODUCER PROCESS PID: $pid"
#kill $pid
echo "___________________________________________________________"
echo " "
echo $1 loops and $count deadlocks
echo " "
echo " "

for varfiles in {c1,c2,c3,c4,c5}
do
	echo ""
	echo "Jelly            Choco            Mint       	Sugar		 (Customer:$varfiles)"
	awk -F "," 'BEGIN{i=j=k=l=0}$0 !~ /^#/{
		if ($1 ~ /0/){a[i]=$2;i++};
		if ($1 ~ /1/){b[j]=$2;j++};
		if ($1 ~ /2/){c[k]=$2;k++};
		if ($1 ~ /3/){d[l]=$2;l++};
		}
	END {
		for(i=0;i<=11;i++){
			a1=(a[i]=="")?0:a[i];
			b1=(b[i]=="")?0:b[i];
			c1=(c[i]=="")?0:c[i];
			d1=(d[i]=="")?0:d[i];
			printf("%d%s%d%s%d%s%d\n",a1,"\t\t ",b1,"\t\t ",c1,"\t\t ",d1); 
		}
	}' $varfiles
done

