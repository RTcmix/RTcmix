rtsetparams(44100, 1, 1024);
load("LOOP");

rtinput("../../../snd/input.wav")

LOOP(st=0, inskip=0, 20.0, amp=10, transp=1.0, loopstart=19999, looplen=10000);

