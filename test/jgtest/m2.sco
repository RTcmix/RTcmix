print_off()
if (n_arg() < 3) {
   str_num("usage:  CMIX rtlevel headertype < scorefile")
   str_num("  (rtlevel: 0=diskbased, 1=rtdiskonly, 2=rtaudioonly, 3=rtboth)")
   str_num("  (headertype: 0=sun, 1=aif, 2=wav, 3=ircam)")
   exit(1)
}
rtlevel = i_arg(0)
format = i_arg(1)
if (rtlevel < 0 || rtlevel > 3) {
   str_num("rtlevel: 0=diskbased, 1=rtdiskonly, 2=rtaudioonly, 3=rtboth")
   exit(1)
}
if (format < 0 || format > 3) {
   str_num("headertype: 0=sun, 1=aif, 2=wav, 3=ircam")
   exit(1)
}
sun = 0; aif = 1; wav = 2; ircam = 3;
print_on()
/* -------------------------------------------------------------------------- */

infile = stringify("num.snd")

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
   else if (format == aif) {
      system("rm -f m2.aif")
      system("sfcreate -c 1 -f m2.aif")
      output("m2.aif")
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

   rtinput(infile)

   if (rtlevel != 2) {
      if (format == sun)
         rtoutput("m2rt.snd", "sun", "float")
      else if (format == aif)
         rtoutput("m2rt.aif", "aiff", "float")
      else if (format == wav)
         rtoutput("m2rt.wav", "wav", "float")
      else if (format == ircam)
         rtoutput("m2rt.sf", "ircam", "float")
   }

   MIX(outskip, inskip, dur, amp, 0,0)
}

