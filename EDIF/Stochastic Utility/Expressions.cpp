
#include "Common.h"

//Useful Functions
int Round(float Value)
{
    return (Value > 0) ? (int)floor(Value + 0.5f) : (int)ceil(Value - 0.5f);
}

//Float Expressions
float Extension::GenerateRandom(float Minimum, float Maximum)
{
	return ((Maximum-Minimum)*((float)rand()/RAND_MAX))+Minimum;
}
float Extension::Limit(float Value, float Minimum, float Maximum)
{
	if (Minimum < Maximum)
		return (Value < Minimum) ? (Minimum) : (Value > Maximum ? Maximum : Value);
		return (Value < Maximum) ? (Maximum) : (Value > Minimum ? Minimum : Value);
}


float Extension::Nearest(float Value, float Minimum, float Maximum)
{
	return ((Minimum > Value) ? (Minimum - Value) : (Value - Minimum)) > 
		((Maximum > Value) ? (Maximum - Value) : (Value - Maximum)) ?
		Maximum : Minimum;
}

float Extension::Normalise(float Value, float Minimum, float Maximum, int LimitRange)
{
	Value = (Value - Minimum) / (Maximum - Minimum);

	if (LimitRange != 0)
		return Limit(Value,0,1);
    return Value;
}

float Extension::ModifyRange(float Value, float Minimum, float Maximum, float NewMinimum, float NewMaximum, int LimitRange)
{
    Value = NewMinimum + (Value - Minimum) * (NewMaximum - NewMinimum) / (Maximum - Minimum);

	if(LimitRange != 0) return Limit(Value,NewMinimum,NewMaximum);
    return Value;
}

float Extension::Wave(int Waveform, float Value, float CycleStart, float CycleEnd, float Minimum, float Maximum)
{
    switch(Waveform)
    {
        case 0:
        {
            // Sine
			return ModifyRange(sin(ModifyRange(Value,CycleStart,CycleEnd,0,6.283185307179586476925286766559f,0)),-1,1,Minimum,Maximum,0);
        }

        case 1:
        {
            // Cosine
			return ModifyRange(cos(ModifyRange(Value,CycleStart,CycleEnd,0,6.283185307179586476925286766559f,0)),-1,1,Minimum,Maximum,0);
        }

		case 2:
		{
			// Saw
			return UberMod(ModifyRange(Value, CycleStart, CycleEnd, Minimum, Maximum, 0), Minimum, Maximum);
		}

		case 3:
		{
			// Inverted Saw
			return UberMod(ModifyRange(Value, CycleStart, CycleEnd, Maximum, Minimum, 0), Minimum, Maximum);
		}

		case 4:
		{
			// Triangle
			return Mirror(ModifyRange(Value, CycleStart, CycleStart+(CycleEnd-CycleStart)/2, Minimum, Maximum, 0), Minimum, Maximum);
		}

		case 5:
		{
			// Square
			if (UberMod(Value, CycleStart, CycleEnd) < CycleStart+(CycleEnd-CycleStart)/2) return Minimum; else return Maximum;
		}

		default:
        {            
            // Non-existing waveform
			return 0;
        }
    };
}



float Extension::EuclideanMod(float Dividend, float Divisor)
{
	return fmod((fmod(Dividend,Divisor)+Divisor),Divisor);
}
float Extension::UberMod(float Dividend, float Lower, float Upper)
{
	return ModifyRange(EuclideanMod(Normalise(Dividend,Lower,Upper,0),1),0,1,Lower,Upper,0);
}

float Extension::Interpolate(float Value, float From, float To, int LimitRange)
{
    Value = From+Value*(To-From);

	if(LimitRange != 0) return Limit(Value,From,To);
    return Value;
}
float Extension::Mirror(float Value, float From, float To)
{
	if (From < To) {
		return From+fabs(EuclideanMod(Value-To,(To-From)*2)-(To-From));
	} else {
		return To+fabs(EuclideanMod(Value-From,(From-To)*2)-(From-To));
	}
}

float Extension::ExpressionCompare(float First, float Second, int ComparisonType, float ReturnIfTrue, float ReturnIfFalse)
{
	if (Compare(First,Second,ComparisonType)) return ReturnIfTrue; else return ReturnIfFalse;
}

float Extension::Approach(float Value, float Amount, float Target)
{
	return (Value<Target) ? min(Value + Amount, Target) : max(Value - Amount, Target);
}

//Integer versions of the float expressions
int Extension::IntGenerateRandom(float Minimum, float Maximum)
{
	return Round(GenerateRandom(Minimum,Maximum));
}


int Extension::IntLimit(float Value, float Minimum, float Maximum)
{
	return Round(Limit(Value,Minimum,Maximum));
}
int Extension::IntNearest(float Value, float Minimum, float Maximum)
{
	return Round(Nearest(Value,Minimum,Maximum));
}
int Extension::IntNormalise(float Value, float Minimum, float Maximum, int LimitRange)
{
	return Round(Normalise(Value,Minimum,Maximum,LimitRange));
}

int Extension::IntModifyRange(float Value, float Minimum, float Maximum, float NewMinimum, float NewMaximum, int LimitRange)
{
	return Round(ModifyRange(Value,Minimum,Maximum,NewMinimum,NewMaximum,LimitRange));
}

int Extension::IntWave(int Waveform, float Value, float CycleStart, float CycleEnd, float Minimum, float Maximum)
{
	return Round(Wave(Waveform,Value,CycleStart,CycleEnd,Minimum,Maximum));
}
int Extension::IntEuclideanMod(float Dividend, float Divisor)
{
	return Round(EuclideanMod(Dividend,Divisor));
}
int Extension::IntUberMod(float Dividend, float Lower, float Upper)
{
	return Round(UberMod(Dividend,Lower,Upper));
}

int Extension::IntInterpolate(float Value, float From, float To, int LimitRange)
{
	return Round(Interpolate(Value,From,To,LimitRange));
}
int Extension::IntMirror(float Value, float From, float To)
{
	return Round(Mirror(Value,From,To));
}

int Extension::IntExpressionCompare(float First, float Second, int ComparisonType, float ReturnIfTrue, float ReturnIfFalse)
{
	return Round(ExpressionCompare(First, Second, ComparisonType, ReturnIfTrue, ReturnIfFalse));
}

int Extension::IntApproach(float Value, float Amount, float Target)
{
	return Round(Approach(Value, Amount, Target));
}

//String expressions
const char * Extension::Substr(const char * String, int Start, int Length)
{
	if(Start >= 0)
		String += Start;
	else
		String = (String + strlen(String)) + Start;

	if(Length >= 0)
	{
		char * Return = (char *) Runtime.Allocate(Length + 1);
		memcpy(Return, String, Length);
		Return[Length] = 0;

        return Return;
	}

	int RealLength = strlen(String);

	char * Return = (char *) Runtime.Allocate(RealLength + 1);
	
    memcpy(Return, String, RealLength);
	Return[RealLength + Length] = 0;

    return Return;
}

const char * Extension::StrExpressionCompare(float First, float Second, int ComparisonType, const char * ReturnIfTrue, const char * ReturnIfFalse)
{
	if (Compare(First,Second,ComparisonType)) return ReturnIfTrue; else return ReturnIfFalse;
}