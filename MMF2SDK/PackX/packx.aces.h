#Start Actions
SetKey;	"Set Key";	"Set key to %0";	NoFlag;	String;	"Set key to (eg '[A-Z]')"
#End Actions

#Start Conditions
IsInRange;	"Is String small enough to pack using key?";	"If %0 fits in key %1";	AlwaysNegFlag;	String;	"String to pack"
#End Conditions

#Start Expressions
PackX;	"Pack String into Number";	"PackX(";	RetInt;	String
UnPackX;	"Unpack a number back into a string";	"UnPackX$(";	RetString;	Int
StringMaxLength;	"Find max length for key";	"MaxLen(";	RetInt
KeyCharCount;	"Count the amount of chars in a key";	"KeyCharCount(";	RetInt
#End Expressions

