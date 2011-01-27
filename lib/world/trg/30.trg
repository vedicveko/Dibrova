#3000
newbie helper login~
0 e 100
has entered the realm.~
if (%actor.level% <= 10 && %actor.vnum% == -1)
wait 2 s
bow %actor.name%
wait 3 s
say I am at your service, %actor.name%. Type the word 'newbie' and I will transport you to realms filled with creatures which you may slay without fear of death.
else
return
~
#3001
trash picker-upper~
0 b 5
~
emote mumbles something quietly about the baker's wife, 'that bitch'.
~
#3002
zifnab~
0 b 2
~
look %actor.name%
wait 3 s
say Well, %actor.name%, ye been in here quite enough time before to know what is and isn't for sale.
wait 2 s
say If'n ya want to buy somethin' go ahead 'n buy it fer cryin out loud!
~
#3003
newbie city note~
1 b 10
~
%send% %actor% You notice that you are carrying a note with some directions on it.
~
#3004
citizen flee~
0 e 0
hits~
emote gives a frightened shriek and runs like hell.
say Fight! Fight!
e
e
e
s
s
s
e
e
e
s
s
s
e
e
e
s
s
s
~
#3005
warren baseball ask~
0 b 10
~
say Hav-have you seen my baseball?
~
#3006
warren ball receive 1~
0 j 100
~
if (%object.vnum% == 26190)
  say My baseball!
  hug %actor.name%
  mqpadd %actor.name% %actor.qpoints(10)%
  wait 3 s
  say There you go - ten big honking quest points, all for you!
wait 2 s
  emote grins sloppily, a bit of spittle drooling out his mouth.
  junk baseball
  wait 2 s
say Ooops...
~
#3007
warren ball receive 2~
0 j 100
~
if (%object.vnum% == 26191)
 say My baseball!
 hug %actor.name%
 mqpadd %actor.name% %actor.qpoints(10)%
 wait 3 s
 say There you go - ten big honking quest points, all for you!
 wait 2 s
 emote grins sloppily, a bit of spittle drooling out his mouth.
 junk baseball
 wait 2 s
 say Ooops...
~
#3008
warren ball receive 3~
0 j 100
~
if (%object.vnum% == 26192)
 say My baseball!
 junk baseball
 hug %actor.name%
 mqpadd %actor.name% %actor.qpoints(10)%
 wait 3 s
junk baseball
 say There you go - ten big honking quest points, all for you!
 wait 2 s
junk baseball
 emote grins sloppily, a bit of spittle drooling out his mouth.
 say Ooops...
~
#3009
Newbie Guide Loader~
2 c 100
assist~
wait 1 s
wforce Orian say Ahh of course, let me call a guide to assist you.
wait 1 s
if (%actor.class% == Thief)
   wforce orian emote calls for the guide to the thieven guild.
   wait 2 s
   wload mob 3081
   wecho Imoen glances what looks to be a plesent smile at you as she enters the room.
   wait 1 s
   wforce orian emote gives a small pouch of coins to Imoen.
   wait 2 s
   if (%actor.sex% == MALE)
      wforce Imoen say Evening squire, i hear you'd like a tour around town.
   elseif (%actor.sex% == FEMALE)
      wforce Imoen say Evening Mam, i hear you'd like a tour around town.
   else
      wforce Imoen say Evening ummm, what are you anyhow?
   end
   wait 1 s
   wforce Imoen emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Imoen say Well, the coins good and my fees paid, so if you'd care to
   wforce Imoen say follow me, we'll be off, oh, and dont fret, i wont steal from you.
   wait 1 s
   wforce imoen smile %actor.name%
   wait 2 s
elseif (%actor.class% == Cleric)
   wforce orian emote calls for the guide to the cleric guild.
   wait 2 s
   wload mob 3086
   wforce Gaellyn enters the room with a plesant smile on her face.
   wait 1 s
   wforce Gaellyn embrace %actor.name%
   wait 1 s
   wforce orian emote gives a small pouch of coins to Gaellyn.
   wait 2 s
   if (%actor.sex% == MALE)
      wforce Gaellyn say Evening Sir, I hear you'd like a tour around town.
   elseif (%actor.sex% == FEMALE)
      wforce Gaellyn say Evening Madam, I hear you'd like a tour around town.
   else
      wforce Gaellyn say Evening ummm, what are you anyhow?
   end
   wait 1 s
   wforce Gaellyn emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Gaellyn say A handsome donation has been made to the guild, so if you'd like
   wforce Gaellyn say to follow me, we'll go on our way.
   wait 1 s
   wforce Gaellyn smile %actor.name%
   wait 2 s
elseif (%actor.class% == Sorcerer)
   wforce orian emote calls for the guide to the sorcerer's guild.
   wait 2 s
   wload mob 3083
   wforce Elenydd emote enters the room with a smirked expresion on his face.
   wait 1 s
   wforce orian emote gives a small pouch of coins to Elenydd.
   wait 2 s
   wforce Elenydd say so you'd like a tour around town?
   wait 1 s
   wforce Elenydd emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Elenydd say A donation has been made, it should surfice, so if you'd like
   wforce Elenydd say to follow me, we'll go on our way.
   wait 2 s
elseif (%actor.class% == Dark Knight)
   wforce orian emote calls for the guide to the dark knights guild.
   wait 2 s
   wload mob 3084
   wforce Elvuldor enters the room with an insane look on his face.
   wait 1 s
   wforce Elvuldor cackle
   wait 1 s
   wforce orian emote gives a small pouch of coins to Elvuldor.
   wait 2 s
   if (%actor.sex% == MALE)
      wforce Elvuldor say So boy, I hear you'd like a tour around this here town.
   elseif (%actor.sex% == FEMALE)
      wforce Elvuldor say So lassy, I hear you'd like a tour around this here town.
   else
      wforce Elvuldor say So... ummm, what are you anyhow?
   end
   wait 1 s
   wforce Elvuldor emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Elvuldor say Well the coins good even if the ammounts not, so if you'd like
   wforce Elvuldor say to follow me, we'll get this over and done with as painfully as
   wforce Elvuldor say possible.
   wait 1 s
   wforce Elvuldor grin %actor.name%
   wait 2 s
