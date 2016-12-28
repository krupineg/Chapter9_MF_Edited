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
DEFINE_GUID(MF_MT_BITCOUNT, 0xc496f370, 0x2f8b, 0x4f51, 0xae, 0x46, 0x9c, 0xfc, 0x1b, 0xc8, 0x2a, 0x47);
using namespace std;
using namespace ATL;

#define BREAK_ON_FAIL(value)            if(FAILED(value)) break;
#define BREAK_ON_NULL(value, newHr)     if(value == NULL) { hr = newHr; break; }

#define THROW_ON_FAIL(value)     if(FAILED(value)) { throw; }
#define THROW_ON_NULL(value)     if(value == NULL) { throw; }
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }