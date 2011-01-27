#31800
caliantha receive~
0 j 100
~
if (%object.vnum% == 13142)
 wait 3 s
 nod
wait 4 s
say I have been awaiting your arrival, %actor.name%.
wait 4 s
say Bless your perserverance - take this as reward.
wait 2 s
mload obj 31874
give sphere %actor.name%
give 2500 coin %actor.name%
wait 4 s
bow %actor.name%
wait 3 s
junk rod
wait 3 s
mecho You barely have time to blink before Caliantha vanishes - without a trace.
mpurge me
else
return
~
$~
