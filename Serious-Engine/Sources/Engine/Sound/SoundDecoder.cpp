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

#include "StdH.h"

#include <Engine/Sound/SoundDecoder.h>
#include <Engine/Base/Stream.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/FileName.h>
#include <Engine/Base/Unzip.h>
#include <Engine/Base/Translation.h>
#include <Engine/Math/Functions.h>
#include <dlfcn.h>
#include <config.h>

// generic function called if a dll function is not found
static void FailFunction_t(const char *strName) {
  ThrowF_t(TRANS("Function %s not found."), strName);
}


// ------------------------------------ AMP11
#include <amp11lib/amp11lib.h>

class CDecodeData_MPEG {
public:
  ALhandle mpeg_hMainFile; // mainfile handle if using subfile
  ALhandle mpeg_hFile;     // file handle
  ALhandle mpeg_hDecoder;  // the decoder handle
  FLOAT mpeg_fSecondsLen;  // length of sound in seconds
  WAVEFORMATEX mpeg_wfeFormat; // format of sound
};


// ------------------------------------ Ogg Vorbis
#include <vorbis/vorbisfile.h>  // we define needed stuff ourselves, and ignore the rest

// vorbis vars
extern BOOL _bOVEnabled = FALSE;
static HINSTANCE _hOV = NULL;

class CDecodeData_OGG {
public:
  FILE *ogg_fFile;      // the stdio file that ogg is in
  SLONG ogg_slOffset;   // offset where the ogg starts in the file (!=0 for oggs in zip)
  SLONG ogg_slSize;     // size of ogg in the file (!=filesize for oggs in zip)
  OggVorbis_File *ogg_vfVorbisFile;  // the decoder file
  WAVEFORMATEX ogg_wfeFormat; // format of sound
};

// define vorbis function pointers
#ifdef STATIC_LINKING
#define DLLFUNCTION(dll, output, name, inputs, params, required) \
  auto p##name = &name;
