/* STORE:   */
/* p0 = start  */
/* p1 = duration (-endtime)  */
/* p2 = amt of audio to save  */
/* p3 = inchan  */
/* p4 = audio index */

/* HOLD:   */
/* p0 = outsk */
/* p1 = duration (-endtime) */
/* p2 = amp */
/* p3 = aud_idx  */
/* p4 = fade time */
/* p5 = stereo spread */

/* FADE:   */
/* p0 = fade start time - used mainly for performance */

/* slot 1 is an envelope for the looped buffer */
/* slot 2 is envelope for FADE */

set_option("full_duplex_on")
rtsetparams(44100, 2, 256)
rtinput("AUDIO")
bus_config("STORE","in0","aux0out");
bus_config("HOLD","aux0out");
bus_config("JDELAY","aux0in","out0-1")
load("iHAR")
load("JDELAY")
makegen(1.0,24,1000,0,0,1,1,2,1,3,0)
makegen(2,24,1000,0,1,1,0)
JDELAY(0,0,30,1,1.1,0.5,0.9,0,0.80,0,0.5)
STORE(0,30,2,0,0)
HOLD(5, 30, 1, 0, 2, 0.5)
FADE(10)

