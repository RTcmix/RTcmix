float srate;
float chans;
float bufsize;
float dur;
float opt1;
float opt2;
float opt3;
float args;

args = n_arg();

if (args < 4) {
	print("Test score needs at least CMIX args SR CHANS RTBUFSAMPS DUR");
	exit();
}
	
srate = i_arg(0);
chans = i_arg(1);
bufsize = i_arg(2);
dur = i_arg(3);
opt1 = s_arg(4);
opt2 = s_arg(5);
opt3 = s_arg(6);

print_on(1);

if (args > 6) {
	set_option("clobber_on", "full_duplex_on", opt1, opt2, opt3);
}
else {
	if (args > 5) {
		set_option("clobber_on", "full_duplex_on", opt1, opt2);
	}
	else {
		if (args > 4) {
			set_option("clobber_on", "full_duplex_on", opt1);
		}
		else {
			if (args == 4) {
				set_option("clobber_on", "full_duplex_on");
			}
		}
	}
}

rtsetparams(srate, chans, bufsize);

rtinput("AUDIO");

if (chans == 1) {
	bus_config("MIX", "in0", "out0");
	MIX(0,0,dur,1,0);
}
else {
	bus_config("MIX", "in0-1", "out0-1");
	MIX(0,0,dur,1,0, 1);
}
