// this is for non-rt instruments to link to in order to resolve
// the rtprofile symbol.  rt instruments use this to make a symbol
// entry for Minc

extern "C" void rtprofile() {};
