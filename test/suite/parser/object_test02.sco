// Testing boring down through derived classes

struct Base {
	float _f,
	method float baseMethod() { ++this._f; return this._f; }
};

struct Derived structbase Base {
	float _g,
	method float derivedMethod() { return this.baseMethod() + this._g; }
};

struct DoubleDerived structbase Derived {
	method float doubleDerivedMethod() { return this.derivedMethod(); }
};

base = Base(10);

derived = Derived(10, 100);

dderived = DoubleDerived(10, 100);

printf("\nTesting Base\n");

base.baseMethod();

printf("\nTesting Derived\n");

derived.derivedMethod();

derived.baseMethod();

printf("\nTesting DoubleDerived\n");

dderived.doubleDerivedMethod();

dderived.derivedMethod();

dderived.baseMethod();
