#20201
Mortianna~
0 g 100
~
wait 50
mecho Mortianna mutters, '...exactly what are you doing here!?, sitting here doing nothing for hours!. Some people have to work you know!'
wait 20
mecho Mortianna mutters, 'Just look at me!...I am so tired of denying all theese warriors from buying my sacred potions.'
wait 20
mecho Mortianna mutters, 'I was hoping that the sign I put up outside the shop, informing everyone that I only sell my goods to people who has magic in their souls.'
wait 30
mecho Mortianna screams, 'GET OUT!'
mforce %actor% west
end
~
#20203
Friar Tuck~
0 b 80
~
eval numb %random.10%
if (%numb% == 10)
mecho Friar Tuck says, 'Hohoho!'
wait 15
mecho Friar Tuck leans back with his hands on his belly.
else if (%numb% == 2)
elseif (%numb% == 2)
wait 10
think
elseif (%numb% == 8)
wait 10
smile
wait 15
mecho Friar Tuck says, '/cwSo, what are you doing in theese parts of the land?/c0'
wait 15
mecho Friar Tuck says, '/cwMyself, well, I am waiting for Robin!/c0'
wait 15
mecho Friar Tuck says, '/cwWonder where he is...He is allways out looking for some girl to rescue, or stealing from the wealthy citizens of Nothingham!/c0'
else
wait 15
yawn
end
~
#20204
Friar Tuck II~
0 j 100
~
7s
~
#20210
Heart of Yellow Dragon~
1 j 100
~
oechoaround %actor.name% ...there is a strange sound as a sense of danger comes close...
~
#20211
Robin hood~
0 g 100
~
wait 10
if (%actor.gold% >= 10000)
msend %actor% Robin Hood looks up at you and say, '/cwI have heard of you %actor.name%/cg'/c0
wait 15
msend %actor% Robin Hood takes a firm grip around your throat.
wait 15
mecho Robin Hood says, '/cwI would suggest you to give me 10,000 gold coins!/cg'/c0
wait 15
mforce %actor.name% give 10000 coins robin
mecho Robin Hood says, '/cwLet this be a warning %actor.name%!/cg'/c0
wait 10
mecho Robin Hood says, '/cwNow get going %actor.name%!/cg'/c0
wait 15
mhunt %actor%
mkill %actor.name%
else
if (%actor.vnum% == -1)
scan
wait 10
mecho Robin Hood turns around and looks at you.
wait 10
mecho /cy/cgRobin Hood says, '/cwOh damn, another poor wanderer visiting theese lands.../cg'/c0
wait 15
mecho /cy/cgRobin Hood says, '/cwWell, you might as well join me and I will make sure you fall asleep with a full stomach!/cg'/c0
end
~
#20217
Prince John~
0 g 100
~
if (%actor.gold% < 10000)
wait 5
mecho Prince John looks at %actor.name%.
wait 5
mecho Prince John says, '/cwAck! What is this poor thing doing here?/c0'
wait 10
mecho Prince John screams, '/cwGuards! Guards! Some poor idiot who calls itself %actor.name% is here, come kill it!/c0'
wait 5
mload mob 20227
mecho A royal guard suddenly runs in, trying to catch his breath!
wait 5
mecho Prince John points at %actor.name%.
wait 5
mforce guard kill %actor.name%
mforce guard kill %actor.name%
mecho Prince John suddenly dissapears in a puff of smoke!
mpurge self
elseif (%actor.gold% >= 20000)
mecho Prince John says, '/cwWelcome my dear noble %actor.name%!/c0'
end
~
#20218
Prince John, Quest(hopefully)~
0 j 100
~
if (%object.vnum% == 20242)
wait 10
mecho Prince John says, '/cwExcellent! Finally a brave adventurer has captured and eliminated the damn thief Robin Hood!/c0'
wait 10
mecho Prince John says. '/cwYou shall be rewarded greatly %actor.name%!/c0'
mgold %actor% 10000
msend %actor% Prince John gives you 10,000 shining gold coins.
wait 15
mecho Prince John says, '/cwI need to roast this thing!/c0'
wait 2
snap
wait 5
mecho Prince John shouts, '/cwServants!, I need a fire over here!/c0'
wait 6
mload mob 20299
mecho A servant comes running in, catching it's breath.
wait 5
mload obj 20243
mecho A servant lights a fire on the floor here.
wait 5
mecho Prince John shouts, '/cwAhh!!/c0'
mecho Prince John holds Robin Hood's head over the fire.
wait 10
mecho Prince John takes the head out of the fire.
wait 2
mecho Prince John starts eating Robin Hood' head.
wait 5
mecho Prince John exclaims, '/cwNothing is as good as a thieves barbequed head!/c0'
mpurge head
else
wait 10
mecho Prince John says, '/cwHrmph!, I dont need this item!/c0'
wait 15
mecho Prince John says, '/cwYou shall die!/c0'
wait 5
mkill %actor.name%
end
~
#20223
immortal sword trigger~
1 j 100
~
oecho The ground starts to shatter as someone wields a sword of great powers.
oechoaround The ground starts to shatter as someone wields a sword of great powers.
~
#20224
Trolls at Yellow Dragon~
0 g 100
~
if (%random.5% == 1)
wait 5
slap %actor.name%
wait 5
grin %actor.name%
wait 5
snicker
else
wait 5
snicker
wait 5
growl
end
~
#20225
Yellow Dragon~
0 g 100
~
if (%random.10% == 5)
kill %actor.name%
else
grin %actor.name%
end
~
#20232
Monks praying~
0 b 100
~
wait %random.30%
if (%random.2% == 1)
wait %random.15%
mecho A monk starts praying.
else
wait %random.12%
mecho A monk continues his prayers.
end
~
#20234
Horses sold by Stablemaster(nottingham)~
0 b 10
~
mecho A strange feeling comes over you....
wait 20
mecho You suddenly gain the ancient knowledge to understand horses.
wait 20
mecho The black stallion says, 'Damn, my master is really heavy to carry around.'
wait 15
mecho The black stallion says, 'Infact, I am sure that my master has gained some weight recently!'
wait 25
mecho The black stallion says, 'They never think of the poor horses carrying them!'
wait 20
mecho The black stallion says, 'Might as well throw my master off and run away...'
wait 10
mutter
wait 15
south
north
east
west
up
down
end
~
#20235
A small glowing pink potion(restore mana)~
1 ghi 100
~
oecho The ground starts to shatter...
wait 1
oecho ...the ground stops shattering.
~
#20256
Tree, climbing~
2 g 45
~
wsend %actor% You feel tired as you climb up the Large Tree.
wait 5
wforce %actor% down
~
#20262
Enterin the town Nottingham~
2 g 100
~
wait 5
wecho A voice of the night says, '/cwWelcome to Nottingham!...Great adventures lies before you, and great adventures you leave behind.../c0'
~
#20298
Robin Hood - Thieves~
0 g 100
~
Nothing.
~
#20299
Servant, for purging~
0 b 100
~
wait 50
mecho The servant puts the fire out.
mpurge fire
wait 10
mecho The servant runs away.
mpurge self
~
$~
