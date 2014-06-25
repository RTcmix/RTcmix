/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 8, 4096)
load("MIXN")
set_option("clobber_on")
rtinput("/snd/Public_Sounds/sin.aiff")
rtoutput("test.aiff")
reset(10000)

setline(0,0,1,1,10,1,11,0)
bus_config("MIXN","in0","out0-7")

rates(0,1)
len = 100
len2 = 5
speakerloc_p(len,0,len,45,len,90,len,135,len,180,len,225,len,270,len,315,len,360)
path_p(0,len2,0,
       1,len2,45,
       2,len2,90,
       3,len2,135,
       4,len2,180,
       5,len2,225,
       6,len2,270,
       7,len2,315,
       8,len2,360)

out = 0
in = 5
dur = 10
inchan = 0 /* works with 1 too ?!?! */
amp = 5

MIXN(out,in,dur,inchan,amp)

