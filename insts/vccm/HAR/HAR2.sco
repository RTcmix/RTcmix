/* HOLD:  p0 = start; p1 = duration (-endtime); p2 = hold dur; p3 = inchan; p4 = audio index */
/* RELEASE:  p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; p4 = aud_idx p5 = fade time; p6 = stereo spread */

load("iHAR")
set_option("full_duplex_on")
rtsetparams(44100, 2, 256)
rtinput("AUDIO")

HOLD(0,20,3,0,0)
makegen(1.0,24,1000,0,0,1,1,10,1,11,0)
makegen(2,24,1000,0,1,1,0)
RELEASE(4, 0, 30, 1, 0, 5, 0.5)
FADE(10)
x
