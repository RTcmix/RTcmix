/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 2)
rtinput("/snd/pablo1.snd")
setline(0,0, 1, 1)
MIX(0, 0, 7.0, 1, 0, 0)
setline(0, 1, 1, 0)
MIX(0.1, 0, 7.0, 1, 1, 1)
