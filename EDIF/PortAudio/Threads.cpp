#define NoExt // Makes sure references point to the right positions
#include "common.h"
// ============================================================================
//
// THREADS
// 
// ============================================================================
DWORD WINAPI Record(StructPassThru *pDataArray)
{
	//Open struct and set variables
    int totalFrames = pDataArray -> para_totalFrames;		// Total number of frames
    int numSamples = pDataArray -> para_numSamples;			// Number of samples
    int numBytes = pDataArray -> para_numBytes;				// Number of bytes
	Extension * Extension = pDataArray -> para_Extension;	// Get Extension
	delete pDataArray; //Scat! 
	
ThreadSafe_Start();
	uchar ThreadID = Extension->NewThreadID;
	Extension->NewThreadID++;
ThreadSafe_End();
	PaStreamParameters inputParameters, outputParameters;

	PaStream *stream;
    PaError err;
    SAMPLE *recordedSamples;
    int i;
    SAMPLE max, average, val;
    
    Report("patest_read_record.c");

    totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
    numSamples = totalFrames * NUM_CHANNELS;

    numBytes = numSamples * sizeof(SAMPLE);
    recordedSamples = (SAMPLE *) malloc( numBytes );
    if ( recordedSamples == NULL )
    {
        Explode( "Could not allocate record array.");
		return 0;
    }
	ZeroMemory(&recordedSamples, numBytes);
    //for ( i=0; i<numSamples; i++ ) recordedSamples[i] = 0;
    err = Pa_Initialize(); //Dodginess!
	if ( err != paNoError )
	{
		Explode( "Unknown error @ line 46ish");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
	}


    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice)
	{
		Explode( "Unknown error @ line 60ish");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
	}
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,                  /* &outputParameters, */
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */
    if ( err != paNoError )
	{
		Explode( "Unknown error @ line 86ish");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
	}
	Report("Ran Pa_OpenStream...");
    err = Pa_StartStream( stream );

    if ( err != paNoError )
	{
		Explode( "Unknown error @ line 100ish");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
	}

    Report("Now recording!");

    err = Pa_ReadStream( stream, recordedSamples, totalFrames );
    if ( err != paNoError )
	{
		Explode( "Unknown error @ line 135ish");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
	}

    err = Pa_CloseStream( stream );
    if ( err != paNoError )
	{
		Explode( "Unknown error @ line 128ish");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
	}

    /* Measure maximum peak amplitude. */
    max = 0;
    average = 0;
    for ( i=0; i<numSamples; i++ )
    {
        val = recordedSamples[i];
        if ( val < 0 ) val = -val; /* ABS */
        if ( val > max )
        {
            max = val;
        }
        average += val;
    }

    average = average / numSamples;

    Report("Sample max amplitude = (variable) max - line 174");
    Report("Sample average = (variable) average - line 175");
/*  Was as below. Better choose at compile time because this
    keeps generating compiler-warnings:
    if ( PA_SAMPLE_TYPE == paFloat32 )
    {
        printf("sample max amplitude = %f\n", max );
        printf("sample average = %f\n", average );
    }
    else
    {
        printf("sample max amplitude = %d\n", max );
        printf("sample average = %d\n", average );
    }
*/
    /* Write recorded data to a file. */
#if 0
    {
        FILE  *fid;
        fid = fopen("recorded.raw", "wb");
        if ( fid == NULL )
        {
            printf("Could not open file.");
        }
        else
        {
            fwrite( recordedSamples, NUM_CHANNELS * sizeof(SAMPLE), totalFrames, fid );
            fclose( fid );
            printf("Wrote data to 'recorded.raw'\n");
        }
    }
#endif
    /* Playback recorded data.  -------------------------------------------- */
    Report( "Bypassed writing to file: data still in memory");

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
		Explode("Error: No default output device.");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
    }
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    Report("Beginning playback.");
    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */
    if ( err != paNoError )
	{
		Explode( "Unknown error @ line 217ish");
		TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
		_stprintf_s(temp, 260, _T("Error number: %d"), err);
		Extension->Unreferenced_Report(temp, ThreadID);
		_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
		Extension->Unreferenced_Report(temp, ThreadID);
		Pa_Terminate();
		return 0;
		return 0;
	}

    if ( stream )
    {
        err = Pa_StartStream( stream );
        if ( err != paNoError )
		{
			Explode( "Unknown error @ line 233ish");
			TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
			_stprintf_s(temp, 260, _T("Error number: %d"), err);
			Extension->Unreferenced_Report(temp, ThreadID);
			_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
			Extension->Unreferenced_Report(temp, ThreadID);
			Pa_Terminate();
			return 0;
		}
        Report("Waiting for playback to finish.");

        err = Pa_WriteStream( stream, recordedSamples, totalFrames );
        if ( err != paNoError )
		{
			Explode( "Unknown error @ line 247ish");
			TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
			_stprintf_s(temp, 260, _T("Error number: %d"), err);
			Extension->Unreferenced_Report(temp, ThreadID);
			_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
			Extension->Unreferenced_Report(temp, ThreadID);
			Pa_Terminate();
			return 0;
		}

        err = Pa_CloseStream( stream );
        if ( err != paNoError )
		{
			Explode( "Unknown error @ line 260ish");
			TCHAR temp[260], temp2[260]; //Not sure if temp2 is needed here.
			_stprintf_s(temp, 260, _T("Error number: %d"), err);
			Extension->Unreferenced_Report(temp, ThreadID);
			_stprintf_s(temp2, 260, _T("Error number: %s"), Pa_GetErrorText( err ));
			Extension->Unreferenced_Report(temp, ThreadID);
			Pa_Terminate();
			return 0;
		}
        Report("Done playback.");
    }

    free( recordedSamples );
	Report("Successfully completed both playback and recording. Line 273.");
    Pa_Terminate();
    return 0;
}

#undef NoExt