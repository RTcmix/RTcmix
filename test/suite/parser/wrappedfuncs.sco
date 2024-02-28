include wrappertest.sco

rtsetparams(44100, 2);

rtinput("../../../snd/loocher.aiff");

cmix.trans(0, 0, DUR(), 1, -0.07, -1.0);

