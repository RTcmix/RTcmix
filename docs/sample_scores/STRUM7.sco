/* VSTART1:
   p0 = start; p1 = dur; p2 = pitch (oct.pc); p3 = fundamental decay time
   p4 = nyquist decay time; p5 = distortion gain; p6 = feedback gain
   p7 = feedback pitch (oct.pc); p8 = clean signal level
   p9 = distortion signal level; p10 = amp; p11 = squish
   p12 = low vibrato freq range; p13 = hi vibrato freq range
   p14 = vibrato freq depth (expressed in cps); p15 = random seed value
   p16 = pitch update (default 200/sec)
   p17 = stereo spread [optional]
   p18 = flag for deleting pluck arrays (used by FRET, BEND, etc.) [optional]
   assumes makegen 1 is the amplitude envelope, makegen 2 is the vibrato
   function, and makegen 3 is the vibrato amplitude envelope
*/

rtsetparams(44100, 2)
load("STRUM")
makegen(1, 18, 1000, 0,1, 19,1, 20,0)
makegen(2, 10, 1000, 1)
makegen(3, 24, 1000, 0, 0, 1, 1, 2, 0)
VSTART1(0, 7, 7.07, 1, 1, 10, 0.05, 7.00, 0, 1, 10000, 2, 4, 7, 5, 0.314, 200, .5)
