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
      system("rm -f m1.snd")
      system("sfcreate -c 1 -t sun -i m1.snd")
      output("m1.snd")
   }
   else if (format == aif) {
      system("rm -f m1.aif")
      system("sfcreate -c 1 -i m1.aif")
      output("m1.aif")
   }
   else if (format == wav) {
      system("rm -f m1.wav")
      system("sfcreate -c 1 -t wav -i m1.wav")
      output("m1.wav")
   }
   else if (format == ircam) {
      system("rm -f m1.sf")
      system("sfcreate -c 1 -t ircam -i m1.sf")
      output("m1.sf")
   }

   mix(outskip, inskip, dur, amp, 0)
}

else {
   rtsetparams(44100, 1, 32768 / 2)   /* to match disk-based buffer size */
   set_option("clobber_on")
   if (rtlevel == 1)
      set_option("audio_off")

   rtinput(infile)

   if (rtlevel != 2) {
      if (format == sun)
         rtoutput("m1rt.snd", "sun")
      else if (format == aif)
         rtoutput("m1rt.aif", "aiff")
      else if (format == wav)
         rtoutput("m1rt.wav", "wav")
      else if (format == ircam)
         rtoutput("m1rt.sf", "ircam")
   }

   MIX(outskip, inskip, dur, amp, 0,0)
}

