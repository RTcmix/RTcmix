rtsetparams(44100, 8)
load("NPAN")

// 8 speakers arranged in circle, with speakers directly in front of (0 deg)
// and behind (180 deg) listener.

NPANspeakers("polar",
    45, 1,   // front left
   -45, 1,   // front right
    90, 1,   // side left
   -90, 1,   // side right
   135, 1,   // rear left
  -135, 1,   // rear right rear
     0, 1,   // front center
   180, 1)   // rear center

rtinput("/snd/Public_Sounds/egg.snd")
inskip = 0

amp = 1.0
dur = DUR()
start = 0

// move counter-clockwise around circle
angle = maketable("line", "nonorm", 1000, 0,0, 1,360)

dist = 1
//dist = maketable("line", "nonorm", 1000, 0,.5, 1,4, 6,.4)

NPAN(start, inskip, dur, amp, "polar", angle, dist)

