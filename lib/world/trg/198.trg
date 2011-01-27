#19801
Golem nr1~
0 g 100
~
wait 2
look %actor.name%
say You're not a golem!
if %direction%
wait 5
kill %actor.name%
else
kill %actor.name%
end
~
#19802
Golem nr2~
0 g 100
~
wait 5
consider %actor.name%
wait 5
think
wait 5
say Thou are a fiend to The Osididan Golems, %actor.name%!
wait 5
say I shall kill you!
wait 5
kill %actor.name%
end
~
#19803
Golem, Keeper of Secret~
0 d 1
can i help you~
wait 10
mecho /cwThe keeper of secrets says, '/crAhh, a brave mortal!/cw'/c0
wait 15
mecho /cwThe keeper of secrets says, '/crI have been waiting for one like you, I need your service./cw'/c0
wait 10
mecho /cwThe keeper of secrets says, '/crLong ago, when forces of evil tried to exterminate the golem race, a secret book was given to a great sorceror who were on our side./cw'/c0
wait 20
mecho /cwThe keeper of secrets says, '/crThere were old, ancient, secrets about golems written in that book. Finding it is a priority for the whole race./cw'/c0
wait 15
mecho /cwThe keeper of secrets says, '/crHowever, the old sorceror who the book was given to died 296 years ago when trying to defeat a great Demon./cw'/c0
wait 10
mecho /cwThe keeper of secrets says, '/crThe only clue we have is a prophet, who has read the birth of the sorceror's son in the stars above./cw'/c0
wait 15
mecho /cwThe keeper of secrets says, '/crPlease find the sorcerors son, and maybe he can tell you where his father hid the book./cw'/c0
wait 5
mecho /crThe Golem Keeper of Secrest bows before you./c0
end
~
#19804
Golem Socials~
0 b 20
~
if (%random.6% == 1)
growl
else if (%random.6% == 2)
grin
else if (%random.6% == 3)
think
else if (%random.6% == 4)
snicker
else if (%random.6% == 5)
muttar
else if (%random.6% == 6)
yawn
end
~
#19805
Golem Attacks~
0 g 100
~
if (%random.10% == 5)
scan
say aha!
kill %actor.name%
end
~
$~
