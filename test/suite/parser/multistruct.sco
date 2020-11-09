struct HoldAFloat { float f };

struct HoldAFloat myHF;

struct HoldAList { list l };

struct HoldAList myHL;

struct HoldAMap { map m };

struct HoldAMap myHM;

struct Hold {
	struct HoldAFloat hf,
	struct HoldAList hl,
	struct HoldAMap hm
};

struct Hold myHold;

