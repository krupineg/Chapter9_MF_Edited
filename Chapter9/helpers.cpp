#pragma once
#include "helpers.h"

inline bool operator< (const GUID &firstGUID, const GUID &secondGUID) {
    return (memcmp(&firstGUID, &secondGUID, sizeof(GUID)) < 0 ? true : false);
}

std::wstringstream& operator<<(std::wstringstream& os, REFGUID guid) {

    os << std::uppercase;
    os.width(8);
    os << std::hex << guid.Data1 << '-';

    os.width(4);
    os << std::hex << guid.Data2 << '-';

    os.width(4);
    os << std::hex << guid.Data3 << '-';

    os.width(2);
    os << std::hex
        << static_cast<short>(guid.Data4[0])
        << static_cast<short>(guid.Data4[1])
        << '-'
        << static_cast<short>(guid.Data4[2])
        << static_cast<short>(guid.Data4[3])
        << static_cast<short>(guid.Data4[4])
        << static_cast<short>(guid.Data4[5])
        << static_cast<short>(guid.Data4[6])
        << static_cast<short>(guid.Data4[7]);
    os << std::nouppercase;
    return os;
}

void DebugGuid(std::wstring pref, GUID guid) {
    std::wstring strng;
    std::wstringstream strstream;
    strstream << pref << L": ";
    strstream << guid << L"\n";
    strng = strstream.str();
    OutputDebugStringW(strng.c_str());
}

void DebugLongLong(std::wstring pref, LONGLONG anything) {
    std::wstring strng;
    std::wstringstream strstream;
    strstream << pref << anything << L"\n";
    strng = strstream.str();
    OutputDebugStringW(strng.c_str());
}

void DebugInfo(std::wstring info) {
    std::wstring strng;
    std::wstringstream strstream;
    strstream << info;
    strng = strstream.str();
    OutputDebugStringW(strng.c_str());
}

void DetectSubtype(GUID guid) {
    std::wstring  str;
    std::map<GUID, std::wstring> my_map = {
        { MFVideoFormat_RGB8, L"MFVideoFormat_RGB8" },
        { MFVideoFormat_RGB555, L"MFVideoFormat_RGB555" },
        { MFVideoFormat_RGB565, L"MFVideoFormat_RGB565" },
        { MFVideoFormat_RGB24, L"MFVideoFormat_RGB24" },
        { MFVideoFormat_RGB32, L"MFVideoFormat_RGB32" },
        { MFVideoFormat_ARGB32, L"MFVideoFormat_ARGB32" },
        { MFVideoFormat_AI44,  L"MFVideoFormat_AI44" },
        { MFVideoFormat_AYUV,  L"MFVideoFormat_AYUV" },
        { MFVideoFormat_I420,  L"MFVideoFormat_I420" },
        { MFVideoFormat_IYUV,  L"MFVideoFormat_IYUV" },
        { MFVideoFormat_NV11,  L"MFVideoFormat_NV11" },
        { MFVideoFormat_NV12,  L"MFVideoFormat_NV12" },
        { MFVideoFormat_UYVY,  L"MFVideoFormat_UYVY" },
        { MFVideoFormat_Y41P,  L"MFVideoFormat_Y41P" },
        { MFVideoFormat_Y41T,  L"MFVideoFormat_Y41T" },
        { MFVideoFormat_Y42T,  L"MFVideoFormat_Y42T" },
        { MFVideoFormat_YUY2,  L"MFVideoFormat_YUY2" },
        { MFVideoFormat_YVU9,  L"MFVideoFormat_YVU9" },
        { MFVideoFormat_YV12,  L"MFVideoFormat_YV12" },
        { MFVideoFormat_YVYU,  L"MFVideoFormat_YVYU" },
        { MFVideoFormat_P010,  L"MFVideoFormat_P010" },
        { MFVideoFormat_P016,  L"MFVideoFormat_P016" },
        { MFVideoFormat_P210,  L"MFVideoFormat_P210" },
        { MFVideoFormat_P216,  L"MFVideoFormat_P216" },
        { MFVideoFormat_v210,  L"MFVideoFormat_v210" },
        { MFVideoFormat_v216,  L"MFVideoFormat_v216" },
        { MFVideoFormat_v410,  L"MFVideoFormat_v410" },
        { MFVideoFormat_Y210,  L"MFVideoFormat_Y210" },
        { MFVideoFormat_Y216,  L"MFVideoFormat_Y216" },
        { MFVideoFormat_Y410,  L"MFVideoFormat_Y410" },
        { MFVideoFormat_Y416,  L"MFVideoFormat_Y416" },
        { MFVideoFormat_DV25,  L"MFVideoFormat_DV25" },
        { MFVideoFormat_DV50,  L"MFVideoFormat_DV50" },
        { MFVideoFormat_DVC,   L"MFVideoFormat_DVC" },
        { MFVideoFormat_H264,  L"MFVideoFormat_H264" },
        { MFVideoFormat_H263, L"MFVideoFormat_H263" },
        { MFVideoFormat_DVSL, L"MFVideoFormat_DVSL" },
        { MFVideoFormat_H264_ES, L"MFVideoFormat_H264_ES" },
        { MFVideoFormat_MJPG, L"MFVideoFormat_MJPG" },
        { MFVideoFormat_WMV1, L"MFVideoFormat_WMV1" },
        { MFVideoFormat_WMV2, L"MFVideoFormat_WMV2" },
        { MFVideoFormat_WMV3, L"MFVideoFormat_WMV3" },
    };
    if (my_map.count(guid) != 0) {
        str = my_map.find(guid)->second;
        DebugInfo(str.c_str());
        
    }
    else {
        DebugGuid(L"undefined subtype: ", guid);
    }
}

