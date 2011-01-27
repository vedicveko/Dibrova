#5800
beggar~
0 b 8
~
say Please, suh - a coin or two for a poor man...?
~
#5801
beggar bribe~
0 m 1
~
wait 2 s
emote looks around and lowers his voice.
wait 3 s
say Go 'n' see lil' Feeliz, he just may have some information you want.
wait 2 s
emote straightens up and begins ignoring you.
wait 5 s
say Alms! Alms for the poor!
~
#5802
feeliz~
0 g 89
~
if (%actor.vnum% == -1)
wait 15 s
wait 2 s
say What the hell 'r you lookin at?!
wait 3 s
say Somethin' on yer mind?
wait 4 s
say Oh, fine. Obviously that damn beggar sent you to find me.
wait 2 s
say Here, take this - and get out of my damn face - you're crampin my style!
wait 3 s
mload obj 5867
give note %actor.name%
else
return
~
#5803
koslo's death cry~
0 f 100
~
say You will not get away with this, %actor.name%!
say I will be avenged!
~
#5804
beggar plead~
0 g 85
~
wait 2 s
say Alms! Alms for the poor!
~
#5805
beggar plead II~
0 b 65
~
say Alms! Alms for the poor!
~
#5806
chancellor greet~
0 g 35
~
wait 1 s 
say Good day to you, traveler.
wait 3 s
look %actor.name%
wait 4 s
ponder
wait 10 s
say I do not think that I have had the pleasure of meeting you yet.
wait 3 s
say Could I have your name for the city registry?
~
#5807
chancellor response~
0 e 1
says~
wait 2 s
nod
wait 3 s
say Thank you for your time, %actor.name%.
wait 2 s
smirk
s
s
e
e
s
s
w
w
n
n
~
$~
