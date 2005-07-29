/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
	See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
	the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// Write a basic .rtcmixrc file in the user's home directory.
// -JGG, 7/1/04; rev. for MIDI/audio devices, 7/29/05.

#include <Option.h>
#include <stdio.h>
#include "../src/control/midi/portmidi/pm_common/portmidi.h"


int chooseAudioDevices()
{
	return 0;
}


#if defined(MACOSX) || defined(ALSA)

void makeMIDIChoice(const PmDeviceInfo *info[], const int numDevices,
	const bool input)
{
	const char *direction = input ? "input" : "output";

	printf("\nHere are the names of MIDI %s devices in your system...\n\n"
	       "\tID\tName\n"
	       "----------------------------------------------------------\n",
	       direction);

	for (int id = 0; id < numDevices; id++) {
		if (info[id] != NULL) {
			if ((input && info[id]->input) || (!input && info[id]->output))
				printf("\t%d\t\"%s\"\n", id, info[id]->name);
		}
	}
	printf("\nEnter the ID number of one of the listed devices...\n");
	bool trying = true;
	while (trying) {
		int chosenID;
		if (scanf("%d", &chosenID) == 1) {
			if (chosenID >= 0 && chosenID < numDevices && info[chosenID] != NULL) {
				if (input && info[chosenID]->input) {
					Option::midiInDevice(info[chosenID]->name);
					trying = false;
				}
				else if (!input && info[chosenID]->output) {
					Option::midiOutDevice(info[chosenID]->name);
					trying = false;
				}
			}
			if (trying)
				printf("The number you typed was not one of the listed ID "
						 "numbers.  Try again...\n");
		}
		else {
			printf("You must enter a number.  Try again...\n");
			char str[1024];
			scanf("%1023s", str);		// swallow last input
		}
	}
}

int chooseMIDIDevices()
{
	int status = 0;
	Pm_Initialize();

	const int numdev = Pm_CountDevices();
	const PmDeviceInfo *info[numdev];

	bool hasinputdev = false;
	bool hasoutputdev = false;
	for (int id = 0; id < numdev; id++) {
		info[id] = Pm_GetDeviceInfo(id);
		if (info[id] != NULL) {
			if (info[id]->input)
				hasinputdev = true;
			else if (info[id]->output)
				hasoutputdev = true;
		}
	}

	if (hasinputdev)
		makeMIDIChoice(info, numdev, true);
	else
		printf("NOTE: Your system appears to have no MIDI input devices.\n");

#ifdef NOTYET // XXX we don't support MIDI output yet
	if (hasoutputdev)
		makeMIDIChoice(info, numdev, false);
	else
		printf("NOTE: Your system appears to have no MIDI output devices.\n");
#endif

	Pm_Terminate();
	return status;
}

#else // !defined(MACOSX) && !defined(ALSA)

int chooseMIDIDevices()
{
	printf("NOTE: MIDI not supported on your platform.\n");
	return 0;
}

#endif


int main()
{
	Option::init();

	if (chooseAudioDevices() != 0)
		return -1;
	if (chooseMIDIDevices() != 0)
		return -1;

	if (Option::writeConfigFile(Option::rcName()) != 0)
		return -1;
	printf("Configuration file \"%s\" successfully written.\n",
	                                                Option::rcName());
	return 0;
}


