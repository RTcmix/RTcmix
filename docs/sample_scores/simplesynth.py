# Simple example of using a function to encapsulate a little of the work.
# Shows how to pass a tuple (or an array -- same construction) to a Python
# function and then along to an RTcmix function.
#
# John Gibson, 11 Feb 2004

from rtcmix import *

rtsetparams(44100, 2)
load("WAVETABLE")

def play(start, dur, amp, freq, pan, env, wave):
   if (env != None):
      makegen(1, *env)
   if (wave != None):
      makegen(2, *wave)
   WAVETABLE(start, dur, amp, freq, pan)

# -- play notes! ---------------------------------------------
dur = 7
env = (4, 2000, 0,0,1, 2,1,-1, dur,0)   # a makegen without the table number
wave1 = (10, 2000, 1)                   # ditto
wave2 = (10, 2000, 1, 0, 1./3, 0, 1./5, 0, 1./7)

start = 0
play(start, dur, 20000, 8.09, .2, env, wave1)

start += dur - 4
# None means: use same envelope as before
play(start, dur + 1, 2000, 9.07, .8, None, wave2)

start += dur - 5
play(start, dur - 1, 10000, 7.11, .5, None, wave1)

