/* BUTTER - time-varying Butterworth filters

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = type of filter (1: lowpass, 2: highpass, 3: bandpass, 4: bandreject)
   p5 = steepness (> 0) [optional, default is 1]
   p6 = balance output and input signals (0:no, 1:yes) [optional, default is 1]
   p7 = input channel [optional, default is 0]
   p8 = percent to left channel [optional, default is .5]
   p9 = bypass filter (0: no, 1: yes) [optional, default is 0]

   p5 (steepness) is just the number of filters to add in series.  Using more
   than 1 steepens the slope of the filter.  If you don't set p6 (balance)
   to 1, you'll need to change p3 (amp) to adjust for loss of power caused
   by connecting several filters in series.  Guard your ears!

   p6 (balance) tries to adjust the output of the filter so that it has
   the same power as the input.  This means there's less fiddling around
   with p3 (amp) to get the right amplitude when steepness is > 1.  However,
   it has drawbacks: it can introduce a click at the start of the sound, it
   can cause the sound to pump up and down a bit, and it eats extra CPU time.

   Here are the function table assignments:

      1: amplitude curve
      2: cutoff (or center) frequency curve, described by time,cf pairs
         (e.g., gen 18)
      3: bandwidth curve, described by time,bw pairs [only for bandpass and
         bandreject types].  If positive, bandwidth is in Hz; if negative,
         the '-' sign acts as a flag to interpret the bw values as percentages
         (from 0 to 1) of the current cf.

   John Gibson (johgibso at indiana dot edu), 12/1/01.
*/
rtsetparams(44100, 1)
load("WAVETABLE")
load("BUTTER")

// feed wavetable into filter
bus_config("WAVETABLE", "aux 0 out")
bus_config("BUTTER", "aux 0 in", "out 0")

dur = 10.0
amp = 10000
pitch = 7.00
makegen(2, 10, 15000,
   1, 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9, 1/10, 1/11, 1/12,
   1/13, 1/14, 1/15, 1/16, 1/18, 1/19, 1/20, 1/21, 1/22, 1/23, 1/24)  // saw
reset(10000)
WAVETABLE(0, dur, amp, pitch)
WAVETABLE(0, dur, amp, pitch+.0005)

type = 4   // 1: lowpass, 2: highpass, 3: bandpass, 4: bandreject
amp = 1.0
steepness = 5

if (type == 1) {
   balance = 0
   lowcf = 500
   highcf = 5000
}
else if (type == 2) {
   balance = 1
   amp = amp * .4
   lowcf = 1
   highcf = 1400
}
else if (type == 3) {
   balance = 0
   amp = amp * 2
   lowcf = 1
   highcf = 2400
   bw = 200
}
else if (type == 4) {
   balance = 0
   amp = amp * .5
   lowcf = 500
   highcf = 4000
   bw = 300
}
setline(0,0, 1,1, 7,1, 10,0)
makegen(2, 18, 2000,
   0,lowcf, dur*.2,lowcf, dur*.5,highcf, dur*.9,lowcf, dur,lowcf)
makegen(3, 18, 2000, 0, bw, 1, bw)
BUTTER(0, 0, dur, amp, type, steepness, balance)

