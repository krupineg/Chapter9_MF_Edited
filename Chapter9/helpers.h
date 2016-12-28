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
#include <Propvarutil.h>
void DetectSubtype(GUID guid);
void DebugLongLong(std::wstring pref, LONGLONG anything);
void DebugInfo(std::wstring info);
IMFMediaType* GetMediaType(IMFStreamDescriptor * pStreamDescriptor);
GUID GetVideoSubtype(IMFMediaType * mediaType);
GUID GetMajorType(IMFMediaType * mediaType);
HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key);

HRESULT CopyVideoType(IMFMediaType * in_media_type, IMFMediaType * out_mf_media_type);

IMFTransform* FindEncoderTransform(GUID out_video_format);

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
