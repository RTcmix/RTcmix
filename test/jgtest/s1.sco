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
   else if (format == aiff) {
      system("rm -f s1.aiff")
      system("sfcreate -i s1.aiff")
      output("s1.aiff")
   }
   else if (format == aifc) {
      system("rm -f s1.aifc")
      system("sfcreate -t aifc -i s1.aifc")
      output("s1.aifc")
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

   stereo(outskip, inskip, dur, amp, pan=.3)
}

else {
   load("STEREO")
   rtsetparams(44100, 2)
   set_option("clobber_on")
   if (rtlevel == 1)
      set_option("audio_off")
   bus_config("STEREO", "in 0", "out 0-1")

   rtinput(infile)

   if (rtlevel != 2) {
      if (format == sun)
         rtoutput("s1rt.snd", "sun")
      else if (format == aiff)
         rtoutput("s1rt.aiff", "aiff")
      else if (format == aifc)
         rtoutput("s1rt.aifc", "aifc")
      else if (format == wav)
         rtoutput("s1rt.wav", "wav")
      else if (format == ircam)
         rtoutput("s1rt.sf", "ircam")
   }

   STEREO(outskip, inskip, dur, amp, pan=.3)
}

