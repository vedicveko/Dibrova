#6000
Aquarius~
0 j 100
~
if (%object.vnum% == 6051)
emote smiles gently at you.
emote thanks you wholeheartedly.
say See if you can find the owner of this talisman...
mload obj 6052
give pisces %actor.name%
else
emote looks upon you and sighs sadly.
emote claps your hand and softly shakes his head.
say You are not ready to be one of us.
emote blurs and fades from view, smiling sadly.
mteleport %actor.name% 3096
endif
~
#6001
Aye Trigger~
0 c 1
enter~
if (%arg% == portal)
   *enter correct commands here
   say testing, trigger called fine
else
   *do nothing
   break
end
~
#6002
Stheno~
0 g 100
~
wait 1 s
emote crawls forth, the writhing snakes on her head hissing loudly!
wait 2 s
emote thrashes her tail in disgust upon looking at you!
wait 1 s
say Sssssssso, Yousssss gonna be a Titan?
say Well, then...we, the three gorgons, offer any aid you can use.
emote hisses at her sisters, approaching the edge of the Unseen.
wait 3 s
say I am Stheno, at your sssservice sssshould you become a Titan.
wait 3 s
emote turns and slithers off into the Unseen as Euryale slithers forth!
mload mob 6069
mpurge me
~
#6003
Euryale~
0 b 100
~
wait 1 s
say It'ssss been a while ssssince I've ssssseen a mortal make it thisss far.
wait 1 s
emote tantalizingly slides up to you and gives you a hug!
wait 3 s
say Answer thisss and you sssshall be one ssstep closer to the chalice!
wait 2 s
say Of the three, which one of us is mortal?
say /cwWould it be /ccStheno/cw, me (/ccEuryale/cw), or her (/ccMedusa/cw)./c0
wait 1 s
emote slides into the Unseen as Medusa slithers forth!
mload mob 6070
mpurge me
~
#6004
Medusa~
0 d 100
stheno euryale medusa~
if (%speech% == stheno)
wait 1 s
emote hisses, her eyes blaze brightly as you turn into stone!
mteleport %actor.name% 3096
wait 3 s
mload mob 6068
mpurge me
elseif (%speech% == euryale)
wait 1 s
emote switfly draws her bow and smites you with a venom-tipped arrow!
mteleport %actor.name% 3096
wait 3 s
mload mob 6068
mpurge me
elseif (%speech% == medusa)
wait 1 s
emote smiles softly and warmly takes your hand.
say The last path to being a Titan will be hard...
wait 2 s
say but I have faith in you.
mteleport %actor.name% 6096
wait 3 s
mload mob 6068
mpurge me
endif
~
#6005
Pephredo~
0 g 100
~
wait 1 s
emote shuffles forth to inspect you closer.
emote sniffs with disdain and holds her eye close to your face.
wait 2 s
say Hmm, here be a mere mortal with great aspirations!
wait 1 s
say Pephredo, one of the three Graiae, at your service.
emote clumsily curtseys before you.
wait 3 s 
say Hrmph, my sister wishes to meet you.
emote retreats into the Unseen, giving the eye to the approaching Euryale.
mload mob 6072
mpurge me
~
#6006
Enyo~
0 b 100
~
wait 1 s
emote cackles gleefully as she looks thru her eye at you!
wait 2 s
say I am Enyo, one of the three Graiae.  The pleasure is mine.
emote slowly pulls away from you and heads for the Unseen.
wait 1 s
say Before I leave, and Deino comes forth, answer me this:
wait 4 s
say Which Titan was cruely punished for aiding Cronos against the Gods?
wait 2 s
say /cwIs it /ccEpimetheus/cw, /ccPrometheus/cw, or /ccAtlas/cw?/c0
wait 1 s
emote snickers as she steps into the Unseen and Deino steps forth.
mload mob 6073
mpurge me
~
#6007
Deino~
0 d 100
epimetheus prometheus atlas~
if (%speech% == epimetheus)
wait 1 s
emote sighs forlornly and bids you goodbye.
emote whispers, 'Perhaps next time, my child.'
mteleport %actor.name% 3096
wait 3 s
mload mob 6071
mpurge me
elseif (%speech% == prometheus)
wait 1 s
say Alas, that is not the correct answer, my child.
emote sadly looks at you as you are swept away!
mteleport %actor.name% 3096
wait 3 s
mload mob 6071
mpurge me
elseif (%speech% == atlas)
wait 1 s
say Yes, my child, that is the Titan who suffers everyday.
say He bears the great weight of the heavens and earth upon his shoulders.
emote pauses for a silent moment.
wait 3 s
say Come child, one last quest awaits thee...
emote smiles tenderly upon you and bids you good luck!
mteleport %actor.name% 6096
wait 3 s
mload mob 6071
mpurge me
endif
~
#6020
Typhon~
0 g 100
~
emote rears up, a towering mass of pure terror!
wait 2 s
emote lowers down to get a good look at you.
emote growls, it's eyes blazing with ancient hatred!
wait 1 s
say /cwWelcome weakling!  Take a good look around you (type /ccexit/cw)!
wait 1 s
say Choose one direction, based upon your confidence in your
say knowledge of the lores regarding the Titans!
wait 2 s
say Be WARNED! Once you proceed in a direction, there is NO return!
wait 3 s
emote laughs mightily, shaking the very heavens and earth!
emote dives into the wellspring and vanishes from sight!
mpurge me
~
#6060
Polaris~
0 hi 100
~
if (%actor.vnum% == -1)
wait 1 s
say Welcome to the Zodiac and the challenges that lie
say within.  You have come a long ways, alas, much more
say is yet to come.  Take the time to prepare for the
say rigors that lie ahead.  When you are ready and up
say to the tasks, enter the rip in the void.
wait 1 s
say Before you go, I have an ancient talisman that needs
say to be return to it's rightful owner.  Please take
say it and be honored upon doing so.
wait 2 s
say /cwSimply /ccgive /cwthe talisman to the owner./c0
mload obj 6051
give aquarius %actor.name%
wait 2 s
say Godspeed and good luck to you my friend.
emote smiles and waves you on your way.
wait 3 s
mpurge me
~
#6064
Capricorn~
0 j 100
~
if (%object.vnum% == 6063)
emote cheers wildly!
say Congratulations! for completing the first set of trials!
emote gestures and smiles as a rip in space opens up.
say Fare thee well and be prepared!
say For the last trial will be the hardest! and perhaps fatal.
emote respectfully bows before you and prances away.
mteleport %actor.name% 6092
else
emote groans loudly!
say Once more, I shall not be deceived!  Be gone!
emote gives you a swift kick in the behind!
mteleport %actor.name% 3096
endif
~
#6065
Libra~
0 j 100
~
if (%object.vnum% == 6060)
emote nods sagely.
say It's been a while, but with this, balance shall be restored.
say See if you can find the owner of this talisman...
mload obj 6061
give scorpio %actor.name%
else
emote draws out her great broadsword with a deadly finesse.
emote points the tip of her blade at your throat!
say Perhaps when the scales tip in your favor, I'll aid you then.
emote thrusts at you with blinding speed, making you scream!
mteleport %actor.name% 3096
endif
~
#6066
Pisces~
0 j 100
~
if (%object.vnum% == 6052)
emote beams a huge smile at you!
say I have been looking for this talisman for eons!
emote happily thanks you!
say See if you can find the owner of this talisman...
mload obj 6053
give aries %actor.name%
else
emote heaves a great sigh and shakes his head gently.
say Alas my friend, this talisman belongs to another.
say You are not ready to be one of us.
emote sadly waves and turns away, fading from view.
mteleport %actor.name% 3096
endif
~
#6067
Cancer~
0 j 100
~
if (%object.vnum% == 6057)
emote clicks and clacks his claws together in excitement!
emote whispers "Where in the world did you find this?!?"
emote smiles and thanks you wholeheartedly!
say See if you can find the owner of this talisman...
mload obj 6058
give leo %actor.name%
else
emote angrily snaps at you with his giant claws!
say Tsk-tsk, I expected much better from you!
emote frowns and takes one last parting shot at you!
emote scuttles out of sight in a blink of an eye!
mteleport %actor.name% 3096
endif
~
#6072
Saggitarius~
0 j 100
~
if (%object.vnum% == 6062)
emote beams a huge smile and laughs out aloud!
say Well done! and thank you for the talisman.
emote prances over and firmly shakes your hand.
say See if you can find the owner of this last talisman...
mload obj 6063
give capricorn %actor.name%
else
emote takes a few steps back and rears up his bow!
emote notches an arrow and lets it fly!
say Do not despair.  Success comes to those who persist!
emote smiles forlornly as the arrow strikes you!
mteleport %actor.name% 3096
endif
~
#6073
Taurus ~
0 j 100
~
if (%object.vnum% == 6054)
emote grins and accepts the talisman.
emote lowers his massive head in respect and than looks up.
say See if you can find the owner of this talisman...
mload obj 6055
give one %actor.name%
else
emote turns away, his great horns almost gorging you!
say When you are worthy, come back again...
say meanwhile, do not waste my time or feel my wrath!
emote ambles off and disappears into the horizon.
mteleport %actor.name% 3096
endif
~
#6074
Scylla~
0 g 100
~
wait 3 s
emote roars, her multiple heads writhing in anticipation!
say Welcome! and congratulations on passing the first stage of the quest.
emote snarls angrily as one of her many heads errantly snaps at you!
wait 1 s
say I am Scylla, one of the terrors of the seas.  Look upon me and fear me!
emote looks upwards to the heavens and cackles with glee!
wait 2 s
say Before I let you go on to the final stage of the quest...
say Answer me this:
wait 1 s
say /cwAre Charybdis and I sisters in blood? /ccYes /cwor /ccno/cw?/c0
wait 3 s
emote takes one last look and dives into the depths of the sea!
mload mob 6075
mpurge me
~
#6075
Charybdis~
0 d 100
yes no~
if (%speech% == yes)
wait 1 s
emote growls and snarls in rage!
emote lunges at you, all hundred heads snapping at you!
emote tears you into pieces and devours you!
mteleport %actor.name% 3096
wait 3 s
mload mob 6074
mpurge me
elseif (%speech% == no)
wait 1 s
emote smiles, all one hundred heads sporting a toothy grin.
emote chuckles loudly, the sound reverbrating over the seas!
wait 1 sec
say Well done fledging, go on ahead to the final stage of the quest.
emote smiles one last time and disappears into the depths of the sea!
mteleport %actor.name% 6096
wait 3 s
mload mob 6074
mpurge me
endif
~
#6078
Scorpio~
0 j 100
~
if (%object.vnum% == 6061)
emote relaxes and lowers the deadly stinger.
say Welcome, just a few more challenges to go.
emote smiles happily.
say Oh, and thank you for returning my talisman.
say See if you can find the owner of this talisman...
mload obj 6062
give saggitarius %actor.name%
else
emote growls softly and becomes agitated.
emote wickedly jabs at you with it's pinchers!
say Get thee hence!
emote strikes at you with it's stinger, only to fade away.
mteleport %actor.name% 3096
endif
~
#6079
Virgo~
0 j 100
~
if (%object.vnum% == 6059)
emote smiles seductively and gently takes your hand.
emote comes tantalizingly close and places a kiss on your cheek.
say Thank you for this gift.  Another time, I shall return the favor.
say See if you can find the owner of this talisman...
mload obj 6060
give libra %actor.name%
else
emote pouts and whines softly.
say I wish you were the one, for I have much to show you.
emote stands up and slowly gravitates away.
mteleport %actor.name% 3096
endif
~
#6083
Aries~
0 j 100
~
if (%object.vnum% == 6053)
emote lows and sends forth a deafening bellow!
say It's been a long time since a worthy applicant has
say passed this way.
say See if you can find the owner of this Talisman...
emote smiles and snorts for good luck!
mload obj 6054
give taurus %actor.name%
else
emote snorts and bellows in rage!
emote scoffs at you!
say You are not worthy of the honor of being a Titan!
emote roars and charges, horns lowered and hooves flailing at you!
mteleport %actor.name% 3096
endif
~
#6085
Gemini smiling~
0 j 100
~
if (%object.vnum% == 6055)
emote smiles and gives you a hug!
say Thank you, my valiant one!
say See if you can find the owner of this talisman...
mload obj 6056
give malignant %actor.name%
mload mob 6056
mpurge me
else
emote frowns and tries to hold back a tear.
emote turns away, stiffling her sobs.
emote waves her hand at you, dismissing you.
emote fades out of sight behind a watery veil.
mteleport %actor.name% 3096
endif
~
#6086
Gemini snarling~
0 j 100
~
if (%object.vnum% == 6056)
emote snarls once more and smiles slyly.
emote hugs you, as if the two of you have some sinister bond.
say See if you can find the owner of this talisman...
mload obj 6057
give cancer %actor.name%
else
emote grins manically and laughs evily!
emote giggles as she dances about insanely!
say Come back another time, my dear, should you EVER get this right!
emote smiles broadly as she disappears behind a veil of darkness.
mteleport %actor.name% 3096
wait 3 s
mload mob 6055
mpurge me
endif
~
#6087
Leo~
0 j 100
~
if (%object.vnum% == 6058)
emote growls, which slowly turns into blissful purring!
say It has been centuries!  Never thought I would ever get this back.
emote playfully cuffs you with his giant paw, claws retracted!
say See if you can find the owner of this talisman...
mload obj 6059
give virgo %actor.name%
else
emote snarls and goes berserk!
emote leaps at you, ready to land the fatal blow!
say You are not ready to be a Titan.
say Do not bring dishonor upon us, should you fail again!
emote growls deeply, shaking the very fabric of the cosmos.
mteleport %actor.name% 3096
endif
~
#6088
Typhon~
0 b 100
~
say My trigger commandlist is not complete!
~
#6090
Alecto~
0 g 100
~
wait 3 s
say Hello
emote grins slyly and looks towards her unseen sisters!
wait 1 s
say We are known as the Furies, woe to those who stir our wrath!
say I am Alecto, the One who never Rests.
wait 1 s
emote curtseys before you and fades into the Unseen.
mload mob 6080
mpurge me
~
#6091
Magaera~
0 b 100
exa magaera~
wait 1 s
say Congratulations upon making it this far...
emote smiles sincerely, then grins sinisterly!
wait 1 s
say I am Magaera, the Jealous One.
slyly looks at you and acknowledges your presence.
wait 1 s
say Before you can continue on, answer me this question:
wait 3 s
say Are we the /ccfirst/cw, /ccsecond/cw, /ccthird/cw, or the /cclast/cw
say offspring of Uranus?
wait 2s
emote fades into the Unseen as Tisiphone, the Avenger, strides forth!
mload mob 6081
mpurge me
~
#6092
Tisiphone~
0 d 1
first second third last~
if (%speech% == first)
wait 1 s
emote hurls herself at you and slays you!
mteleport %actor.name% 3096
wait 3 s
mload mob 6079
mpurge me
elseif (%speech% == second)
wait 1 s
emote swings her terrible sword at you, slicing you in half!
mteleport %actor.name% 3096
wait 3 s
mload mob 6079
mpurge me
elseif (%speech% == third)
wait 1 s
emote snarls and tears you apart in a rage of fury!
mteleport %actor.name% 3096
wait 3 s
mload mob 6079
mpurge me
elseif (%speech% == last)
wait 1 s
emote relaxes and smiles gently at you.
say That is correct! Good luck with the final stage of the quest!
mteleport %actor.name% 6094
wait 3 s
mload mob 6079
mpurge me
endif
~
#6093
Arges~
0 g 100
~
wait 1 s
emote grumbles something incomprehensible under his breath.
say Arrghcome shu aur ummm!
wait 2 s
emote hacks and clears his throat!
wait 1 s
emote spits out a humanoid legbone and grins wickedly!
wait 2 s
say Welcome to me home!
say Me and me brothers, us three, our home!
wait 2 s
say Arrrgh! Steropes comes!
emote thumps his chest as he vanishes into the Unseen!
mload mob 6077
mpurge me
~
#6094
Steropes~
0 b 100
~
wait 1 s
emote ambles forth, muttering something nasty under his breath!
wait 2 s
say Why, uhh, you like a fly?!?!
say Buh-ther me like bug!  Me no like bugs!
wait 1 s
growls softly as he eyes you.
wait 2 s
emote scratches his head, scattering a horde of fleas!
wait 1 s
emote mutters softly, 'Me forget! Arrrgh!   No 'member tha qwes-shun!'
wait 3 s
say Ahh-ehh Me cyclop! Me one of three!
wait 3 s
say Know this?!? if no, me squish you like bug!!
emote roars in anticipation!
wait 2 s
say Who come our home? Stick hot thing in brutha's eye?!
wait 3 s
say /cwBe /ccJason/cw, /ccHector/cw, or uhhh /ccHercules/cw?/c0
wait 1 s
emote shuffles off into the Unseen as Brontes approaches.
mload mob 6078
mpurge me
~
#6095
Brontes~
0 d 100
jason hector hercules~
if (%speech% == hercules)
wait 1 s
say Die!!!! Argh!
emote crushes you into a bloody pulp!
mteleport %actor.name% 3096
wait 3 s
mload mob 6076
mpurge me
elseif (%speech% == hector)
wait 1 s
emote screams in rage and smashes the life out of you!
mteleport %actor.name% 3096
wait 3 s
mload mob 6076
mpurge me
elseif (%speech% == jason)
wait 1 s
emote whoops in joy and clasps your hand!
say You are WIN!!!
wait 2 s
emote smiles and wishes you a good journey as he lumbers into the Unseen.
mteleport %actor.name% 6096
wait 3 s
mload mob 6076
mpurge me
endif
~
#6096
Briareus~
0 g 100
~
wait 2 s
emote lumbers forth, a mass of rippling muscle!
wait 1 s
emote says in a rumbling voice, 'Welcome to our prison!'
say We have waited eons for someone like you, to pay us a visit.
wait 1 s
say I am Briareus, the Strong, origin of earthquakes!
emote smiles as the very foundation of the earth begins to shake and rumble!
say I am but one of the three powerful Hecatoncheires.
wait 2 s
emote waves and lumbers off as Cottus approaches from the Unseen.
mload mob 6066
mpurge me
~
#6097
Cottus~
0 b 100
~
wait 1 s
say Hello, I am Cottus, the Striker.
emote smiles and bolts of lightning leap from the heavens!
wait 2 s
say This is our question for you.
say Answer this correctly and we will willingly let thee pass onto
say the last stage of your quest.
wait 3 s
say /cwWho enlisted our help in defeating the Titans.../c0
say /cwand then betrayed us by returning us to this inescapable prision?/c0
wait 2 s
say /cwWas it /ccHercules/cw, /ccZeus/cw, or /ccCronos/cw?/c0
emote growls and fades into the Unseen as Gyes leaps forth!
mload mob 6067
mpurge me
~
#6098
Gyes~
0 d 100
hercules zeus cronus~
if (%speech% == hercules)
wait 1 s
emote roars in rage as a terrible tornado sweeps forth and smites you!
mteleport %actor.name% 3096
wait 3 s
mload mob 6065
mpurge me
elseif (%speech% == cronus)
wait 1 s
say Alas, we do not tolerate those who do not truly know who we are!
emote growls and crushes you with a blast of arctic wind!
mteleport %actor.name% 3096
wait 3 s
mload mob 6065
mpurge me
elseif (%speech% == zeus)
wait 1 s
emote snarls in rage and offers his hand in a solemn vow.
wait 1 s
say Call upon us my friend, should you pass the last stage of the quest,
say and we shall be there for you!
emote bows respectfully before you and whirls away into the Unseen
mteleport %actor.name% 6096
wait 3 s
mload mob 6065
mpurge me
endif
~
#6099
Cronos~
0 g 100
~
wait 1 s
emote shifts his massive frame to get a better look at you.
wait 3 s
emote ponders deeply for a moment or two.
wait 3 s
say So...here you are, having passed all the prior challenges.
emote smiles happily and lifts you in a great bearhug!
wait 2 s
say Kneel before me! for thou art worthy of being a Titan!
wait 4 s
emote slowly reaches deep into the gaping wound in his chest...
wait 3 s
emote and pulls out the fabled Chalice of the Titans!
mload obj 6098
wait 4 s
say Here, drink from this and welcome to the family!
give chalice %actor.name%
wait 4 s
wecho /cYBEHOLD!! Welcome /cC%actor.name% /cY, the newest /cRTITAN/cY in Dibrova!/c0
mteleport %actor.name% 3096
wait 3 s
mpurge me
~
$~
