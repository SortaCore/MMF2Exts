
#include "Common.h"

bool Extension::Compare(float First, float Second, int ComparisonType)
{
    switch(ComparisonType)
    {
        case 0:
            return First == Second;

        case 1:
            return First != Second;

        case 2:
            return First > Second;

        case 3:
            return First < Second;

        case 4:
            return First >= Second;

        case 5:
            return First <= Second;

        default:
            return false;
    };
}
