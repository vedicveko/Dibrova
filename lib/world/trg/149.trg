#14900
telepath greet~
0 g 100
~
wait 2 s
say Ah, yes. %actor.name%. I have been awaiting your arrival.
wait 2 s
say I can tell you that which you need to know, oh yes.
wait 4 s
say But for a fee.
~
#14901
telepath scam~
0 m 10000
~
if (%amount% <= 9999)
   wait 2 s
  emote counts slowly through the coins you have given him.
  wait 6 s
  say I am sorry, but this is not enough. My services require much more.
else
  wait 2 s
  emote counts slowly through the coins you have given him, very slowly.
  wait 10 s
  say Only a fool would pay someone for a service which they have not
  say yet received. My knowledge that I impart to you is to never be
  say fooled in this way again.
  wait 2 s
  say Now get out, fool!
  wait 1 s
  cackle
~
$~
