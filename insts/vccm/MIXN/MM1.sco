/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 8)
load("iMIXN")
rtinput("/snd/Public_Sounds/doorbell.aiff")

bus_config("MIXN","in0","out0-7")

out = 0
in = 0
dur = 20
inchan = 0
amp = 1
ch0 = 0
ch1 = 0
ch2 = 1
ch3 = 0
ch4 = 0
ch5 = 0
ch6 = 0
ch7 = 0


MIXN(out,in,dur,inchan,amp,ch0,ch1,ch2,ch3,ch4,ch5,ch6,ch7)

