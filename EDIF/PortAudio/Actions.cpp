
#include "Common.h"

// ID = 0
// ============================================================================
//
// ACTIONS
// 
// ============================================================================

// ID = 0
void Extension::InitialiseRecordingToMemory()
{
	ThreadSafe_Start();
	bool temp = RecordingAlreadyStarted;
	ThreadSafe_End();
	if (temp)
		Explode("Already started recording...");
	else
	{
		StructPassThru* Parameters = new StructPassThru;
		Parameters -> para_Extension = this;
		Parameters -> para_totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
		Parameters -> para_numSamples = (NUM_SECONDS * SAMPLE_RATE) * NUM_CHANNELS;
		Parameters -> para_numBytes = ((NUM_SECONDS * SAMPLE_RATE) * NUM_CHANNELS) * sizeof(SAMPLE);
		Report("Reporting just before recording thread start.");
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&Record, Parameters, NULL, NULL);
		Report("Reporting just after recording thread end.");
	}
}

// ID = 1
void Extension::TestReportAndExplode()
{
	Report("Test of action 1's Report() okay.");
	Explode("Test of action 1's Explode() okay.");
}
