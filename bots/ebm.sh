#!/bin/bash
# he was a cop. a damn good one
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
#	echo $blub | sed "s/worwor/wor/;s/bvbv/bv/;s/plpl/pl/;s/evev/ev/" | grep -v "^wor/" 2>/dev/null >> /tmp/world.$$.$ITERATION
#	WOR=$?
#done

logging reading world done for $ITERATION

}

getinfo() {

INFOTYPE=$1

awk '{if ( $1 == "from/") { print "from/" ; exit 1 } else { print $0 } }' > /tmp/$INFOTYPE.$$.$ITERATION

if [ $? -eq 0 ]
then
        logging it s the end of the world as we know it
        exit 0
fi

#rm -f /tmp/$INFOTYPE.$$.$ITERATION
#echo ITERATION: $ITERATION > /tmp/$INFOTYPE.$$.$ITERATION

#INF=0

#while [ $INF -eq 0 ]
#do
#        read blub
#        echo $blub | sed "s/fromfrom/from/;s/infinf/inf/;s/planplan/plan/" | grep -v "^from/" 2>/dev/null >> /tmp/$INFOTYPE.$$.$ITERATION
#        INF=$?
#done

logging reading $INFOTYPE done for $ITERATION

}


getskel() {

awk '{if ( $1 == "wor/") { print "wor/" ; exit 0 } else { print $0 } }' > /tmp/skel.$$.all

alllen=`wc -l /tmp/skel.$$.all | awk '{print $1}'`
skellen=`grep -n 'wsk/' /tmp/skel.$$.all | awk -F: '{print $1}'`
((difflen=alllen-skellen))

tail -$difflen /tmp/skel.$$.all > /tmp/world.$$.$ITERATION
head -$alllen /tmp/skel.$$.all > /tmp/skel.$$


#rm -f /tmp/skel.$$

#WSK=0
#while [ $WSK -eq 0 ]
#do
#	read blub
#	echo $blub | sed "s/wskname/name/;s/nodnod/nod/;s/edgedg/edg/" | grep -v "^wsk/" 2>/dev/null >> /tmp/skel.$$
#	WSK=$?
#done

(cat /tmp/skel.$$ | grep "^edg: " | grep -v "car$" | awk '{print $2" "$3}' > /tmp/edge_unifoot.$$
cat /tmp/skel.$$ /tmp/edge_unifoot.$$ | grep "^edg: " | grep -v "car$" | awk '{print $3" "$2}' ) | sort | uniq  > /tmp/edge_foot.$$
cat /tmp/skel.$$ | grep "^edg: " | grep "car$" | awk '{print $2" "$3}' > /tmp/edge_car.$$
cat /tmp/skel.$$ | grep "^edg: " > /tmp/edge_all.$$
grep "^nod: " /tmp/skel.$$ | grep " bank " > /tmp/nod_bank.$$

MYSELF=`cat /tmp/skel.$$ | grep "^name:" | awk '{print $2}'`

}

myrandom() {
	((RND=RANDOM%10))
}

otherplans_car() {

for othercop_pair in `cat /tmp/world.$$.$ITERATION | grep "^pl: " | grep -v " "$MYSELF" " | grep "cop-car$" | awk '{print $2"_"$3}'`
do

COPNAME=`echo $othercop_pair | awk -F_ '{print $1}'`
COPPOS=`echo $othercop_pair | awk -F_ '{print $2}'`

logging COP-CAR: $COPNAME $COPPOS

myrandom
COPDESTS[0]=$COPPOS
COPWEIGHT[0]=$RND

if [ $RND -lt 7 ]
then
	EDGES="/tmp/edge_unifoot.$$  /tmp/edge_car.$$"
else
	EDGES="/tmp/edge_unifoot.$$"
fi

q=1
for COP_WHERE_TO_GO in `cat $EDGES | grep "^"$COPPOS" " | awk '{print $2}'`
do

#echo $WHERE_TO_GO
COPDESTS[$q]=$COP_WHERE_TO_GO
        myrandom
        COPWEIGHT[$q]=$RND

logging COP-CAR: look around around ${COPDESTS[$q]} ${COPWEIGHT[$q]}

# avoid running into a cop
grep " "$COP_WHERE_TO_GO" " /tmp/world.$$.$ITERATION | grep "^pl:" | grep " "cop >/dev/null
if [ $? -eq 0 ]
then
        COPWEIGHT[$q]=-1000
        logging allready a cop on $COP_WHERE_TO_GO
        WEIGHT[0]=-1000
else
	logging grep "^nod: "$COP_WHERE_TO_GO" " /tmp/nod_bank.$$ > /dev/null
	grep "^nod: "$COP_WHERE_TO_GO" " /tmp/nod_bank.$$ > /dev/null
	if [ $? -eq 0 ]
	then
		((COPWEIGHT[$q]=COPWEIGHT[$q]+7))
		logging logging COP-CAR: BANK at ${COPDESTS[$q]} ${COPWEIGHT[$q]}
	fi	
fi

logging COP-CAR: now look around around ${COPDESTS[$q]} ${COPWEIGHT[$q]}

((q=q+1))

done

# find best

COPCHOICE=0
j=0
while [ $j -lt $q ]
do
        if [ ${COPWEIGHT[$j]} -gt ${COPWEIGHT[$COPCHOICE]} ]
        then
                COPCHOICE=$j
        fi
#	logging "COP-CAR: CHOICE: $COPNAME ${COPDESTS[$j]} ${COPWEIGHT[$COPCHOICE]}"
        ((j=j+1))
done

logging "COP-CAR: PLAN: $COPNAME ${COPDESTS[$COPCHOICE]} cop-car $NEXTROUND"
echo "plan: $COPNAME ${COPDESTS[$COPCHOICE]} cop-car $NEXTROUND"

done #coppair

}

