/* DISTORT - time-varying Butterworth filters

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = type of distortion (1: soft clip, 2: tube)
        [NOTE: 2 doesn't work correctly yet!]
   p5 = gain (before distortion)
   p6 = cutoff freq for low-pass filter (in cps)  (0 to disable filter)
        (The filter comes after the distortion in the signal chain.)
   p7 = input channel [optional, default is 0]
   p8 = percent to left channel [optional, default is .5]
   p9 = bypass all processing (0: no, 1: yes) [optional, default is 0]

   Function table 1 is the amplitude envelope.

   John Gibson (johgibso at indiana dot edu), 8/12/03.
   Distortion algorithms taken from STRUM, by Charlie Sullivan.
*/
rtsetparams(44100, 2)
load("WAVETABLE")
load("DISTORT")

bus_config("WAVETABLE", "aux 0 out")
bus_config("DISTORT", "aux 0 in", "out 0-1")

dur = 6.0
amp = 10000
pitch = 7.00
makegen(2, 10, 15000,
   1, 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9, 1/10, 1/11, 1/12,
   1/13, 1/14, 1/15, 1/16, 1/18, 1/19, 1/20, 1/21, 1/22, 1/23, 1/24)  // saw
reset(10000)
WAVETABLE(0, dur, amp, pitch)
WAVETABLE(0, dur, amp, pitch+.0002)

// distort wavetable output
bypass = 0
type = 1   // 1: soft clip, 2: tube
amp = 0.1
gain = 20.0
cf = 0
setline(0,0, 1,1, 7,1, 10,0)
DISTORT(0, 0, dur, amp, type, gain, cf, 0, 1, bypass)
DISTORT(0.2, 0, dur, amp, type, gain, cf, 0, 0, bypass)

