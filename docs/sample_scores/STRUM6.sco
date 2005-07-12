/* START1:
   p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
   p7 = feedback pitch (oct.pc); p8 = clean signal level
   p9 = distortion signal level; p10 = amp; p11 = squish
   p12 = stereo spread [optional]
   p13 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]

  BEND1:
   p0 = start; p1 = dur; p2 = pitch0 (oct.pc); p3 = pitch1 (oct.pc)
   p4 = gliss function #; p5 = fundamental decay time
   p6 = nyquist decay time; p7 = distortion gain; p8 = feedback gain
   p9 = feedback pitch (oct.pc); p10 = clean signal level
   p11 = distortion signal level; p12 = amp; p13 = update gliss nsamples
   p14 = stereo spread [optional]

  FRET1:
   p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
   p7 = feedback pitch (oct.pc); p8 = clean signal level
   p9 = distortion signal level; p10 = amp; p11 = stereo spread [optional]

*/

rtsetparams(44100, 2)
load("STRUM")
START1(0, 2, 6.08, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, 2, .5)
makegen(10, 24, 1000, 0, 0, 1, 1, 2, 0)
BEND1(2, 4, 6.08, 7.00, 10, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, 100, .5)
FRET1(6, .2, 6.10, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, .5)
FRET1(6.2, .1, 7.00, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, .5)
FRET1(6.3, .1, 7.02, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, .5)
FRET1(6.4, .1, 7.00, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, .5)
FRET1(6.5, 1, 6.10, 1, 1, 10, 0.5, 7.00, 0, 1, 10000, .5)
FRET1(7.5, 1, 6.10, 1, 1, 10, 0.5, 7.07, 0, 1, 10000, .5)
FRET1(8.5, 1, 6.10, 1, 1, 10, 0.5, 7.09, 0, 1, 10000, .5)
FRET1(9.5, 2, 6.10, 1, 1, 10, 0.5, 6.01, 0, 1, 10000, .5)
FRET1(11.5, 2, 6.10, 1, 1, 10, 0.5, 8.01, 0, 1, 10000, .5)
