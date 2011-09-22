rtsetparams(44100, 2)
load("WIGGLE")

dur = 14
amp = 10000
pitch = 6.00
mod_depth_type = 2      // mod index

env = maketable("line", 2000, 0,0, 1,1, 2,1, 5,0)

/* This score uses negative values for mod. freq, which are interpreted
   as C:M ratios where C = 1.  E.g., first note has 1:1 moving to 1:1.05;
   2nd note has 1:2 moving to 1:2.2.
*/
carwave = maketable("wave", 2000, "sine")
modwave = maketable("wave", 2000, "sine")
modfreq = maketable("line", "nonorm", 10, 0,-1, 1,-1.05)
moddepth = maketable("line", "nonorm", 10, 0,30, 1,1)

WIGGLE(st=0.00, dur, amp * env, pitch, mod_depth_type, filttype=0, steepness=0,
	balance=0, carwave, modwave, modfreq, moddepth, lpcf=0, pan=0)

modfreq = maketable("line", "nonorm", 10, 0,-2, 1,-2.2)
moddepth = maketable("line", "nonorm", 10, 0,20, 1,1)
WIGGLE(st=0.01, dur, amp * env, pitch, mod_depth_type, filttype, steepness,
	balance=0, carwave, modwave, modfreq, moddepth, lpcf=0, pan=1)

