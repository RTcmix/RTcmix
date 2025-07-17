
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <lo/lo.h>
#include <unistd.h>
#include <fstream>
#include <string>

int main(int argc, char *argv[]){    

    lo_address t = lo_address_new(NULL, "7777");
	std::istream *input;
    char *stuff = NULL;
	if (argc == 1) {
		input = &std::cin;
	}
    else if (argc == 2) {
        char *filename = argv[1];
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            fprintf(stderr, "Unable to open file '%s'\n", filename);
            return -1;
        }
		input = &ifs;
	}
    else {
        std::cout << "usage: " << argv[0] << " <scorefile>\nIf no scorefile is given, it will read from stdin." << std::endl;
        return -1;
    }
	std::string content( (std::istreambuf_iterator<char>(*input) ),
                   (std::istreambuf_iterator<char>()    ) );
	stuff = new char[content.length()+1];
	std::strcpy(stuff, content.c_str());

    std::cout << "sending '" << stuff << "'" << std::endl;

    if (lo_send(t, "/RTcmix/ScoreCommands", "s", stuff) == -1) {
        fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
               lo_address_errstr(t));
        return -1;
    }
    return 0;
}



