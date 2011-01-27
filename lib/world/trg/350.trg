#35000
Janta~
0 h 100
~
wait 10
bow
wait 1
say My people have been told of your arrival
wait 5
say My chief wishes to speak with you
wait 3
say Go south of the stream he is in his tent awaiting your arrival
~
#35001
Mouse~
0 h 45
~
wait 3
say Can you help me?
wait 3
say I can't seem to find my home, I was once a great warrior now am a small creature
wait 3
say A year ago I angered a sorcerer by killing his child, so he turned me into a mouse....
wait 2
sniff
wait 5
cry
wait 5
mecho The mouse looks to you for any help you might offer.
~
#35002
credit heheh :P~
0 d 100
credit trigger~
say I have been programmed and created by Tindra 
~
#35003
whispering words~
2 h 100
~
wait 3
wecho Whispering voice says to you "/cB I am watching you/cw "
wait 5
wecho Whispering voice says to you "/cB I know who you are and why you're here/cw "
wait 5
wecho Whispering voice says to you "/cB I see all and don't like what you are doing/cw "
wait 5
wecho Whispering voice says to you "/cB So you can just leave now before I get REALLY upset with your  trespasses!/cw "
~
#35004
new trigger~
2 c 0
%actor% slump~
wait 5
wecho /cL GET OUT! Before I throw you out!!/cw
~
#35005
test object trigger~
1 h 100
~
wait 1
oecho The Pipe Stops smoking.
~
#35006
new trigger~
2 c 0
drop~
wait 5 s
wforce %actor% get all
end
~
#35007
Chief ~
0 g 100
%direction% == east~
wait 5
mecho The chief looks at you and smiles.
wait 5
say I have been waiting one such as you to aid my tribe.
wait 5
mecho The chief lights his pipe and tokes on it thoughtfully for a moment before continuing.
wait 7
say A year ago my strongest warrior disappeared and I fear it might be the under dwellers.
wait 2s
say He was last seen at the small alcove on the stream, south and east and west of here, follow it and you shall see it.
wait 3
say if you complete this quest you shall be rewarded justly for bring piece of mind to my people.
say if you choose to accept type accept now
end
~
#35008
hunters~
0 i 100
~
wait 3 s
mecho The muscular man scans the area for life and possible prey.
say Game has been so poor lately.
~
#35009
Chief's Tent~
2 c 0
accept~
wait 3s
wecho The chief looks at you deeply for a moment, studying your every movement.
wecho A faint voice of spirits long dead can be heard whispering on the wind "You have chosen a worthy path hero"
wait 4s
wecho Chief Kraznon says 'Go find a small creature and talk with him, he shall lead you to the place you need to be.'
~
#35010
Guardian trigger~
0 g 100
~
if (%direction% == east)
wait 5
say 'Halt who goes there?!  I demmand the password now or you must perish!!'
~
#35011
Password for guardian~
0 c 100
say %*%~
wait 3
if (* == Shanta) smile
else kill %actor%
if %actor% says 'shanta' 
bow
else if actor say *
else if %actor% say %*%
attack %actor%
~
$~
