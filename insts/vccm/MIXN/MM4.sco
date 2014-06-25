/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44010, 8, 4096)
load("MIXN")
set_option("clobber_on")
rtinput("/snd/Public_Sounds/jimi3dSoundDN.aiff")
rtinput("/snd/Public_Sounds/sin.aiff")
rtoutput("test.aiff")

xsetline(0,0,1,1,10,1,11,0)
bus_config("MIXN","in0","out0-7")

speakerloc(4,0,
           2,2,
	   0,4,
	   -2,2,
	   -4,0,
	   -2,-2,
	   0,-4,
	   2,-2)
	   
path(0,0,0,
     1,0,4,
     2,-2,-2,
     5,2,-2,
     6,0,4)
     


out = 0
in = 0
dur = DUR()
inchan = 0 /* works with 1 too ?!?! */
amp = .2

MIXN(out,in,dur,inchan,amp)

