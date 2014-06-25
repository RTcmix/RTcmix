/* HOLD:  
p0 = start; 
p1 = duration (-endtime); 
p2 = hold dur; 
p3 = inchan; 
p4 = audio index */
/* RELEASE:  
p0 = outsk; 
p1 = insk; 
p2 = duration (-endtime); 
p3 = amp; 
p4 = aud_idx */

load("iHAR")
set_option("full_duplex_on")
rtsetparams(44100, 2, 512)
xrtinput("/snd/Public_Sounds/jimi3dSound.aiff")
rtinput("AUDIO")
bus_config("HOLD", "in0-1","aux0out")

HOLD(0,5,3,0,0)
setline(0,0, 10, 1, 11, 0)
RELEASE(1, 0, 20, 1, 0)

