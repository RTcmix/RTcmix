/* p0 = outsk; p1 = insk; p2 = dur (normal) OR (if - ,end-time); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 2)
load("STEREO")
rtinput("/snd/pablo1.snd")
setline(0,0, 1, 1, 1.1, 0)
STEREO(0, 0, 3.5, 0.7, 0.5, 0.5)
setline(0,0, 0.1, 1, 1, 0)
STEREO(2, 0, 3.5, 0.7, 0.1, 0.1)
