/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 2)
rtinput("/snd/Public_Sounds/doorbell.aiff")

bus_config("MIX","in0","out0-1")

out = 0
in = 0
dur = 2
amp = 1
ch1 = 0
ch2 = 0

MIX(out,in,dur,amp,ch1,ch2)

