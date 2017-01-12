#pragma once
#include "Common.h"
#include "SampleTransform.h"
#include "AbstractVideoSink.h"
#include <Wmcodecdsp.h>
#include "SampleGrabberCB.h"
#include<mfreadwrite.h>
class SampleGrabberVideoSink : public AbstractVideoSink {
public:
    SampleGrabberVideoSink(PCWSTR filePath) 
    {
        HRESULT hr = MFCreateSinkWriterFromURL(filePath,
            NULL,
            NULL,
            &m_pWriter);
        THROW_ON_FAIL(hr);
    }

    IMFTopologyNode * SampleGrabberVideoSink::GetBranch(IMFTopology * topology, IMFMediaType * in_type, GUID output_type, GUID out_subtype) {
        HRESULT hr = CreateFileSink(in_type, output_type, out_subtype);
        CComPtr<IMFTopologyNode> node = GetNode(topology);
        return node;
    }

private: 
    PCWSTR _filePath;
    CComPtr<IMFSinkWriter> m_pWriter;
    CComPtr<IMFTransform> transform;
    CComPtr<SampleGrabberCB> m_Sink;
    IMFTransform * CreateSampleTransform() {
        return new (std::nothrow) SampleTransform();
    }

    IMFTopologyNode * GetNode(IMFTopology * topology) {
        DWORD sink_count;
        CComPtr<IMFStreamSink> stream_sink;
        CComPtr<IMFTopologyNode> output_node;
        HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &output_node);
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

    HRESULT SampleGrabberVideoSink::Shutdown() {
        if (m_Sink) {
            m_Sink->Stop();
            m_Sink.Detach();
            m_Sink = NULL;
        }
        return S_OK;
    }
    
    HRESULT  SampleGrabberVideoSink::Start() {
        m_Sink->Start();
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
        hr = SampleGrabberCB::CreateInstance(m_pWriter,NULL, in_mf_media_type, out_mf_media_type, &m_Sink);
    }

};