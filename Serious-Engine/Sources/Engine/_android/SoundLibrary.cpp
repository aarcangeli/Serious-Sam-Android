/* Copyright (c) 2002-2012 Croteam Ltd. 
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include <Engine/StdH.h>

#include <Engine/Sound/SoundLibrary.h>
#include <Engine/Base/Translation.h>

#include <Engine/Base/Shell.h>
#include <Engine/Base/Memory.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/ListIterator.inl>
#include <Engine/Base/Console.h>
#include <Engine/Base/Console_internal.h>
#include <Engine/Base/Statistics_Internal.h>
#include <Engine/Base/IFeel.h>

#include <Engine/Sound/SoundProfile.h>
#include <Engine/Sound/SoundListener.h>
#include <Engine/Sound/SoundData.h>
#include <Engine/Sound/SoundObject.h>
#include <Engine/Sound/SoundDecoder.h>
#include <Engine/Network/Network.h>

#include <Engine/Templates/StaticArray.cpp>
#include <Engine/Templates/StaticStackArray.cpp>

template CStaticArray<CSoundListener>;

// pointer to global sound library object
CSoundLibrary *_pSound = NULL;


// console variables
extern FLOAT snd_tmMixAhead = 0.2f; // mix-ahead in seconds
extern FLOAT snd_fSoundVolume = 1.0f;   // master volume for sound playing [0..1]
extern FLOAT snd_fMusicVolume = 1.0f;   // master volume for music playing [0..1]
// NOTES: 
// - these 3d sound parameters have been set carefully, take extreme if changing !
// - ears distance of 20cm causes phase shift of up to 0.6ms which is very noticable
//   and is more than enough, too large values cause too much distorsions in other effects
// - pan strength needs not to be very strong, since lrfilter has panning-like influence also
// - if down filter is too large, it makes too much influence even on small elevation changes
//   and messes the situation completely
extern FLOAT snd_fDelaySoundSpeed = 1E10;   // sound speed used for delay [m/s]
extern FLOAT snd_fDopplerSoundSpeed = 330.0f; // sound speed used for doppler [m/s]
extern FLOAT snd_fEarsDistance = 0.2f;   // distance between listener's ears
extern FLOAT snd_fPanStrength = 0.1f;   // panning modifier (0=none, 1= full)
extern FLOAT snd_fLRFilter = 3.0f;   // filter for left-right
extern FLOAT snd_fBFilter = 5.0f;   // filter for back
extern FLOAT snd_fUFilter = 1.0f;   // filter for up
extern FLOAT snd_fDFilter = 3.0f;   // filter for down

ENGINE_API extern INDEX snd_iFormat = 3;
extern INDEX snd_bMono = FALSE;
static INDEX snd_iDevice = -1;
static INDEX snd_iInterface = 2;   // 0=WaveOut, 1=DirectSound, 2=EAX
static INDEX snd_iMaxOpenRetries = 3;
static INDEX snd_iMaxExtraChannels = 32;
static FLOAT snd_tmOpenFailDelay = 0.5f;
static FLOAT snd_fEAXPanning = 0.0f;

static FLOAT snd_fNormalizer = 0.9f;
static FLOAT _fLastNormalizeValue = 1;

extern HWND _hwndMain; // global handle for application window
static HWND _hwndCurrent = NULL;
static HINSTANCE _hInstDS = NULL;
static INDEX _iWriteOffset = 0;
static INDEX _iWriteOffset2 = 0;
static BOOL _bMuted = FALSE;
static INDEX _iLastEnvType = 1234;
static FLOAT _fLastEnvSize = 1234;
static FLOAT _fLastPanning = 1234;


// TEMP! - for writing mixer buffer to file
static FILE *_filMixerBuffer;
static BOOL _bOpened = FALSE;


#define WAVEOUTBLOCKSIZE 1024
#define MINPAN (1.0f)
#define MAXPAN (9.0f)



/**
 * ----------------------------
 *    Sound Library functions
 * ----------------------------
**/

/*
 *  Construct uninitialized sound library.
 */
CSoundLibrary::CSoundLibrary(void) {}


/*
 *  Destruct (and clean up).
 */
CSoundLibrary::~CSoundLibrary(void) {}



// post sound console variables' functions

static FLOAT _tmLastMixAhead = 1234;
static INDEX _iLastFormat = 1234;
static INDEX _iLastDevice = 1234;
static INDEX _iLastAPI = 1234;

static void SndPostFunc(void *pArgs) {}



/*
 *  some internal functions
 */


// DirectSound shutdown procedure
static void ShutDown_dsound(CSoundLibrary &sl) {}


/*
 *  Set wave format from library format
 */
static void SetWaveFormat(CSoundLibrary::SoundFormat EsfFormat, WAVEFORMATEX &wfeFormat) {}


/*
 *  Set library format from wave format
 */
static void SetLibraryFormat(CSoundLibrary &sl) {}


static BOOL DSFail(CSoundLibrary &sl, char *strError) {}

static void DSPlayBuffers(CSoundLibrary &sl) {}


// init and set DirectSound format (internal)

static BOOL StartUp_dsound(CSoundLibrary &sl, BOOL bReport = TRUE) {}


// set WaveOut format (internal)

static INDEX _ctChannelsOpened = 0;

static BOOL StartUp_waveout(CSoundLibrary &sl, BOOL bReport = TRUE) {}


/*
 *  set sound format
 */
static void
SetFormat_internal(CSoundLibrary &sl, CSoundLibrary::SoundFormat EsfNew, BOOL bReport) {}


/*
 *  Initialization
 */
void CSoundLibrary::Init(void) {}


/*
 *  Clear Sound Library
 */
void CSoundLibrary::Clear(void) {}

/* Clear Library WaveOut */
void CSoundLibrary::ClearLibrary(void) {}


// set listener enviroment properties (EAX)
BOOL CSoundLibrary::SetEnvironment(INDEX iEnvNo, FLOAT fEnvSize/*=0*/) {}


// mute all sounds (erase playing buffer(s) and supress mixer)
void CSoundLibrary::Mute(void) {}


/*
 * set sound format
 */
CSoundLibrary::SoundFormat
CSoundLibrary::SetFormat(CSoundLibrary::SoundFormat EsfNew, BOOL bReport/*=FALSE*/) {}


/* Update all 3d effects and copy internal data. */
void CSoundLibrary::UpdateSounds(void) {}


/*
 * This is called every TickQuantum seconds.
 */
void CSoundTimerHandler::HandleTimer(void) {}


/*
 *  MIXER helper functions
 */


// copying of mixer buffer to sound buffer(s)

static LPVOID _lpData, _lpData2;
static DWORD _dwSize, _dwSize2;

static void CopyMixerBuffer_dsound(CSoundLibrary &sl, SLONG slMixedSize) {}


static void CopyMixerBuffer_waveout(CSoundLibrary &sl) {}


// finds room in sound buffer to copy in next crop of samples
static SLONG PrepareSoundBuffer_dsound(CSoundLibrary &sl) {}


static SLONG PrepareSoundBuffer_waveout(CSoundLibrary &sl) {}


/* Update Mixer */
void CSoundLibrary::MixSounds(void) {}


//
//  Sound mode awareness functions
//

/*
 *  Add sound in sound aware list
 */
void CSoundLibrary::AddSoundAware(CSoundData &CsdAdd) {};

/*
 *  Remove a display mode aware object.
 */
void CSoundLibrary::RemoveSoundAware(CSoundData &CsdRemove) {};

// listen from this listener this frame
void CSoundLibrary::Listen(CSoundListener &sl) {}
