#pragma once
#include "Common.h"
#include "SampleTransform.h"
#include "AbstractVideoSink.h"
#include <Wmcodecdsp.h>
#include<mfreadwrite.h>
class MediaMP4VideoSink : public AbstractVideoSink {

public:
    MediaMP4VideoSink(PCWSTR filePath)
    {
        HRESULT hr = MFCreateFile(
            MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE,
            filePath, &byte_stream);
        THROW_ON_FAIL(hr);
    }

    IMFTopologyNode * MediaMP4VideoSink::GetBranch(IMFTopology * topology, IMFMediaType * in_type, GUID output_type, GUID out_subtype) {
        HRESULT hr = CreateFileSink(in_type, output_type, out_subtype);
        CComPtr<IMFTopologyNode> node = GetNode(topology);
        stream_index++;
        return node;
    }

private:
    CComPtr<IMFMediaSink>  m_MediaSink;
    PCWSTR _filePath;
    CComPtr<IMFTransform> transformVideo = NULL, transformAudio = NULL;
    CComPtr<IMFMediaType> videoType = NULL, audioType = NULL;
    CComPtr<IMFByteStream> byte_stream;
    UINT stream_index = 0;

   
    IMFTopologyNode * GetNode(IMFTopology * topology) {
        CComPtr<IMFTransform> transform;
        DWORD sink_count;
        CComPtr<IMFStreamSink> stream_sink;
        CComPtr<IMFTopologyNode> output_node;

        HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &output_node);

        hr = m_MediaSink->GetStreamSinkCount(&sink_count);
        THROW_ON_FAIL(hr);
        ATLASSERT(sink_count == 1);
        hr = m_MediaSink->GetStreamSinkByIndex(stream_index, &stream_sink);
        THROW_ON_FAIL(hr);
        hr = output_node->SetObject(stream_sink);
        THROW_ON_FAIL(hr);

        THROW_ON_FAIL(hr);
        hr = output_node->SetObject(stream_sink);
        THROW_ON_FAIL(hr);
        hr = output_node->SetUINT32(MF_TOPONODE_STREAMID, 0);
        THROW_ON_FAIL(hr);
        hr = output_node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
        THROW_ON_FAIL(hr);
        hr = topology->AddNode(output_node);
        THROW_ON_FAIL(hr);
        return output_node.Detach();
    }

    HRESULT MediaMP4VideoSink::Shutdown() {
        if (m_MediaSink) {
            HRESULT hr = m_MediaSink->Shutdown();
            THROW_ON_FAIL(hr);
            m_MediaSink.Release();
            m_MediaSink = NULL;
        }
        return S_OK;
    }

    HRESULT  MediaMP4VideoSink::Start() {
        HRESULT hr =MFCreateMPEG4MediaSink(byte_stream, videoType, audioType, &m_MediaSink);
        THROW_ON_FAIL(hr);
        return S_OK;
    }

    HRESULT CreateFileSink(IMFMediaType * in_mf_media_type, GUID out_type, GUID out_subtype)
    {
        HRESULT hr = S_OK;
        CComPtr<IMFMediaType> out_mf_media_type;
        hr = MFCreateMediaType(&out_mf_media_type);
        THROW_ON_FAIL(hr);
        hr = out_mf_media_type->SetGUID(MF_MT_MAJOR_TYPE, out_type);
        THROW_ON_FAIL(hr);
        hr = out_mf_media_type->SetGUID(MF_MT_SUBTYPE, out_subtype);
        THROW_ON_FAIL(hr);
        hr = CopyType(in_mf_media_type, out_mf_media_type);
        GUID currentMajor = GetMajorType(in_mf_media_type);
        GUID currentMinor = GetSubtype(in_mf_media_type);
        if (currentMajor == MFMediaType_Video)
        {
            if (currentMinor != DEFAULT_VIDEO) {
                DebugLog(L"need to add transform for video");
                transformVideo = CreateEncoderMft(in_mf_media_type, stream_index, MFMediaType_Video, DEFAULT_VIDEO);
            }
            videoType = out_mf_media_type.Detach();
        }
        else if (currentMajor == MFMediaType_Audio) 
        {
            if (currentMinor != DEFAULT_AUDIO) {
                DebugLog(L"need to add transform for audio");
                transformAudio = CreateEncoderMft(in_mf_media_type, stream_index, MFMediaType_Audio, DEFAULT_AUDIO);
            }
            audioType = out_mf_media_type.Detach();
        }
    }

};