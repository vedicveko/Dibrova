#22500
Vampiress Seducing~
0 b 12
~
mecho /crThe Vampiress slowly seduces a young man./c0
wait 100
mecho /crThe Vampiress turns around./c0
~
#22501
Jonathan Harker~
0 b 5
~
mecho /cwThe young man makes some funny noises,/c0
~
#22502
Dracula, Prince~
0 g 100
~
if %actor.align% > -300
msend %actor% /cw/crPrince Dracula bites you, blood raining from the sky. You realise its your blood./c0
mkill %actor%
~
#22503
Lestat I~
0 l 50
~
wait 50
say Mine magnificence is beyond thy ken! It cannot be extinguished by thy lame hand!
wait 10
if %actor.align% > -300
say Thy soul shall be mine, and thy beauty I shall add to my own! Thy life will be severed - come my minions!!! Devour the %actor.sex%!!!
mload mob 22525
mforce vampire kill %actor.name%
mload mob 22525
mforce vampire kill %actor.name%
wait 200
~
#22504
Dracula II~
0 f 100
~
mecho /crYour heart /cCfreezes/cr to /cWice/cr as you hear Prince Dracula's Deathcry/c0
mload mob 22527
mload mob 22528
mforce %actor% look
mecho Van Helsing shouts, 'Hahahahahahaha'
mecho Mina Harker spins around, with a dreadful smile.
~
#22505
Armand~
0 n 100
~
shout My soul is bound to the destiny of /crDracula/cy!
wait 1
shout DIE!
mload mob 22504
kill van
~
#22506
Vittorio~
0 n 100
~
growl
say My master is dead!
scream
mload mob 22503
kill mina
~
#22507
Gaia~
0 k 5
~
wait 200
mecho /cmGaia sings the words, '/cCCome my children!/cm'/c0
mload mob 22518
mforce werechild kill %actor.name%
~
#22508
Mina Harker and Van Helsing II~
0 k 100
~
kill vamp
kill 2.vamp
~
#22509
Sekhmet - Dangerous~
0 k 5
~
wait 10
mecho /crSekhmet shouts, 'I call upon the Vampyre Race to Assist me!'/c0
mload mob 22525
mforce vamp kill %actor.name%
~
#22510
Sekhmet Claws I~
1 j 100
~
osend %actor% /crYou feel the power of Sekhmet flow trough your veins./c0
if %actor.sex% = Male
   oechoaround %actor% /cr%actor.name% shimmers strangly as a surge of power pulsates through him./c0
else
   oechoaround %actor% /cr%actor.name% shimmers strangly as a surge of power pulsates through her./c0
end if
~
#22511
Sekhmet Claws II~
1 l 100
~
osend %actor% /crThe power of Sekhmet flows out of your veins/c0
~
$~
