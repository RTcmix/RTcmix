// Definition itself will not fail unless it is called

float funcTakingUnknownStruct(struct CurrentlyUnknown s) {
	s._unknownMember = 13;
	return 0;
}

funcTakingUnknownStruct();

