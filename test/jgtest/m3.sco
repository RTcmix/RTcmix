print_off()
if (n_arg() < 2) {
   str_num("usage:  CMIX rtlevel < scorefile")
   str_num("  (rtlevel: 0=diskbased, 1=rtdiskonly, 2=rtaudioonly, 3=rtboth)")
   exit(1)
}
rtlevel = i_arg(0)
if (rtlevel < 0 || rtlevel > 3) {
   str_num("rtlevel: 0=diskbased, 1=rtdiskonly, 2=rtaudioonly, 3=rtboth")
   exit(1)
}
print_on()
/* -------------------------------------------------------------------------- */

infile = stringify("num.snd")

dur = trunc(2.0)
amp = 1

if (rtlevel == 0) {
   load("mix")

   input(infile)
   system("rm -f m3.aif")
   system("sfcreate m3.aif")
   output("m3.aif")
   for (st=0; st < dur; st = st + .5) {
      odd = (st - trunc(st) > 0)
      mix(st, 0, .2, amp, odd)
   }
}

else {
   rtsetparams(44100, 2, 32768 / 2)   /* to match disk-based buffer size */
   set_option("clobber_on")
   if (rtlevel == 1)
      set_option("audio_off")

   rtinput(infile)

   if (rtlevel != 2)
      rtoutput("m3rt.aif", "aiff")

   for (st=0; st < dur; st = st + .5) {
      odd = (st - trunc(st) > 0)
      MIX(st, 0, .2, amp, odd)
   }
}
