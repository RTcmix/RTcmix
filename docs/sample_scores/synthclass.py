# Example of making a synth object and then playing notes on it.
# The class definition is first, followed by the "score".  You could
# put classes like this into their own files, and load them in with
# an import statement (as is done for the rtcmix extension).
#
# NOTE: This instrument can't play overlapping notes in a way that
# you would expect, because the entire stream of WIGGLE notes flows
# through the waveshaper and filter instruments.
#
# To run this script, type: "PYCMIX < synthclass.py"
#
# John Gibson, 8 Jan 2004

from rtcmix import *

rtsetparams(44100, 2)
load("BUTTER")
load("SHAPE")
load("WIGGLE")

tempo(0, 120)

class Synth:
   def __init__(self):
      bus_config("WIGGLE", "aux 0 out")
      bus_config("SHAPE", "aux 0 in", "aux 2-3 out")
      bus_config("BUTTER", "aux 2-3 in", "out 0-1")
      self.__wigamp = 20000
      self.__modtype = 2
      self.__vibseed = 4
      self.__detune = 0.001
      self.__shwarp = 0
      self.__shjaggedness = 8
      self.__shquantum = 0.5
      self.__shminindex = 0.0
      self.__ftype = 3
      self.__fsteep = 1
      self.__mincf = 200
      self.__maxcf = 10000
      self.__bwpct = -0.18

   def play(self, start, dur, gain, pitch, modfreq, moddepth, shspeed, shseed,
                                                            shmaxindex, fseed):
      """Play one note, and return <start> + <dur>.  <start>, <dur> and the
         return value are in beats."""

      # start, dur and shspeed are in beats; convert to time
      start = tb(start)
      dur = tb(dur)
      shspeed = 1 / tb(shspeed)

      # synthesizer -----------------------------------------------------------
      makegen(2, 10, 2000, 1,0,1,0,1,0,1,0,1,0,1,0,1)
      makegen(3, 18, 2000, 0,0, 1,0)
      makegen(4, 20, 15, 1, self.__vibseed)
      makegen(5, 18, 2000, 0,modfreq, 1,modfreq)
      makegen(6, 18, 2000, 0,moddepth, 1,moddepth)
      WIGGLE(start, dur, self.__wigamp, pitch, self.__modtype)
      WIGGLE(start, dur, self.__wigamp, pitch + self.__detune, self.__modtype)

      # wave shaper -----------------------------------------------------------
      makegen(2, 4, 1000, 0,-1,self.__shwarp, 1,0,-self.__shwarp, 2,1)
      shsize = dur * shspeed
      makegen(3, 20, shsize, 1, shseed)
      copygen(3, 3, shsize * self.__shjaggedness, 0)
      quantizegen(3, self.__shquantum)
      #fplot(3, 5, "with lines")
      makegen(99, 4, 1000, 0,1,-2, 1,0)  # normalization function
      #fplot(99, 5)
      reset(20000)
      amp = ampdb(gain)
      SHAPE(start, 0, dur, amp, self.__shminindex, shmaxindex, 99, 0, 1)
      # vary distortion index for other channel
      makegen(3, 20, shsize, 1, shseed + 1)
      copygen(3, 3, shsize * self.__shjaggedness, 0)
      quantizegen(3, self.__shquantum)
      SHAPE(start, 0, dur, amp, self.__shminindex, maxindex, 99, 0, 0)

      # filter ----------------------------------------------------------------
      reset(5000)
      amp = 3.0
      speed = shspeed * 0.8
      shsize = dur * shspeed
      makegen(-2, 20, shsize, 1, fseed, self.__mincf, self.__maxcf) 
      copygen(2, 2, shsize * self.__shjaggedness, 0)
      quantizegen(2, self.__shquantum)
      #fplot(2, 5, "with lines")
      makegen(-3, 18, 1000, 0,self.__bwpct, 1,self.__bwpct)
      BUTTER(start, 0, dur, amp, self.__ftype, self.__fsteep, 0, 0, 1)
      BUTTER(start, 0, dur, amp, self.__ftype, self.__fsteep, 0, 1, 0)

      return bt(start + dur)

   def set_steep(self, steep=1):
      self.__fsteep = steep

   def set_cf(self, mincf=200, maxcf=10000):
      self.__mincf = mincf
      self.__maxcf = maxcf

   def set_detune(self, detune=0.001):
      self.__detune = detune

   def set_warp(self, warp=0):
      self.__shwarp = warp


# play notes -------------------------------------------------------------------

synth = Synth()

pitches = (6.05, 6.10, 6.08, 7.01)
durs = (2, 1.5, .75, 4.1)
modfreq = 5
moddepth = 1
shspeed = 0.25    # beats
shseed = 3
fseed = 1
maxindex = 5.0
gain = 4.0        # dB
synth.set_cf(mincf=80)
synth.set_warp(-2)

start = 0.0       # beats
for n in range(4):
   pitch = pitches[n]
   dur = durs[n]
   start = synth.play(start, dur, gain, pitch, modfreq, moddepth, shspeed,
                     shseed, maxindex, fseed)
   shseed += 1
   fseed += 1


