rtsetparams(44100, 2)
load("./libTONE.so")

rtinput("../../../snd/input.wav")

TONE(0, 0, 2, 1, 10000);

TONE(2, 0, 2, 1, 1000);

TONE(4, 0, 2, 1, 100);

