// First run netaudio_receiver, then this score.  This is just a simple
// test, but you could do any sort of processing in place of the MIX call.

set_option("outdevice=net:localhost:9999")
rtsetparams(44100, 2, 1024)
rtinput("../../snd/nucular.wav")
MIX(0, 0, DUR(), 1, 0)

