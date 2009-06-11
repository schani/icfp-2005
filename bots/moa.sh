#!/bin/bash
# constricta!
#
# identify 

logging() {
echo $* >> /tmp/$$.log
}

getworld() {

awk '{if ( $1 == "wor/") { print "wor/" ; exit 1 } else { print $0 } }' > /tmp/world.$$.$ITERATION

if [ $? -eq 0 ]
then
	logging it s the end of the world as we know it
	exit 0
fi

#rm -f /tmp/world.$$.$ITERATION
#echo ITERATION: $ITERATION > /tmp/world.$$.$ITERATION

#WOR=0

#while [ $WOR -eq 0 ]
#do
#	read blub
#	echo $blub | sed "s/worwor/wor/;s/bvbv/bv/;s/plpl/pl/" | grep -v "^wor/" 2>/dev/null >> /tmp/world.$$.$ITERATION
#	WOR=$?
#done

logging reading done for $ITERATION

bankdistrict

}

bankdistrict() {

rm -f /tmp/world.$$.$ITERATION.banks

for BANKPAIR in `cat /tmp/world.$$.$ITERATION | grep "^bv:" | awk '{print $2"_"$3}'`
do
BANK=`echo $BANKPAIR | awk -F_ '{print $1}'` 
BANKVAL=`echo $BANKPAIR | awk -F_ '{print $2}'` 
grep $BANK"$" /tmp/edge_foot.$$ | awk -v val=$BANKVAL '{printf "bv: "$1" %d\n",val*0.75}' >> /tmp/world.$$.$ITERATION.banks
done

}

copland() {

rm -f /tmp/world.$$.$ITERATION.copland

for COP in `cat /tmp/world.$$.$ITERATION | grep "^pl:" | grep "foot$" | awk '{print $3}'` 
do
grep "^"$COP /tmp/edge_foot.$$ | grep -v $MYPOS"$" | awk '{print "pl: footcop "$2" cop-foot"}' >> /tmp/world.$$.$ITERATION.copland
done

for COP in `cat /tmp/world.$$.$ITERATION | grep "^pl:" | grep "car$" | awk '{print $3}'`
do
cat /tmp/edge_car.$$ /tmp/edge_unifoot.$$ | grep "^"$COP| grep -v $MYPOS"$" | awk '{print "pl: carcop "$2" cop-car"}' >> /tmp/world.$$.$ITERATION.copland
done

}

getskel() {

rm -f /tmp/skel.$$

awk '{if ( $1 == "wor/") { print "wor/" ; exit 0 } else { print $0 } }' > /tmp/skel.$$.all

alllen=`wc -l /tmp/skel.$$.all | awk '{print $1}'`
skellen=`grep -n 'wsk/' /tmp/skel.$$.all | awk -F: '{print $1}'`
((difflen=alllen-skellen))

tail -$difflen /tmp/skel.$$.all > /tmp/world.$$.0
head -$alllen /tmp/skel.$$.all > /tmp/skel.$$

#WSK=0
#while [ $WSK -eq 0 ]
#do
#	read blub
#	echo $blub | sed "s/wskwsk/wsk/;s/nodnod/nod/;s/edgedg/edg/" | grep -v "^wsk/" 2>/dev/null >> /tmp/skel.$$
#	WSK=$?
#done

cat /tmp/skel.$$ | grep "^edg: " | grep -v "car$" | awk '{print $2" "$3}' > /tmp/edge_unifoot.$$
cat /tmp/skel.$$ | grep "^edg: " | grep -v "car$" | ( awk '{print $3" "$2}' ; cat /tmp/edge_unifoot.$$ ) | sort | uniq  > /tmp/edge_foot.$$
cat /tmp/skel.$$ | grep "^edg: " | grep "car$" | awk '{print $2" "$3}' > /tmp/edge_car.$$
cat /tmp/skel.$$ | grep "^edg: " > /tmp/edge_all.$$
grep "^nod: " /tmp/skel.$$ | grep " bank " > /tmp/nod_bank.$$

MYSELF=`cat /tmp/skel.$$ | grep "^name:" | awk '{print $2}'`

bankdistrict

touch /tmp/world.$$.0.copland

}


myrandom() {
	((RND=RANDOM%10))
}

MYSELF=MOA

echo "reg: "$MYSELF" robber"

# read world blabla

ITERATION=0

getskel

logging booooooring, read world

# where am I
MYPOS=`cat /tmp/world.$$.$ITERATION | grep "^pl: "$MYSELF" " | awk '{print $3}'`
LASTPOS=$MYPOS
LAST2POS=$MYPOS
LAST3POS=$MYPOS

while true
do

logging here I stand $MYPOS at $ITERATION

