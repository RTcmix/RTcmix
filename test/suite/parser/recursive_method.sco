struct Inner {
	float _f,
	method float getInnerF() { return this._f; }
};

struct Inner createInner() { struct Inner i = { 11 }; return i; }

struct Outer {
	float _of,
	struct Inner _ii,
	method float setOuterF() {
		value = this._ii.getInnerF();
		this._of = value;
		return this._of;
	}
}

struct Outer oo;
oo._ii = createInner();

oo.setOuterF();