elseif (%actor.class% == Monk)
   wforce orian emote calls for the guide to the Monks guild.
   wait 2 s
   wload mob 3085
   wforce Trirewyr enters the room with a stange look on his face.
   wait 1 s
   wforce Trirewyr chuckle
   wait 1 s
   wforce orian emote gives a small pouch of coins to Trirewyr.
   wait 2 s
   wforce Trirewyr say I hope you realise i break a vow of silence to do this?
   wforce Trirewyr say Anyway I hear you'd like a tour around the city.
   wait 1 s
   wforce Trirewyr emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Trirewyr say A handsome donation to the guild, If you'd like to follow me,
   wforce Trirewyr say we'll go for a stroll around town, and then out to the field.
   wait 1 s
   wforce Trirewyr smile %actor.name%
   wait 2 s
elseif (%actor.class% == Gladiator)
   wforce orian emote calls for the guide to the gladiator's guild.
   wait 2 s
   wload mob 3087
   wecho Relakor glances what looks to be a plesent smile at you as he enters the room.
   wait 1 s
   wforce orian emote gives a small pouch of coins to Relakor.
   wait 2 s
   if (%actor.sex% == MALE)
      wforce Relakor say Evening squire, i hear you'd like a tour around Midgaard.
   elseif (%actor.sex% == FEMALE)
      wforce Relakor say Evening Mam, i hear you'd like a tour around Midgaard.
   else
      wforce Relakor say Evening ummm, what are you anyhow?
   end
   wait 1 s
   wforce Relakor emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Relakor say Well, the coins good and my fees paid, so if you'd just
   wforce Relakor say follow me, we'll be on our way.
   wait 1 s
   wforce Relakor shake %actor.name%
   wait 2 s
elseif (%actor.class% == Paladin)
   wforce orian emote calls for the guide to the paladin guild.
   wait 2 s
   wload mob 3099
   wecho Adrerinyth glances what looks to be a plesent smile at you as he enters the room.
   wait 1 s
   wforce orian emote gives a small pouch of coins to Adrerinyth.
   wait 2 s
   if (%actor.sex% == MALE)
      wforce Adrerinyth say Evening Squire, i hear you'd like a tour around town.
   elseif (%actor.sex% == FEMALE)
      wforce Adrerinyth say Evening Mam, i hear you'd like a tour around town.
   else
      wforce Adrerinyth say Evening ummm, what are you anyhow?
   end
   wait 1 s
   wforce Adrerinyth emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Adrerinyth say Helm will be pleased with the donation, so if you'd
   wforce Adrerinyth say care to follow me, we shall be on our way.
   wait 1 s
   wforce Adrerinyth smile %actor.name%
   wait 2 s
elseif (%actor.class% == Druid)
   wforce orian emote calls for the guide to the druid's guild.
   wait 2 s
   wload mob 3082
   wecho Kaaylia glances what looks like smile in your general direction as she enters the room.
   wait 1 s
   wforce orian emote gives a small pouch of coins to Kaaylia.
   wait 2 s
   if (%actor.sex% == MALE)
      wforce Kaaylia say Evening Sir, i hear you'd like a tour around town.
   elseif (%actor.sex% == FEMALE)
      wforce Kaaylia say Evening Mam, i hear you'd like a tour around town.
   else
      wforce Kaaylia say Evening ummm, what are you anyhow?
   end
   wait 1 s
   wforce Kaaylia emote examines the contents of a small purple coin pouch.
   wait 1 s
   wforce Kaaylia say Well, the coins good and my fees paid, so if you'd care to
   wforce Kaaylia say follow me, we will be off to the city of Midgaard.
   wait 1 s
   wforce Kaaylia embrace %actor.name%
   wait 2 s
else
   wecho You are of an unknown class, the classes name is %actor.class%
