/* This cmix instrument was ported from the carl package, where it was
   a command-line filter program. The man page for denoise, as well as
   Dolson's comments at the top of denoise.c, is still essential reading,
   even though the interface to the cmix instrument is different.
   The variable names for the cmix pfields are the same as the names
   in the denoise documentation.

   Generally, you want to leave M, L and D alone. The duration of the
   excerpt from the noise reference file must be at least .25 seconds.

   First, open the input file as unit 0, the output file as unit 1,
   and the noise reference file as unit 2. (The noise reference file
   can be the same as the input file.) Denoise can only process one
   channel at a time.

   p0  N: number of bandpass filters (size of FFT) [must be power of 2]
   p1  M: analysis window length [N]
   p2  L: synthesis window length [M]
   p3  D: decimation factor [M/8]
   p4  b: begin time in noise reference soundfile
   p5  e: end time in noise reference soundfile
   p6  t: threshold above noise reference in dB [try 30]
   p7  s: sharpness of noise-gate turnoff [1-5]
   p8  n: number of fft frames to average over [5]
   p9  m: minimum gain of noise-gate when off, in dB [-40]
   p10 channel number (0=left, 1=right)
*/
load("denoise")

system("rm -f denoise2.snd")
system("F2 denoise2.snd")
output("denoise2.snd")

input("needs_cleaning_stereo.snd")
input("noiseref_stereo.snd", 2)

N = 1024 * 4
M = N
L = M
D = M/8
b = 5.7
e = 6.0
t = 30
s = 1
n = 5
m = -40

denoise(N, M, L, D, b, e, t, s, n, m, chan=0)
denoise(N, M, L, D, b, e, t, s, n, m, chan=1)

