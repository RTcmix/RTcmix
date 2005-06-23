rtsetparams(44100, 2)
load("./libMSAXOFONY.so")

/* MSAXOFONY - the "Saxofony" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = noise gain (0.0-1.0)
   p5 = max pressure (0.0-1.0)
   p6 = reed stiffness (0.0-1.0)
   p7 = reed aperture (0.0-1.0)
   p8 = blow position (0.0-1.0)
   p9 = percent of signal to left output channel [optional, default is .5]
*/

noiseamp = maketable("line", 1000, 0,1, 1,0, 2,1)
aperture = makeLFO("sine", 2.0, 0.1, 0.7)
blowpos = maketable("line", 1000, 0,0.5, 1,0.2, 3,0.8)
MSAXOFONY(0, 3.5, 15000.0, 243.0, noiseamp, 0.7, 0.5, aperture, blowpos)

amp = maketable("line", 1000, 0,0,1,1, 2,0)
freq = makeLFO("sine", 4.0, 145, 150)
reed = makeLFO("sine", 1.5, 0.1, 0.9)
pan = maketable("line", 1000, 0,0.5, 1,1, 2,0, 2.5,1)
MSAXOFONY(4, 3.5, amp*10000.0, freq, 0.2, 0.7, reed, 0.3, 0.6, pan)

bamp = maketable("line", 1000, 0,0, 2,1, 10,1, 11,0)
MSAXOFONY(8, 3.5, 20000.0, 178.0, 0.2, 0.7, 0.5, 0.3, 0.6, 0.5, bamp)