std::wstring DetectMajorType(GUID guid) {
    std::wstring  str;
    std::map<GUID, std::wstring> my_map = {
        { MFMediaType_Audio, L"MFMediaType_Audio" },
        { MFMediaType_Binary, L"MFMediaType_Binary" },
        { MFMediaType_FileTransfer, L"MFMediaType_FileTransfer" },
        { MFMediaType_HTML, L"MFMediaType_HTML" },
        { MFMediaType_Image, L"MFMediaType_Image" },
        { MFMediaType_Protected, L"MFMediaType_Protected" },
        { MFMediaType_SAMI,  L"MFMediaType_SAMI" },
        { MFMediaType_Script,  L"MFMediaType_Script" },
        { MFMediaType_Stream,  L"MFMediaType_Stream" },
        { MFMediaType_Video,  L"MFMediaType_Video" }
    };
    if (my_map.count(guid) != 0) {
        str = my_map.find(guid)->second;
        return str.c_str();
    }
    else {
        return L"UNDEFINED";
    }
    
}

IMFMediaType* GetMediaType(IMFStreamDescriptor * pStreamDescriptor) {
    HRESULT hr = S_OK;
    CComPtr<IMFMediaTypeHandler>  handler;
    hr = pStreamDescriptor->GetMediaTypeHandler(&handler);
    THROW_ON_FAIL(hr);
    CComPtr<IMFMediaType> mediaType;
    hr = handler->GetCurrentMediaType(&mediaType);
    THROW_ON_FAIL(hr);
    return mediaType;
}

GUID GetVideoSubtype(IMFMediaType * mediaType) {
    GUID minorType;
    HRESULT hr = mediaType->GetGUID(MF_MT_SUBTYPE, &minorType);
    THROW_ON_FAIL(hr);
    return minorType;
}

GUID GetMajorType(IMFMediaType * mediaType) {
    GUID major;
    HRESULT hr = mediaType->GetMajorType(&major);
    THROW_ON_FAIL(hr);
    return major;
}

HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key)
{
    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = S_OK;

    hr = pSrc->GetItem(key, &var);
    if (SUCCEEDED(hr))
    {
        hr = pDest->SetItem(key, var);
    }

    PropVariantClear(&var);
    return hr;
}

HRESULT CopyVideoType(IMFMediaType * in_media_type, IMFMediaType * out_mf_media_type) {
    UINT32 frameRate = 0;
    UINT32 frameRateDenominator;
    UINT32 aspectRatio = 0;
    UINT32 interlace = 0;
    UINT32 denominator = 0;
    UINT32 width, height, bitrate;
    HRESULT hr = S_OK;
  
   
    //hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AVG_BITRATE);
    THROW_ON_FAIL(hr);
    if (SUCCEEDED(in_media_type ->GetUINT32(MF_MT_AVG_BITRATE, &bitrate)))
    {
        out_mf_media_type->SetUINT32(MF_MT_AVG_BITRATE, bitrate);
    }
    hr = MFGetAttributeRatio(in_media_type, MF_MT_FRAME_SIZE, &width, &height);
    THROW_ON_FAIL(hr);
    hr = MFGetAttributeRatio(in_media_type, MF_MT_FRAME_RATE, &frameRate, &frameRateDenominator);
    THROW_ON_FAIL(hr);
    hr = MFGetAttributeRatio(in_media_type, MF_MT_PIXEL_ASPECT_RATIO, &aspectRatio, &denominator);
    THROW_ON_FAIL(hr);
    hr = MFSetAttributeRatio(out_mf_media_type, MF_MT_FRAME_SIZE, width, height);
    THROW_ON_FAIL(hr);
    hr = MFSetAttributeRatio(out_mf_media_type, MF_MT_FRAME_RATE, frameRate, frameRateDenominator);
    THROW_ON_FAIL(hr);
    hr = MFSetAttributeRatio(out_mf_media_type, MF_MT_PIXEL_ASPECT_RATIO, aspectRatio, denominator);
    THROW_ON_FAIL(hr);
    /* hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_FRAME_SIZE);
    THROW_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_FRAME_RATE);
    THROW_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_PIXEL_ASPECT_RATIO);
    THROW_ON_FAIL(hr);*/
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_INTERLACE_MODE);
    THROW_ON_FAIL(hr);
    return hr;
}