RUNRUN=`cat /tmp/world.$$.$ITERATION | grep "^bv: "$MYPOS | awk '{print $3}'`

DESTS[0]=$MYPOS
if [ "x$RUNRUN" != "x0" ]
then
	myrandom
	WEIGHT[0]=$RND
else	
	logging just robbed this place
	WEIGHT[0]=-950
fi

i=1
for WHERE_TO_GO in `cat /tmp/edge_foot.$$ | grep "^"$MYPOS" " | tac | awk '{print $2}'` 
do

#echo $WHERE_TO_GO
DESTS[$i]=$WHERE_TO_GO
WEIGHT[$i]=-10
HUNTED=0
BANKPOS=0
logging look around around ${DESTS[$i]} ${WEIGHT[$i]}

echo $MYPOS | egrep "culdesac|and\-lake\-shore|loop" > /dev/null
if [ $? -eq 0 ]
then
	logging i dont like culdesacs
        ((WEIGHT[$i]=WEIGHT[$i]-700))
fi

# avoid running into a cop 
cat /tmp/world.$$.$ITERATION /tmp/world.$$.$ITERATION.copland | grep " "$WHERE_TO_GO" " | grep "^pl:" | grep " cop" >/dev/null
if [ $? -eq 0 ]
then
	WEIGHT[$i]=-1000
	logging o mamamia, a cop at $WHERE_TO_GO let me go let me go
	WEIGHT[0]=-900
	HUNTED=1
else
	cat /tmp/world.$$.$ITERATION /tmp/world.$$.$ITERATION.banks | grep "^bv:" | grep " "$WHERE_TO_GO" " > /dev/null
	if [ $? -eq 0 ]
	then
		WEIGHT[$i]=`cat /tmp/world.$$.$ITERATION /tmp/world.$$.$ITERATION.banks | grep "^bv:" | grep " "$WHERE_TO_GO" " | awk '{print $3}' 2>/dev/null`
		logging this bank is my bank at $WHERE_TO_GO with ${WEIGHT[$i]} cash
		BANKPOS=$i
	else
		echo $WHERE_TO_GO | egrep "culdesac|and\-lake\-shore|loop" > /dev/null
		if [ $? -eq 0 ]
		then	
			logging i dont like culdesacs
			((WEIGHT[$i]=WEIGHT[$i]-700))
		else
			echo $WHERE_TO_GO | egrep "park" > /dev/null
			if [ $? -eq 0 ]
	                then
       		                logging i dont like parks
                        	WEIGHT[$i]=-600
			else
				if echo $WHERE_TO_GO | egrep "park|museum\-and\-columbia|56\-and\-everett " > /dev/null
				then
                                	logging i dont like parks
                                	WEIGHT[$i]=-650
				fi
			fi
		fi
	fi
fi

myrandom
((WEIGHT[$i]=WEIGHT[$i]+$RND))


# avoid running close to a cop
cat /tmp/world.$$.$ITERATION.copland | grep " "$WHERE_TO_GO" " | grep "^pl:" | grep " cop" >/dev/null
if [ $? -eq 0 ]
then
        ((WEIGHT[$i]=WEIGHT[$i]-850))
        logging this place $WHERE_TO_GO smells like doughnuts
        ((WEIGHT[0]=WEIGHT[0]-800))
	HUNTED=1
fi

if [ ${DESTS[$i]} == $LASTPOS ]
then
        logging camping is fun 
        ((WEIGHT[$i]=WEIGHT[$i]-40))
fi

if [ ${DESTS[$i]} == $LAST2POS ]
then
	logging campers always return to the camp scene
	((WEIGHT[$i]=WEIGHT[$i]-30))
fi

if [ ${DESTS[$i]} == $LAST3POS ]
then
	logging camping is not for kids
        ((WEIGHT[$i]=WEIGHT[$i]-20))
fi

logging now look around around ${DESTS[$i]} ${WEIGHT[$i]}

((i=i+1))

done

if [ $HUNTED -eq 1 ]
then
	if [ ${DESTS[$BANKPOS]} == "58-culdesac" ]
	then
		((WEIGHT[$BANKPOS]=WEIGHT[$BANKPOS]-900))
	fi
fi

# find best

CHOICE=0
j=0
while [ $j -lt $i ]
do
	if [ ${WEIGHT[$j]} -gt ${WEIGHT[$CHOICE]} ]
	then
		CHOICE=$j
	fi
	((j=j+1))
done

logging mov: ${DESTS[$CHOICE]} robber
echo mov: ${DESTS[$CHOICE]} robber

((ITERATION=ITERATION+2))

LAST3POS=$LAST2POS
LAST2POS=$LASTPOS
LASTPOS=$MYPOS
MYPOS=${DESTS[$CHOICE]}

getworld

copland

# global loop
done
