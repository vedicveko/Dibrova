#16600
oak trans~
0 k 5
~
if (%self.vnum% == 16600)
%echo% 
emote begins to /ccshimmer/c0, slowly changing from an old oak into a beautiful young woman with wild, green hair.
%echo% 
mtransform 16601
~
#16601
turn to tree~
0 k 1
~
if (%self.vnum% == 16601)
%echo%  
emote looks to the heavens, opening her mouth in a rictus of pain.
emote slowly melds into her tree and is gone.
mload obj 16605
%echo%   
mpurge me
~
#16602
oak trans II~
0 k 5
~
if (%self.vnum% == 16600)
%echo%    
emote begins to /ccshimmer/c0, slowly melding into the form of a beautiful yound dryad.
%echo%    
mtransform 16602
~
#16603
oak trans III~
0 k 5
~
if (%self.vnum% == 16600)
%echo%    
emote begins to /ccshimmer/c0, slowly melding into the form of a beautiful yound dryad.
%echo%    
mtransform 16605
~
#16604
oak trans IV~
0 k 5
~
if (%self.vnum% == 16600)
%echo%    
emote begins to /ccshimmer/c0, slowly melding into the form of a beautiful yound dryad.
%echo%    
mtransform 16606
~
#16605
oak trans V~
0 k 5
~
if (%self.vnum% == 16600)
%echo%    
emote begins to /ccshimmer/c0, slowly melding into the form of a beautiful yound dryad.
%echo%    
mtransform 16607
~
#16606
oak trans VI~
0 k 5
~
if (%self.vnum% == 16600)
%echo%    
emote begins to /ccshimmer/c0, slowly melding into the form of a beautiful yound dryad.
%echo%    
mtransform 16608
~
#16608
turn to tree II~
0 k 1
~
if (%self.vnum% == 16602)
%echo%    
emote looks to the heavens, opening her mouth in a rictus of pain.
emote melds into her tree and is gone.
mload obj 16605
%echo%    
mpurge me
~
#16609
turn to tree III~
0 k 1
~
if (%self.vnum% == 16605)
%echo%    
emote looks to the heavens, opening her mouth in a rictus of pain.
emote melds into her tree, vanishing from sight.
mload obj 16605
%echo%    
mpurge me
~
#16610
turn to tree IV~
0 k 1
~
if (%self.vnum% == 16606)
%echo%    
emote looks to the heavens, opening her mouth in a rictus of pain.
emote melds into her tree, vanishing from sight.
mload obj 16605
%echo%    
mpurge me
~
#16611
turn to tree V~
0 k 1
~
if (%self.vnum% == 16607)
%echo%    
emote looks to the heavens, opening her mouth in a rictus of pain.
emote melds into her tree, vanishing from sight.
mload obj 16605
%echo%    
mpurge me
~
#16612
turn to tree VI~
0 k 1
~
if (%self.vnum% == 16608)
%echo%    
emote looks to the heavens, opening her mouth in a rictus of pain.
emote melds into her tree, vanishing from sight.
mload obj 16605
%echo%    
mpurge me
~
#16613
dryad death~
0 f 75
~
if (%self.vnum% == 16601)
mload obj 16600
~
#16614
dryad death II~
0 f 75
~
if (%self.vnum% == 16602)
mload obj 16601
~
#16615
dryad death III~
0 f 75
~
if (%self.vnum% == 16605)
mload obj 16602
~
#16616
dryad death IV~
0 f 75
~
if (%self.vnum% == 16606)
mload obj 16603
~
#16617
dryad death V~
0 f 75
~
if (%self.vnum% == 16607)
mload obj 16604
~
#16645
greenhag trg~
0 g 100
~
if (%actor.vnum% == -1)
wait 2 s
emote moans in pain.
wait 2 s
say Oh, those wretched, wretched ogres!
wait 3 s
say Oh, please, brave adventurer..
say Could you help me?
wait 2 s
say I have money, I could pay you to take me to safety.
wait 3 s
say Please, help me. Come, help me up.
wait 2 s
say You must come close enough to..
wait 3 s
emote begins to shake, her body blurring around the edges.
wait 2 s
%echo%    
%echo%    
%echo% /cRWith a hideous shriek the maiden morphs into a greenhag, leaping at you!/c0
%echo%    
%echo%    
mtransform 16645
mkill %actor.name%
~
#16646
seductive trg to annis~
0 g 100
~
if (%actor.vnum% == -1)
wait 2 s
emote gives you a very slight, seductive smile.
wait 1 s
lick
wait 2 s
say Hello. What's your name?
wait 3 s
say Would you like to... join me down here?
wait 3 s
mtransform 16646
%echo%      
%echo%      
%echo% /cRWith a screech and snarl, the young maiden transforms into an awful hag!/c0
%echo% /cRShe Leaps at your throat!/c0
%echo%      
%echo%      
mkill %actor.name%
~
#16647
old woman hurt to annis~
0 g 100
~
if (%actor.vnum% == -1)
wait 2 s
emote rocks painfully back and forth clenching a hurt knee.
wait 2 s
say Oh, my knee!
wait 3 s
say Could you help me, do you think?
wait 2 s
say I think that it has simply popped out of place but it pains me so..
wait 2 s
say Could you try and look at it for me?
wait 2 s
say Could you...
wait 2 s
mtransform 16646
%echo%     
%echo%     
%echo% /cRWith a snarl, the old woman leaps to her feet, changing from an old woman to an even older hag./c0
%echo% /cRShe attacks!/c0
%echo%     
%echo%     
mkill %actor.name%
~
#16648
prisoner trg~
0 g 100
~
if (%actor.vnum% == -1)
wait 2 s
emote struggles to his feet.
wait 2 s
bow
wait 2 s
say I am Morphiaal and I am so very glad to see you.
wait 3 s
say I have been held here as prisoner for the last two weeks without light.
say Without any food but stale bread.
say And without any water but the rain that falls.
wait 3 s
say I am indebted to you. I would aid you for a time if you would have it.
wait 5 s
emote begins to shake and shiver, his skin melting off of his flesh.
mtransform 16646
wait 2 s
emote shrieks in triumph!
mkill %actor.name%
~
#16649
prisoner follow~
0 e 1
beckons~
wait 1 s
bow
wait 2 s
say And so I shall aid as I can for as long it may be.
wait 2 s
follow %actor.name%
~
#16650
greenhag death~
0 f 50
~
if (%self.vnum% == 16645)
mload obj 16645
~
#16651
annis death I~
0 f 50
~
if (%self.vnum% == 16646)
mload obj 16646
~
#16652
annis death II~
0 f 50
~
if (%self.vnum% == 16646)
mload obj 16647
~
#16653
prisoner give amut~
0 d 1
amulet~
wait 2 s
say You wish to have my amulet?
wait 1 s
say If it please you, then please accept it as a gift.
wait 2 s
rem amul
wait 1 s
give amulet %actor.name%
wait 2 s
bow
~
$~
