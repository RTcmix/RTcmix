/* HOLO: stereo FIR filter to perform crosstalk cancellation
*
*  p0 = outsk
*  p1 = insk
*  p2 = dur
*  p3 = amp
*  p4 = xtalk amp mult
*
*/
set_option("full_duplex_on")
rtsetparams(44100, 2)
load("HOLO")
rtinput("AUDIO")

HOLO(0, 0, 7.8, 1.0, 0.5)
