#9300
patron greet~
0 g 100
~
wait 2 s
emote notices your arrival with a big smile and a clap of his hand on the bar.
wait 2 s
say Ho! Look at here! Another newcomer to Villspah!
wait 1 s
emote chuckles drunkenly.
wait 2 s
say Welcome... welcome indeed...
~
#9301
patron chatter~
0 b 5
~
emote looks your way with a sloppy, dark smile.
wait 2 s
say So how long do you think until it's all-out war with the Barons?
wait 25 s
say They say Villspah has the crown... that he intends to give it to Robin...
~
#9302
local argument 1~
0 d 0
intends to give it~
wait 2 s
say Pah!
say You and your talk, Grumphas. Why don't you just get drunk in quiet?
wait 6 s
say Awww, shut up woman...
~
#9303
local woman argument 1~
0 d 0
drunk in quiet~
wait 2 s
slap hank
wait 1 s
say I'm shocked at you, Hank Longaberger - talking like that to one of our neighbors!
~
#9304
quiet woman~
0 b 4
~
emote looks up for a moment, smiles warmly at you, and then goes back to her book.
~
#9305
server~
0 d 0
shut up~
wait 1 s
say Now, I'll have none of that in my shop!
wait 1 s
say If you all want to bicker and bitch like a bunch of old women, you do it somewheres else!
wait 3 s
emote grumbles as she wipes a mug.
~
#9306
logger smirk~
0 d 0
somewheres else~
smirk.
wait 1 s
drink beer
~
#9307
random drink~
0 b 10
~
drink beer
~
#9308
general store welcome~
0 g 100
~
wait 2 s
say Ho! And welcome to my shop!
wait 1 s
say Anything you need, anything! You just name it.
~
#9309
smithy trg~
0 b 10
~
%echo% The smithy pounds at the piece of metal he holds.
%echo%    
%echo% /cctink!/c0
wait 2 s
%echo% /cctink!/c0
~
#9310
coffee woman~
0 g 50
~
say Welcome! Come in, come in!
wait 2 s
say Can I get you a fresh pastry, by chance?
~
#9311
miller~
0 e 1
cup~
%echo% The miller replaces the spot where the cup was with a new one.
wait 2 s
smile %actor.name%
~
#9312
herbalist smile~
0 g 35
~
wait 2 s
smile %actor.name%
~
#9313
jeweller~
0 g 20
~
wait 2 s
emote gives a nod without looking up from his eyepiece.
say I'll be with you in just one moment, I'm sorry for the delay.
~
#9314
bird lady trg~
0 b 5
~
say SQUAWK!
wait 1 s
say No, just kidding.
wait 1 s
smile
wait 2 s
say What can I do for you today, my friend? I'm sorry that I currently have no stock, but maybe I could take an order?
~
#9315
knight trg~
0 g 10
~
wait 2 s
emote gives you a long, piercing look.
wait 3 s
emote grunts noncommitally.
w
~
#9316
knight trg II~
0 b 10
~
emote nods your way.
wait 3 s
say Do I know you friend?
wait 1 s
say I don't believe you're from around here.
~
#9317
local trg I~
0 g 5
~
emote sends you a happy wave of his hand.
wait 1 s
smile
wait 2 s
say Beautiful day its been, isn't it?
~
#9318
local trg II~
0 b 5
~
say New to these parts? Need some directions?
~
#9319
local trg III~
0 k 5
~
wait 2 s
say Here, now!
wait 1 s
say You stop fighting this instant!
wait 3 s
say Now - this a peaceful place, we want none of your ruffhousing - you can just get right on out!
~
#9320
merchant trg~
0 g 5
~
emote tips his hat to you.
wait 2 s
say Would you happen to know the way to the inn?
~
#9321
cook trg I~
0 b 5
~
emote clucks in disapproval of a dirty pan.
wait 1 s
say You'd better get on those pots and pans, girl. We got plenty more where that come from.
~
#9322
cook trg II~
0 b 5
~
emote begins kneading at an enormous lump of bread dough.
~
#9323
cook trg III~
0 b 5
~
emote leans back slightly, knuckling her sore back.
~
#9324
dishgirl piff~
0 b 3
~
emote blows a piff of air at the stray strand of hair in her face.
~
#9325
serving girl trg~
0 b 5
~
w
wait 2 s
emote sets a few full dishes upon the table, removing the old, empty ones.
wait 2 s
curtsey
wait 1 s
e
~
#9326
imposter trg~
0 g 80
~
emote stands, executing a very warm bow of welcome to you.
wait 2 s
say Welcome to my home, friend.
wait 2 s
say Of what assistance can I be to you today?
~
#9327
guard die~
0 f 100
~
mload obj 9305
~
#9328
lieutenant trg~
0 g 100
~
if (%actor.room% == 9366)
wait 2 s
emote makes an attempt to stand and then slumps back to the ground.
wait 5 s
emotes breathes raggedly.
wait 2 s
say Go on. Kill me if you will. But know this..
wait 2 s
wait 2 s
emote takes deep gulping breaths - this man has been through Hell.
wait 3 s
say I am the Baron Westlawn's chief cavalry lieutenant and I would follow to the ends of the earth any man who might spare my life - I am an accomplished warrior..
wait 8 s
sigh
wait 2 s
say Do as you will...
~
#9329
lieutenant follow~
0 e 1
beckons~
wait 1 s
follow %actor.name%
~
#9330
darwin strategist shield~
0 g 35
~
say AARHHH!
emote shields his eyes against the brilliance of your light.
~
#9331
personal guard~
0 g 100
~
wait 1 s
say You will leave now or face death!
wait 1 s
say So be it!
~
#9332
personal guard death~
0 f 100
~
mload obj 9306
~
#9333
mistress trg~
0 g 50
~
wait 2 s
emote staggers backward slightly, obviously in fear of you.
wait 3 s
say Please!
say Please don't kill him!
wait 2 s
say I beseech you - he is a good man!
~
#9334
toll guard death~
0 f 100
~
mload obj 9328
~
#9335
counselor greet~
0 g 100
~
if (%actor.vnum% == -1)
wait 3 s 
say Good day to you, my fine friend.
wait 2 s
say Do you have an appointment with the Baron?
wait 3 s
say No?
wait 2 s
say Well, then - there is always the Feasting Hall for all those 'in need'.
wait 2 s
say If you are hungry, simply tell me and I can let you into the hall.
wait 5 s
wait 2 s
say You know... a man needs to feed his family... if you truly wish to get in to see the Baron, perhaps I could arrange something.
wait 3 s
emote delicately clears his throat.
~
#9336
counselor death~
0 f 100
~
mload obj 9303
~
#9337
counselor hungry trg~
0 d 0
hungry~
wait 2 s
say Well, then! No trouble whatsoever.
wait 1 s
smile
emote fishes around in one of his coat pockets.
wait 3s
mload obj 9302
unlock door n
wait 1 s
open door n
wait 2 s
emote grandly gestures the way northward.
wait 2 s
sat After you, my good fellow.
wait 5 s
drop key
~
#9338
counselor bribe~
0 m 10000
~
if (%amount% <= 9999)
  wait 2 s
  say Oh my, but you'll have to do much better than that.
  wait 2 s
  emote smiles delicately.
else
  wait 2 s
  say Well, now. It seems that the Baron had a last minute cancellation.
  wait 2 s
  emote fishes around in a pocket.
  mload obj 9300
  wait 2 s
  unlock door e
  wait 1 s
  open door e
  say After you, my good man.
  wait 10 s
  say You do not wish to see the Baron? Alright..
  close door e
  lock door e
  drop key
~
#9339
bribe II~
0 m 1
~
wait 3 s
emote clears his throat delicately.
wait 2 s
say You are joking, right?
wait 1 s
say I have a rather large brood of children at home... it takes more than %amount% gold to feed all of them.
~
#9340
waiting room trg~
0 b 3
~
Nothing.
~
#9341
servant trg~
0 b 5
~
bow
wait 2 s
say The Baron will see you now.
wait 2 s
mload obj 9301
emote retrieves a small key from his robes.
unlock door e
say Please follow me.
wait 2 s
open door e
e
wait 10 s
wait 2 s
bow %actor.name%
wait 1 s
bow baron
w
close door e
lock door e
drop key
~
$~
