#pragma once
#include "Common.h"
#include "SampleTransform.h"
#include "AbstractVideoSink.h"
#include <Wmcodecdsp.h>
class FileVideoSink : public AbstractVideoSink {
public:
    FileVideoSink(PCWSTR filePath, IMFMediaType * in_type, IMFMediaType * out_type) :
        AbstractVideoSink(in_type, out_type),
        _filePath(filePath) {      
    }

    IMFTopologyNode * FileVideoSink::GetBranch(IMFTopology * topology) {
        GUID neededSubtype = GetVideoSubtype(_out_type);
        
        CComPtr<IMFTopologyNode> node = GetNode(topology);
        return node;
    }



private:
    PCWSTR _filePath;
    CComPtr<IMFTransform> transform;
    CComPtr<IMFMediaSink> m_Sink;
    IMFTransform * CreateSampleTransform() {
        return new (std::nothrow) SampleTransform();
    }

    IMFTopologyNode * GetNode(IMFTopology * topology) {
        DWORD sink_count;
        CComPtr<IMFStreamSink> stream_sink;
        CComPtr<IMFTopologyNode> output_node;
        HRESULT hr = CreateFileSink(_filePath, _in_type, _out_type);
        THROW_ON_FAIL(hr);
        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &output_node);
        THROW_ON_FAIL(hr);
        hr = m_Sink->GetStreamSinkCount(&sink_count);
        THROW_ON_FAIL(hr);
        ATLASSERT(sink_count == 1);
        hr = m_Sink->GetStreamSinkByIndex(0, &stream_sink);
        THROW_ON_FAIL(hr);
        hr = output_node->SetObject(stream_sink);
        THROW_ON_FAIL(hr);
        hr = output_node->SetUINT32(MF_TOPONODE_STREAMID, 0);
        THROW_ON_FAIL(hr);
        hr = output_node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
        THROW_ON_FAIL(hr);
        hr = topology->AddNode(output_node);
        THROW_ON_FAIL(hr);
        // connect the first Tee node output to the network sink node
       // output_node = AddEncoderIfNeed(_in_type, _out_type, topology, output_node);

        return output_node.Detach();
    }

    HRESULT FileVideoSink::Shutdown() {
        if (m_Sink) {
            HRESULT hr = m_Sink->Shutdown();
            THROW_ON_FAIL(hr);
            m_Sink.Release();
            m_Sink = NULL;
        }
        return S_OK;
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


    IMFTransform* CreateVideoEncoderMFT(IMFMediaType * in_media_type, GUID out_video_format)
    {
        IMFTransform * pEncoder = FindEncoderTransform(out_video_format);
        HRESULT hr = S_OK;
        DWORD inputstreamsCount;
        DWORD outputstreamsCount;
        hr = pEncoder->GetStreamCount(&inputstreamsCount, &outputstreamsCount);
        THROW_ON_FAIL(hr);
        HRESULT inputHr = negotiateInputType(pEncoder, in_media_type);
        hr = negotiateOutputType(pEncoder, out_video_format, in_media_type);
        DWORD mftStatus = 0;
        pEncoder->GetInputStatus(0, &mftStatus);
        if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus) {
            DebugInfo(L"error");
        }
        THROW_ON_FAIL(hr);
        if (FAILED(inputHr)) {
            hr = negotiateInputType(pEncoder, in_media_type);
            THROW_ON_FAIL(hr);
        }
        return pEncoder;
    }

    IMFTopologyNode* AddEncoderIfNeed(IMFMediaType * inputMediaType, IMFMediaType * outputMediaType, IMFTopology * topology, IMFTopologyNode * output_node)
    {
        CComPtr<IMFTopologyNode> transformNode;
        HRESULT hr = S_OK;
       
        GUID majorType = GetMajorType(inputMediaType);
        GUID minorType = GetVideoSubtype(inputMediaType);
        GUID neededVideoSubtype = GetVideoSubtype(outputMediaType);
        DetectSubtype(minorType);
        DetectSubtype(neededVideoSubtype);
        if (majorType == MFMediaType_Video && minorType != neededVideoSubtype) {
            CComPtr<IMFTopologyNode> sampleTransformNode;
            CComPtr<IMFTransform> sampleTransform;
            sampleTransform = CreateSampleTransform();
            hr = AddTransformNode(topology, sampleTransform, output_node, &sampleTransformNode);
            THROW_ON_FAIL(hr);
            hr = sampleTransformNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
            THROW_ON_FAIL(hr);
            hr = sampleTransformNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
            THROW_ON_FAIL(hr)
            transform = CreateVideoEncoderMFT(_in_type, neededVideoSubtype);
            hr = AddTransformNode(topology, transform, sampleTransformNode, &transformNode);
            THROW_ON_FAIL(hr);
            hr = transformNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
            THROW_ON_FAIL(hr);
            hr = transformNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
            THROW_ON_FAIL(hr)
                return transformNode.Detach();
        }
        else {
            return output_node;
        }
    }

    HRESULT CreateFileSink(PCWSTR filePath, IMFMediaType * in_mf_media_type, IMFMediaType * out_media_type)
    {
        HRESULT hr = S_OK;
        CComPtr<IMFByteStream> byte_stream;
        CComPtr<IMFStreamSink> stream_sink;
        hr = MFCreateFile(
            MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE,
            filePath, &byte_stream);
        THROW_ON_FAIL(hr);
        GUID major = GetMajorType(in_mf_media_type);
        if (major == MFMediaType_Video) {
            if (transform != NULL) {
                CComPtr<IMFMediaType> transformMediaType;
                hr = transform->GetOutputCurrentType(0, &transformMediaType);
                THROW_ON_FAIL(hr);
                UINT32 pcbBlobSize = { 0 };
                hr = transformMediaType->GetBlobSize(MF_MT_MPEG_SEQUENCE_HEADER, &pcbBlobSize);
                THROW_ON_FAIL(hr);
                UINT8* g_blob = (UINT8*)malloc(pcbBlobSize);
                hr = transformMediaType->GetBlob(MF_MT_MPEG_SEQUENCE_HEADER, g_blob, pcbBlobSize, NULL);
                THROW_ON_FAIL(hr);
                hr = out_media_type->SetBlob(MF_MT_MPEG_SEQUENCE_HEADER, g_blob, pcbBlobSize);
                THROW_ON_FAIL(hr);
            }
            hr = out_media_type->SetUINT32(MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY, 0);
            THROW_ON_FAIL(hr);
            hr = out_media_type->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 16);
            THROW_ON_FAIL(hr);
            hr = out_media_type->SetUINT32(MF_MT_MPEG2_PROFILE, 77);
            THROW_ON_FAIL(hr);
            hr = out_media_type->SetUINT32(MF_MT_SAMPLE_SIZE, 1);
            THROW_ON_FAIL(hr);
            hr = out_media_type->SetGUID(MF_MT_AM_FORMAT_TYPE, MFVideoFormat_MPEG2);
            THROW_ON_FAIL(hr);
            hr = out_media_type->SetUINT32(MF_NALU_LENGTH_SET, 0);
            THROW_ON_FAIL(hr);
            hr = out_media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            THROW_ON_FAIL(hr);
            hr = out_media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
            THROW_ON_FAIL(hr);
            hr = MFCreateMPEG4MediaSink(byte_stream, out_media_type, NULL, &m_Sink);
            THROW_ON_FAIL(hr);
        }
        return S_OK;
    }
};