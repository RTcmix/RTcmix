// sample.sco by Joel Matthys
// random seed, so it's different every time
srand()

// use float inlet to control parameters. the "1" indicates the first inlet. You can have
// up to 9
inlet_control = makeconnection("inlet",1,100)

total_length = 60

// singing bowl
bowlenv = maketable("line",1000,0,0,1,1,2,0)
pitches = {6.00, 6.03, 6.04, 6.07, 6.08, 6.11, 7.00, 7.03, 7.04, 7.07, 7.08, 7.11}
pitchlen = len(pitches)
for (start = 0; start < total_length; start += irand(5,15))
{
        thispitch = trand(pitchlen-1)
        MBANDEDWG(start, irand(8,12), irand(1500,2000)*bowlenv, cpspch(pitches[thispitch]), inlet_control/100, 1, 0.5, 3, 0.0, 1.0, 0.0, random())
        thispitch = trand(pitchlen-1)
        MBANDEDWG(start, irand(8,12), irand(1500,2000)*bowlenv, cpspch(pitches[thispitch]), inlet_control/100, 1, 0.5, 3, 0.0, 1.0, 0.0, random())

}

// fast notes
tempo = {0.33333, 0.25, 0.125, 0.16666, 0.083333}
which_tempo = 2;
for (start = 5; start < total_length; start += tempo[which_tempo])
{
        humanize_tempo = random() * 0.02;
        pan = irand(0.4,0.6)
        MMESH2D(start+humanize_tempo, 1, 8000*start/total_length, trand(2,7), trand(2,7), 0.8, 0.9, 1.0, irand(0.8,0.9), pan)
        if (random()<0.1)
                which_tempo = trand(0,5)
}

// big hits
for (start = 10; start < total_length; start += irand(2,5))
{
        pan = irand(0,0.6)
        MMESH2D(start, 5, 25000, trand(6,12), trand(6,12), 0.8, 0.9, 1, 1, pan)
	MAXBANG()
}
for (start = 10; start < total_length; start += irand(2,5))
{
        pan = irand(0.4,1)
        MMESH2D(start, 5, 25000, trand(6,12), trand(6,12), 0.8, 0.9, 1, 1, pan)
	MAXBANG()
}
