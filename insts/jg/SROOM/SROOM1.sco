/* SROOM - room simulation with stationary sound source
      p0     output start time
      p1     input start time
      p2     input duration
      p3     amplitude multiplier
      p4     distance from middle of room to right wall (i.e., 1/2 of width)
      p5     distance from middle of room to front wall (i.e., 1/2 of depth)
      p6,p7  x,y position of source  (middle of room is 0,0)
      p8     reverb time (in seconds)
      p9     reflectivity (0 - 100; the higher, the more reflective)
      p10    "inner room" width (try 8)
      p11    input channel number   [optional]
*/
rtsetparams(44100, 2)
load("SROOM")

rtinput("/tmp/1stmove.snd")

outskip = 0
inskip = 13
dur = 9
amp = 0.8
xdim = 20
ydim = 20
xsrc = 10
ysrc = 10
rvbtime = 1.0
reflect = 90.0
innerwidth = 4.0
inchan = 0

SROOM(outskip, inskip, dur, amp,
      xdim, ydim, xsrc, ysrc, rvbtime, reflect, innerwidth, inchan)

outskip = 12
amp = 0.7
ydim = 50
xsrc = -30
reflect = 80.0

SROOM(outskip, inskip, dur, amp,
      xdim, ydim, xsrc, ysrc, rvbtime, reflect, innerwidth, inchan)

