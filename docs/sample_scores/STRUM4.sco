/* START1
   p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
   p7 = feedback pitch (oct.pc); p8 = clean signal level
   p9 = distortion signal level; p10 = amp; p11 = squish
   p12 = stereo spread [optional]
   p13 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]
*/

rtsetparams(44100, 2)
load("STRUM")
START1(0, 7, 6.08, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, 2, .5)
START1(8, 7, 6.08, 1, 1, 10, 0.05, 7.01, 0, 1, 10000, 2, .5)
START1(16, 7, 6.08, 1, 1, 10, 0.05, 6.01, 0, 1, 10000, 2, .5)
