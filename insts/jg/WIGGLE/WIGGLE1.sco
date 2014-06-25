/* WIGGLE - wavetable oscillator with frequency modulation and filter

   This instrument is like WAVETABLE, except that it lets you change
   the pitch with a glissando curve and/or frequency modulation.  The
   modulation can be subaudio rate (for vibrato) or audio rate (for
   basic Chowning FM).  There is an optional filter, either lowpass or
   highpass.  Many of the parameters are time-varying, specified by
   gen functions.

   p0 = output start time
   p1 = duration
   p2 = carrier amplitude
   p3 = carrier oscillator frequency (or oct.pc if < 15)
   p4 = modulator depth control type (0: no modulation at all, 1: percent
        of carrier frequency, 2: modulation index) [optional, default is 0]
   p5 = type of filter (0: no filter, 1: low-pass, 2: high-pass)
        [optional, default is 0]
   p6 = steepness (> 0) [optional, default is 1]
   p7 = balance output and input signals (0:no, 1:yes) [optional, default is 0]

   p4 (modulator depth control type) tells the instrument how to interpret
   the values in the modulator depth function table.  You can express these
   as a percentage of the carrier (useful for subaudio rate modulation) or
   as a modulation index (useful for audio rate FM).  If you don't want
   to use the modulating oscillator at all, pass 0 for this pfield.  Then
   you don't need to create function tables 4-6.

   p6 (steepness) is just the number of filters to add in series.  Using more
   than 1 steepens the slope of the filter.  If you don't set p7 (balance)
   to 1, you'll need to change p2 (carrier amp) to adjust for loss of power
   caused by connecting several filters in series.

   p7 (balance) tries to adjust the output of the filter so that it has
   the same power as the input.  This means there's less fiddling around
   with p2 (amp) to get the right amplitude when steepness is > 1.  However,
   it has drawbacks: it can introduce a click at the start of the sound, it
   can cause the sound to pump up and down a bit, and it eats extra CPU time.

   Here are the function table assignments:

      1: amplitude curve (setline)
      2: carrier oscillator waveform (e.g., gen 9 or 10)
      3: carrier glissando curve (linear octave offsets from p3 frequency)
      4: modulator oscillator waveform
      5: modulator frequency (in Hz; or, if negative, ratio to carrier freq)
         E.g., "makegen(5, 18, 1000, 0,-2, 1,-2.3)" will change gradually from
         a C:M ratio of 1:2 to a ratio of 1:2.3 over the course of the note.
      6: modulator depth (p4 determines how these values are interpreted)
      7: filter cutoff frequency
      8: pan curve (from 0 to 1)

   NOTE: The glissando table is read without interpolation.  This was done
   on purpose, to permit sub-audio modulation with "jagged edges."
   For example, if you say
      makegen(3, 20, 1, 10)
   you'll get audible random stairsteps (with no transitions between the
   steps).

   John Gibson <johgibso at indiana dot edu>, 12/4/01.
*/
rtsetparams(44100, 2)
load("WIGGLE")

dur = 12
amp = 10000
pitch = 8.00
mod_depth_type = 1      /* % of car freq */

setline(0,0, 1,1, 2,1, 5,0)

makegen(2, 10, 8000, 1,1/2,1/3,1/4,1/5,1/6,1/7,1/8)  /* car waveform */
makegen(3, 18, 1000, 0,-0.02, 1,0.07)                /* car gliss */
makegen(4, 10, 8000, 1)                              /* mod waveform */
makegen(5, 18, 1000, 0,20, 1,3, 2,0)                 /* mod freq */
makegen(6, 18, 1000, 0,1, 1,20, 2,5)                 /* mod depth */
makegen(8, 18, 5000, 0,1, 1,.9, 5,0)                 /* pan */

WIGGLE(st=0.00, dur, amp, pitch, mod_depth_type)

amp = amp * 0.4
makegen(2, 10, 8000, 1,0,1/9,0,1/25,0,1/49)          /* car waveform */
makegen(3, 18, 1000, 0,0.02, 3,-0.10, 4,-2.05)       /* car gliss */
makegen(5, 18, 1000, 0,21, 1,1, 2,0)                 /* mod freq */
makegen(6, 18, 1000, 0,1, 1,20, 2,8)                 /* mod depth */
makegen(8, 18, 5000, 0,0, 2,.1, 5,1)                 /* pan */

WIGGLE(st=0.05, dur, amp, pitch+2.005, mod_depth_type)

