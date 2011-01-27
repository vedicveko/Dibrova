#41700
rock pickup trg~
1 g 75
~
wait 1 s
%echo%  A brilliant flash of light blinds your vision! 
wait 1 s
%teleport% %actor.name% 41707
%echo%  When your vision clears, you find yourself in a new locale.
wait 4 s
oforce %actor% l
wait 3s
opurge rock
~
#41701
eldar race check trg~
0 g 100
~
wait 1 s
if (%actor.race% != Eldar)
  say You are not of the Eldar Race.
  wait 1 s
  say You may not enter into the Chambers of Questioning.
  wait 1 s
  emote waves a hand in a semi-circle.
  wait 1 s
  %echo% Your vision is obscured as you are taken to another place.
  %teleport% %actor.name% 41701
~
#41702
first question~
0 e 0
bows~
wait 2 s
nod
wait 1 s
say You have come to your first Question, %actor.name%.
wait 2 s
say The question is this:
wait 3 s
say In the Elven Valley, what is the name of the most powerful wizard there?
wait 2 s
say You have ten seconds to answer my query.
wait 15 s
say You have failed to answer my query correctly.
wait 2 s
say I am forbidden to aid you any further.
wait 1 s
bow
wait 2 s
%echo% You blink and an ancient eldar is gone.
mpurge me
~
#41703
first question answer~
0 d 0
talemon~
wait 1 s
say You are correct, Eldar %actor.name%.
wait 1 s
say You will now be taken to the Second Chamber of Questioning.
wait 1 s
%echo% Blinding light again obscures your vision.
%teleport% %actor.name% 41709
~
#41704
second question~
0 e 0
bows~
wait 2 s
nod
wait 1 s
say You have come to you Second Question, %actor.name%.
wait 2 s
say The question is this:
wait 3 s
say The wizard Damorask is cloaked in what?
wait 2 s
say You have ten seconds to answer my query.
wait 15 s
say You have failed to answer my query correctly.
wait 2 s
say I am forbidden to aid you any further.
wait 1 s
bow
wait 2 s
%echo% You blink and an ancient eldar is gone.
mpurge me
~
#41705
second question answer~
0 d 0
blood~
wait 1 s
say You are correct, %actor.name%.
wait 1 s
say You will now be taken to the Third Chamber of Questioning.
wait 1 s
%echo% Blinding light again obscures your vision.
%teleport% %actor.name% 41710
~
#41706
third question~
0 e 0
bows~
wait 2 s
nod
wait 1 s
say You have come to your third Question, %actor.name%.
wait 2 s
say The question is this:
wait 3 s
say In what great city does the Archmage Tuo reside?
wait 2 s
say You have ten seconds to answer my query.
wait 15 s
say You have failed to answer my query correctly.
wait 2 s
say I am forbidden to aid you any further.
wait 1 s
bow
wait 2 s
%echo% You blink and an ancient eldar is gone.
mpurge me
~
#41707
third question answer~
0 d 0
malathar~
wait 1 s
say You are correct, %actor.name%.
wait 1 s
say You will now be taken to the Fourth Chamber of Questioning.
wait 1 s
%echo% Blinding light again obscures your vision.
%teleport% %actor.name% 41711
~
#41708
fourth question~
0 e 0
bows~
wait 2 s
nod
wait 1 s
say You have come to your fourth Question, %actor.name%.
wait 2 s
say The question is this:
wait 3 s
say What is the name of the leader of the Minotaur Mages of Mahn-Tor?
wait 2 s
say You have ten seconds to answer my query.
wait 15 s
say You have failed to answer my query correctly.
wait 2 s
say I am forbidden to aid you any further.
wait 1 s
bow
wait 2 s
%echo% You blink and an ancient eldar is gone.
mpurge me
~
#41709
fourth question answer~
0 d 0
amyrok~
wait 1 s
say You are correct, %actor.name%.
wait 1 s
say You will now be taken to the Fifth and final Chamber of Questioning.
wait 1 s
%echo% Blinding light again obscures your vision.
%teleport% %actor.name% 41712
~
#41710
fifth question~
0 e 0
bows~
wait 2 s
nod
wait 1 s
say You have come to your fifth and final Question, %actor.name%.
say Listen closely.
wait 2 s
say The question is this:
wait 3 s
say For whom does the evil, foul-tempered wizard Axtraxes work?
wait 2 s
say You have ten seconds to answer my query.
wait 15 s
say You have failed to answer my query correctly.
wait 2 s
say I am forbidden to aid you any further.
wait 1 s
bow
wait 2 s
%echo% You blink and an ancient eldar is gone.
mpurge me
~
#41711
fifth question answer~
0 d 0
krothgar~
wait 1 s
say You are correct, %actor.name%.
wait 1 s
say You have gained access to the Eldar Caverns.
wait 1 s
say May your Magic remain strong.
wait 1 s
%echo% You only have time to blink before you find yourself in a new room.
%teleport% %actor.name% 41713
~
#41712
non-eldar attack~
0 g 100
~
if (%actor.race% != Eldar)
say You are not of the Eldar.
say Begone.
%teleport% %actor.name% 41701
~
#41713
room portal trg~
2 d 0
midgaard~
%echo% You are blinded by a flash of light.
%teleport% %actor.name% 3014
%force% %actor.name% l
~
#41714
mcgintey tlp~
2 d 0
mcgintey~
%echo% You are blinded by a flash of light.
%teleport% %actor.name% 6777
%force% %actor.name% l
~
#41715
insanity tlp~
2 d 0
silverthorne~
%echo% You are blinded by a flash of light.
%teleport% %actor.name% 5747
%force% %actor.name% l
~
#41716
sundhaven tlp~
2 d 0
sundhaven~
%echo% You are blinded by a flash of light.
%teleport% %actor.name% 6608
%force% %actor.name% l
~
#41717
new thalos tlp~
2 d 0
jareth~
%echo% You are blinded by a flash of light.
%teleport% %actor.name% 1005
%force% %actor.name% l
~
#41718
first ward entry tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in a blinding flash of light!
%teleport% %actor.name% 41740
opurge entry
~
#41719
second ward entry tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in a blinding flash of light!
%teleport% %actor.name% 41749
opurge entry
~
#41720
third ward entry tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in a blinding flash of light!
%teleport% %actor.name% 41758
opurge entry
~
#41721
fourth ward entry tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in a blinding flash of light!
%teleport% %actor.name% 41767
opurge entry
~
#41722
fifth ward entry tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in a blinding flash of light!
%teleport% %actor.name% 41771
opurge entry
~
#41723
sixth ward entry tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in a blinding flash of light!
%teleport% %actor.name% 41785
opurge entry
~
#41724
seventh ward entry tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in a blinding flash of light!
%teleport% %actor.name% 41794
opurge entry
~
#41725
teachings tlp~
1 g 100
~
wait 2 s
%echo% You are engulfed in brilliant light!
%teleport% %actor.name% 41734
opurge teaching
~
#41726
teaching tlp death load~
0 f 100
~
mload obj 41717
~
#41727
first entry death load~
0 f 100
~
mload obj 41710
~
#41728
second entry death load~
0 f 100
~
mload obj 41711
~
#41729
third entry death load~
0 f 100
~
mload obj 41712
~
#41730
fourth entry death load~
0 f 100
~
mload obj 41713
~
#41731
fifth entry death load~
0 f 100
~
mload obj 41714
~
#41732
sixth entry death load~
0 f 100
~
mload obj 41715
~
#41733
seventh entry death load~
0 f 100
~
mload obj 41716
~
#41734
keeper defense load 1~
0 g 100
~
if (%actor.vnum% == -1)
wait 6 s
say What are you doing in this room, Eldar %actor.name%?
wait 2 s
say It is forbidden that you be here!
wait 2 s
say Understand that I must call upon the Guardians if you do not leave immediately.
wait 2 s
emote thrusts his arms above his head.
wait 2 s
say Guardians of the First Ward! Come Forth!
wait 2 s
mkill %actor.name%
mat statue_one_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_one_ward mload mob 41716
mat statue_one_ward mpurge statue_one_ward
mat statue_one_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_one_ward mload mob 41716
mat statue_one_ward mpurge statue_one_ward
mat statue_one_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_one_ward mload mob 41716
mat statue_one_ward mpurge statue_one_ward
mat statue_one_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_one_ward mload mob 41717
mat statue_one_ward mpurge statue_one_ward
~
#41735
keeper defense load 2~
0 g 100
~
if (%actor.vnum% == -1)
wait 6 s
say What are you doing in this room, Eldar %actor.name%?
wait 2 s
say It is forbidden that you be here!
wait 2 s
say Understand that I must call upon the Guardians if you do not leave immediately
wait 2 s
emote thrusts his arms above his head.
wait 2 s
say Guardians of the Second Ward! Come Forth!
wait 2 s
mkill %actor.name%
mat statue_two_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_two_ward mload mob 41716
mat statue_two_ward mpurge statue_two_ward
mat statue_two_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_two_ward mload mob 41716
mat statue_two_ward mpurge statue_two_ward
mat statue_two_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_two_ward mload mob 41716
mat statue_two_ward mpurge statue_two_ward
mat statue_two_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_two_ward mload mob 41717
mat statue_two_ward mpurge statue_two_ward
~
#41736
keeper defense load 3~
0 g 100
~
if (%actor.vnum% == -1)
wait 6 s
say What are you doing in this room, Eldar %actor.name%?
wait 2 s
say It is forbidden that you be here!
wait 2 s
say Understand that I must call upon the Guardians if you do not leave immediately.
wait 2 s
emote thrusts his arms above his head.
wait 2 s
say Guardians of the Third Ward! Come Forth!
wait 2 s
mkill %actor.name%
mat statue_three_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_three_ward mload mob 41716
mat statue_three_ward mpurge statue_three_ward
mat statue_three_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_three_ward mload mob 41716
mat statue_three_ward mpurge statue_three_ward
mat statue_three_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_three_ward mload mob 41716
mat statue_three_ward mpurge statue_three_ward
mat statue_three_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_three_ward mload mob 41717
mat statue_three_ward mpurge statue_three_ward
~
#41737
keeper defense load 4~
0 g 100
~
if (%actor.vnum% == -1)
wait 6 s
say What are you doing in this room, Eldar %actor.name%?
wait 2 s
say It is forbidden that you be here!
wait 2 s
say Understand that I must call upon the Guardians if you do not leave immediately.
wait 2 s
emote thrusts his arms above his head.
wait 2 s
say Guardians of the Fourth Ward! Come Forth!
wait 2 s
mkill %actor.name%
mat statue_four_ward %echo% The statue begins to shake and crumble as a Guardian
 steps forth.
