/* WIGGLE - wavetable oscillator with frequency modulation and filter

   This instrument is a kind of combination of WAVETABLE and FMINST.
   The time-varying capabilities in the first version of WIGGLE are
   now possible with WAVETABLE and FMINST, so WIGGLE may no longer be
   worth the bother, especially since a lot of complexity resulted from
   making it backward-compatible with older scores while supporting the
   new features of RTcmix v4.  Here it is anyway...

   If you write new scores for it, use all 14 of the pfields described
   below, and make sure that there are no makegens in your score.  The
   documentation below does not say how WIGGLE operates with the old gen
   scheme, but it will still run old scores with no change in sound.

   The parameters marked with '*' can receive dynamic updates from a table
   or a real-time control source.

      p0  = output start time
      p1  = duration
    * p2  = carrier amplitude
    * p3  = carrier oscillator frequency - Hz or oct.pc (see note 1)
      p4  = modulator depth control type (0: no modulation at all, 1: percent
            of carrier frequency, 2: modulation index - see note 2)
      p5  = type of filter (0: no filter, 1: low-pass, 2: high-pass)
      p6  = steepness (> 0) - see note 3
      p7  = balance output and input signals (0:no, 1:yes) - see note 4
      p8  = carrier wavetable
      p9  = modulator wavetable
    * p10 = modulator frequency - see note 5
    * p11 = modulator depth - see note 2
    * p12 = lowpass filter cutoff frequency
    * p13 = pan (in percent-to-left form: 0-1)

   NOTES

   1. Oct.pc format generally will not work as you expect for p3 (car freq)
      if the pfield changes dynamically.  Use Hz instead in that case.

   2. The modulator depth control type (p4) tells WIGGLE how to interpret 
      modulator depth values (p11).  You can express these as a percentage of
      the carrier (0-100), useful for subaudio rate modulation, or as a
      modulation index (useful for audio rate FM).  If you don't want to use
      the modulating oscillator at all, pass 0 for this pfield.

   3. Steepness (p6) is just the number of filters to add in series.  Using more
      than 1 steepens the slope of the filter.  If you don't set p7 (balance)
      to 1, you'll need to change p2 (carrier amp) to adjust for loss of power
      caused by connecting several filters in series.

   4. Balance (p7) tries to adjust the output of the filter so that it has
      the same power as the input.  This means there's less fiddling around
      with p2 (amp) to get the right amplitude when steepness is > 1.  However,
      it has drawbacks: it can introduce a click at the start of the sound, it
      can cause the sound to pump up and down a bit, and it eats extra CPU time.

   5. Modulator frequency is in Hz.  Or, if it is negative, then its absolute
      value will be interpreted as the modulator's ratio to the carrier
      frequency.  E.g...

         modfreq = maketable("line", "nonorm", 100, 0,-2, 1,-2.3)

      will change gradually from a C:M ratio of 1:2 to a ratio of 1:2.3 over
      the course of the note.

   6. The carrier and modulator wavetables can be updated in real time using
      modtable(..., "draw", ...).


   John Gibson <johgibso at indiana dot edu>, 12/4/01; rev. for v4, 6/17/05.
*/

rtsetparams(44100, 2)
load("WIGGLE")

dur = 12
amp = 10000
pitch = octpch(8.00)
mod_depth_type = 1      // % of car freq

env = maketable("line", 1000, 0,0, 1,1, 2,1, 5,0)

car_wavetable = maketable("wave", 8000, "saw8")
mod_wavetable = maketable("wave", 8000, "sine")

oct = maketable("line", "nonorm", 1000, 0,pitch - 0.02, 1,pitch + 0.07)
freq = makeconverter(oct, "cpsoct")

mod_freq = maketable("line", "nonorm", 1000, 0,20, 1,3, 2,0)
mod_depth = maketable("line", "nonorm", 1000, 0,1, 1,20, 2,5)

filter_type = 0; steep = 0; balance = false; filter_cf = 0   // no filter

pan = maketable("line", 1000, 0,1, 1,.9, 5,0)

WIGGLE(st = 0, dur, amp * env, freq, mod_depth_type, filter_type, steep,
	balance, car_wavetable, mod_wavetable, mod_freq, mod_depth, filter_cf, pan)

amp *= 0.4
car_wavetable = maketable("wave", 8000, "tri7")

pitch += 2.005
a = pitch + 0.02
b = pitch - 0.10
c = pitch - 2.05
oct = maketable("line", "nonorm", 1000, 0,a, 3,b, 4,c)
freq = makeconverter(oct, "cpsoct")

mod_freq = maketable("line", "nonorm", 1000, 0,21, 1,1, 2,0)
mod_depth = maketable("line", "nonorm", 1000, 0,1, 1,20, 2,8)
pan = maketable("line", 1000, 0,0, 2,.1, 5,1)

WIGGLE(st = 0.05, dur, amp * env, freq, mod_depth_type, filter_type, steep,
	balance, car_wavetable, mod_wavetable, mod_freq, mod_depth, filter_cf, pan)

