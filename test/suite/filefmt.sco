args = n_arg();

if (args < 2) {
	print("Test score needs at least CMIX args output_filename and output_fileformat");
	exit();
}

print_on(1);
set_option("full_duplex_off", "clobber_on", "audio_off");
rtsetparams(44100, 1, 2048);
rtinput("sinetone.wav");
rtoutput(s_arg(0), s_arg(1));
MIX(0,0,DUR(0),1,0);
