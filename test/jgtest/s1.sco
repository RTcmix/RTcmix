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
amp = 1.0

setline(0,1, 1,1)

if (rtlevel == 0) {
   load("mix")

   input(infile)

   if (format == sun) {
      system("rm -f s1.snd")
      system("sfcreate -t sun -i s1.snd")
      output("s1.snd")
   }
   else if (format == aif) {
      system("rm -f s1.aif")
      system("sfcreate -i s1.aif")
      output("s1.aif")
   }
   else if (format == wav) {
      system("rm -f s1.wav")
      system("sfcreate -t wav -i s1.wav")
      output("s1.wav")
   }
   else if (format == ircam) {
      system("rm -f s1.sf")
      system("sfcreate -t ircam -i s1.sf")
      output("s1.sf")
   }

   dur = dur(0)
   stereo(outskip, inskip, dur, amp, pan=.3)
}

else {
   load("STEREO")
   rtsetparams(44100, 2)
   set_option("clobber_on")
   if (rtlevel == 1)
      set_option("audio_off")

   rtinput(infile)

   if (rtlevel != 2) {
      if (format == sun)
         rtoutput("s1rt.snd", "sun")
      else if (format == aif)
         rtoutput("s1rt.aif", "aiff")
      else if (format == wav)
         rtoutput("s1rt.wav", "wav")
      else if (format == ircam)
         rtoutput("s1rt.sf", "ircam")
   }

   dur = DUR()
   STEREO(outskip, inskip, dur, amp, pan=.3)
}

