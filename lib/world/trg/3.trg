#300
homeless man shiver~
0 b 10
~
emote shivers with hunger pains, moaning.
~
#301
homeless man mumble~
0 b 10
~
say Hungry, so very hungry...
~
#302
feed homeless man~
0 j 100
~
if (%object.vnum% == 3009)
wait 2 s
say Oh, thank you good sir! Thank you!
wait 2 s
mecho A bit of drool slips from the corner of a homeless man's mouth.
wait 2 s
emote fishes around in his tattered clothing.
wait 3 s
mecho With a flourish, a homeless man brandishes what appears to be a tin can!
wait 1 s
emote smiles proudly.
wait 1 s
say Here you go! And many thanks!
wait 2 s
mload obj 350
give can %actor.name%
elseif (%object.vnum% == 368)
wait 1 s
emote gags in disgust and looks at you sorrowfully!
wait 2 s
say Is that all you have to offer? just cruelty?!?
wait 3 s
drop mass
wait 2 s
say Leave me alone, if this is the extent of your kindness.
~
#303
pastor charity~
0 g 100
~
wait 3 s
emote scrambles to his feet.
wait 2 s
emote looks at you with hope gleaming in his eyes.
wait 3 s
say Charity, if you please...?
wait 4 s
emote tentatvely holds out a grubby hand.
~
#304
pastor charity (bribe)~
0 m 1000
~
if (%amount% <= 999)
  wait 2 s
  say Now how can a man of my limited means get by on such a pittance...?
  wait 2 s
  emote looks at you with innoncent eyes.
else
  wait 2 s
  Say Now that's what I call charity!
  wait 2 s
  say Thank you, kind heart! You are truly a giving person.
  wait 2 s
  say Please - take this as a token of my gratitude.
  wait 2 s
  mload obj 351
  give rosary %actor.name%