#else
#define DLLFUNCTION(dll, output, name, inputs, params, required) \
  output (__cdecl *p##name) inputs = NULL;
#endif
#include "ov_functions.h"
#undef DLLFUNCTION

static void OV_SetFunctionPointers_t(void) {
#ifndef STATIC_LINKING
  const char *strName;
  // get vo function pointers
  #define DLLFUNCTION(dll, output, name, inputs, params, required) \
    strName = #name ;  \
    p##name = (output (__cdecl *) inputs) dlsym( _hOV, strName); \
    if(p##name == NULL) FailFunction_t(strName);
  #include "ov_functions.h"
  #undef DLLFUNCTION
#endif
}
static void OV_ClearFunctionPointers(void) {
#ifndef STATIC_LINKING
  // clear vo function pointers
  #define DLLFUNCTION(dll, output, name, inputs, params, required) p##name = NULL;
  #include "ov_functions.h"
  #undef DLLFUNCTION
#endif
}

// ogg file reading callbacks
//

static size_t ogg_read_func  (void *ptr, size_t size, size_t nmemb, void *datasource)
{
  CDecodeData_OGG *pogg = (CDecodeData_OGG *)datasource;
  // calculate how much can be read at most
  SLONG slToRead = size*nmemb;
  SLONG slCurrentPos = ftell(pogg->ogg_fFile)-pogg->ogg_slOffset;
  SLONG slSizeLeft = ClampDn(pogg->ogg_slSize-slCurrentPos, 0L);
  slToRead = ClampUp(slToRead, slSizeLeft);

  // rounded down to the block size
  slToRead/=size;
  slToRead*=size;
  // if there is nothing to read
  if (slToRead<=0) {
    return 0;
  }
  return fread(ptr, size, slToRead/size, pogg->ogg_fFile);
}

static int ogg_seek_func  (void *datasource, ogg_int64_t offset, int whence)
{
  return -1;
/*  !!!! seeking is evil with vorbisfile 1.0RC2
  CDecodeData_OGG *pogg = (CDecodeData_OGG *)datasource;
  SLONG slCurrentPos = ftell(pogg->ogg_fFile)-pogg->ogg_slOffset;
  if (whence==SEEK_CUR) {
    return fseek(pogg->ogg_fFile, offset, SEEK_CUR);
  } else if (whence==SEEK_END) {
    return fseek(pogg->ogg_fFile, pogg->ogg_slOffset+pogg->ogg_slSize-offset, SEEK_SET);
  } else {
    ASSERT(whence==SEEK_SET);
    return fseek(pogg->ogg_fFile, pogg->ogg_slOffset+offset, SEEK_SET);
  }
*/
}

static int ogg_close_func (void *datasource)
{
  return 0;
/* !!!! closing is evil with vorbisfile 1.0RC2
  CDecodeData_OGG *pogg = (CDecodeData_OGG *)datasource;
  fclose(pogg->ogg_fFile);
  */
}
static long ogg_tell_func  (void *datasource)
{
  return -1;
/*  !!!! seeking is evil with vorbisfile 1.0RC2
  CDecodeData_OGG *pogg = (CDecodeData_OGG *)datasource;
  ftell(pogg->ogg_fFile)-pogg->ogg_slOffset;
  */
}

static ov_callbacks ovcCallbacks = {
  ogg_read_func,
  ogg_seek_func,
  ogg_close_func,
  ogg_tell_func,
};


// initialize/end the decoding support engine(s)
void CSoundDecoder::InitPlugins(void)
{
  try {
    // load vorbis
#ifndef STATIC_LINKING
    if (_hOV==NULL) {
#ifndef NDEBUG
  #define VORBISLIB "libvorbis.so"
#else
  #define VORBISLIB "libvorbis.so"
#endif
      _hOV = dlopen(VORBISLIB, RTLD_NOW);
    }
    if( _hOV == NULL) {
      ThrowF_t(TRANS("Cannot load libvorbis.so."));
    }
    // prepare function pointers
    OV_SetFunctionPointers_t();
#endif

    // if all successful, enable mpx playing
    _bOVEnabled = TRUE;
    CPrintF(TRANS("  libvorbis.so loaded, ogg playing enabled\n"));

  } catch ( const char *strError) {
    CPrintF(TRANS("OGG playing disabled: %s\n"), strError);
  }

  // initialize amp11lib before calling any of its functions
  alInitLibrary();

  // if all successful, enable mpx playing
  CPrintF(TRANS("  alInitLibrary initialized, mpx playing enabled\n"));
}

void CSoundDecoder::EndPlugins(void)
{
  // cleanup vorbis when not needed anymore
  if (_bOVEnabled) {
    OV_ClearFunctionPointers();
    FreeLibrary(_hOV);
    _hOV = NULL;
    _bOVEnabled = FALSE;
  }
}

// decoder that streams from file
CSoundDecoder::CSoundDecoder(const CTFileName &fnm)
{
  sdc_pogg = NULL;
  sdc_pmpeg = NULL;

  CTFileName fnmExpanded;
  INDEX iFileType = ExpandFilePath(EFP_READ, fnm, fnmExpanded);

  // if ogg
  if (fnmExpanded.FileExt()==".ogg") {
    if (!_bOVEnabled) {
      return;
    }

    sdc_pogg = new CDecodeData_OGG;
    sdc_pogg->ogg_fFile = NULL;
    sdc_pogg->ogg_vfVorbisFile = NULL;
    sdc_pogg->ogg_slOffset = 0;
    sdc_pogg->ogg_slSize = 0;
    INDEX iZipHandle = 0;

    try {
      // if in zip
      if (iFileType==EFP_BASEZIP || iFileType==EFP_MODZIP) {
        // open it
        iZipHandle = UNZIPOpen_t(fnmExpanded);

        CTFileName fnmZip;
        SLONG slOffset;
        SLONG slSizeCompressed;
        SLONG slSizeUncompressed;
        BOOL bCompressed;
        UNZIPGetFileInfo(iZipHandle, fnmZip, slOffset, slSizeCompressed, slSizeUncompressed, bCompressed);

        // if compressed
        if (bCompressed) {
          ThrowF_t(TRANS("encoded audio in archives must not be compressed!\n"));
        }
        // open ogg file
        sdc_pogg->ogg_fFile = fopen(fnmZip, "rb");
        // if error
        if (sdc_pogg->ogg_fFile==0) {
          ThrowF_t(TRANS("cannot open archive '%s'"), (const char*)fnmZip);
        }
        // remember offset and size
        sdc_pogg->ogg_slOffset = slOffset;
        sdc_pogg->ogg_slSize = slSizeUncompressed;
        fseek(sdc_pogg->ogg_fFile, slOffset, SEEK_SET);

      // if not in zip
      } else if (iFileType==EFP_FILE) {
        // open ogg file
        sdc_pogg->ogg_fFile = fopen(fnmExpanded, "rb");
        // if error
        if (sdc_pogg->ogg_fFile==0) {
          ThrowF_t(TRANS("cannot open encoded audio file"));
        }
        // remember offset and size
        sdc_pogg->ogg_slOffset = 0;

        fseek(sdc_pogg->ogg_fFile, 0, SEEK_END);
        sdc_pogg->ogg_slSize = ftell(sdc_pogg->ogg_fFile);
        fseek(sdc_pogg->ogg_fFile, 0, SEEK_SET);
      // if not found
      } else {
        ThrowF_t(TRANS("file not found"));
      }

      // initialize decoder
      sdc_pogg->ogg_vfVorbisFile = new OggVorbis_File;
      int iRes = pov_open_callbacks(sdc_pogg, sdc_pogg->ogg_vfVorbisFile, NULL, 0, ovcCallbacks);

      // if error
      if (iRes!=0) {
        ThrowF_t(TRANS("cannot open ogg decoder"));
      }

      // get info on the file
      vorbis_info *pvi = pov_info(sdc_pogg->ogg_vfVorbisFile, -1);

      // remember it's format
      WAVEFORMATEX form;
      form.wFormatTag=WAVE_FORMAT_PCM;
      form.nChannels=pvi->channels;
      form.nSamplesPerSec=pvi->rate;
      form.wBitsPerSample=16;
      form.nBlockAlign=form.nChannels*form.wBitsPerSample/8;
      form.nAvgBytesPerSec=form.nSamplesPerSec*form.nBlockAlign;
      form.cbSize=0;

      // check for stereo
      if (pvi->channels!=2) {
        ThrowF_t(TRANS("not stereo"));
      }
    
      sdc_pogg->ogg_wfeFormat = form;

    } catch ( const char *strError) {
      CPrintF(TRANS("Cannot open encoded audio '%s' for streaming: %s\n"), (const char*)fnm, (const char*)strError);
      if (sdc_pogg->ogg_vfVorbisFile!=NULL) {
        delete sdc_pogg->ogg_vfVorbisFile;
        sdc_pogg->ogg_vfVorbisFile = NULL;
      }
      if (sdc_pogg->ogg_fFile!=NULL) {
        fclose(sdc_pogg->ogg_fFile);
        sdc_pogg->ogg_fFile = NULL;
      }
      if (iZipHandle!=0) {
        UNZIPClose(iZipHandle);
      }
      Clear();
      return;
    }
    if (iZipHandle!=0) {
      UNZIPClose(iZipHandle);
    }

  // if mp3
  } else if (fnmExpanded.FileExt()==".mp3") {

    sdc_pmpeg = new CDecodeData_MPEG;
    sdc_pmpeg->mpeg_hMainFile = 0;
    sdc_pmpeg->mpeg_hFile = 0;
    sdc_pmpeg->mpeg_hDecoder = 0;
    INDEX iZipHandle = 0;

    try {
      // if in zip
      if (iFileType==EFP_BASEZIP || iFileType==EFP_MODZIP) {
        // open it
        iZipHandle = UNZIPOpen_t(fnmExpanded);

        CTFileName fnmZip;
        SLONG slOffset;
        SLONG slSizeCompressed;
        SLONG slSizeUncompressed;
        BOOL bCompressed;
        UNZIPGetFileInfo(iZipHandle, fnmZip, slOffset, slSizeCompressed, slSizeUncompressed, bCompressed);

        // if compressed
        if (bCompressed) {
          ThrowF_t(TRANS("encoded audio in archives must not be compressed!\n"));
        }
        // open the zip file
        sdc_pmpeg->mpeg_hMainFile = alOpenInputFile(fnmZip);
        // if error
        if (sdc_pmpeg->mpeg_hMainFile==0) {
          ThrowF_t(TRANS("cannot open archive '%s'"), (const char*)fnmZip);
        }
        // open the subfile
        sdc_pmpeg->mpeg_hFile = alOpenSubFile(sdc_pmpeg->mpeg_hMainFile, slOffset, slSizeUncompressed);
        // if error
        if (sdc_pmpeg->mpeg_hFile==0) {
          ThrowF_t(TRANS("cannot open encoded audio file"));
        }

      // if not in zip
      } else if (iFileType==EFP_FILE) {
        // open mpx file
        sdc_pmpeg->mpeg_hFile = alOpenInputFile(fnmExpanded);
        // if error
        if (sdc_pmpeg->mpeg_hFile==0) {
          ThrowF_t(TRANS("cannot open mpx file"));
        }
      // if not found
      } else {
        ThrowF_t(TRANS("file not found"));
      }

      // get info on the file
      int layer, ver, freq, stereo, rate;
      if (!alGetMPXHeader(sdc_pmpeg->mpeg_hFile, &layer, &ver, &freq, &stereo, &rate)) {
        ThrowF_t(TRANS("not a valid mpeg audio file."));
      }

      // remember it's format
      WAVEFORMATEX form;
      form.wFormatTag=WAVE_FORMAT_PCM;
      form.nChannels=stereo?2:1;
      form.nSamplesPerSec=freq;
      form.wBitsPerSample=16;
      form.nBlockAlign=form.nChannels*form.wBitsPerSample/8;
      form.nAvgBytesPerSec=form.nSamplesPerSec*form.nBlockAlign;
      form.cbSize=0;

      // check for stereo
      if (!stereo) {
        ThrowF_t(TRANS("not stereo"));
      }
    
      sdc_pmpeg->mpeg_wfeFormat = form;

      // initialize decoder
      sdc_pmpeg->mpeg_hDecoder = alOpenDecoder(sdc_pmpeg->mpeg_hFile);

      // if error
      if (sdc_pmpeg->mpeg_hDecoder==0) {
        ThrowF_t(TRANS("cannot open mpx decoder"));
      }
    } catch ( const char *strError) {
      CPrintF(TRANS("Cannot open mpx '%s' for streaming: %s\n"), (const char*)fnm, (const char*)strError);
      if (iZipHandle!=0) {
        UNZIPClose(iZipHandle);
      }
      Clear();
      return;
    }

    if (iZipHandle!=0) {
      UNZIPClose(iZipHandle);
    }
    sdc_pmpeg->mpeg_fSecondsLen = alDecGetLen(sdc_pmpeg->mpeg_hDecoder);
  }
}

CSoundDecoder::~CSoundDecoder(void)
{
  Clear();
}

void CSoundDecoder::Clear(void)
{
  if (sdc_pmpeg!=NULL) {
    if (sdc_pmpeg->mpeg_hDecoder!=0)  alClose(sdc_pmpeg->mpeg_hDecoder);
    if (sdc_pmpeg->mpeg_hFile!=0)     alClose(sdc_pmpeg->mpeg_hFile);
    if (sdc_pmpeg->mpeg_hMainFile!=0) alClose(sdc_pmpeg->mpeg_hMainFile);

    sdc_pmpeg->mpeg_hMainFile = 0;
    sdc_pmpeg->mpeg_hFile = 0;
    sdc_pmpeg->mpeg_hDecoder = 0;
    delete sdc_pmpeg;
    sdc_pmpeg = NULL;

  } else if (sdc_pogg!=NULL) {

    if (sdc_pogg->ogg_vfVorbisFile!=NULL) {
      pov_clear(sdc_pogg->ogg_vfVorbisFile);
      delete sdc_pogg->ogg_vfVorbisFile;
      sdc_pogg->ogg_vfVorbisFile = NULL;
    }
    if (sdc_pogg->ogg_fFile!=NULL) {
      fclose(sdc_pogg->ogg_fFile);
      sdc_pogg->ogg_fFile = NULL;
    }
    delete sdc_pogg;
    sdc_pogg = NULL;
  }
}

// reset decoder to start of sample
void CSoundDecoder::Reset(void)
{
  if (sdc_pmpeg!=NULL) {
    alDecSeekAbs(sdc_pmpeg->mpeg_hDecoder, 0.0f);
  } else if (sdc_pogg!=NULL) {
    // so instead, we reinit
    pov_clear(sdc_pogg->ogg_vfVorbisFile);
    fseek(sdc_pogg->ogg_fFile, sdc_pogg->ogg_slOffset, SEEK_SET);
    pov_open_callbacks(sdc_pogg, sdc_pogg->ogg_vfVorbisFile, NULL, 0, ovcCallbacks);
  }
}

BOOL CSoundDecoder::IsOpen(void) 
{
  if (sdc_pmpeg!=NULL && sdc_pmpeg->mpeg_hDecoder!=0) {
    return TRUE;
  } else if (sdc_pogg!=NULL && sdc_pogg->ogg_vfVorbisFile!=0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

void CSoundDecoder::GetFormat(WAVEFORMATEX &wfe)
{
  if (sdc_pmpeg!=NULL) {
    wfe = sdc_pmpeg->mpeg_wfeFormat;

  } else if (sdc_pogg!=NULL) {
    wfe = sdc_pogg->ogg_wfeFormat;

  } else {
    NOTHING;
  }
}

// decode a block of bytes
INDEX CSoundDecoder::Decode(void *pvDestBuffer, INDEX ctBytesToDecode)
{
  // if ogg
  if (sdc_pogg!=NULL && sdc_pogg->ogg_vfVorbisFile!=0) {
    // decode ogg
    static int iCurrrentSection = -1; // we don't care about this
    char *pch = (char *)pvDestBuffer;
    INDEX ctDecoded = 0;
    while (ctDecoded<ctBytesToDecode) {
      long iRes = pov_read(sdc_pogg->ogg_vfVorbisFile, pch, ctBytesToDecode-ctDecoded, 
        0, 2, 1, &iCurrrentSection);
      if (iRes<=0) {
        return ctDecoded;
      }
      ctDecoded+=iRes;
      pch+=iRes;
    }
    return ctDecoded;

  // if mpeg
  } else if (sdc_pmpeg!=NULL && sdc_pmpeg->mpeg_hDecoder!=0) {
    // decode mpeg
    return alRead(sdc_pmpeg->mpeg_hDecoder, pvDestBuffer, ctBytesToDecode);

  // if no decoder
  } else {
    // play all zeroes
    memset(pvDestBuffer, 0, ctBytesToDecode);
    return ctBytesToDecode;
  }
}
