struct StructWithStructMember {
	float 							_floatMember,
	struct StructWithStructMember	_structMember
};

struct StructWithStructMember s, s2;

// finally, make sure we can initialize and access variables

s2._floatMember = 777;

s._structMember = s2;

if (s._structMember._floatMember == 777) {
	printf("SUCCEEDED\n");
}
else {
	printf("FAILED TO READ CORRECT VALUE FROM IMBEDDED STRUCT MEMBER\n");
}