~
#305
tormented girl screaming~
0 k 75
~
emote screams in agony as if her soul were being torn out!
~
#306
tormented girl greet~
0 g 100
~
wait 3 s
emote turns your way, eyes wide in self-loathing.
wait 2 s
say Turn back. You cannot save me, but you can save yourself!
wait 2 s
emote turns away and then back again, face contorted in pain.
wait 2 s
emote struggles to say, '/cwPlease! Please! Save me! Kill it! Its killing me! Kill it!/c0'
~
#307
demon death~
0 l 7
~
say You haven't seen the last of me, bitch! You /crwill/cw be mine!/c0
~
#308
tormented girl thanks~
0 d 0
seen the last of~
wait 2 s
say You did it!
wait 2 s
emote looks astonishingly more healthy.
wait 2 s
say You have exorcised the demon from my soul!
wait 2 s
say Thank you, oh thank you!
wait 2 s
say Please. Take this as a token of my gratitude.
wait 2 s
mload obj 3032
mload obj 352
put breath bag
emote kneels down and places an object at your feet.
drop bag
wait 2 s
kiss %actor.name%
wait 2 s
bow
wait 1 s
w
s
mpurge me
~
#309
victim greet~
0 g 100
~
wait 1 s
emote gasps and shudders as pain racks his body.
wait 2 s
emote looks at you with pleading eyes, begging for help.
wait 1 s
say 'SAVE ME!
say 'Please...help me...please...'
wait 3 s
emote slumps forward and faints from loss of blood.
~
#310
thug 1~
0 g 100
~
wait 2 s
say SHUT UP!!
emote snarls softly and kicks the man in the ribs!
wait 2 s
say Hey you!  Should you be in here?
emote scoffs and grins towards his partner.
wait 3 s
mkill man
~
#311
thug 2~
0 g 100
~
wait 3 s
say Should I kill him boss?
wait 1 s
emote flexes and cracks his knuckles.
wait 3 s
emote cackles with glee and begins to pummel the hapless man!
mkill man
~
#312
victim thanks~
0 g 100
~
wait 30 s
emote sighs and gasps in pain.
wait 2 s
emote pulls himself up and offers a bloody hand in thanks.
wait 1 s
emote says, in a ragged whisper, 'I owe you...please take this...'
emote says' 'as a reminder and proof of what Saints must endure.'
wait 2 s
mload obj 353
emote gasps painfully as he wipes a wound with a rag.
wait 2 s
emote wads the rag up and hands it to you.
give rag %actor.name%
wait 3 s
say I am going to rest here a bit, you go on.
mpurge me
~
#313
ransacker 1~
0 g 100
~
wait 1 s
emote looks up, surprised to see you walk in upon them!
wait 3 s
say Well?!? What are you looking at?!
wait 2 s
emote crouches into a defensive position and begins to circle around you!
wait 3 s
emote begins to push and rough you up!
emote smiles as a crazed look fills his eyes.
wait 3 s
massist crazed
~
#314
ransacker 2~
0 g 100
~
wait 4 s
emote giggles as she looks you over.
wait 3 s
emote suddenly frowns and grows angry!
wait 2 s
say You won't tell!!! I won't let you!!!
emote screams in rage and comes flying at you!
wait 1 s
mkill %actor.name%
~
#315
ransacker death~
0 f 100
~
emote screams in agony as his last breath expires.
mload obj 354
~
#316
dying man~
0 g 100
~
wait 3 s
emote moans softly as he struggles in vain to move.
wait 1 s
say Help me please...heal me...rid me of my aliments...
wait 3 s
say The cure is out there, somewhere in the streets...
say I've forgotten what it is, but I know it's there.
wait 5 s
emote sobs softly and rolls on his side, away from you.
wait 2 s
say No one can help me, many have tried and failed.
wait 5 s
emote sighs deeply and fades out of consciousness.
~
#317
dying man healed~
0 j 100
~
if (%object.vnum% == 370)
wait 1 s
emote fumbles with the syringe and nearly drops it!
wait 2 s
emote steadies his hands and injects himself!
junk syringe
wait 4 s
emote seems to be breathing easier now and slowly rolls over to face you.
wait 3 s
emote with a shaky movement, he reaches out his hand in thanks.
emote whispers, 'Thank you kindly...and please take this...
wait 2 s
emote reaches under his diseased coversheet and pulls out something.
mload obj 355
wait 1 s
give linen %actor.name%
wait 2 s
say I am going to rest and then head on home...
say to a family I have not seen in years.
emote smiles happily.
wait 6 s
mpurge me
elseif (%object.vnum% == 369)
wait 2 s
emote slowly props himself up and pauses for a minute.
wait 3 s
emote looks at the potion and begins to cry.
wait 1 s
say This is not the cure I need...
say Please keep looking, I know it's out there on the streets.
wait 3 s
say I cannot remember what it is, but it's there...
wait 2 s
emote lays down and falls into a restless sleep.
~
#318
murderer~
0 g 100
~
wait 3 s
emote 's hand suddenly twitches in a vague stabbing motion!
wait 3 s
say Heh, sorry about that...just nerves.
wait 5 s
emote mutters to no one in general, something about shedding blood?
wait 2 s
emote slowly grins as a maddened look overtakes his face!
wait 3 s
emote screams in bloodlust as he lunges for you!
mkill %actor.name%
~
#319
murderer demon~
0 g 100
~
wait 8 s
emote gleefully cackles as it watches his will slowly break!
~
#320
murderer death~
0 f 100
~
mload obj 356
~
#321
irritating man~
0 g 100
~
wait 2 s
emote spits at you and begins tautnting you!
wait 2 s
say Your mama was an booger beast! Na na na na!
wait 1 s
say Oh wait! I forgot to tell you...
wait 2 s
emote giggles in a very annoying manner.
wait 3 s
say Tell you that you are an ass!
emote laughs wildly while side-slapping his leg in mirth!
wait 3 s
say Oh? What's that?  What are you gonna do about it ehh?
wait 3 s
emote scoffs and sneers at you!
wait 4s
say Figures, you saint-wanna-be people are all wimps!
emote chuckles as if he was superior over you.
wait 3 s
say Had enough?
wait 4 s
emote blows a raspberry at you, spraying spittle all over your face!
wait 3 s
emote breaks out laughing and slaps your back!
wait 5 s
emote sombers up and smile softly.
wait 2 s
say Well done!  You have passed this trial with patience...
say as well as compassion (for you could've simply killed me).
wait 3 s
emote bows at you and turns to go home.
wait 2 s
say Don't underestimate the power of forgivingness.
emote smiles softly.
wait 1 s
mload obj 357
wait 1 s
give epitah %actor.name%
wait 1 s
emote waves good-bye and wishes you luck!
wait 1 s
mpurge me
~
#322
pious man~
0 g 100
~
wait 2 s
emote smiles as he looks up at you.
wait 1 s
say It's been a long time, my friend...
say I bid you welcome.
wait 3 s
say If you can answer this question...
wait 2 s
say You will find yourself closer to salvation.
wait 3 s
emote coughs softly and clears his throat.
wait 1 s
say What saint watches over the Gates of Heaven?
wait 2 s
say ehh?  Saint _________ , fill in the blank.
wait 1 s
emote smiles softly.
wait 2 s
say Here's a hint, it's one of the following...
wait 1 s
say /ccPaul/cw, /ccLuke/cw, /ccJohn/cw, or /ccPeter/cw./c0
~
#323
smiling woman~
0 g 100
~
wait 2 s
emote stops whatever she was doing and looks at you.
wait 3 s
emote mumbles something about what a kind soul you are.
wait 3 s
say Yes, child, I can see deep into your soul...
say and see if your heart is true.
wait 2 s
say I would like to help you...
say achieve your dream of being a Saint.
wait 5 s
emote smiles and leans close to your ear.
wait 1 s
emote whispers, 'Answer me this...
wait 3 s
emote whispers, 'Who is the greatest angel of all?'
wait 1 s
emote smiles and moves a few paces away from you.
wait 2 s
say Here is a hint...
say It's one of the following -
wait 3 s
say /ccMichael/cw, /ccGabriel/cw, or /ccMathias/cw./c0
wait 4 s
emote smiles and patiently awaits your answer.
~
#324
guardian angelic~
0 g 100
~
wait 1 s
say No passport, no go!
wait 1 s
emote steps back and stands solidly.
~
#325
supreme one~
0 g 100
~
wait 3 s
emote swirls about, coelescing into a vage form.
wait 2 s
emote gently says, 'Welcome child, you have done us proud...
say and you are now worthy to be a saint.
wait 5 s
say Remember, keep to your ways as a Saint...
say and always seek to do good unto others!
wait 3 s
emote smiles as energy crackles about it!
wait 2 s
say Take this and wear it with pride!
wait 1 s
mload obj 361
wait 1 s
give angel %actor.name%
wait 2 s
smile %actor.name%
hug %actor.name%
wait 5 s
say Of the three, faith, hope, and love...
wait 4 s
say the greatest of these is love.
mteleport %actor.name% 3096
wait 3 s
mpurge me
~
#326
pious man answer~
0 d 100
paul luke john peter~
if (%speech% == paul)
wait 1 s
emote whispers, 'Perhaps next time, my child.'
mteleport %actor.name% 3096
wait 3 s
mpurge me
elseif (%speech% == luke)
wait 1 s
say Alas, that is not the correct answer, my child.
mteleport %actor.name% 3096
wait 3 s
mpurge me
elseif (%speech% == john)
wait 1 s
say My child, to be a Saint, you need to know more.
mteleport %actor.name% 3096
wait 3 s
mpurge me
elseif (%speech% == peter)
wait 1 s
emote smiles as a bright light surrounds him.
say There, there now...
say You are indeed worthy.
wait 2 s
say Please take this token to help you to the next quest.
wait 1 s
mload obj 359
wait 1 s
give passport %actor.name%
wait 3 s
emote smiles and gives you his blessings.
wait 2 s
emote waves and disappears into the mist in the park!
wait 1 s
mpurge me
~
#327
smiling woman answer~
0 d 100
michael gabriel mathias~
if (%speech% == michael)
wait 1 s
say He is powerful indeed, but not the greatest of all.
mteleport %actor.name% 3096
wait 3 s
mpurge me
elseif (%speech% == mathias)
wait 1 s
emote sighs softly and turns away.
say Mathias is not even one of the Archangels.
mteleport %actor.name% 3096
wait 3 s
mpurge me
elseif (%speech% == gabriel)
wait 1 s
emote smiles and takes your hand.
wait 2 s
say Well done!  You are worthy.
wait 1 s
say Please take this key...
say and continue on your quest to be a Saint.
wait 2 s
mload obj 358
wait 1 s
give key %actor.name%
wait 2 s
emote smiles and walks away.
wait 1 s
mpurge me
~
#328
angelic guardian receive~
0 j 100
~
if (%object.vnum% == 359)
wait 1 s
nod %actor.name%
wait 2 s
say You may pass.
wait 1 s
mload obj 360
unlock heavens
open heavens
wait 2 s
emote steps aside and respectfully lets you through.
wait 1 s
junk passport
junk key
emote disappears in a flash of light!
mpurge me
~
#329
guiding angel~
0 g 100
~
wait 1 s
smile
wait 1 s
curtsey
wait 3 s
say Welcome child, please take the time to listen to me.
wait 2 s
say Ahead of you, you will find a city...
say a once-proud city, now in ruins.
wait 3 s
say Along the way...
say you will see various people in need of aid.
wait 3 s
say You are, after all, aiming for Sainthood...
wait 2 s
say so keep that in mind.
wait 2 s
say Please keep an open mind and pay attention...
say and expect the unexpected.
wait 1 s
emote smiles and gives you her blessings.
wait 3 s
say Once again, remember what you are striving to be...
say for that is what will dictate your actions.
wait 3 s
say Godspeed and good luck!
wait 2 s
emote floats away, higher and higher into the heavens!
mpurge me
~
#330
homeless man crap~
0 j 100
~
if (%object.vnum% == 368)
wait 1 s
emote gags in complete disgust and throws it aside!
drop mass
wait 2 s
say Have you no heart?!?  only cruelty?!?
wait 1 s
sob
wait 3 s
say Please leave me alone, if that is all you have to give.
~
#331
dying man wrong stuff~
0 j 100
~
if (%object.vnum% == 369)
wait 1 s
emote slowly props himself up and pauses for a minute.
wait 3 s
emote looks at the potion and begins to cry.
wait 1 s
say This is not the cure, please keep looking...
wait 3 s
say I know it's out there on the streets somewhere.
wait 3 s
junk potion
wait 4 s
emote collapses and goes back to a restless sleep.
~
#332
waybread created~
0 j 100
None~
 if (%object.vnum% == 10)
   wait 2 s
   say Oh, thank you good sir! Thank you!
   wait 2 s
   mecho A bit of drool slips from the corner of a homeless man's mouth.
   wait 2 s
   emote fishes around in his tattered clothing.
   wait 3 s
   mecho With a flourish, a homeless man brandishes what appears to be a tin can!
   wait 1 s
   emote smiles proudly.
   wait 1 s
   say Here you go! And many thanks!
   wait 2 s
   mload obj 350
   give can %actor.name%
~
#333
waybread2~
0 j 100
None~
   if (%object.vnum% == 1009)
   wait 2 s
   say Oh, thank you good sir! Thank you!
   wait 2 s
   mecho A bit of drool slips from the corner of a homeless man's mouth.
   wait 2 s
   emote fishes around in his tattered clothing.
   wait 3 s
   mecho With a flourish, a homeless man brandishes what appears to be a tin can!
   wait 1 s
   emote smiles proudly.
   wait 1 s
   say Here you go! And many thanks!
   wait 2 s
   mload obj 350
   give can %actor.name%
~
$~
