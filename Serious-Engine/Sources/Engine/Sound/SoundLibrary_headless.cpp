#include <Engine/StdH.h>

#include <Engine/Sound/SoundLibrary.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/ListIterator.inl>
#include <Engine/Base/Memory.h>
#include <Engine/Base/Shell.h>
#include <Engine/Sound/SoundData.h>
#include <Engine/Sound/SoundDecoder.h>
#include <Engine/Sound/SoundListener.h>
#include <Engine/Sound/SoundObject.h>

CSoundLibrary *_pSound = NULL;

FLOAT snd_tmMixAhead = 0.2f;
FLOAT snd_fSoundVolume = 1.0f;
FLOAT snd_fMusicVolume = 1.0f;
FLOAT snd_fDelaySoundSpeed = 1E10f;
FLOAT snd_fDopplerSoundSpeed = 330.0f;
FLOAT snd_fEarsDistance = 0.2f;
FLOAT snd_fPanStrength = 0.1f;
FLOAT snd_fLRFilter = 3.0f;
FLOAT snd_fBFilter = 5.0f;
FLOAT snd_fUFilter = 1.0f;
FLOAT snd_fDFilter = 3.0f;
ENGINE_API INDEX snd_iFormat = CSoundLibrary::SF_NONE;
INDEX snd_bMono = FALSE;

CSoundLibrary::CSoundLibrary(void) {
  sl_csSound.cs_iIndex = 3000;
  sl_EsfFormat = SF_NONE;
  sl_pslMixerBuffer = NULL;
  sl_pubBuffersMemory = NULL;
  sl_pswDecodeBuffer = NULL;
  sl_sentSamples = 0;
  sl_lastPosition = 0;
  sl_slSamplePerLoop = 0;
  sl_slMixerBufferSize = 0;
  sl_slDecodeBufferSize = 0;
}

CSoundLibrary::~CSoundLibrary(void) {
  Clear();
  CSoundDecoder::EndPlugins();
}

void CSoundLibrary::Init(void) {
  CPrintF("Initializing sound (headless)...\n");
  _pShell->DeclareSymbol("persistent user FLOAT snd_fSoundVolume;", &snd_fSoundVolume);
  _pShell->DeclareSymbol("persistent user FLOAT snd_fMusicVolume;", &snd_fMusicVolume);
  _pShell->DeclareSymbol("persistent user INDEX snd_iFormat;", &snd_iFormat);
  _pShell->DeclareSymbol("user INDEX snd_bMono;", &snd_bMono);

  SetFormat(SF_NONE);
  CSoundDecoder::InitPlugins();
}

void CSoundLibrary::Clear(void) {
  FOREACHINLIST(CSoundData, sd_Node, sl_ClhAwareList, itCsdStop) {
    FOREACHINLIST(CSoundObject, so_Node, (itCsdStop->sd_ClhLinkList), itCsoStop) {
      itCsoStop->Stop();
    }
    itCsdStop->ClearBuffer();
  }
  ClearLibrary();
}

void CSoundLibrary::ClearLibrary(void) {
  if (sl_thTimerHandler.th_Node.IsLinked()) {
    _pTimer->RemHandler(&sl_thTimerHandler);
  }

  if (sl_pslMixerBuffer != NULL) {
    FreeMemory(sl_pslMixerBuffer);
    sl_pslMixerBuffer = NULL;
  }
  if (sl_pswDecodeBuffer != NULL) {
    FreeMemory(sl_pswDecodeBuffer);
    sl_pswDecodeBuffer = NULL;
  }
  if (sl_pubBuffersMemory != NULL) {
    FreeMemory(sl_pubBuffersMemory);
    sl_pubBuffersMemory = NULL;
  }

  sl_sentSamples = 0;
  sl_lastPosition = 0;
  sl_slSamplePerLoop = 0;
  sl_slMixerBufferSize = 0;
  sl_slDecodeBufferSize = 0;
  sl_EsfFormat = SF_NONE;
}

bool CSoundLibrary::SetFormat(CSoundLibrary::SoundFormat EsfNew) {
  CTSingleLock slHooks(&_pTimer->tm_csHooks, TRUE);
  CTSingleLock slSounds(&sl_csSound, TRUE);

  sl_EsfFormat = EsfNew;
  snd_iFormat = EsfNew;

  FOREACHINLIST(CSoundData, sd_Node, sl_ClhAwareList, itCsdStop) {
    itCsdStop->PausePlayingObjects();
  }

  if (sl_EsfFormat == SF_NONE) {
    ClearLibrary();
    return true;
  }

  ULONG samplesPerSec = getFramesPerSec();
  if (samplesPerSec == 0) {
    sl_EsfFormat = SF_NONE;
    return false;
  }

  sl_slSamplePerLoop = (SLONG)ceil(snd_tmMixAhead * (FLOAT)samplesPerSec);
  if (sl_slSamplePerLoop < 256) {
    sl_slSamplePerLoop = 256;
  }
  sl_slMixerBufferSize = sl_slSamplePerLoop * 2 * sizeof(SLONG);
  sl_slDecodeBufferSize = sl_slSamplePerLoop * 2 * sizeof(SWORD);

  if (sl_pslMixerBuffer != NULL) {
    FreeMemory(sl_pslMixerBuffer);
  }
  if (sl_pswDecodeBuffer != NULL) {
    FreeMemory(sl_pswDecodeBuffer);
  }
  sl_pslMixerBuffer = (SLONG *)AllocMemory(sl_slMixerBufferSize);
  sl_pswDecodeBuffer = (SWORD *)AllocMemory(sl_slDecodeBufferSize);
  memset(sl_pslMixerBuffer, 0, sl_slMixerBufferSize);
  memset(sl_pswDecodeBuffer, 0, sl_slDecodeBufferSize);

  return true;
}

void CSoundLibrary::UpdateSounds(void) {
  FOREACHINLIST(CSoundListener, sli_lnInActiveListeners, sl_lhActiveListeners, itsli) {
    itsli->sli_lnInActiveListeners.Remove();
  }
}

void CSoundLibrary::MixSounds(void) {}

void CSoundLibrary::Mute(void) {}

BOOL CSoundLibrary::SetEnvironment(INDEX iEnvNo, FLOAT fEnvSize) {
  (void)iEnvNo;
  (void)fEnvSize;
  return FALSE;
}

void CSoundTimerHandler::HandleTimer(void) {
  if (_pSound != NULL) {
    _pSound->MixSounds();
  }
}

void CSoundLibrary::AddSoundAware(CSoundData &CsdAdd) {
  sl_ClhAwareList.AddTail(CsdAdd.sd_Node);
}

void CSoundLibrary::RemoveSoundAware(CSoundData &CsdRemove) {
  CsdRemove.sd_Node.Remove();
}

void CSoundLibrary::Listen(CSoundListener &sl) {
  if (sl.sli_lnInActiveListeners.IsLinked()) {
    sl.sli_lnInActiveListeners.Remove();
  }
  sl_lhActiveListeners.AddTail(sl.sli_lnInActiveListeners);
}

ULONG CSoundLibrary::getFramesPerSec() {
  switch (sl_EsfFormat) {
    case CSoundLibrary::SF_11025_16:
      return 11025;
    case CSoundLibrary::SF_22050_16:
      return 22050;
    case CSoundLibrary::SF_44100_16:
      return 44100;
    case CSoundLibrary::SF_NONE:
      return 0;
    default:
      ASSERTALWAYS("Unknown Sound format");
      return 0;
  }
}
