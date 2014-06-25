/* RECORD:  p0 = start; p1 = duration (-endtime); p2 = inchan; p3 = audio index */
/* PLAY:  p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; p4 = aud_idx */

load("iRAP")
rtsetparams(44100, 2)
rtinput("../../../snd/stereo.wav");
bus_config("RECORD", "in0-1","aux0out")

RECORD(0,10,0,0)
setline(0,0, 10, 1, 11, 0)
PLAY(0, 0, 4, 1, 0)
PLAY(4, 3, 2, 1, 0)
PLAY(5, 2, 2, 1, 0)
PLAY(6, 1, 2, 1, 0)
PLAY(7, 0, 2, 1, 0)

