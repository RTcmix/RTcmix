// Record from network, play via ALSA
//
// NOTE NOTE NOTE: Make sure you have configured RTcmix using --with-netplay
// before compilation!
//
// (Device descriptor needs "net:" to distinguish it from HW devices which
//     use name:number as their device -- like ALSA does here).
// For recording from socket, we don't use the "net:localhost", but it
// makes it easy to tell the system to use network audio.

// Run this score first.  It will wait on clients to connect.  You then
// run the netaudio_sender.sco any number of times.  The server will exit
// when <dur> seconds have elapsed.  You can kill it earlier with cntl-C.

set_option("full_duplex_on", "indevice=net:localhost:9999", "outdevice=plughw")
rtsetparams(44100, 2, 1024)
rtinput("AUDIO")
dur = 100   // listen for 100 seconds
MIX(0, 0, dur, 1, 0)

