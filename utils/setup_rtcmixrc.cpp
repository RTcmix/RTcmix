/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
	See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
	the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Write a basic .rtcmixrc file in the user's home directory.	-JGG, 7/1/04 */

#include <Option.h>
#include <stdio.h>
#include <unistd.h>
#include "../src/control/midi/portmidi/pm_common/portmidi.h"

/* We make our own warn function so that we don't have to pull in more
	RTcmix code.  This must have the same signature as the real one in
	message.c.	It doesn't print WARNING ***, though.
*/
#include <stdarg.h>

#define BUFSIZE 1024

extern "C" {
void
warn(const char *inst_name, const char *format, ...)
{
	// ignore inst_name

	char		buf[BUFSIZE];
	va_list	args;

	va_start(args, format);
	vsnprintf(buf, BUFSIZE, format, args);
	va_end(args);

	fprintf(stderr, "\n%s\n\n", buf);
}
} // extern "C"


int chooseMIDIDevice()
{
	int status = 0;
#if defined(MACOSX) || defined(ALSA)
	Pm_Initialize();

	const int numdev = Pm_CountDevices();

	bool hasinputdev = false;
	for (int id = 0; id < numdev; id++) {
		const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
		if (info->input) {
			hasinputdev = true;
			break;
		}
	}
	if (hasinputdev) {
		printf("\nHere are the names of MIDI input devices in your system...\n\n"
				 "\tID\tName\n"
				 "----------------------------------------------------------\n");
		for (int id = 0; id < numdev; id++) {
			const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
			if (info->input)
				printf("\t%d\t\"%s\"\n", id, info->name);
		}
		printf("\nEnter the ID number of one of the listed devices...\n");
		bool trying = true;
		while (trying) {
			int chosenID;
			if (scanf("%d", &chosenID) == 1) {
				const PmDeviceInfo *info = Pm_GetDeviceInfo(chosenID);
				if (info != NULL && info->input) {
					Option::midiInDevice(info->name);
					trying = false;
				}
				else
					printf("The number you typed was not one of the listed ID "
					       "numbers.  Try again...\n");
			}
			else {
				trying = false;
			}
		}
	}
	else
		printf("NOTE: Your system appears to have no MIDI input devices.\n");

	Pm_Terminate();
#else // !defined(MACOSX) && !defined(ALSA)
	printf("NOTE: MIDI not supported on your platform.\n");
#endif
	return status;
}


int main()
{
	Option::init();

	if (chooseMIDIDevice() != 0)
		exit(1);

	if (Option::writeConfigFile(Option::rcName()) != 0)
		return -1;
	printf("Configuration file \"%s\" successfully written.\n",
	                                                Option::rcName());
	return 0;
}


