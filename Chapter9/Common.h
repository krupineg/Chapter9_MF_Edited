#pragma once

#define WIN32_LEAN_AND_MEAN
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#include <assert.h>
#include <atlbase.h>
#include "helpers.h"
#include <MMSystem.h>
#include <uuids.h>
#include <Mferror.h>
#include <MFapi.h>
#include <Vfw.h>
#include <hash_map>
#include <initguid.h>
#include <comdef.h>
DEFINE_GUID(CLSID_CMSH264RemuxMFT, 0x05a47ebb, 0x8bf0, 0x4cbf, 0xad, 0x2f, 0x3b, 0x71, 0xd7, 0x58, 0x66, 0xf5);
DEFINE_GUID(MF_MT_BITCOUNT, 0xc496f370, 0x2f8b, 0x4f51, 0xae, 0x46, 0x9c, 0xfc, 0x1b, 0xc8, 0x2a, 0x47);
using namespace std;
using namespace ATL;

const GUID DEFAULT_AUDIO = MFAudioFormat_MP3;
const GUID DEFAULT_VIDEO = MFVideoFormat_H264;

#define BREAK_ON_FAIL(value)            if(FAILED(value)) break;
#define BREAK_ON_NULL(value, newHr)     if(value == NULL) { hr = newHr; break; }

#define DEBUG_ON_FAIL(value)     if(FAILED(value)) {  _com_error err(value); LPCTSTR errMsg = err.ErrorMessage(); DebugInfo(L"\n================\nERROR:\n"); DebugInfo(errMsg); DebugInfo(L"\n================\n");}
#define THROW_ON_FAIL(value)     if(FAILED(value)) { DEBUG_ON_FAIL(value); throw; }
#define THROW_ON_NULL(value)     if(value == NULL) { throw; }
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }