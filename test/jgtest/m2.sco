print_off()
if (n_arg() < 2) {
   str_num("usage:  CMIX rtlevel headertype < scorefile")
   str_num("  (rtlevel: 0=diskbased, 1=rtdiskonly, 2=rtaudioonly, 3=rtboth)")
   str_num("  (headertype: 0=sun, 1=aiff, 2=aifc, 3=wav, 4=ircam)")
   exit(1)
}
rtlevel = i_arg(0)
format = i_arg(1)
if (rtlevel < 0 || rtlevel > 3) {
   str_num("rtlevel: 0=diskbased, 1=rtdiskonly, 2=rtaudioonly, 3=rtboth")
   exit(1)
}
if (format < 0 || format > 4) {
   str_num("headertype: 0=sun, 1=aiff, 2=aifc, 3=wav, 4=ircam")
   exit(1)
}
sun = 0; aiff = 1; aifc = 2; wav = 3; ircam = 4;
print_on()
/* -------------------------------------------------------------------------- */

infile = stringify("test.snd")

outskip = 0.0
inskip = 0.0
dur = .5
amp = 1

if (rtlevel == 0) {
   load("mix")

   input(infile)

   if (format == sun) {
      system("rm -f m2.snd")
      system("sfcreate -c 1 -t sun -f m2.snd")
      output("m2.snd")
   }
   else if (format == aiff) {
      system("rm -f m2.aiff")
      system("sfcreate -c 1 -f m2.aiff")
      output("m2.aiff")
   }
   else if (format == aifc) {
      system("rm -f m2.aifc")
      system("sfcreate -c 1 -t aifc -f m2.aifc")
      output("m2.aifc")
   }
   else if (format == wav) {
      system("rm -f m2.wav")
      system("sfcreate -c 1 -t wav -f m2.wav")
      output("m2.wav")
   }
   else if (format == ircam) {
      system("rm -f m2.sf")
      system("sfcreate -c 1 -t ircam -f m2.sf")
      output("m2.sf")
   }

   mix(outskip, inskip, dur, amp, 0)
}

else {
   rtsetparams(44100, 1, 32768 / 4)  /* to match disk-based buffer size */
   set_option("clobber_on")
   if (rtlevel == 1)
      set_option("audio_off")
   bus_config("MIX", "in 0", "out 0")

   rtinput(infile)

   if (rtlevel != 2) {
      if (format == sun)
         rtoutput("m2rt.snd", "sun", "float")
      else if (format == aiff)
         rtoutput("m2rt.aiff", "aiff", "float")
      else if (format == aifc)
         rtoutput("m2rt.aifc", "aifc", "float")
      else if (format == wav)
         rtoutput("m2rt.wav", "wav", "float")
      else if (format == ircam)
         rtoutput("m2rt.sf", "ircam", "float")
   }

   MIX(outskip, inskip, dur, amp, 0,0)
}

