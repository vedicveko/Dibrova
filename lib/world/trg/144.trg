#14458
Greeting~
0 g 100
~
wait 2 s
emote smiles slyly, welcoming you to the Castle of Souls.
wait 1 s
emote snickers softly to himself.
wait 2 s 
emote grins evily and mockingly salutes you!
wait 1 s
emote shrugs and ignores your insignificant presence.
~
#14474
Blacksmith~
0 h 100
None~
   wait 1 s
   say Hello %actor.name% how are you today?
   wait 1 s
   say Would you by chance have seen a dragon scale lying around?
   wait 2 s 
   say Please respond to the first question with a yes or no. And to the second with a yea or nay
   wait 1 s
~
#14475
respond ~
0 d 100
yes no~
if (%speech% == yes)
wait 1 s
say If you don't mind parting with it. I could probably creat something useful for you.
wait 2 s
elseif (%speech% == no)
say Well if you happen to find one. I wouldn't mind taking it off of your hands.
~
#14476
respond and receive~
0 dj 100
yea nay~
   if (%speech% == yea)
   wait 1 s
   say Well can I have it then please.
       if (%object.vnum% == 14453)
      wait 2 s
      echo starts to hammer the scale and combining it with metal.
      wait 4 s
      say %actor.name% Thank-you kindly.  For giving this to me and here you go.
      mload obj 14474
      give plate %actor.name%
      wait 1 s
      say %actor.name% have a good day now.
      elseif (%object.vnum% == 4816)
    wait 1 s
    say Well it doesn't look like a good quality of scale but i will see what can be done.
    wait 3 s
    echo starts to hammer on the scale and as he does you can here him start to curse.
    wait 2 s
    say %actor.name% sorry but the scale was of very poor quality and it broke.
    wait 1 s
    mload obj 14447
    drop obj 14447
   elseif (%speech% == nay)
   wait 2 s
   say Well if you happen to change your mind let me know.
~
#14477
respond 2~
0 d 100
yea nay~
   if (%speech% == yea)
   wait 1 s
   say Well can I have it then please.
   elseif (%speech% == nay)
   wait 2 s
   say Well if you happen to change your mind let me know.
~
#14478
receive~
0 j 100
~
if (%object.vnum% == 14453)
wait 1 s
emote starts to hammer the scale and combining it with metal.
wait 4 s
say %actor.name% Thank-you kindly.  For giving me the scale here you go.
mload obj 14474
give plate %actor.name%
mpurge scale
      wait 1 s
      say %actor.name% have a good day now.
      elseif (%object.vnum% == 4816)
    wait 1 s
    say Well it doesn't look like a good quality of scale but i will see what can be done.
    wait 3 s
    echo starts to hammer on the scale and as he does you can here him start to curse.
    wait 2 s
    say %actor.name% sorry but the scale was of very poor quality and it broke.
    wait 1 s
    mload obj 14447
    drop obj 14447 
~
$~
