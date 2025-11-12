
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <lo/lo.h>
#include <unistd.h>
#include <fstream>
#include <string>

void fail_error(const char *cmd)
{
	std::cout << "usage: " << cmd << " [-i <hostIP>] <scorefile>\nIf no hostIP, it will run locally.  If no scorefile is given, it will read from stdin." << std::endl;
	exit(1);
}

int main(int argc, char *argv[]){    
	const char *hostIP = NULL;
	const char *filename = NULL;
	std::ifstream ifs;            // keep it alive in this scope
	std::istream *input = &std::cin;

	if (argc == 1) {	// no args
	}
    else if (argc == 2) {	// filename
		const char *arg = argv[1];
		if (arg[0] != '-') {
        	filename = arg;
		}
		else fail_error(argv[0]);
	}
	else if (argc > 2) {	// -i <IP>
		const char *arg = argv[1];
		if (arg[0] == '-' && arg[1] == 'i') {
			hostIP = argv[2];
		}
		if (argc == 4) {
			filename = argv[3];
		}
	}
    else {
		fail_error(argv[0]);
    }
	if (filename != NULL) {
        ifs.open(filename);
        if (!ifs.is_open()) {
            fprintf(stderr, "Unable to open file '%s'\n", filename);
            return -1;
        }
		input = &ifs;
	}
    lo_address t = lo_address_new(hostIP, "7777");
	std::string content( (std::istreambuf_iterator<char>(*input) ),
                   (std::istreambuf_iterator<char>()    ) );

    std::cout << "sending '" << content << "'" << std::endl;

    if (lo_send(t, "/RTcmix/ScoreCommands", "s", content.c_str()) == -1) {
        fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
               lo_address_errstr(t));
        return -1;
    }
    return 0;
}



