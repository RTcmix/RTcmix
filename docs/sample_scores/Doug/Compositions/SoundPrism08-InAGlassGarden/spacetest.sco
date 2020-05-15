include /Users/dscott/studies/glass_garden/audio.info
include /Users/dscott/studies/glass_garden/space.info

configureRoom(50, 65, 40, 5.0, 1.5, 5);

rtinput("/Users/dscott/studies/glass_garden/clap.wav");

setSpace(kForeground);

start = 0;

while (start < 2)
{
	roomPlaceRandom(start, 0, DUR(), 1);
	start += 0.5;
}

setSpace(kMidground);

while (start < 4)
{
	roomPlaceRandom(start, 0, DUR(), 1);
	start += 0.5;
}

setSpace(kBackground);

while (start < 6)
{
	roomPlaceRandom(start, 0, DUR(), 1);
	start += 0.5;
}

/*

setSpace(kForeground);

while (start < 8)
{
	roomPlaceAngle(start, 0, DUR(), 1, 45);
	start += 0.25;
}
while (start < 10)
{
	roomPlaceAngleRange(start, 0, DUR(), 1, 30, 60);
	start += 0.25;
}
*/
roomRun(32, 0.1);


