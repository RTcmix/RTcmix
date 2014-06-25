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

   p0  input start time
   p1  input duration (use dur(0) to get total file duration)
   p2  N: number of bandpass filters (size of FFT) [must be power of 2]
   p3  M: analysis window length [N]
   p4  L: synthesis window length [M]
   p5  D: decimation factor [M/8]
   p6  b: begin time in noise reference soundfile
   p7  e: end time in noise reference soundfile
   p8  t: threshold above noise reference in dB [try 30]
   p9  s: sharpness of noise-gate turnoff [1-5]
   p10 n: number of fft frames to average over [5]
   p11 m: minimum gain of noise-gate when off, in dB [-40]
   p12 channel number (0=left, 1=right)
*/
load("denoise")

system("rm -f denoise1.snd")
system("F1 denoise1.snd")
output("denoise1.snd")

input("needs_cleaning_mono.snd")
inskip = 0
input("noiseref_mono.snd", 2)

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
chan = 0

denoise(inskip, dur(0), N, M, L, D, b, e, t, s, n, m, chan)

