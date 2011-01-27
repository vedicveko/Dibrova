#16900
Magic disabler~
0 c 100
c~
  if (%actor.vnum% == -1)
    msend %actor% Impossible!  You can't concentrate with all of these bugs around! 
  end
~
#16901
Reeds Prevent Movement~
0 c 100
n~
msend %actor% The dense reeds block your path!
mechoaround %actor% The dense reeds block %actor.name%'s path!
~
#16902
Yellow Blinder Snake~
0 g 80
~
  if (%actor.vnum% == -1)
     msend %actor% A little yellow snake slithers up and bites you!
     mechoaround %actor% A little yellow snake slithers up and bites %actor.name%!
     dg_cast 'blind' %actor%
  end
~
#16903
Toad Bites Poison~
0 gi 75
~
  if (%actor.vnum% == -1)
   msend %actor% You startle an arrow toad.  It flicks its tongue in your eyes!
   mechoaound %actor.name% A startled toad jabs its tongue into %actor.name%'s eye!
   dg_cast 'poison' %actor%
  end
~
#16906
Harridan Death Load~
0 f 100
~
say My trigger commandlist is not complete!
mload obj 3022
~
#16922
Guards Deny Access~
0 c 100
n~
say Get lost, punk.
msend %actor% The gateguard blocks your way.
mechoaround %actor% The gateguard blocks %actor.name%'s way.
~
#16927
Carried Off To Nest~
2 g 12
~
say My trigger commandlist is not complete!
~
#16942
Arrows From Above~
2 g 100
~
eval damage %random.110%
wait 1
wsend %actor% Your arrival is greeted with a hail of arrows!
wechoaround %actor% A flight of arrows zip down from the fortress striking %actor.name%!
wdamage %actor% %damage%
~
$~
