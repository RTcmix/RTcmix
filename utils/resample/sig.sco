/* This makes a short test tone file at 48000kHz sr. */

rtsetparams(48000, 1)
load("WAVETABLE")

writeit = 1

if (writeit) {
   set_option("clobber_on", "audio_off")
   rtoutput("sig.au", "sun")
}

start = 0.0
dur = 5.0
amp = 10000
freq = 440
spread = 0

makegen(1, 10, 12000, 1)  /* sine wave */
makegen(2, 18, 1000, 0,0, .2,1, dur-.2,1, dur,0)

WAVETABLE(start, dur, amp, freq, spread)

