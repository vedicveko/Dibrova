#23900
WIND_BLOW~
2 b 25
~
Wecho The wind blows by, briefly disturbing the calm oasis water.    
~
#23901
Fence_Push~
2 g 100
~
if ((%direction% == south) && (%actor.room% == 23977))
     wsend %actor% You push your way past the fence, briefly getting caught on a flimsy wire.
elseif ((%direction% == north) && (%actor.room% == 23972))
     wsend %actor% You push your way past the fence, briefly getting caught on a flimsy wire.
end
~
$~
