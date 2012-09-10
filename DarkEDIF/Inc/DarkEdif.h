#pragma once

// Stops Visual Studio complaining it cannot generate copy functions because of the 0-sized array
#pragma warning (disable:4200)

// Struct contains information about a/c/e
struct ACEInfo {
	short	FloatFlags,		// Float flags (if bit n is true, parameter n returns a float)
			ID;				// ID, non-EDIFified
	union {  
	short	Flags,			// Flags
			Returns;		// Expression only; see ExpParams namespace
	};
	short	NumOfParams,	// Number of parameters this A/C/E is called with (Parameters[n])
			Parameter [0];	// Parameter information (max sizeof(FloatFlags)*8 params)

	ACEInfo(short _ID, short _Flags, short _NumOfParams) :
			FloatFlags(0), ID(_ID),
			Flags(_Flags), NumOfParams(_NumOfParams)
	{
		// No code needed. Leave Parameters[n] construction to parent struct.
	}

	// Simple function to return the part of the struct that is expected by MMF2
	inline void * MMFPtr (void) { return &ID; }
};

bool CreateNewActionInfo();
bool CreateNewConditionInfo();
bool CreateNewExpressionInfo();

#if 0 // NOPROPS
void InitialisePropertiesFromJSON(mv *, EDITDATA *);

Prop * GetProperty(EDITDATA *, size_t);
char * GetPropertyChbx(EDITDATA *, size_t);
#endif // NOPROPS