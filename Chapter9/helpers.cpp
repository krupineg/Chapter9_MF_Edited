#pragma once
#include "helpers.h"
#include <array>
inline bool operator< (const GUID &firstGUID, const GUID &secondGUID) {
    return (memcmp(&firstGUID, &secondGUID, sizeof(GUID)) < 0 ? true : false);
}

std:: wstring guidToString(GUID guid) {
    std::array<wchar_t, 40> output;
    wnsprintf(output.data(), output.size(), L"{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    return std::wstring(output.data());
}

void DebugGuid(std::wstring pref, GUID guid) {
    std::wstring strng;
    std::wstringstream strstream;
    strstream << L"[DEBUG] ";
    strstream << pref;
    strstream << guidToString(guid) << L"\n";
    strng = strstream.str();
    OutputDebugStringW(strng.c_str());
}

void DebugLongLong(std::wstring pref, LONGLONG anything) {
    std::wstring strng;
    std::wstringstream strstream;
    strstream << L"[DEBUG] " << pref << anything << L"\n";
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

void DebugLog(std::wstring info) {
    std::wstring strng;
    std::wstringstream strstream;
    strstream << L"[DEBUG] ";
    strstream << info;
    strstream << L"\n";
    strng = strstream.str();
    OutputDebugStringW(strng.c_str());
}

const std::map<GUID, std::wstring> video_type_map = {
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
const std::map<GUID, std::wstring> major_type_map = {
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

std::wstring DetectSubtype(GUID guid) {
    std::wstring  str;
    if (video_type_map.count(guid) != 0) {
        str = video_type_map.find(guid)->second;
        return str;
    }
    else {
        return L"undefined subtype " + guidToString(guid);
    }
}

std::wstring DetectMajorType(GUID guid) {
    std::wstring  str;
    if (major_type_map.count(guid) != 0) {
        str = major_type_map.find(guid)->second;
        return str.c_str();
    }
    else {
        return L"undefined major type " + guidToString(guid);
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

GUID GetSubtype(IMFMediaType * mediaType) {
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

HRESULT AddTransformNode(
    IMFTopology *pTopology,     // Topology.
    IMFTransform *pMFT,         // MFT.
    IMFTopologyNode *output,
    IMFTopologyNode **ppNode    // Receives the node pointer.
)
{
    *ppNode = NULL;

    CComPtr<IMFTopologyNode> pNode = NULL;

    // Create the node.
    HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pNode);
    THROW_ON_FAIL(hr);

    // Set the object pointer.
    hr = pNode->SetObject(pMFT);
    THROW_ON_FAIL(hr);

    hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, 0);

    // Add the node to the topology.
    hr = pTopology->AddNode(pNode);
    THROW_ON_FAIL(hr);

    hr = pNode->ConnectOutput(0, output, 0);
    THROW_ON_FAIL(hr);
    // Return the pointer to the caller.

    *ppNode = pNode;
    (*ppNode)->AddRef();

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
    if (SUCCEEDED(in_media_type ->GetUINT32(MF_MT_AVG_BITRATE, &bitrate)))
    {
        out_mf_media_type->SetUINT32(MF_MT_AVG_BITRATE, bitrate);
    }
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AM_FORMAT_TYPE);
    DEBUG_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_FIXED_SIZE_SAMPLES);
    DEBUG_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_ALL_SAMPLES_INDEPENDENT);
    DEBUG_ON_FAIL(hr); 
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_FRAME_RATE_RANGE_MIN);
    DEBUG_ON_FAIL(hr); 
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_FRAME_RATE_RANGE_MAX);
    DEBUG_ON_FAIL(hr);
    hr = MFGetAttributeRatio(in_media_type, MF_MT_FRAME_SIZE, &width, &height);
    DEBUG_ON_FAIL(hr);
    hr = MFGetAttributeRatio(in_media_type, MF_MT_FRAME_RATE, &frameRate, &frameRateDenominator);
    DEBUG_ON_FAIL(hr);
    hr = MFGetAttributeRatio(in_media_type, MF_MT_PIXEL_ASPECT_RATIO, &aspectRatio, &denominator);
    DEBUG_ON_FAIL(hr);
    hr = MFSetAttributeRatio(out_mf_media_type, MF_MT_FRAME_SIZE, width, height);
    DEBUG_ON_FAIL(hr);
    hr = MFSetAttributeRatio(out_mf_media_type, MF_MT_FRAME_RATE, frameRate, frameRateDenominator);
    DEBUG_ON_FAIL(hr);
    hr = MFSetAttributeRatio(out_mf_media_type, MF_MT_PIXEL_ASPECT_RATIO, aspectRatio, denominator);
    DEBUG_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_INTERLACE_MODE);
    DEBUG_ON_FAIL(hr);
    return hr;
}


