# Simple example of using a function to encapsulate a little of the work.
# Shows how to pass a tuple (or an array -- same construction) to a Python
# function and then along to an RTcmix function.
#
# John Gibson, 2/11/2004; rev 7/12/05

from rtcmix import *

rtsetparams(44100, 2)
load("WAVETABLE")

def play(start, dur, amp, freq, pan, env, wave):
   env = maketable("curve", 2000, *env)
   wavet = maketable("wave", 2000, *wave)
   WAVETABLE(start, dur, amp * env, freq, pan, wavet)

# -- play notes! ---------------------------------------------
dur = 7
env = (0,0,1, 2,1,-1, dur,0)   # args to maketable("curve"...)
wave1 = ("sine", )             # args to maketable("wave"...)
wave2 = (1, 0, 1./3, 0, 1./5, 0, 1./7)

start = 0
play(start, dur, 20000, 8.09, .2, env, wave1)

start += dur - 4
play(start, dur + 1, 2000, 9.07, .8, env, wave2)

start += dur - 5
play(start, dur - 1, 10000, 7.11, .5, env, wave1)

