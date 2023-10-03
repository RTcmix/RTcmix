
load("TRANS");	// This would not need to be here in the new world

// special private function to convert pan units from new to old
float _pan_convert(float newpan) { return (newpan+1) / 2; }

struct _cmix { 
	mfunction trans
	// etc., etc., for all EZ functions!
};

float cmix_trans(float outskip, float inskip, float dur, float gain, float interval, float pan)
{
    return TRANS(outskip, inskip, dur, gain, interval, _pan_convert(pan));
}

struct _cmix cmix = { cmix_trans };

