#pragma once

// Stops Visual Studio complaining it cannot generate copy functions because of the 0-sized array
#pragma warning (disable:4200)

// Struct contains information about a/c/e
struct ACEInfo {
	short	FloatFlags,		// Float flags (if bit n is true, parameter n returns a float)
			ID;				// ID, non-EDIFified; used to be called 'code'
	short	Flags;			// For conds: EVFLAGS_ALWAYS, etc; whether condition is negatable
							// For exps: EXPFLAG_DOUBLE, etc; return type
							// For acts: unused, always 0
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

#ifndef NOPROPS
void InitialisePropertiesFromJSON(mv *, EDITDATA *);

Prop * GetProperty(EDITDATA *, size_t);

void PropChangeChkbox(EDITDATA * edPtr, unsigned int PropID, bool newValue);
void PropChange(mv * mV, EDITDATA * &edPtr, unsigned int PropID, void * newData, size_t newSize);
char * PropIndex(EDITDATA * edPtr, unsigned int ID, unsigned int * size);

#endif // NOPROPS