HRESULT IPropertyStore_CopyFromAttribute(IMFAttributes * pSrc, IPropertyStore *pps, const GUID& attributeKey, REFPROPERTYKEY key) {
    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = S_OK;

    hr = pSrc->GetItem(attributeKey, &var);
    if (SUCCEEDED(hr))
    {
        hr = pps->SetValue(key, var);
    }

    PropVariantClear(&var);
    return hr;
}

HRESULT IPropertyStore_SetValue(IPropertyStore *pps, REFPROPERTYKEY pkey, PCWSTR pszValue)
{
    PROPVARIANT var;
    HRESULT hr = InitPropVariantFromString(pszValue, &var);
    if (SUCCEEDED(hr)) {
        hr = pps->SetValue(pkey, var);
        PropVariantClear(&var);
    }
    return hr;
}

HRESULT IPropertyStore_SetValue(IPropertyStore *pps, REFPROPERTYKEY pkey, UINT32 pszValue)
{
    PROPVARIANT var;
    HRESULT hr = InitPropVariantFromUInt32(pszValue, &var);
    if (SUCCEEDED(hr)) {
        hr = pps->SetValue(pkey, var);
        PropVariantClear(&var);
    }
    return hr;
}

IMFTransform* FindEncoderTransform(GUID out_video_format) {
    UINT32 count = 0;
    IMFActivate **ppActivate = NULL;
    DetectSubtype(out_video_format);

    MFT_REGISTER_TYPE_INFO info = { 0 };
    info.guidMajorType = MFMediaType_Video;
    info.guidSubtype = out_video_format;

    MFTEnumEx(
        MFT_CATEGORY_VIDEO_ENCODER,
        0x00000073,
        NULL,       // Input type
        &info,      // Output type
        &ppActivate,
        &count
    );

    if (count == 0)
    {
        THROW_ON_FAIL(MF_E_TOPO_CODEC_NOT_FOUND);
    }
    HRESULT hr;
    IMFTransform *pEncoder;
    // Create the first encoder in the list.
    THROW_ON_FAIL(ppActivate[0]->ActivateObject(__uuidof(IMFTransform), (void**)&pEncoder));

    CoTaskMemFree(ppActivate);
    return pEncoder;
}


void DebugInfoWithLevel(std::wstring pref, int level) {

    DebugInfo(L"\\");
    for (int i = 0; i < level; i++)
    {
        DebugInfo(L"--");
    }
    DebugInfo(pref);
}

HRESULT HandleChildren(IMFTopologyNode * node, int level) {
    DWORD outputs = 0;
    HRESULT hr = node->GetOutputCount(&outputs);
    for (int i = 0; i < outputs; i++) {
        DWORD nextInput = 0;
        CComPtr<IMFTopologyNode> childNode;
        hr = node->GetOutput(0, &childNode, &nextInput);
        if (SUCCEEDED(hr))
        {
            UnwrapPartialTopo(childNode, level);
        }        
    }
    return hr;
}

