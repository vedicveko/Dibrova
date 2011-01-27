#16700
guard 1~
0 g 75
~
if (%actor.vnum% == -1)
wait 3 s
emote grunts.
wait 2 s
say You go. No come in.
wait 3 s
say You GO!
wait 1 s
say Or me an' Rooga kill.
wait 2 s
emote puts a thick finger directly on your chest indicating that 'kill' and 'you' are probably one and the same.
wait 6 s
%echo% Take his advice.
~
#16701
entry trg~
0 g 100
~
if (%actor.vnum% == 16701)
mload obj 16700
wait 1 s
nod %actor.name%
wait 2 s
emote pulls a key out from under his grubby jerkin.
wait 2 s
emote fumbles for a moment, but his big fingers finally get the lock.
wait 2 s
unlock door
wait 1 s
open door
wait 10 s
close door
wait 2 s
lock door
wait 1 s
drop key
~
#16702
wolfen trg~
0 f 100
~
mload obj 16701
~
#16703
wolfen trg II~
0 g 70
~
wait 3 s
emote stares your way, his eyes narrowed to slits.
wait 5 s
emote says quietly, /cw'We have an intruder within the walls.'/c0
wait 1 s
grin
wait 2 s
mkill %actor.name%
~
#16704
dorag flee~
0 g 50
~
if (%actor.vnum% == -1)
wait 3 s
emote gives you an unblinking, solid stare.
wait 5 s
emote gets to his feet.
wait 3 s
say You are not one of my men.
wait 2 s
n
unlock secret
d
open secret
d
close secret
lock secret
e
u
s
e
e
s
e
e
e
e
e
e
e
e
e
e
e
n
n
wait 3 s
emote looks right and then left, then up at the sky.
wait 2 s
s
s
s
s
s
s
wait 3 s
emote scratched under one ear, thinking about his next move.
~
#16705
dorag death key~
0 f 100
~
mload obj 16705
~
#16707
hench death trg I~
0 f 30
~
mload obj 16702
~
#16708
hench death trg II~
0 f 30
~
mload obj 16703
~
#16709
ogre death trg~
0 f 65
~
mload obj 16704
~
#16710
rogue trg~
0 g 100
~
if (%actor.vnum% == -1)
wait 2 s
emote looks for a moment at you and gets a worried look.
wait 3 s
e
wait 2 s
n
wait 2 s
n
wait 2 s
n
wait 2 s
e
wait 2 s
e
wait 11 s
e
e
s
emote vanishes without a trace - blending into the compound as if he had never been here.
mpurge me
~
#16750
felz greet~
0 g 75
~
wait 2 s
emote turns to face you squarely.
wait 3 s
say You will tell me your intention for the Kailaani, or suffer the fate of an enemy of the Forest Guardians.
wait 10 s
say You are not a Friend of the Guardians. Die, hostile intruder!
wait 2 s
mkill %actor.name%
~
#16751
forest friend felz~
0 d 1
Forest Friend~
wait 2 s
bow
wait 1 s
say Bless you, Friend. May all Guardians of the forest grant you safe passage always.
d
d
d
d
d
w
~
$~
