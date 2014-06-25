rtsetparams(44100,1)
load("WAVETABLE")
load("AM")
bus_config("WAVETABLE", "aux 0 out")
bus_config("AM", "aux 0 in", "out 0")

dur = 10
// test out default sine wave
//makegen(2, 10, 1000, 1,0,1/9,0,1/25,0,1/49,0,1/81)
//makegen(2, 10, 1000, 1)
WAVETABLE(0,dur,amp=5000,freq=500)

/* -------------------------------------------------------------------------- */
setline(0,0, 1,1, 9,1, 10,0)
//makegen(2, 10, 1000, 1)               /* bipolar - for ring mod */
//makegen(2, 9, 1000, 0,.5,0, 1,.5,0)  /* unipolar - for amp mod */

freq = 0  /* use table... */
makegen(3, 18, 1000, 0,.2, 1,10, 2,80)

AM(0,0,dur,amp=1,freq)
