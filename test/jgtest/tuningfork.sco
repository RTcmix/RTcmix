/* Simple tuning fork script, by John Gibson.
   Shows how cmix scripts can take command line args and print err msgs.
   Can use this with the ``tune'' shell script in this dir.
*/
print_off()
if (n_arg() < 2) {
   str_num("usage:  CMIX freq [dur [amp]] < tuningfork.sco")
   str_num("        (freq in hz or oct.pc; amp 1-32767, 10000 if omitted)")
   exit(1)
}

sr = 44100
envramp = .05
default_dur = .5
default_amp = 10000

freq = f_arg(0)
if (freq <= 0 || freq > sr / 2) {      /* NB: if < 15, oct.pc (conv in inst) */
  xstr_num("freq out of range")
   exit(1)
}

if (n_arg() > 2) {
   dur = f_arg(1)
   if (n_arg() > 3)
      amp = f_arg(2)
   else
      amp = default_amp
}
else {
   dur = default_dur
   amp = default_amp
}

if (dur < envramp * 2 || dur > 30) {
   str_num("dur must be between ", envramp * 2, " and 30")
   exit(1)
}
if (amp < 1 || amp > 32767) {
   str_num("amp must be between 1 and 32767")
   exit(1)
}

/* -------------------------------------------------------------------------- */
rtsetparams(sr, 2)
load("WAVETABLE")

makegen(1, 18, 5000, 0,0, envramp,1, dur-envramp,1, dur,0)
makegen(2, 10, 5000, 1)
reset(10000)
WAVETABLE(0, dur, amp, freq, .5)
