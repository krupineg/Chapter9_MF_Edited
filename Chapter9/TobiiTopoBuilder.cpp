#pragma once
#include "TobiiTopoBuilder.h"

#include "TopoBuilder.h"
#include "mftransform.h"
#include <MMSystem.h>
#include "SampleTransform.h"
#include <shlwapi.h>
#include <Wmcodecdsp.h>
#include "AviSink.h"
#include "SampleGrabberCB.h"

HRESULT TobiiTopoBuilder::CreateMediaSource()
{
    IMFActivate **ppDevices;
    UINT32 count;
    HRESULT hr = S_OK;
    MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
    CComPtr<IUnknown> pSource;
    IMFAttributes *pAttributes = NULL;
    hr = MFCreateAttributes(&pAttributes, 2);

    THROW_ON_FAIL(hr);

    // Ask for source type = video capture devices.

    hr = pAttributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );
    hr = pAttributes->SetUINT32(
        MFT_HW_TIMESTAMP_WITH_QPC_Attribute,
        true
    );

    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
    hr = ppDevices[0]->ActivateObject(
        __uuidof(IMFMediaSource),
        (void**)&pSource
    );


    // Get the IMFMediaSource interface from the media source.
    m_pSource = pSource;
    // BREAK_ON_NULL(m_pSource, E_NOINTERFACE);

    return hr;
}

void TobiiTopoBuilder::AddSink(AbstractVideoSink * sink) {
    sinks.push_back(sink); 

}

//
// Since we created the source, we are responsible for shutting it down.
//
HRESULT TobiiTopoBuilder::ShutdownSource(void)
{
    HRESULT hr = S_OK;

    if (m_pSource != NULL)
    {
        // shut down the source
        hr = m_pSource->Shutdown();

        // release the source, since all subsequent calls to it will fail
        m_pSource.Release();
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT TobiiTopoBuilder::Finish(IMFMediaSession * m_pSession) {
    HRESULT hr = S_OK;
    if (m_pSession) {
        hr = m_pSession->Close();
    }
    return hr;
}

HRESULT TobiiTopoBuilder::AfterSessionClose(IMFMediaSession * m_pSession) {
    HRESULT hr = S_OK;
    for each (AbstractVideoSink * sink in sinks)
    {
        hr = sink->Shutdown();
        THROW_ON_FAIL(hr);
    }
    if (m_pSource) {
        hr = m_pSource->Shutdown();
        THROW_ON_FAIL(hr);
        m_pSource.Release();
    }
    if (m_pSession) {
        hr = m_pSession->Shutdown();
        THROW_ON_FAIL(hr);
        m_pSession = NULL;
    }
}

//
//  Creates a playback topology from the media source by extracting presentation
// and stream descriptors from the source, and creating a sink for each of them.
//
HRESULT TobiiTopoBuilder::CreateTopology(void)
{
    bool imfmediasink = true;
    HRESULT hr = S_OK;
    CComQIPtr<IMFPresentationDescriptor> pPresDescriptor;
    DWORD nSourceStreams = 0;

    // release the old topology if there was one        
    m_pTopology.Release();

    // Create a new topology.
    hr = MFCreateTopology(&m_pTopology);
    THROW_ON_FAIL(hr);

    // Create the presentation descriptor for the media source - a container object that
    // holds a list of the streams and allows selection of streams that will be used.
    hr = m_pSource->CreatePresentationDescriptor(&pPresDescriptor);
    THROW_ON_FAIL(hr);

    // Get the number of streams in the media source
    hr = pPresDescriptor->GetStreamDescriptorCount(&nSourceStreams);
    THROW_ON_FAIL(hr);

    //hr = pPresDescriptor->SetUINT32(MF_NALU_LENGTH_SET, 0);
    // THROW_ON_FAIL(hr);
    // For each stream, create source and sink nodes and add them to the topology.
    for (DWORD x = 0; x < nSourceStreams; x++)
    {
        hr = AddBranchToPartialTopology(pPresDescriptor, x);

        // if we failed to build a branch for this stream type, then deselect it
        // that will cause the stream to be disabled, and the source will not produce
        // any data for it
        if (FAILED(hr))
        {
            hr = pPresDescriptor->DeselectStream(x);
            BREAK_ON_FAIL(hr);
        }
    }

    return hr;
}


//
//  Adds a topology branch for one stream.
//
//  pPresDescriptor: The source's presentation descriptor.
//  nStream: Index of the stream to render.
//
//  For each stream, we must do the following steps:
//    1. Create a source node associated with the stream.
//    2. Create an output node for the renderer.
//    3. Connect the two nodes.
//  The media session will resolve the topology, inserting intermediate decoder and other 
//  transform MFTs that will process the data in preparation for consumption by the renderers.
//
HRESULT TobiiTopoBuilder::AddBranchToPartialTopology(
    CComPtr<IMFPresentationDescriptor> pPresDescriptor,
    DWORD nStream)
{
    HRESULT hr = S_OK;
    CComPtr<IMFStreamDescriptor> pStreamDescriptor;
    CComPtr<IMFTopologyNode> pSourceNode;
    CComPtr<IMFTopologyNode> pOutputNode;
    BOOL streamSelected = FALSE;

    THROW_ON_NULL(m_pTopology);

    // Get the stream descriptor for this stream (information about stream).
    hr = pPresDescriptor->GetStreamDescriptorByIndex(nStream, &streamSelected, &pStreamDescriptor);
    THROW_ON_FAIL(hr);

    // Create the topology branch only if the stream is selected - IE if the user wants to play it.
    if (streamSelected)
    {
        if (GetMajorType(GetMediaType(pStreamDescriptor)) == MFMediaType_Audio) {
            return E_FAIL;
        }

        // Create a source node for this stream.
        hr = CreateSourceStreamNode(pPresDescriptor, pStreamDescriptor, pSourceNode);
        THROW_ON_FAIL(hr);

        // Create the output node for the renderer.
        hr = CreateOutputNode(pPresDescriptor, pStreamDescriptor, m_videoHwnd, pSourceNode, &pOutputNode);
        THROW_ON_FAIL(hr);


        // Add the source and sink nodes to the topology.
        hr = m_pTopology->AddNode(pSourceNode);
        THROW_ON_FAIL(hr);

      //  hr = m_pTopology->AddNode(pOutputNode);
        THROW_ON_FAIL(hr);


        // Connect the source node to the output node.  The topology will find the
        // intermediate nodes needed to convert media types.
        hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);
    }


    return hr;
}

//
//  Create a source node for the specified stream
//
//  pPresDescriptor: Presentation descriptor for the media source.
//  pStreamDescriptor: Stream descriptor for the stream.
//  pNode: Reference to a pointer to the new node - returns the new node.
//
HRESULT TobiiTopoBuilder::CreateSourceStreamNode(
    CComPtr<IMFPresentationDescriptor> pPresDescriptor,
    CComPtr<IMFStreamDescriptor> pStreamDescriptor,
    CComPtr<IMFTopologyNode> &pNode)
{
    HRESULT hr = S_OK;

    THROW_ON_NULL(pPresDescriptor, E_POINTER);
    THROW_ON_NULL(pStreamDescriptor, E_POINTER);

    // Create the topology node, indicating that it must be a source node.
    hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
    THROW_ON_FAIL(hr);

    // Associate the node with the source by passing in a pointer to the media source,
    // and indicating that it is the source
    hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, m_pSource);
    THROW_ON_FAIL(hr);

    // Set the node presentation descriptor attribute of the node by passing 
    // in a pointer to the presentation descriptor
    hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPresDescriptor);
    THROW_ON_FAIL(hr);

    // Set the node stream descriptor attribute by passing in a pointer to the stream
    // descriptor
    hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pStreamDescriptor);
    THROW_ON_FAIL(hr);

    return hr;
}



IMFTopologyNode* TobiiTopoBuilder::ConnectOutputs(IMFMediaType * in_type, GUID output_type, GUID out_subtype) {
    if (sinks.size() == 0)
    {
        return NULL;
    }
    if (sinks.size() == 1) {
        return  sinks.at(0)->GetBranch(m_pTopology, in_type, output_type, out_subtype);
    }
    CComPtr<IMFTopologyNode> pTeeNode;
    HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &pTeeNode);

    hr = m_pTopology->AddNode(pTeeNode);
    THROW_ON_FAIL(hr);
    for (int i = 0; i < sinks.size(); i++) {

        CComPtr<IMFTopologyNode> node = sinks.at(i)->GetBranch(m_pTopology, in_type, output_type, out_subtype);
        hr = pTeeNode->ConnectOutput(i, node, 0);
        THROW_ON_FAIL(hr);
    }
    return pTeeNode.Detach();
}

//
//  This function creates an output node for a stream (sink) by going through the
//  following steps:
//  1. Select a renderer based on the media type of the stream - EVR or SAR.
//  2. Create an IActivate object for the renderer.
//  3. Create an output topology node.
//  4. Put the IActivate pointer in the node.
//
//  pStreamDescriptor: pointer to the descriptor for the stream that we are working
//  with.
//  hwndVideo: handle to the video window used if this is the video stream.  If this is
//  an audio stream this parameter is not used.
//  pNode: reference to a pointer to the new node.
//
HRESULT TobiiTopoBuilder::CreateOutputNode(
    CComPtr<IMFPresentationDescriptor> pPresDescriptor,
    CComPtr<IMFStreamDescriptor> pStreamDescriptor,
    HWND hwndVideo,
    IMFTopologyNode* pSNode,
    IMFTopologyNode** ppOutputNode)
{
    HRESULT hr = S_OK;
    CComPtr<IMFMediaTypeHandler> pHandler = NULL;
    CComPtr<IMFActivate> pRendererActivate = NULL;
    CComPtr<IMFTopologyNode> pSourceNode = pSNode;
    CComPtr<IMFTopologyNode> pOutputNode;
    CComPtr<IMFMediaType> in_media_type;
    GUID majorType = GUID_NULL;
    GUID minorType = GUID_NULL;
    DebugLog(L"Create output node");
    // Get the media type handler for the stream which will be used to process
    // the media types of the stream.  The handler stores the media type.
    hr = pStreamDescriptor->GetMediaTypeHandler(&pHandler);
    THROW_ON_FAIL(hr);
    hr = pHandler->GetCurrentMediaType(&in_media_type);
    THROW_ON_FAIL(hr);
    // Get the major media type (e.g. video or audio)
    majorType = GetMajorType(in_media_type);
    minorType = GetSubtype(in_media_type);
    DebugLog(L"stream input: " + DetectSubtype(minorType));
    GUID targetSubtype = GUID_NULL;
    if (majorType == MFMediaType_Audio) {
        targetSubtype = DEFAULT_AUDIO;
    }
    else if (majorType == MFMediaType_Video) {
        targetSubtype = DEFAULT_VIDEO;
    }
    pOutputNode = ConnectOutputs(in_media_type, majorType, targetSubtype);

    *ppOutputNode = pOutputNode.Detach();

    return hr;
}
