// Bash on the scheduler a bit
rtsetparams(44100, 2);
bus_config("MIX", "in0-1", "out0-1");
rtinput("./input.wav");

float count;
float maxout;
float outskip;

maxout = 30;
count = 0;

while (count < 100)
{
	outskip = irand(0, maxout);
	MIX(outskip, 0, DUR(0), 0.1, 0, 1);
	count = count + 1;
}