mat statue_four_ward mload mob 41716
mat statue_four_ward mpurge statue_four_ward
mat statue_four_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_four_ward mload mob 41716
mat statue_four_ward mpurge statue_four_ward
mat statue_four_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_four_ward mload mob 41716
mat statue_four_ward mpurge statue_four_ward
mat statue_four_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_four_ward mload mob 41717
mat statue_four_ward mpurge statue_four_ward
~
#41738
keeper defense load 5~
0 g 100
~
if (%actor.vnum% == -1)
wait 6 s
say What are you doing in this room, Eldar %actor.name%?
wait 2 s
say It is forbidden that you be here!
wait 2 s
say Understand that I must call upon the Guardians if you do not leave immediately.
wait 2 s
emote thrusts his arms above his head.
wait 2 s
say Guardians of the Fifth Ward! Come Forth!
wait 2 s
mkill %actor.name%
mat statue_five_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_five_ward mload mob 41716
mat statue_five_ward mpurge statue_five_ward
mat statue_five_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_five_ward mload mob 41716
mat statue_five_ward mpurge statue_five_ward
mat statue_five_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_five_ward mload mob 41716
mat statue_five_ward mpurge statue_five_ward
mat statue_five_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_five_ward mload mob 41717
mat statue_five_ward mpurge statue_five_ward
~
#41739
keeper defense load 6~
0 g 100
~
if (%actor.vnum% == -1)
wait 6 s
say What are you doing in this room, Eldar %actor.name%?
wait 2 s
say It is forbidden that you be here!
wait 2 s
say Understand that I must call upon the Guardians if you do not leave immediately.
wait 2 s
emote thrusts his arms above his head.
wait 2 s
say Guardians of the Sixth Ward! Come Forth!
wait 2 s
mkill %actor.name%
mat statue_six_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_six_ward mload mob 41716
mat statue_six_ward mpurge statue_six_ward
mat statue_six_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_six_ward mload mob 41716
mat statue_six_ward mpurge statue_six_ward
mat statue_six_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_six_ward mload mob 41716
mat statue_six_ward mpurge statue_six_ward
mat statue_six_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_six_ward mload mob 41717
mat statue_six_ward mpurge statue_six_ward
~
#41740
keeper defense load 7~
0 g 100
~
if (%actor.vnum% == -1)
wait 6 s
say What are you doing in this room, Eldar %actor.name%?
wait 2 s
say It is forbidden that you be here!
wait 2 s
say Understand that I must call upon the Guardians if you do not leave immediately.
wait 2 s
emote thrusts his arms above his head.
wait 2 s
say Guardians of the Seventh Ward! Come Forth!
wait 2 s
mkill %actor.name%
mat statue_seven_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_seven_ward mload mob 41716
mat statue_seven_ward mpurge statue_seven_ward
mat statue_seven_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_seven_ward mload mob 41716
mat statue_seven_ward mpurge statue_seven_ward
mat statue_seven_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_seven_ward mload mob 41716
mat statue_seven_ward mpurge statue_seven_ward
mat statue_seven_ward %echo% The statue begins to shake and crumble as a Guardian steps forth.
mat statue_seven_ward mload mob 41717
mat statue_seven_ward mpurge statue_seven_ward
~
#41741
history of eldar~
1 b 20
~
wait 5 s
%echo% The Eldar have not always been the race of strong magics that they are today.
wait 5 s
%echo% There was a time in the not so long distant past that the Eldar were much like any other race, living in the cities and striving to make their mark upon the world in whatever way they could.
wait 5 s
%echo% Then came the time of the Rendering, a dark time full of peril for not just the Eldar race, but for all races.
wait 5 s
%echo% None, however, lost so much as the Eldars in that terrible blast from the Gods.
wait 5 s
%echo% The great and powerful god of darkness, Rendor, felt that the Eldar race was a hindrance to the world.
wait 5 s
%echo% Rendor felt that the Eldar were inferior to all others, being slight of stature and not with any particular affinity for any useful skills.
wait 5 s
%echo% Rendor, in one of his fits of personal rage, ordered his dark goblin troops to massacre any and all Eldar that they came across.
wait 5 s
%echo% And massacre they did, all across the lands from sea to sea until there were but a handful left.
wait 5 s
%echo% This is how the Eldar came to be a race living in underground caverns, hidden from the view of the legions of Rendor's dark army.
wait 5 s
%echo% Now in the midst of all this chaos, there came one god who saw the plight of the Eldar and took pity - the god Tsol.
wait 5 s
%echo% Tsol appeared to the Eldar, who were huddled in fear of most any creature other than one of their own, and Tsol said to them:
wait 5 s
%echo% "Fear not, My people, for I come to you with a protection. One which will keep you from danger for all the days that you live so long as you in turn protect that which protects you."
wait 5 s
%echo% And with those words, the god Tsol gave unto the Eldar race the Seven Wards of the Arcane, the power of which imbued the Eldar race with an affinity for magic and concealment from Rendor forever.
wait 5 s
%echo% It is said that if the Seven Wards ever were to be taken from their Chambers and brought together by one being, that being would suffer a fate worse than death at the hands of the god Tsol.
wait 5 s
%echo% Not only would that being suffer, but the fate of the entire Eldar nation would be at stake.
wait 5 s
%echo% And so the Seven Keepers of the Seven Wards watch over their charge, keeping the Wards safe for all time so that the Eldar might continue forever as a dominant force in the world.
wait 55 s
~
#41742
eldar babble~
0 b 50
~
wait 2 s
say You know - a lot of people say I talk too much.
wait 2 s
say If I do start to talk too much, just tell me to shutup and I'll go away.
wait 2 s
say I'd certainly hate to be any sort of annoyance to anyone.
wait 2 s
emote fusses with your armor a bit.
wait 2 s
sigh
wait 2 s
say Now you can't very well go in there looking like that!
wait 2 s
emote fusses with your armor a bit more.
wait 2 s
say Now, that's better.
wait 2 s
say Now - you know this is not going to be easy.
wait 2 s
say EVERYBODY fails at least two of the questions the first time.
wait 2 s
say Don't be nervous, but don't forget to bow.
wait 2 s
say Above all, don't forget to bow!
wait 2 s
emote fusses with your armor even more.
wait 2 s
%echo% (this has got to be getting annoying, its annoying *me* to type it all)
wait 2 s
say Remember your manners, and bow for the Questioners.
wait 2 s
say Stand tall, speak your answers clearly.
wait 2 s
say And bow, for God's sake! Bow!
wait 2 s
emote fusses with your armor one last time.
wait 2 s
say Okay, now you look pretty as can be.
wait 2 s
%echo% Pretty!? Who is this guy anyway?!
wait 2 s
say Now, you go get em, my fine friend and DON'T FORGET TO BOW!
wait 2 s
emote gives a little sniffle and gestures you southward.
wait 20 s
emote wipes a bit of water from his eyes.
wait 20 s
~
#41743
ward pick-up trg~
1 g 65
~
wait 1 s
%echo% The Ward thrums with immense power as you pick it up.
~
$~
