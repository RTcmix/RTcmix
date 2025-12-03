
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <lo/lo.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <cstring>

void fail_error(const char *cmd)
{
	printf("usage: %s [-v] [-c|-s] [-i <hostIP>] [-p <portID] [<scorefile>]\nIf no hostIP, it will run locally.\n"
		   "If no scorefile is given, it will read from stdin.\n", cmd);
	exit(1);
}

static enum RunMode { Score=1, Command=2 } mode = Score;
static bool verbose = false;

lo_message scan_double_command(std::string &line, char *outCmd);
lo_message scan_string_command(std::string &line, char *outCmd);

int main(int argc, char *argv[]){    
	const char *hostIP = NULL;
	const char *portID = "7777";
	const char *filename = NULL;

	std::ifstream ifs;            // keep it alive in this scope
	std::istream *input = &std::cin;

	int i = 1;
	while (i < argc) {
		char *a = argv[i];
		if (a[0] == '-' && a[1] != '\0' && a[1] != '-') {
			if (a[2] == '\0') {
				// exactly one short option: "-x"
				if (i + 1 < argc && argv[i + 1][0] != '-') {
					// treat next argument as value: "-x n"
//					std::printf("short option with value: name=\"%c\" value=\"%s\"\n",
//								a[1], argv[i + 1]);
					switch (a[1]) {
						case 'i':
							hostIP = argv[i + 1];
							break;
						case 'p':
							portID = argv[i + 1];
							break;
						default:
							break;
					}
					i += 2;    // consume option and its value
					continue;
				} else {
					// just "-x"
//					std::printf("short option: name=\"%c\"\n", a[1]);
					switch (a[1]) {
						case 'c':
							mode = Command;
							break;
						case 's':
							mode = Score;
							break;
						case 'v':
							verbose = true;
							break;
						case 'h':
							fail_error(argv[0]);
							break;	/*NOTREACHED*/
						default:
							fprintf(stderr, "Unrecognized option %s\n", a);
							fail_error(argv[0]);
							break;
					}
				}
			} else {
				// multiple short options stuck together: "-abc"
//				std::printf("short option group: \"%s\"\n", a + 1);
			}
		}
		else {
			// does not start with '-'
//			std::printf("plain argument: \"%s\"\n", a);
			filename = a;
		}
		i++;
	}
	if (verbose) {
		std::printf("filename=%s, hostIP=%s\n", filename, hostIP);
	}
	if (filename != NULL) {
        ifs.open(filename);
        if (!ifs.is_open()) {
            fprintf(stderr, "Unable to open file '%s'\n", filename);
            return -1;
        }
		input = &ifs;
	}
	else if (isatty(STDIN_FILENO)) {
		std::printf("Enter %s followed by <control>-D\n", mode == Command ? "any number of commands, one per line," : "score");
	}
    lo_address t = lo_address_new(hostIP, portID);
	std::string content( (std::istreambuf_iterator<char>(*input) ),
                   (std::istreambuf_iterator<char>()    ) );

	if (mode == Command) {
		std::istringstream in(content);
		std::string line;
		char cmd[1024];
		if (verbose) printf("Parsing command strings...\n");
		while (std::getline(in, line)) {
			// Handle meta commands (no parenthesis)
			strcpy(cmd, "/RTcmix/");
			if (line.compare("stop") == 0 || line.compare("quit") == 0) {
				strcat(cmd, line.c_str());
				if (verbose) printf("Sending meta command '%s'\n", cmd);
				if (lo_send(t, cmd, "s", NULL) == -1) {
					fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
							lo_address_errstr(t));
				}
				continue;
			}
			// Assuming everything else is an RTcmix command
			std::string::size_type lpar = line.find('(');
			std::string::size_type rpar = line.find(')');

			// Must have both parentheses, and they must be in the correct order.
			if (lpar == std::string::npos ||
				rpar == std::string::npos ||
				rpar < lpar)
			{
				fprintf(stderr,
						"OSC: Malformed command (missing or misplaced parentheses): '%s' - skipping\n",
						line.c_str());
				continue;
			}
			strcpy(cmd, "/RTcmix/");
			lo_message parsedMessage = scan_double_command(line, cmd);
			if (parsedMessage == NULL) {
				if (verbose) { printf("Couldn't parse as doubles - try strings\n"); }
				parsedMessage = scan_string_command(line, cmd);
			}
			if (parsedMessage == NULL) {
				std::fprintf(stderr, "OSC: Parse failure on line - skipping\n");
				continue;
			}
			if (verbose) printf("Sending command '%s'\n", cmd);
			// Send the message with all accumulated arguments
			if (lo_send_message(t, cmd, parsedMessage) == -1) {
				fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
					lo_address_errstr(t));
				return -1;
			}
			lo_message_free(parsedMessage);
		}
	}
	else {
	    std::printf("\nsending score: '\n%s\n'", content.c_str());
		if (lo_send(t, "/RTcmix/ScoreCommands", "s", content.c_str()) == -1) {
			fprintf(stderr, "OSC error %d: %s\n", lo_address_errno(t),
					lo_address_errstr(t));
			return -1;
		}
	}
    return 0;
}

lo_message scan_double_command(std::string &line, char *outCmd) {
	float args[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int matched = sscanf(line.c_str(), " %[^ (](%g ,%g ,%g ,%g ,%g ,%g ,%g ,%g)",
						 &outCmd[8], &args[0], &args[1], &args[2], &args[3], &args[4], &args[5], &args[6], &args[7]);
	if (matched < 1) {
		// nothing usable on this line; skip or handle as error
		std::fprintf(stderr, "OSC: Parse error on line: '%s' - skipping\n", line.c_str());
		return NULL;
	}
	else if (matched == 1) {
		if (verbose) { printf("Command has zero double argument matches - treat as failure\n"); }
		return NULL;		// command does not use double arguments
	}
	int argcount = matched - 1;
	if (verbose) { printf("Read command '%s' with %d double arguments\n", outCmd, argcount); }
	lo_message m = lo_message_new();
	if (!m) return NULL;  // handle error if you care
	for (int i = 0; i < argcount; ++i) {
		lo_message_add_double(m, args[i]);
	}
	return m;
}

lo_message scan_string_command(std::string &line, char *outCmd) {
	char args[8][1024] = {{0} };
	int matched = sscanf(line.c_str(),
						 " %[^ (](\"%[^\"]\" ,\"%[^\"]\" ,\"%[^\"]\" ,\"%[^\"]\" ,\"%[^\"]\" ,\"%[^\"]\" ,\"%[^\"]\" ,\"%[^\"]\")",
						 &outCmd[8], args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
	lo_message m = lo_message_new();
	if (matched < 1) {
		// nothing usable on this line; skip or handle as error
		std::fprintf(stderr, "OSC: Parse error on line: '%s' - skipping\n", line.c_str());
		return NULL;
	}
	else if (matched == 0) {
		if (verbose) { printf("Command has zero argument matches - treat as having none\n"); }
		return m;		// command does not use string arguments, but assume there are simply no args
	}
	int argcount = matched - 1;
	if (verbose) { printf("Read command '%s' with %d string arguments\n", outCmd, argcount); }
	if (!m) return NULL;  // handle error if you care
	for (int i = 0; i < argcount; ++i) {
		lo_message_add_string(m, args[i]);
	}
	return m;
}

