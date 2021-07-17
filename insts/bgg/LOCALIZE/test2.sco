/*
   p[0] = output skip time
   p[1] = input skip time
   p[2] = duration
   *p[3] = overall amp
   *p[4] = source x
   *p[5] = source y
   *p[6] = source z
   *p[7] = dest x
   *p[8] = dest y
   *p[9] = dest z
   *p[10] = headwidth (units)
   p[11] = feet/unit scaler
   p[12] = input channel
   p[13] = behind head filter on/off
   p[14] = amp/distance calculation flag
      0: no amp/distance
      1: linear amp/distance
      2: inverse square amp/distance
   p[15] = minimum amp/distance multiplier
   p[16] = maximum distance (for linear amp/distance scaling)

   * p-fields marked with an asterisk can receive dynamic updates
   from a table or real-time control source
*/

rtsetparams(44100, 2)
load("./libLOCALIZE.so")

reset(44100)

rtinput("/snd/loocher441.aif")
//rtinput("/snd/ww1a.wav")

amp = maketable("line", 1000, 0,0, 1,1, 2,0)

X = maketable("line", "nonorm", 1000, 0,-10, 1,10)
Y = maketable("line", "nonorm", 1000, 0,20, 1,-20)
//X = maketable("line", "nonorm", 1000, 0,0.15, 1,0.15)
//Y = maketable("line", "nonorm", 1000, 0,4, 1,4)

LOCALIZE(0, 0, 9, 1,  X, Y, 0.0, 0.0, 0.0, 0.0,  1.0, 1, 0,   1,
	2, 0.01)