HRESULT CopyAudioType(IMFMediaType * in_media_type, IMFMediaType * out_mf_media_type) {
    HRESULT hr = S_OK;
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AUDIO_NUM_CHANNELS);
    DEBUG_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AUDIO_SAMPLES_PER_SECOND);
    DEBUG_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AUDIO_BLOCK_ALIGNMENT);
    DEBUG_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
    DEBUG_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AVG_BITRATE);
    DEBUG_ON_FAIL(hr);
    return hr;
}

HRESULT CopyType(IMFMediaType * in_media_type, IMFMediaType * out_mf_media_type) {
    GUID major = GetMajorType(in_media_type);
    HRESULT hr = S_OK;
    DebugLog(L"copy subtype of " + DetectMajorType(major));
    if (major == MFMediaType_Audio) {
        hr = CopyAudioType(in_media_type, out_mf_media_type);
    }
    else if (major == MFMediaType_Video) {
        hr = CopyVideoType(in_media_type, out_mf_media_type);
    }
    else {
        hr = E_FAIL;
    }
    THROW_ON_FAIL(hr);
    DebugLog(L"finished copy subtype");    
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

IMFTransform* FindEncoderTransform(GUID major, GUID minor) {
    UINT32 count = 0;
    IMFActivate **ppActivate = NULL;
    DebugInfo(L"find encoder to " + DetectSubtype(minor));

    MFT_REGISTER_TYPE_INFO info = { 0 };
    info.guidMajorType = major;
    info.guidSubtype = minor;

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
                DebugInfo(DetectSubtype(GetSubtype(inputType)));
                DebugInfo(L"; ");
            }
            CComPtr<IMFMediaType> outputType;
            hr = transform->GetOutputCurrentType(0, &outputType);
            if (SUCCEEDED(hr)) {
                DebugInfo(L"output type: ");
                DebugInfo(DetectMajorType(GetMajorType(outputType)));
                DebugInfo(L" ");
                DebugInfo(DetectSubtype(GetSubtype(outputType)));
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
                DebugInfo(DetectSubtype(GetSubtype(inputType)));
                
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
    HRESULT hr = S_OK;
    if (pTopology) {
        hr = pTopology->GetNodeCount(&nodeCount);
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
    }
    else {
        DebugInfo(L"Topology is null");
    }
    DebugInfo(L"======================\n");
    return hr;
}


IMFMediaType * CreateMediaType(GUID major, GUID minor) {
    CComPtr<IMFMediaType> outputType = NULL;
    HRESULT hr = MFCreateMediaType(&outputType);
    THROW_ON_FAIL(hr);
    hr = outputType->SetGUID(MF_MT_MAJOR_TYPE, major);
    THROW_ON_FAIL(hr);
    hr = outputType->SetGUID(MF_MT_SUBTYPE, minor);
    THROW_ON_FAIL(hr);
    return outputType.Detach();
}

HRESULT NegotiateInputType(IMFTransform * transform, DWORD stream_index, IMFMediaType * in_media_type) {
    DebugLog(L"negotiate input type");
    GUID neededInputType = GetSubtype(in_media_type);
    GUID major = GetMajorType(in_media_type);
    CComPtr<IMFMediaType> copyType = CreateMediaType(major, neededInputType);
    HRESULT hr = CopyType(in_media_type, copyType);
    DebugLog(L"needed input subtype: " + DetectSubtype(neededInputType));
    int i = 0;
    //CComPtr<IMFMediaType> outputType = CreateMediaType(major, neededInputType);
   // HRESULT hr = CopyType(in_media_type, outputType);
   // THROW_ON_FAIL(hr);
    hr = transform->SetInputType(stream_index, copyType, 0);
    return hr;
    /*
    IMFMediaType* inputType = NULL;
    HRESULT hr = S_OK;
    while (SUCCEEDED(hr))
    {
        hr = transform->GetInputAvailableType(stream_index, i, &inputType);
        if (FAILED(hr)) {
            DEBUG_ON_FAIL(hr);
            break;
        }
        i++;
        GUID minorType;
        hr = inputType->GetGUID(MF_MT_SUBTYPE, &minorType);
        THROW_ON_FAIL(hr);
        DebugLog(L"availabla subtype found: " + DetectSubtype(minorType));
        if (minorType == neededInputType) {
            hr = CopyType(in_media_type, inputType);
            THROW_ON_FAIL(hr);
            DebugLog(L"set input type");
            hr = transform->SetInputType(stream_index, inputType, 0);
            THROW_ON_FAIL(hr);
            DebugLog(L"successfull set input type");
            return hr;
        }
    }
    return hr;*/
}


HRESULT NegotiateOutputType(IMFTransform * transform, DWORD stream_index, GUID out_format, IMFMediaType * in_media_type) {
    DebugLog(L"negotiate output type");
    int i = 0;
    HRESULT hr = S_OK;
    DebugLog(L"needed output subtype: " + DetectSubtype(out_format));
    GUID major = GetMajorType(in_media_type);
    CComPtr<IMFMediaType> outputType = CreateMediaType(major, out_format);
    hr = CopyType(in_media_type, outputType);
    THROW_ON_FAIL(hr);
    hr = transform->SetOutputType(stream_index, outputType, 0);
    return hr;
    /*
    while (true) {
        hr = transform->GetOutputAvailableType(stream_index, i, &outputType);
        if (FAILED(hr)) {
            break;
        }
        i++;
        GUID minorType;
        hr = outputType->GetGUID(MF_MT_SUBTYPE, &minorType);
        THROW_ON_FAIL(hr);
        DebugLog(L"availabla subtype found: " + DetectSubtype(minorType));
        if (minorType == out_format) {
            hr = CopyType(in_media_type, outputType);
            THROW_ON_FAIL(hr);
            DebugLog(L"set output type");
            hr = transform->SetOutputType(stream_index, outputType, 0);
            THROW_ON_FAIL(hr);
            DebugLog(L"successfull set output type");
            break;
        }
    }*/
}


IMFTransform* CreateEncoderMft(IMFMediaType * in_media_type, DWORD stream_index, GUID out_type, GUID out_subtype)
{
    DebugLog(L"Create encoder from " + DetectSubtype(GetSubtype(in_media_type)) + L" to " + DetectSubtype(out_subtype));
    IMFTransform * pEncoder = FindEncoderTransform(out_type, out_subtype);
    HRESULT hr = S_OK;
    DWORD inputstreamsCount;
    DWORD outputstreamsCount;

    hr = pEncoder->GetStreamCount(&inputstreamsCount, &outputstreamsCount);
    THROW_ON_FAIL(hr);
    HRESULT inputHr = NegotiateInputType(pEncoder, stream_index, in_media_type);
    hr = NegotiateOutputType(pEncoder, stream_index, out_subtype, in_media_type);
    DWORD mftStatus = 0;
    pEncoder->GetInputStatus(0, &mftStatus);
    if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus) {
        DebugLog(L"need to set output type before input");
    }
    THROW_ON_FAIL(hr);
    if (FAILED(inputHr)) {
        hr = NegotiateInputType(pEncoder, stream_index, in_media_type);
        THROW_ON_FAIL(hr);
    }
    DebugLog(L"encoder is created successfully");
    return pEncoder;
}


IMFTransform * CreateSampleTransform() {
    //create color converter
    /*HRESULT hr = DllRegisterServer();
    THROW_ON_FAIL(hr);
    IMFTransform *sampleTransform = NULL;
    hr = CoCreateInstance(CLSID_SampleTransformMFT, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, (void**)&sampleTransform);*/
    return new (std::nothrow) SampleTransform();
}

IMFTransform * CreateRemuxTransform() {
    IMFTransform *pIMFTransform = NULL;
    HRESULT hr = CoCreateInstance(
        CLSID_CMSH264RemuxMFT,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IMFTransform,
        (void**)&pIMFTransform
    );
    return pIMFTransform;
}



IMFTransform* CreateColorConverterMFT()
{
    //register color converter locally
    THROW_ON_FAIL(MFTRegisterLocalByCLSID(__uuidof(CColorConvertDMO), MFT_CATEGORY_VIDEO_PROCESSOR, L"", MFT_ENUM_FLAG_SYNCMFT, 0, NULL, 0, NULL));

    //create color converter
    IMFTransform *pColorConverterMFT = NULL;
    THROW_ON_FAIL(CoCreateInstance(__uuidof(CColorConvertDMO), NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, (void**)&pColorConverterMFT));

    return pColorConverterMFT;
}
