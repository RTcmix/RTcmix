// Test of bus reset code
rtsetparams(44100, 1);
load("TRANS");
bus_config("TRANS", "aix0", "aox1");			// Mono pipe
bus_config("TRANS", "aix0", "aox2");			// Mono pipe
bus_config("TRANS", "aix0", "aox3");			// Mono pipe