otherplans_foot() {

for othercop_pair in `cat /tmp/world.$$.$ITERATION | grep "^pl: " | grep -v " "$MYSELF" " | grep "cop-foot$" | awk '{print $2"_"$3}'`
do

COPNAME=`echo $othercop_pair | awk -F_ '{print $1}'`
COPPOS=`echo $othercop_pair | awk -F_ '{print $2}'`

logging COP-FOOT: $COPNAME $COPPOS

myrandom
COPDESTS[0]=$COPPOS
COPWEIGHT[0]=$RND

q=1
for COP_WHERE_TO_GO in `cat /tmp/edge_foot.$$ | grep "^"$COPPOS" " | awk '{print $2}'`
do

#echo $WHERE_TO_GO
COPDESTS[$q]=$COP_WHERE_TO_GO
        myrandom
        COPWEIGHT[$q]=$RND

logging COP-FOOT: look around around ${COPDESTS[$q]} ${COPWEIGHT[$q]}

# avoid running into a cop
grep " "$COP_WHERE_TO_GO" " /tmp/world.$$.$ITERATION | grep "^pl:" | grep " "cop >/dev/null
if [ $? -eq 0 ]
then
        COPWEIGHT[$q]=-1000
        logging allready a cop on $COP_WHERE_TO_GO
        WEIGHT[0]=-1000
else
        logging grep "^nod: "$COP_WHERE_TO_GO" " /tmp/nod_bank.$$ > /dev/null
        grep "^nod: "$COP_WHERE_TO_GO" " /tmp/nod_bank.$$ > /dev/null
        if [ $? -eq 0 ]
        then
                ((COPWEIGHT[$q]=COPWEIGHT[$q]+3))
                logging logging COP-FOOT: BANK at ${COPDESTS[$q]} ${COPWEIGHT[$q]}
        fi
fi

logging COP-FOOT: now look around around ${COPDESTS[$q]} ${COPWEIGHT[$q]}

((q=q+1))

done

# find best

COPCHOICE=0
j=0
while [ $j -lt $q ]
do
        if [ ${COPWEIGHT[$j]} -gt ${COPWEIGHT[$COPCHOICE]} ]
        then
                COPCHOICE=$j
        fi
        ((j=j+1))
done

logging "COP-FOOT: plan: $COPNAME ${COPDESTS[$COPCHOICE]} cop-foot $NEXTROUND"
echo "plan: $COPNAME ${COPDESTS[$COPCHOICE]} cop-foot $NEXTROUND"

done #coppair

}

MYSELF=$1

logging "reg: "$MYSELF" cop-foot"
echo "reg: "$MYSELF" cop-foot"

# read world blabla

ITERATION=0

getskel

logging booooooring, read world

logging there can be only one $MYSELF

while true
do

# where am I
MYPOS=`cat /tmp/world.$$.$ITERATION | grep "^pl: "$MYSELF" " | awk '{print $3}'` 

logging here I stand $MYPOS at $ITERATION

myrandom
DESTS[0]=$MYPOS
WEIGHT[0]=$RND

i=1
for WHERE_TO_GO in `cat /tmp/edge_foot.$$ | grep "^"$MYPOS" " | awk '{print $2}'` 
do

#echo $WHERE_TO_GO
DESTS[$i]=$WHERE_TO_GO

logging look around around ${DESTS[$i]} ${WEIGHT[$i]}

# avoid running into a cop 
grep " "$WHERE_TO_GO" " /tmp/world.$$.$ITERATION | grep "^pl:" | grep cop >/dev/null
if [ $? -eq 0 ]
then
	WEIGHT[$i]=-1000
	logging allready a cop on $WHERE_TO_GO 
	WEIGHT[0]=-1000
else
	myrandom
	WEIGHT[$i]=$RND
fi

logging now look around around ${DESTS[$i]} ${WEIGHT[$i]}

((i=i+1))

done

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

echo "inf\\"
echo "inf/"

getinfo INFO

((NEXTROUND=ITERATION+2))
logging myplan: "plan: $MYSELF ${DESTS[$CHOICE]} cop-foot $NEXTROUND"

echo "plan\\"

otherplans_foot
otherplans_car

#echo "plan: $MYSELF ${DESTS[$CHOICE]} cop-foot $NEXTROUND"
echo "plan/"

getinfo PLAN

logging "vote: $MYSELF"

echo "vote\\"
echo "vote: $MYSELF"
cat /tmp/world.$$.$ITERATION | grep "^pl:" | grep -v "robber$" | grep -v "MYSELF" | awk '{print "vote: "$2}'
echo "vote/"

read WINNER[$ITERATION]
logging and the winner is ${WINNER[$ITERATION]}

logging mov: ${DESTS[$CHOICE]} cop-foot
echo mov: ${DESTS[$CHOICE]} cop-foot

((ITERATION=ITERATION+2))

getworld

# global loop
done
