/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 2,16384)
rtinput("/snd/pablo1.snd")
setline(0,0, 1,1, 2,0)
reset(10000)
