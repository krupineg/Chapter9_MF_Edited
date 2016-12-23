#pragma once
#include "helpers.h"

inline bool operator< (const GUID &firstGUID, const GUID &secondGUID) {
    return (memcmp(&firstGUID, &secondGUID, sizeof(GUID)) < 0 ? true : false);
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
    str = my_map.find(guid)->second;
    OutputDebugStringW(str.c_str());

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

HRESULT copyTypeParameters(IMFMediaType * in_media_type, IMFMediaType * out_mf_media_type) {
    UINT32 frameRate = 0;
    UINT32 frameRateDenominator;
    UINT32 aspectRatio = 0;
    UINT32 interlace = 0;
    UINT32 denominator = 0;
    UINT32 width, height, bitrate;
    HRESULT hr = S_OK;

    UINT8 blob[] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xc0, 0x1e, 0x96, 0x54, 0x05, 0x01,
        0xe9, 0x80, 0x80, 0x40, 0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x3c, 0x80 };

    hr = out_mf_media_type->SetBlob(MF_MT_MPEG4_SAMPLE_DESCRIPTION, blob, 24);
    THROW_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_AVG_BITRATE);
    THROW_ON_FAIL(hr);

    out_mf_media_type->GetUINT32(MF_MT_AVG_BITRATE, &bitrate);

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
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_PIXEL_ASPECT_RATIO);*/
    THROW_ON_FAIL(hr);

    //hr = out_mf_media_type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    THROW_ON_FAIL(hr);
    hr = CopyAttribute(in_media_type, out_mf_media_type, MF_MT_INTERLACE_MODE);
    THROW_ON_FAIL(hr);
    return hr;
}

IMFTransform* FindEncoderTransform(GUID out_video_format) {
    UINT32 count = 0;
    IMFActivate **ppActivate = NULL;


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