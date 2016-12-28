#pragma once
#include "Common.h"
#include "AbstractVideoSink.h"
class RendererVideoSink : public AbstractVideoSink {
public:
    RendererVideoSink(HWND hwndVideo, IMFMediaType * in_type, IMFMediaType * out_type) :
        AbstractVideoSink(in_type, out_type),
        _hwndVideo(hwndVideo) {

    }

    HRESULT RendererVideoSink::Shutdown() {
        return S_OK;
    }

    IMFTopologyNode * RendererVideoSink::GetBranch(IMFTopology * topology) {
        CComPtr<IMFActivate> pRendererActivate = NULL;
        GUID majorType = GUID_NULL;
        if (_hwndVideo == NULL)
        {
            THROW_ON_FAIL(E_POINTER);
        }
        CComPtr<IMFTopologyNode> pOutputNode;
        // Get the major media type (e.g. video or audio)
        HRESULT hr = _in_type->GetMajorType(&majorType);
        THROW_ON_FAIL(hr);

        // Create an IMFActivate controller object for the renderer, based on the media type.
        // The activation objects are used by the session in order to create the renderers only when 
        // they are needed - IE only right before starting playback.  The activation objects are also
        // used to shut down the renderers.
        if (majorType == MFMediaType_Audio)
        {
            // if the stream major type is audio, create the audio renderer.
            hr = MFCreateAudioRendererActivate(&pRendererActivate);
        }
        else if (majorType == MFMediaType_Video)
        {
            // if the stream major type is video, create the video renderer, passing in the video
            // window handle - that's where the video will be playing.
            hr = MFCreateVideoRendererActivate(_hwndVideo, &pRendererActivate);
        }
        else
        {
            // fail if the stream type is not video or audio.  For example fail
            // if we encounter a CC stream.
            hr = E_FAIL;
        }

        THROW_ON_FAIL(hr);

        // Create the node which will represent the renderer
        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pOutputNode);
        THROW_ON_FAIL(hr);

        // Store the IActivate object in the sink node - it will be extracted later by the
        // media session during the topology render phase.
        hr = pOutputNode->SetObject(pRendererActivate);
        THROW_ON_FAIL(hr);
        hr = pOutputNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
        THROW_ON_FAIL(hr);
        hr = pOutputNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
        THROW_ON_FAIL(hr);

        hr = topology->AddNode(pOutputNode);
        THROW_ON_FAIL(hr);
        return pOutputNode.Detach();
    }

private:
    HWND _hwndVideo;
};
