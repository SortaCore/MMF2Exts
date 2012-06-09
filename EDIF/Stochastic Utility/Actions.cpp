
#include "Common.h"

void Extension::SetRandomSeed(int Seed)
{
    srand(Seed);
}

void Extension::SetRandomSeedToTimer()
{
    srand(GetTickCount());
}





