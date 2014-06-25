/* MROOM - room simulation with moving source

   First call timeset at least twice to set trajectory of moving
   sound source:
      p0     time
      p1     x location
      p2     y location

   Then call MROOM:
      p0     output start time
      p1     input start time
      p2     input duration
      p3     amplitude multiplier
      p4     distance from middle of room to right wall (i.e., 1/2 of width)
      p5     distance from middle of room to front wall (i.e., 1/2 of depth)
      p6     reverb time (in seconds)
      p7     reflectivity (0 - 100; the higher, the more reflective)
      p8     "inner room" width (try 8)
      p9     input channel number   [optional]
      p10    control rate for trajectory   [optional]
*/
rtsetparams(44100, 2)
load("MROOM")

rtinput("/tmp/1stmove.snd")

writeit = 0
if (writeit) {
   set_option("clobber_on", "audio_off")
   rtoutput("MROOM1.snd", "sun", "float")
}

outskip = 0
inskip = 13
dur = 9
amp = 0.35
xdim = 30
ydim = 80
rvbtime = 1.0
reflect = 90.0
innerwidth = 8.0
inchan = 0
quant = 2000

timeset(0,     0-xdim, 0-ydim)
timeset(dur/2, xdim/8, ydim/8)
timeset(dur,   xdim,   ydim)

setline(0,0, dur/8,1, dur-.5,1, dur,0)

MROOM(outskip, inskip, dur, amp,
      xdim, ydim, rvbtime, reflect, innerwidth, inchan, quant)

