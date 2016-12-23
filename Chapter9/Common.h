#pragma once

#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <atlbase.h>
#include "helpers.h"
#include <MMSystem.h>
#include <uuids.h>
#include <Mferror.h>
#include <MFapi.h>
using namespace std;
using namespace ATL;

static void CheckHR(HRESULT hr);

#define BREAK_ON_FAIL(value)            if(FAILED(value)) break;
#define BREAK_ON_NULL(value, newHr)     if(value == NULL) { hr = newHr; break; }

#define THROW_ON_FAIL(value)     if(FAILED(value)) { throw; }
#define THROW_ON_NULL(value)     if(value == NULL) { throw; }
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }