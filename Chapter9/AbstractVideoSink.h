#pragma once
#include "Common.h"
class AbstractVideoSink {
public:
    AbstractVideoSink(IMFMediaType * in_type, IMFMediaType * out_type) :
        _in_type(in_type),
        _out_type(out_type) {

    }

    IMFMediaType * GetOutputMediaType() {
        return _out_type;
    }

    virtual IMFTopologyNode * GetBranch(IMFTopology * topology) = 0;
    virtual HRESULT Shutdown() = 0;

    HRESULT negotiateInputType(IMFTransform * transform, IMFMediaType * in_media_type) {
        GUID neededInputType = GetVideoSubtype(in_media_type);
        DetectSubtype(neededInputType);
        int i = 0;
        IMFMediaType* inputType = NULL;
        HRESULT hr = S_OK;
        while (true)
        {
            hr = transform->GetInputAvailableType(0, i, &inputType);
            i++;
            if (hr == MF_E_TRANSFORM_TYPE_NOT_SET) {
                return hr;
            }
            THROW_ON_FAIL(hr);
            GUID minorType;
            hr = inputType->GetGUID(MF_MT_SUBTYPE, &minorType);
            THROW_ON_FAIL(hr);
            DetectSubtype(minorType);
            if (minorType == neededInputType) {
                hr = CopyVideoType(in_media_type, inputType);
                THROW_ON_FAIL(hr);
                hr = transform->SetInputType(0, inputType, 0);
                THROW_ON_FAIL(hr);
            }
            return hr;
        }
    }

    HRESULT negotiateOutputType(IMFTransform * transform, GUID out_video_format, IMFMediaType * in_media_type) {
        int i = 0;
        HRESULT hr = S_OK;
        IMFMediaType* outputType = NULL;
        while (true) {
            hr = transform->GetOutputAvailableType(0, i, &outputType);
            i++;
            if (FAILED(hr)) {
                break;
            }
            GUID minorType;

            hr = outputType->GetGUID(MF_MT_SUBTYPE, &minorType);
            THROW_ON_FAIL(hr);

            if (minorType == out_video_format) {
                hr = CopyVideoType(in_media_type, outputType);

                THROW_ON_FAIL(hr);
                THROW_ON_FAIL(hr);
                hr = transform->SetOutputType(0, outputType, 0);
                THROW_ON_FAIL(hr);

                break;
            }
        }
        return hr;
    }
protected:


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


    CComPtr<IMFMediaType> _out_type, _in_type;
};