HRESULT HandleNodeObject(IMFTopologyNode * node) {
    
    CComPtr<IUnknown> unknown;
    HRESULT hr = node->GetObjectW(&unknown);
    if (SUCCEEDED(hr)) {
        DebugInfo(L"(");
        CComPtr<IMFActivate> activate;
        unknown->QueryInterface(IID_PPV_ARGS(&activate));
        if (activate != NULL) {
            GUID clsId;
            activate->GetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, &clsId);
            DebugInfo(L"activate");
        }
                
        CComPtr<IMFTransform> transform;
        unknown->QueryInterface(IID_PPV_ARGS(&transform));
        if (transform != NULL) {
            CComPtr<IMFMediaType> inputType;
            hr = transform->GetInputCurrentType(0, &inputType);
            if (SUCCEEDED(hr)) {
                DebugInfo(L"input type: ");
                DebugInfo(DetectMajorType(GetMajorType(inputType)));
                DebugInfo(L" ");
                DetectSubtype(GetVideoSubtype(inputType));
                DebugInfo(L"; ");
            }
            CComPtr<IMFMediaType> outputType;
            hr = transform->GetOutputCurrentType(0, &outputType);
            if (SUCCEEDED(hr)) {
                DebugInfo(L"output type: ");
                DebugInfo(DetectMajorType(GetMajorType(outputType)));
                DebugInfo(L" ");
                DetectSubtype(GetVideoSubtype(outputType));                
            }
            
        }

        CComPtr<IMFStreamSink> streamSink;
        unknown->QueryInterface(IID_PPV_ARGS(&streamSink));
        if (streamSink != NULL) {
            CComPtr<IMFMediaTypeHandler> mediaTypeHandler;
            hr = streamSink->GetMediaTypeHandler(&mediaTypeHandler);
            CComPtr<IMFMediaType> inputType;
            mediaTypeHandler->GetCurrentMediaType(&inputType);
            if (SUCCEEDED(hr)) {
                DebugInfo(L"input type: ");
                DebugInfo(DetectMajorType(GetMajorType(inputType)));
                DetectSubtype(GetVideoSubtype(inputType));
                
            }
        }

        DebugInfo(L")");
    }
    return hr;
}


HRESULT HandleSourceNode(IMFTopologyNode * node) {
    DebugInfoWithLevel(L"[SOURCE]", 0);
    HandleNodeObject(node);
    DebugInfo(L"\n");
    HRESULT hr = HandleChildren(node, 1);   
    return hr;
}


HRESULT HandleTeeNode(IMFTopologyNode * node, int level) {
    DebugInfoWithLevel(L"[TEE]", level);
    HandleNodeObject(node);
    DebugInfo(L"\n");
    HRESULT hr = HandleChildren(node, level + 1);
    return hr;
}

HRESULT HandleTransformNode(IMFTopologyNode * node, int level) {
    DebugInfoWithLevel(L"[TRANSFORM]", level);
    HandleNodeObject(node);
    DebugInfo(L"\n");
    HRESULT hr = HandleChildren(node, level + 1);
    return hr;
}

HRESULT HandleSinkNode(IMFTopologyNode * node, int level) {
    DebugInfoWithLevel(L"[SINK]", level);
    HandleNodeObject(node);
    DebugInfo(L"\n");
    HRESULT hr = HandleChildren(node, level + 1);
    return hr;
}


HRESULT UnwrapPartialTopo(IMFTopologyNode * node, int level) {
    MF_TOPOLOGY_TYPE type;
    HRESULT hr = node->GetNodeType(&type);
    std::wstring strng;
    std::wstringstream *strstream = new std::wstringstream();
  
    if (type == MF_TOPOLOGY_SOURCESTREAM_NODE) {
        HandleSourceNode(node);
    }
    if (type == MF_TOPOLOGY_TEE_NODE) {
        HandleTeeNode(node, level);
    }
    if (type == MF_TOPOLOGY_OUTPUT_NODE) {
        HandleSinkNode(node, level);
    }
    if (type == MF_TOPOLOGY_TRANSFORM_NODE) {
        HandleTransformNode(node, level);
    }
    return hr;
}

HRESULT UnwrapTopo(IMFTopology * pTopology) {
    WORD nodeCount = 0;
    DebugInfo(L"\n");
    DebugInfo(L"======================\n");
    DebugInfo(L"DEBUG TOPOLOGY\n");
    HRESULT hr = pTopology->GetNodeCount(&nodeCount);
    for (int i = 0; i < nodeCount; i++) {
        CComPtr<IMFTopologyNode> node = NULL;
        hr = pTopology->GetNode(i, &node);
        if (SUCCEEDED(hr)) {
            MF_TOPOLOGY_TYPE type;
            hr = node->GetNodeType(&type);
            if (SUCCEEDED(hr)) {
                if (type == MF_TOPOLOGY_SOURCESTREAM_NODE) {
                    hr = UnwrapPartialTopo(node, 0);
                }
            }
           
        }
       
    }
    DebugInfo(L"======================\n");
    return hr;
}