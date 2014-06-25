/* combit --  comb filter instrument
*
* p0 = output skip
* p1 = input skip
* p2 = input duration
* p3 = amplitude multiplier
* p4 = pitch (cps)
* p5 = reverb time
* p6 = input channel [optional]
* p7 = stereo spread [optional]
*
*/

set_option("full_duplex_on")
rtsetparams(44100, 2, 1024)
load("COMBIT")
rtinput("AUDIO")
COMBIT(0, 0, 3.5, 0.08, cpspch(7.09), .5, 0, 0)
COMBIT(0.2, 0, 3.5, 0.08, cpspch(7.07), .5, 0, 1)