end
wait 1 s
wforce Orian say If you're unsure how to, just type /ccfollow guide/cw to follow.
wait 1 s
wforce Orian smile %actor.name%
~
#3010
Gaellyn Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say Ok lets be on our way shall we, you might want to take some notes.
wait 3 s
down
wait 3 s
say This is the temple of Midgaard and with it, the newbie helper, if you type
say /ccnewbie/cw she will transport you, but you don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master, he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw command.
wait 10 s
south
wait 3 s
south
wait 3 s
say I do hope you are paying attention to the direction I am moving in as
say we go along the tour, you might need it later when you come back here.
wait 10 s
say Here's the temple square, and with it temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink, we don't
say want you dieing from dehydration when you just got here now do we?
wait 3 s
drink fountain
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights sleep.
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say This is Market Square, commonly called MS as well.
wait 3 s
say Main Street runs to both the east and west, It would be a good place to start.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a fine place to buy food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, it's best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here we have the Magic shop, again, type /cclist/cw, you'll see he sells
say scrolls and potions, most useful are scrolls of recall, as after level 10 you
say will have to use these scrolls yourself.
wait 4 s
say By the way, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say This is the bank, a place to keep your gold in the sound knowledge that
say no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you can also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 5 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
north
wait 3 s
west
wait 3 s
mteleport %actor% 3003
mteleport self 3003
wait 1 s
emote leaves north.
mecho You follow Gaellyn.
wait 1 s
mforce %actor% look
wait 3 s
west
wait 3 s
say Ahhh, I always feel better when I'm in my guild, how about you?
wait 5 s
say Anyhow, here is our guild master, he can help us to gain levels
say and teach us our prayers. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your prayers, in here however,
say it will only display the list if you have practice sessions, to practice your
say prayers when you do, just type /ccprac pray 'prayer name'/cw and if you can
say practice you will get slightly better at that prayer.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the prays name in the
say command. For example, you would type /ccprac pray 'heavy skin'/cw to learn the
say heavy skin prayer.
wait 10 s
say also on the score sheet, you may have noticed a /ccHITS/cw, /ccMANA/cw and /ccMOVE/cw 
say field, the hits field refers to your current health, you can also display this
say information on the prompt, type /cchelp prompt/cw for more info. You need to
say watch your hits, as when it reads 0, I'm afraid you are dead.
wait 10 s
say Your /ccMANA/cw field refers to your magical energy, which you need in order to
say cast your prayers forth, /ccMOVE/cw determins how far you can walk.
wait 5 s
say To call a prayer, type /ccpray 'prayer name' target/cw where the spell name is the name
say of the prayer you wish to cast enclosed in magical symbels (') and the target is the
say name of the person you wish to cast the pray on, for example,
say /ccpray 'heavy skin' me/cw, would cast the heavy skin prayer on yourself.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 5 s
east
wait 3 s
south
wait 3 s
east
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the temple, if you should for any reason die, or lose your equipment, you
say can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3011
Gaellyn Teachkill Trigger~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to kill, I beleave the Labyrinth would
   say be a good place for us to start at, some nice easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok %actor.name% we are at the enterance to the Labyrinth, i hear there is
   say a minotaur down here somewhere, be sure to behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   wait 2 s
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a couple of warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them.
   wait 2 s
   west
   wait 3 s
   say ok, type /cccon warthog/cw this will give you a rough guide to what your
   say chances are of killing this poor creature.
   wait 5 s
   say ok, now we know that you stand a chance of killing the creature, you should
   say take the precaution of seeting a wimp level, this will make you autoflee
   say should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets kill the warthog, type /cckill warthog/cw to attack it.
~
#3012
Gaellyn Goodbye Trigger~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpse or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as we all know, every little helps in our trade.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 3 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 15 s
   say Right, that just about concludes the tour, one final note, dont forget to watch
   say your health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 10 s
   say now its time for us to part my friend, farewell, and good luck.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   say and when you want to pray, type /ccpray 'prayer name'/cw and dont forget
   say to include the magical /cc'/cw symbols around the prayer name.
   wait 2 s
   emote looks to the heavens and prays.
   emote disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3013
Elenydd Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say Right then, lets go; you might want to take some notes.
wait 3 s
down
wait 3 s
say This is the temple of Midgaard and with it, the newbie helper, if you type
say /ccnewbie/cw she will transport you, but we don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master; he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw command.
wait 10 s
south
wait 3 s
south
wait 3 s
say Now we are at the temple square, I do hope you're paying attention to the
say direction I am moving in as we go along the tour, you might need it later.
wait 10 s
say Here's the temple square, and the temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink, we don't
say don't really want you dieing from dehydration when you just got here.
wait 3 s
drink fountain
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights sleep.
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say Watch your toes, this is Market Square, commonly called MS as well.
wait 3 s
say To both the east and west, runs Main Street, would be a good place for us to go.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a place to buy food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, it's best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say Here is the Magic shop, again, type /cclist/cw, you'll see he sells allsorts
say of scrolls and potions, most useful are scrolls of recall, as after level 10
say you will have to use these scrolls yourself.
wait 4 s
say oh, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here we have the bank, a place to keep your gold in the sound knowledge
say that no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you'll also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 5 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
west
wait 3 s
west
wait 3 s
south
wait 3 s
mteleport %actor% 3018
mteleport self 3018
wait 1 s
emote leaves south.
mecho You follow Elenydd.
wait 1 s
mforce %actor% look
wait 3 s
east
wait 3 s
say Ahhh, I always feel better when I'm in my guild, how about you?
wait 5 s
say Anyhow, this is our guild master, he can help us to gain levels
say and teach us our spells. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your spells, in here however,
say it will only display the list if you have practice sessions, to practice your
say spells when you do, just type /ccprac spell 'spell name'/cw and if you can
say practice you will get slightly better at that spell.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the spell name in the
say command. For example, you would type /ccprac spell 'magic missile'/cw to learn the
say magic missile spell.
wait 10 s
say also on the score sheet, you may have noticed a /ccHITS/cw, /ccMANA/cw and /ccMOVE/cw field,
say the hits field refers to your current health, you can also display this information
say on the prompt, type /cchelp prompt/cw for more info. You need to watch your hits,
say as when it reads 0, You are dead.
wait 10 s
say Your /ccMANA/cw field refers to your magical energy, which you need in order to
say cast your spells forth, /ccMOVE/cw determins how far you can walk.
wait 5 s
say To cast a spell, type /cccast 'spell name' target/cw where the spell name is the name
say of the spell you wish to cast enclosed in magical symbels (') and the target is the
say name of the person you wish to cast the spell on, for example,
say /cccast 'magic missile' warthog/cw, would cast the magic missile spell on a warthog.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 5 s
west
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
east
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the temple, if you should for any reason die, or lose your equipment, you
say can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3014
Elenydd Teachkill Trigger~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to slay, The Labyrinth would
   say be a great place for us to start at, some real easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok %actor.name% we are at the enterance to the Labyrinth, i hear there is
   say a minotaur down here somewhere, be sure to behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   wait 2 s
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a couple of warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them.
   wait 2 s
   west
   wait 3 s
   say ok, type /cccon warthog/cw this will give you a rough guide to what your
   say chances are of killing this creature.
   wait 5 s
   say ok, now we know that you stand a chance of killing the creature, you should
   say take the precaution of seeting a wimp level, this will make you autoflee
   say should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets kill the warthog, type /cckill warthog/cw to attack it.
~
#3015
Elenydd Goodbye Trigger~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpse or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as we all know, every little helps in our trade.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 3 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 15 s
   say Right, that just about concludes the tour, on a final note, dont forget to watch
   say your health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 10 s
   say now its time for us to part my friend, farewell, and good luck.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   wait 2 s
   emote recites a scroll of recall and disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3016
Elvuldor Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say I suppose your the person who wants a dammed tour? ok, make sure to make notes.
wait 3 s
down
wait 3 s
say This is the temple of Midgaard and with it, the newbie helper, if you type
say /ccnewbie/cw she will transport you, but we don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master; he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw
say command, and hopefully he wont be too stindgy when you complete a quest.
wait 10 s
south
wait 3 s
south
wait 3 s
say I do hope you're paying attention to the direction I am moving in as we go
say I dont intend to repeat myself and you might need it later.
wait 10 s
say Here's the temple square, and the temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink, we don't
say don't really want you dieing from dehydration when you just got here.
wait 3 s
drink fountain
wait 2 s
spit
say god, some twats pissed in the dammed thing again!
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights sleep.
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say Watch your toes, this is Market Square, commonly called MS as well.
say possibly the busiest place here and ripe for a bit of thieving.
wait 2 s
nudge %actor.name%
wait 3 s
say To both the east and west, runs Main Street, would be a good place for us to go.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a place to buy crummiest food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, you'd best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say Here is the Magic shop, again, type /cclist/cw, you'll see he sells allsorts
say of scrolls and potions, most useful are scrolls of recall, as after level 10
say you will have to use these scrolls yourself.
wait 4 s
say oh, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here we have the bank, a place to keep your gold in the sound knowledge
say that no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you'll also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 3 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
south
wait 3 s
west
wait 3 s
west
wait 3 s
south
wait 3 s
mteleport %actor% 3078
mteleport self 3078
wait 1 s
emote leaves south.
mecho You follow Elvuldor.
wait 1 s
mforce %actor% look
wait 3 s
south
wait 3 s
say Ahhh, I always feel better when I'm in my guild, how about you?
wait 5 s
say Anyhow, this is our guild master, he can help us to gain levels
say and teach us our spells. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your skills and spells, in here,
say it will only display the list if you have practice sessions, to practice your
say skills and spells when you do, just type /ccprac type 'spell//skill name'/cw and
say if you can practice you will get slightly better at that skill.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the spell//skill name in the
say command. For example, you would type /ccprac spell 'cause light'/cw to learn the
say cause light spell. To learn a skill, use the word /ccskill/cw instead of spell.
wait 10 s
say also on the score sheet, you may have noticed a /ccHITS/cw, /ccMANA/cw and /ccMOVE/cw field,
say the hits field refers to your current health, you can also display this information
say on the prompt, type /cchelp prompt/cw for more info. You need to watch your hits,
say as when it reads 0, You are dead, and I aint about to bury you.
wait 10 s
say Your /ccMANA/cw field refers to your magical energy, which you need in order to
say cast your spells forth, /ccMOVE/cw determins how far you can walk.
wait 5 s
say To cast a spell, type /cccast 'spell name' target/cw where the spell name is the name
say of the spell you wish to cast enclosed in magical symbels (') and the target is the
say name of the person you wish to cast the spell on, for example,
say /cccast 'cause light' warthog/cw, would cast the cause light spell on a warthog.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 5 s
north
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
east
wait 3 s
north
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the temple, if you should for any reason die, or lose your equipment, you
say can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3017
Elvuldor Teachkill Trigger~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to slay, The Labyrinth would
   say be a great place for us to start at, some real easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok %actor.name% we are at the enterance to the Labyrinth, i hear there is a
   say minotaur down here somewhere, make sure you behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   wait 2 s
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a some warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them.
   wait 2 s
   west
   wait 3 s
   say Ugly dammed things arnt they?  Ok, type /cccon warthog/cw this will
   say give you a rough guide to what your chances are of killing the thing.
   wait 5 s
   say ok, now you know you stand a chance of killing the pitiful creator, you
   say should take the precaution of seeting a wimp level, this will make you
   say flee should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets kill the warthog, type /cckill warthog/cw to attack it.
~
#3018
Elvuldor Goodbye Trigger~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpse or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as we all know, every little helps in our trade.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 3 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 15 s
   say Right, that just about ends this dammed tour, dont forget to watch
   say your health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 10 s
   say now its time for us to part, farewell, and try not to get slayed.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   wait 2 s
   emote recites a scroll of recall and disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3019
Trirewyr Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say Ok lets be on our way shall we, you might want to take some notes.
wait 3 s
down
wait 3 s
say This is the temple of Midgaard and with it, the newbie helper, if you type
say /ccnewbie/cw she will transport you, but you don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master, he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw command.
wait 10 s
south
wait 3 s
south
wait 3 s
say I do hope you are paying attention to the direction I am moving in as
say we go along the tour, you might need it later when you come back here.
wait 10 s
say Here's the temple square, and with it temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink, we don't
say want you dieing from dehydration when you just got here now do we?
wait 3 s
drink fountain
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights sleep.
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say This is Market Square, commonly called MS as well.
wait 3 s
say Main Street runs to both the east and west, It would be a good place to begin.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a fine place to buy food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, it's best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here we have the Magic shop, again, type /cclist/cw, you'll see he sells
say scrolls and potions, most useful are scrolls of recall, as after level 10 you
say will have to use these scrolls yourself.
wait 4 s
say By the way, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say This is the bank, a place to keep your gold in the sound knowledge that
say no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you can also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 5 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
west
wait 3 s
west
wait 3 s
west
wait 3 s
west
wait 3 s
north
wait 3 s
north
wait 3 s
mteleport %actor% 3083
mteleport self 3083
wait 1 s
emote leaves north.
mecho You follow Trirewyr.
wait 1 s
mforce %actor% look
wait 3 s
north
wait 3 s
say I always feel better when I'm in my guild.
wait 5 s
say Anyhow, here is our guild master, he can help us to gain levels and
say teach us our skills and chants. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your skills and chants, in here
say it will only display the list if you have practice sessions, to practice your
say chants, just type /ccprac chant 'chant name'/cw and if you can practice you
say will get slightly better at the chant. To practice a skill, use the command
say /ccprac skill 'skill name'/cw.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the chant//skill
say name in the command. For example, you would type /ccprac skill 'trip'/cw
say to learn the skill, trip.
wait 10 s
say also on the score sheet, you may have noticed a /ccHITS/cw, /ccQI/cw and /ccMOVE/cw field,
say the hits field refers to your current health, you can also display this
say information on the prompt, type /cchelp prompt/cw for more info. You need to
say watch your hits, as when it reads 0, I'm afraid you are dead.
wait 10 s
say Your /ccQI/cw field refers to your magical energy, which you need in order
say to cast your chants forth, /ccMOVE/cw determins how far you can walk.
wait 5 s
say To call a chat, type /ccchant 'chant name' target/cw where the spell name is the
say name of the prayer you wish to cast enclosed in magical symbels (') and the target
say is the name of the person you wish to preform the chant on.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 10 s
south
wait 3 s
south
wait 3 s
south
wait 3 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the temple, if you should for any reason die, or lose your equipment, you
say can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3020
Trirewyr Teachkill Trigger~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to kill, I beleave the Labyrinth would
   say be a good place for us to start at, some nice easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok %actor.name% we are at the enterance to the Labyrinth, i hear there is
   say a minotaur down here somewhere, be sure to behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   wait 2 s
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a couple of warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them.
   wait 2 s
   west
   wait 3 s
   say ok, type /cccon warthog/cw this will give you a rough guide to what your
   say chances are of killing this poor creature.
   wait 5 s
   say ok, now we know that you stand a chance of killing the creature, you should
   say take the precaution of seeting a wimp level, this will make you autoflee
   say should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets kill the warthog, type /cckill warthog/cw to attack it.
~
#3021
Trirewyr Goodbye Trigger~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpse or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as we all know, every little helps.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 3 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 15 s
   say Right, that just about concludes the tour, one final note, dont forget to watch
   say your health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 10 s
   say now its time for us to part my friend, farewell, and good luck.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   wait 2 s
   emote recites a scroll of recall and disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3022
Relakor Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say Right then, let's be off, you might want to take some notes along on way.
wait 3 s
down
wait 3 s
say This is the temple of Midgaard and with it, the newbie helper, if you type
say /ccnewbie/cw she will transport you, but you don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master; he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw command.
wait 10 s
south
wait 3 s
south
wait 3 s
say Now we are at the temple square, I do hope you are paying attention to the
say direction I am moving in as we go along the tour, you might need it later.
wait 10 s
say Here's the temple square, and the temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink,
say we don't want you dieing from dehydration.
wait 3 s
drink fountain
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights sleep.
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say Mind your toes, this is Market Square, commonly called MS as well.
say possibly the busiest place here.
wait 3 s
say To both the east and west, runs Main Street, would be a good place to look.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a fine place to buy food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, it's best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here is the Magic shop, again, type /cclist/cw, you'll see he sells allsorts
say of scrolls and potions, most useful are scrolls of recall, as after level 10 you
say will have to use these scrolls yourself.
wait 4 s
say oh, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say This is the bank, a place to keep your gold in the sound knowledge that
say no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you'll also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 5 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
east
wait 3 s
east
wait 3 s
south
wait 2 s
mteleport %actor% 3022
mteleport self 3022
wait 1 s
emote leaves east.
mecho You follow Relakor.
wait 1 s
mforce %actor% look
wait 3 s
south
wait 3 s
say Ahhh, I always feel better when I'm in my guild, how about you?
wait 5 s
say Anyway, this is our guild master, he can help us to gain levels
say and teach us our skills. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your skills, in here however,
say it will only display the list if you have practice sessions, to practice your
say skills when you do, just type /ccprac skill 'skill name'/cw and if you can
say practice you will get slightly better at that skill.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the skill name in the
say command. For example, you would type /ccprac skill 'kick'/cw to learn the
say kick skill.
wait 10 s
say also on the score sheet, you may have noticed a /ccHITS/cw and /ccMOVE/cw field,
say the hits field refers to your current health, you can also display this information
say on the prompt, type /cchelp prompt/cw for more info. You need to watch your hits,
say as when it reads 0, I'm afraid you are dead my friend.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 5 s
north
wait 3 s
west
wait 3 s
north
wait 3 s
west
wait 3 s
west
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the temple, if you should for any reason die, or lose your equipment, you
say can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3023
Relakor Teachkill Trigger~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to kill, I beleave the Labyrinth would
   say be a good place for us to start at, some nice easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok %actor.name% we are at the enterance to the Labyrinth, i hear there is
   say a minotaur down here somewhere, be sure to behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   wait 2 s
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a couple of warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them.
   wait 2 s
   west
   wait 3 s
   say ok, type /cccon warthog/cw this will give you a rough guide to what your
   say chances are of killing this poor creature.
   wait 5 s
   say ok, now we know that you stand a chance of killing the creature, you should
   say take the precaution of seeting a wimp level, this will make you autoflee
   say should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets kill the warthog, type /cckill warthog/cw to attack it.
~
#3024
Relakor Goodbye Trigger~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpse or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as you know, every little helps.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 3 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 15 s
   say Right, that just about concludes the tour, one final note, dont forget to watch
   say your health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 10 s
   say now its time for us to part my friend, farewell, and good luck.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   wait 2 s
   emote recites a scroll of recall and disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3025
Adrerinyth Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say Another crusader of justice in need of a tour? make sure to make notes.
wait 3 s
down
wait 3 s
say This is the holy temple of Midgaard and with it, the newbie helper, if you
say type /ccnewbie/cw she will transport you, but we don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master; he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw
say command, and hopefully he wont be too stindgy when you complete a quest.
wait 10 s
south
wait 3 s
south
wait 3 s
say I do hope you're paying attention to the direction I am moving in as we go.
wait 10 s
say Here's the temple square, and the temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink, we
say don't really want you dieing from dehydration when you just got here.
wait 3 s
drink fountain
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights rest.
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say Watch your toes, this is Market Square, commonly called MS as well.
wait 3 s
say To both the east and west, runs Main Street, would be a good place for us to go.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a place to buy honest food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, you'd best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say Here is the Magic shop, again, type /cclist/cw, you'll see he sells allsorts
say of scrolls and potions, most useful are scrolls of recall, as after level 10
say you will have to use these scrolls yourself.
wait 4 s
say oh, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here we have the bank, a place to keep your gold in the sound knowledge
say that no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you'll also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 3 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
south
wait 3 s
east
wait 3 s
north
wait 2 s
mteleport %actor% 3086
mteleport self 3086
wait 1 s
emote leaves east.
mecho You follow Adrerinyth.
wait 1 s
mforce %actor% look
wait 3 s
east
wait 3 s
say Ahhh, I always feel better when I'm in my guild, how about you?
wait 5 s
say Anyhow, this is our guild master, he can help us to gain levels
say and teach us our spells. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your skills and spells, in here,
say it will only display the list if you have practice sessions, to practice your
say skills and spells when you do, just type /ccprac type 'spell//skill name'/cw and
say if you can practice you will get slightly better at that skill.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the spell//skill name in the
say command. For example, you would type /ccprac spell 'cause light'/cw to learn the
say cause light spell. To learn a skill, use the word /ccskill/cw instead of spell.
wait 10 s
say also on the score sheet, you may have noticed a /ccHITS/cw, /ccMANA/cw and /ccMOVE/cw field,
say the hits field refers to your current health, you can also display this information
say on the prompt, type /cchelp prompt/cw for more info. You need to watch your hits,
say as when it reads 0, You are dead, and will need helms assistance.
wait 10 s
say Your /ccMANA/cw field refers to your magical energy, which you need in order to
say cast your spells forth, /ccMOVE/cw determins how far you can walk.
wait 5 s
say To cast a spell, type /cccast 'spell name' target/cw where the spell name is the name
say of the spell you wish to cast enclosed in magical symbels (') and the target is the
say name of the person you wish to cast the spell on, for example,
say /cccast 'cure light' me/cw, would cast the cure light spell on yourself.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 10 s
west
wait 3 s
west
wait 3 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate to helm.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the holy temple, if you should for any reason die, or lose your
say equipment, you can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3026
Adrerinyth Teachkill Trigger~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to slay, The Labyrinth would
   say be a great place for us to start at, some real easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok %actor.name% we are at the enterance to the Labyrinth, i hear there is a
   say minotaur down here somewhere, make sure you behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   wait 2 s
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a some warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them.
   wait 2 s
   west
   wait 3 s
   say Ugly little things arnt they?  Ok, type /cccon warthog/cw this will
   say give you a rough guide to what your chances are of killing it.
   wait 5 s
   say ok, now you know you stand a chance of killing the creator, you should
   say take the precaution of seeting a wimp level, this will make you autoflee
   say should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets kill the warthog, type /cckill warthog/cw to attack it.
~
#3027
Adrerinyth Goodbye Trigger~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpses or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as we all know, every little helps in our trade.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 3 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 15 s
   say Right, that just about ends this little tour, dont forget to watch your
   say health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 10 s
   say now its time for us to part, farewell my friend, bring order and justice.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   wait 2 s
   emote recites a scroll of recall and disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3028
Kaaylia Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say Another servant of nature in need of a tour? make sure to take notes.
wait 3 s
down
wait 3 s
say This is the temple of Midgaard and with it, the newbie helper, if you type
say /ccnewbie/cw she will transport you, but we don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master; he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw
say command, and hopefully he wont be too stindgy when you complete a quest.
wait 10 s
south
wait 3 s
south
wait 3 s
say I do hope you're paying attention to the direction I am moving in as we go.
wait 10 s
say Here's the temple square, and the temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink, we
say don't really want you dieing from dehydration when you just got here.
wait 3 s
drink fountain
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights rest.
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say Watch your toes, this is Market Square, commonly called MS as well.
wait 3 s
say To both the east and west, runs Main Street, would be a good place for us to go.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a place to buy natural food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, you'd best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say Here is the Magic shop, again, type /cclist/cw, you'll see he sells allsorts
say of scrolls and potions, most useful are scrolls of recall, as after level 10
say you will have to use these scrolls yourself.
wait 4 s
say oh, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here we have the bank, a place to keep your gold in the sound knowledge
say that no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you'll also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 3 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
north
wait 3 s
mteleport %actor% 3074
mteleport self 3074
wait 1 s
emote leaves north.
mecho You follow Kaaylia.
wait 1 s
mforce %actor% look
wait 3 s
north
wait 3 s
say Ahhh, I always feel better when I'm in my guild, how about you?
wait 5 s
say Anyhow, this is our guild master, he can help us to gain levels
say and teach us our spells. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your skills and spells, in here,
say it will only display the list if you have practice sessions, to practice your
say skills and spells when you do, just type /ccprac type 'spell//skill name'/cw
say and if you can practice you will get slightly better at that skill.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the spell//skill name
say in the command. For example, you would type /ccprac skill 'kick'/cw to learn
say the skill kick. To learn a spell, use the word /ccspell/cw instead of skill.
wait 10 s
say also, you may have noticed a /ccHITS/cw, /ccVIM/cw and /ccMOVE/cw field,
say the hits field refers to your current health, you can also display this info.
say on the prompt, type /cchelp prompt/cw for more info. You need to watch your hits,
say as when it reads 0, You are dead, and will need helms assistance.
wait 10 s
say Your /ccVIM/cw field refers to your magical energy, which you need in order to
say cast your spells forth, /ccMOVE/cw determins how far you can walk.
wait 5 s
say To cast a spell, type /cccast 'spell name' target/cw where the spell name is the
say name of the spell you wish to cast enclosed in magical symbels (') and the target
say is the name of the person you wish to cast the spell on.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 5 s
south
wait 3 s
south
wait 3 s
south
wait 3 s
west
wait 3 s
west
wait 3 s
west
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate to helm.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the holy temple, if you should for any reason die, or lose your
say equipment, you can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3029
Kaaylia Teachkill Trigger~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to slay, The Labyrinth would
   say be a great place for us to start at, some real easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok %actor.name% we are at the enterance to the Labyrinth, i hear there is a
   say minotaur down here somewhere, make sure you behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   wait 2 s
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a some warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them.
   wait 2 s
   west
   wait 3 s
   say Strang little things arnt they?  Ok, type /cccon warthog/cw this will
   say give you a rough guide to what your chances are of killing it.
   wait 5 s
   say ok, now you know you stand a chance of killing the creator, you should
   say take the precaution of seeting a wimp level, this will make you autoflee
   say should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets kill the warthog, type /cckill warthog/cw to attack it.
~
#3030
Kaaylia Goodbye Trigger~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpses or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as we all know, every little helps in our trade.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 3 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 15 s
   say Right, that just about ends this little tour, dont forget to watch your
   say health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 10 s
   say now its time for us to part, farewell my friend, bring order and justice.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   wait 2 s
   emote recites a scroll of recall and disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3031
Orians Level Check~
0 h 100
~
if (%actor.level% <= 10 && %actor.vnum% == -1)
wait 5 s
   say Wecome to Dibrova %actor.name%, if you would like a tour
   say around town, type /ccassist/cw and I will summon the guide.
else
   wait 2 s
   say You are too high a level for tours, please leave.
   mforce %actor.name% down
end
~
#3032
Guide Nolonger Followed~
0 e 0
stops following you.~
wait 1 s
say Very well, your tour is over at your own request.
wave %actor.name%
mteleport me 97
wait 2 s
say Job Complete
~
#3050
carp receive~
0 j 100
~
if (%object.vnum% == 10045)
 wait 1 s
 say Now that looks like a tasty carp! But first..
wait 2 s
 emote pries open a nasty carp's mouth and looks inside.
 wait 2 s
 nod
 wait 2 s
 emote reaches into a nasty carp's mouth and pulls out a quest point!
 wait 2 s
 say Here you are, %actor.name%. Thank you for the delicious dinner!
 emote gives you the quest point.
 mqpadd %actor.name% %actor.qpoints(1)%
 wait 2 s
 eat carp
 endif
if (%object.vnum% == 10038)
 wait 1 s
 say Ah, a wonderfully tasty oyster! But first..
 wait 2 s
 emote pries open the oyster's shell.
 wait 2 s
 say Ah, yes. Its always true that there is treasure in every oyster.
 mgrin
 wait 2 s
 emote gets a quest point from inside the oyster!
 wait 2 s
 say Here you are, %actor.name%. Thank you for the delicious dinner!
 emote gives you the quest point.
 mqpadd %actor.name% %actor.qpoints(1)%
 wait 2 s
 eat oyster
 endif
~
#3051
oyster receive~
0 j 100
~
if (%object.vnum% == 10038)
 wait 1 s
 say Now that looks like a real nice oyster! But first..
 wait 2 s
 emote pries open a oyster's shell and looks inside.
 wait 2 s
 nod
 wait 2 s
 emote lifts a quest point out of the oyster's shell!
 wait 2 s
 say They always say that there's treasure in an oyster!
 mgrin
 wait 2 s
 say Here you are, %actor.name%. Thank you for the delicious dinner!
 emote gives you the quest point.
 mqpadd %actor.name% %actor.qpoints(1)%
 wait 2 s
 eat oyster
 endif
~
#3062
fido randoms~
0 b 15
~
switch %random.10%
 case 1
  emote lifts its leg and takes a generous leak on the ground in front of you.
  break
 case 2
  emote sniffs the ground in front of it and moves off in search of some food.
  e
  break
 case 3
  emote growls low in its throat, sensing that you may not be all that kind.
  break
 case 4
   fart
  break
 case 5
  emote trundles around behind you and sniffs your ass.
  break
 case 6
  rest
wait 2 s
  emote lifts one paw, scratching behind an ear.
wait 2 s
  stand
  break
 default
  break
 done
end
~
#3081
Imoens Guide Trigger~
0 e 0
starts following you.~
wait 3 s
say Right then, we'll be off shall we; you might want to take some notes.
wait 3 s
down
wait 3 s
say This is the temple of Midgaard and with it, the newbie helper, if you type
say /ccnewbie/cw she will transport you, but you don't want to do that right now.
wait 10 s
north
wait 3 s
say Here we have the Quest master; he will give you quests upon request. Which
say you can complete for gold, experience and quest points, use the /ccaquest/cw command.
wait 10 s
south
wait 3 s
south
wait 3 s
say Now we are at the temple square, I do hope you are paying attention to the
say direction I am moving in as we go along the tour, you might need it later.
wait 10 s
say Here's the temple square, and the temple fountain, which you can use to
say quench your thirst and refill your canteen, it is important to drink, we don't
say want you dieing from dehydration when you just got here now do we?
wait 3 s
drink fountain
wait 15 s
east
wait 3 s
up
wait 3 s
say Ahhh, the reception, nothing beats a good nights sleep, eh?
wait 3 s
say by typing /ccrent/cw you can rent a room here, this is also the only way you
say can leave the game to return to the real world.
wait 10 s
down
wait 3 s
west
wait 3 s
south
wait 3 s
say opps, watch your toes, this is Market Square, commonly called MS as well.
say possibly the busiest place here and ripe for a bit of thieving.
wait 2 s
nudge %actor.name%
wait 3 s
say To both the east and west, runs Main Street, would be a good place for us to go.
wait 10 s
west
wait 3 s
north
wait 2 s
say This is the bakers shop, a fine place to buy food, type /cclist/cw it will show
say you what the baker has to sell. And unless you want to stave, it's best to eat.
wait 4 s
say Using the /ccbuy/cw and /ccsell/cw commands, you can trade with the shopkeepers.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say And here is the Magic shop, again, type /cclist/cw, you'll see he sells allsorts
say of scrolls and potions, most useful are scrolls of recall, as after level 10 you
say will have to use these scrolls yourself.
wait 4 s
say oh, cats eyes can be handy for discovering invisible items.
wait 10 s
south
wait 3 s
west
wait 3 s
north
wait 3 s
say This is the bank, a place to keep your gold in the sound knowledge that
say no one can steal it from you. The commands are /ccbalance/cw, /ccdeposit/cw
say and /ccwithdraw/cw.
wait 10 s
south
wait 3 s
east
wait 3 s
east
wait 3 s
east
wait 3 s
say Back at Market Square (MS) now, to the west you'll also find an armoury and to the
say east you can seek a weapon smith, general store and a couple of other shops that
say you might like to browse sometime.
wait 5 s
say oh, you might like to fully explore Midgaard yourself at some point, but right now
say we should be getting back to the guild.
wait 10 s
south
wait 3 s
east
wait 3 s
south
wait 3 s
mteleport %actor% 3028
mteleport self 3028
emote leaves east.
mecho You follow Imoen.
wait 1 s
mforce %actor% look
wait 3 s
south
wait 3 s
say Ahhh, I always feel better when I'm in my guild, how about you?
wait 5 s
say Anyway, this is our guild master, he can help us to gain levels
say and teach us our skills. If you type score now, you should see
say a field titled /ccTO LEVEL/cw, its just above the /ccQPOINTS/cw field.
wait 10 s
say Found it? Good. That's how much experience you need to gain your next
say level. When you see a /cc-/cw it means you can come back here to level
say up, to level up, just type /ccgain/cw in this room.
wait 10 s
say Type /ccprac/cw, this will bring up a list of your skills, in here however,
say it will only display the list if you have practice sessions, to practice your
say skills when you do, just type /ccprac skill 'skill name'/cw and if you can
say practice you will get slightly better at that skill.
wait 15 s
say oh by the way, you do have to include the /cc'/cw around the skill name in the
say command. For example, you would type /ccprac skill 'sneak'/cw to learn the
say sneak skill.
wait 10 s
say also on the score sheet, you may have noticed a /ccHITS/cw and /ccMOVE/cw field,
say the hits field refers to your current health, you can also display this
say information on the prompt, type /cchelp prompt/cw for more info. You need to watch
say your hits, as when it reads 0, I'm afraid you are dead my friend.
wait 15 s
say When your health is low, the guild is a good place to come back to, sleeping
say in your guild will give a healing bonus, use the /ccsleep/cw command to goto
say sleep, then get back up by typing /ccwake/cw followed by /ccstand/cw.
wait 15 s
say ok, lets go to the donation room now and see if there's anything you can use.
wait 5 s
north
wait 3 s
west
wait 3 s
north
wait 3 s
west
wait 3 s
north
wait 3 s
north
wait 3 s
north
wait 3 s
east
wait 3 s
say Now and again, other players will donate equipment, this is where it is
say stored when they do, you might want to come back here from time to time
say to see if there's anything of interest. Use the /ccdonate/cw command if you
say would like to donate.
wait 15 s
west
wait 3 s
west
wait 3 s
say This is the board room, where players post messages and talk to each other as
say well as keep in touch, to the North, South and West lie more boards each bearing
say there own subjects. Next time your here, type /cchelp board/cw.
wait 3 s
say Type /ccexit/cw it will give you a list of the other rooms here.
wait 10 s
east
wait 3 s
say Back at the temple, if you should for any reason die, or lose your equipment, you
say can ask the newbie helper for another kit by typing /ccoutfit/cw.
wait 3 s
say Ok, type /ccnewbie/cw and we shall head over to the newbie zones.
wait 2 s
emote asks the newbie guide for assistance and is taken to the newbie zone.
mteleport self 3066
~
#3082
Imoens Teach Kill~
0 c 100
continue~
   wait 2 s
   say Right, lets go and find something to kill, I beleave the Labyrinth would
   say be a good place for us to start at, some nice easy kills there.
   wait 5 s
   west
   wait 3 s
   south
   wait 3 s
   east
   wait 3 s
   say ok, here we are, the enterance to the Labyrinth, i hear there is a minotaur
   say down here somewhere, be sure to behead it if you manage to find it.
   wait 5 s
   down
   mteleport self 12003
   mload mob 12000
   mload mob 12000
   mteleport self 12002
   wait 3 s
   say ok, type /ccscan/cw, you should notice a couple of warthogs to the west.
   wait 5 s
   say come on, lets take a closer look at them shall we.
   wait 2 s
   west
   wait 3 s
   say ok, type /cccon warthog/cw this will give you a rough guide to what your
   say chances are of killing this foul creature and doing us all a favor.
   wait 5 s
   say ok, now we know that you stand a chance of killing the creature, you should
   say take the precaution of seeting a wimp level, this will make you autoflee
   say should your hit points get to low, type /ccwimp 7/cw to set it to 7 HP.
   wait 10 s
   say ok, lets get rid of the warthog, type /cckill warthog/cw to attack it.
~
#3083
Imoens Guide End~
0 e 0
Your blood freezes as you hear a warthog's death cry.~
   wait 2 s
   cheer %actor.name%
   wait 2 s
   say ok, now its dead, type /cclook in corpse/cw you will be able to see if the
   say warthog was carrying anything you could use, to get stuff out of corpse or
   say other containers, type /ccget (itemname) (container)/cw for example, type
   say /ccget gold corpse/cw to get any gold out of the corpse, you can also use
   say /ccget all corpse/cw to get everything from the body.
   wait 15 s
   say ok, now type /ccsac corpse/cw you will gain a little extra experance for
   say your sacrifice. And as we all know, every little helps in our trade.
   wait 10 s
   say Alternativly, your can automaticly take gold, items and sacrifice corpses
   say by using any of the following commands.
   wait 2 s
   say Typing /ccautosplit/cw will loot gold and split it amoung your party if any.
   say Typing /ccautoloot/cw will loot any items from the corpse after you make the kill.
   say Typing /ccautosac/cw will sacrifice the corpse after you have killed the victim.
   wait 10 s
   say Right, that just about concludes the tour, one final note, dont forget to watch
   say your health, and return to the guild to sleep when needs be. Also if you need
   say further help, type /cchelp/cw followed by what you would like help on, for
   say example, type /cchelp sacrifice/cw for information on the sacrifice command.
   wait 5 s
   say now its time for us to part my friend, farewell, and good luck.
   wait 2 s
   wave %actor.name%
   wait 2 s
   say oh, one last thing, dont forget you can type /ccrecall/cw to return to the
   say temple at anytime, this will only work whilst you are below level 10.
   wait 2 s
   emote recites a scroll of recall and disappears.
   mteleport self 97
   wait 2 s
   say Job Complete
~
#3090
drunk~
0 g 100
~
wait 5 s
emote snorts and looks about sheepishly.
wait 8 s
emote gooses you tenderly on the butt!
emote giggles and lovingly stares at you!
wait 8 s
say Umm, beware of cooties!
emote screams sillily and freaks out!
wait 8 s
say Whoa!  What happened??
emote smile sheepishly and smacks his head!
wait 8 s
emote burps loudly!
emote returns back to the bar and orders another drink.
~
#3091
Mortal General Board Room~
2 g 10
~
wait 5 s
wecho Someone whispers, '/cwPssst...go read the changes board/c0'
wait 2 s
wecho Someone whispers, '/cwIt's been said changes happen every now and then.../c0'
wait 3 s
wecho Someone whispers, '/cwThe changes board can be found above this room.../c0'
wait 3 s
wecho Someone bows infront of you.
~
$~
