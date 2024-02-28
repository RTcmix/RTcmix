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
#
# I converted this to use tables instead of gens, and the rhythm
# changed noticeably, and not for the better. But I left it, because
# makegen should go away.  -JG, 9/25/20 (gasp)

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
      carwave = maketable("wave", 2000, 1,0,1,0,1,0,1,0,1,0,1,0,1)
      cargliss = maketable("line", 2000, 0,0, 1,0)
      modwave = maketable("random", 15, "low", -1, 1, self.__vibseed)
      WIGGLE(start, dur, self.__wigamp, pitch, self.__modtype, 0, 0, 0, carwave, modwave, modfreq, moddepth)
      WIGGLE(start, dur, self.__wigamp, pitch + self.__detune, self.__modtype, 0, 0, 0, carwave, modwave, modfreq, moddepth)

      # wave shaper -----------------------------------------------------------
      func = maketable("curve", 1000, 0,-1,self.__shwarp, 1,0,-self.__shwarp, 2,1)
      shsize = dur * shspeed
      guide = maketable("random", shsize, "low", 0, 1, shseed)
      guide = copytable(guide, shsize * self.__shjaggedness, "nointerp")
      guide = makefilter(guide, "quantize", self.__shquantum)
      #plottable(guide, 5, "with lines")
      normtab = maketable("curve", 1000, 0,1,-2, 1,0)  # normalization function
      #plottable(normtab, 5, "with lines")
      control_rate(20000)
      amp = ampdb(gain)
      SHAPE(start, 0, dur, amp, self.__shminindex, shmaxindex, normtab, 0, 1, func, guide)
      # vary distortion index for other channel
      guide = maketable("random", shsize, "low", 0, 1, shseed + 1)
      guide = copytable(guide, shsize * self.__shjaggedness, "nointerp")
      guide = makefilter(guide, "quantize", self.__shquantum)
      SHAPE(start, 0, dur, amp, self.__shminindex, maxindex, normtab, 0, 0, func, guide)

      # filter ----------------------------------------------------------------
      reset(5000)
      amp = 3.0
      speed = shspeed * 0.8
      shsize = dur * shspeed
      cf = maketable("random", "nonorm", shsize, "low", self.__mincf, self.__maxcf, fseed)
      cf = copytable(cf, shsize * self.__shjaggedness, "nointerp")
      cf = makefilter(cf, "quantize", self.__shquantum)
      #plottable(cf, 5, "with lines")
      BUTTER(start, 0, dur, amp, self.__ftype, self.__fsteep, 0, 0, 1, 0, cf, self.__bwpct)
      BUTTER(start, 0, dur, amp, self.__ftype, self.__fsteep, 0, 1, 0, 0, cf, self.__bwpct)

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


