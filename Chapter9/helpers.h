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
std::wstring DetectSubtype(GUID guid);
std::wstring DetectMajorType(GUID guid);
void DebugLongLong(std::wstring pref, LONGLONG anything);
void DebugInfo(std::wstring info);
void DebugLog(std::wstring info);
IMFMediaType* GetMediaType(IMFStreamDescriptor * pStreamDescriptor);
GUID GetSubtype(IMFMediaType * mediaType);
GUID GetMajorType(IMFMediaType * mediaType);
HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key);
HRESULT UnwrapTopo(IMFTopology * pTopology);
HRESULT CopyType(IMFMediaType * in_media_type, IMFMediaType * out_mf_media_type);
HRESULT NegotiateInputType(IMFTransform * transform, DWORD stream_index, IMFMediaType * in_media_type);
HRESULT NegotiateOutputType(IMFTransform * transform, DWORD stream_index, GUID out_format, IMFMediaType * in_media_type);
IMFMediaType * CreateMediaType(GUID major, GUID minor);
HRESULT UnwrapPartialTopo(IMFTopologyNode * node, int level);
IMFTransform* FindEncoderTransform(GUID major, GUID minor);

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
