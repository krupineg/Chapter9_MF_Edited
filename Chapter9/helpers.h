#pragma once
#include "Common.h"
#include <map>
#include <atlbase.h>
#include <sstream>
#include "mftransform.h"
#include <MMSystem.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <string>
void DetectSubtype(GUID guid);
void DebugLongLong(std::wstring pref, LONGLONG anything);

IMFMediaType* GetMediaType(IMFStreamDescriptor * pStreamDescriptor);
GUID GetVideoSubtype(IMFMediaType * mediaType);

HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key);

HRESULT copyTypeParameters(IMFMediaType * in_media_type, IMFMediaType * out_mf_media_type);

IMFTransform* FindEncoderTransform(GUID out_video_format);