# Illustrates using a function to play one note consisting of 
# multiple RTcmix notes. Direct translation of risset_bell.sco.

from rtcmix import *

rtsetparams(44100, 2)
load("WAVETABLE")

# Set to 1 to write a sound file (if processor too slow for rt playback).
writeit = 0

if writeit:
	set_option("play = off", "clobber = on")
	rtoutput("/tmp/bell.aiff")

# just a sine wave (try extra harmonics for more complex bell sound)
wavet = maketable("wave", 5000, "sine")

# exponential amplitude envelope
env = maketable("expbrk", 1000,  1, 1000, .0005)

# Play one bell note.  Parameters are start time, duration, amplitude,
# and fundamental frequency.  (Based on description of Risset's bell
# instrument in the Dodge book.)

def bell(start, dur, amp, freq):
	global env, wavet
	print "fundamental frequency:", freq
	WAVETABLE(start, dur,        amp * env,        freq * .56,       1.0, wavet)
	WAVETABLE(start, dur * .9,   amp * .67 * env,  freq * .56 + 1,   0.0, wavet)
	WAVETABLE(start, dur * .65,  amp * env,        freq * .92,       0.9, wavet)
	WAVETABLE(start, dur * .55,  amp * 1.8 * env,  freq * .92 + 1.7, 0.1, wavet)
	WAVETABLE(start, dur * .325, amp * 2.67 * env, freq * 1.19,      0.8, wavet)
	WAVETABLE(start, dur * .35,  amp * 1.67 * env, freq * 1.7,       0.2, wavet)
	WAVETABLE(start, dur * .25,  amp * 1.46 * env, freq * 2,         0.7, wavet)
	WAVETABLE(start, dur * .2,   amp * 1.33 * env, freq * 2.74,      0.3, wavet)
	WAVETABLE(start, dur * .15,  amp * 1.33 * env, freq * 3,         0.6, wavet)
	WAVETABLE(start, dur * .1,   amp * env,        freq * 3.76,      0.4, wavet)
	WAVETABLE(start, dur * .075, amp * 1.33 * env, freq * 4.07,      0.5, wavet)

# individual bell notes
#    start dur    amp   freq
bell(0,     5.0,  4000, 1000)
bell(3,     8.0,  5000, 1200)
bell(6,    12.0,  3000,  800)

incr = 0.1
st = 10
while st < 20:
	freq = (random() * 1000) + 400
	bell(st, 8, 2000, freq)
	incr += 0.2
	st += incr

# [scorefile by John Gibson]
