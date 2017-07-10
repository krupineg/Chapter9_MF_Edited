#pragma once
#include "TopoBuilder.h"
#include "mftransform.h"
#include <MMSystem.h>
#include "SampleTransform.h"
#include <shlwapi.h>
#include <Wmcodecdsp.h>
#include "AviSink.h"
#include "SampleGrabberCB.h"
#include "FormatReader.h"
#include "TempFileStreamActivate.h"
IMFTransform * CreateSampleTransform() {
    //create color converter
    /*HRESULT hr = DllRegisterServer();
    THROW_ON_FAIL(hr);
    IMFTransform *sampleTransform = NULL;
    hr = CoCreateInstance(CLSID_SampleTransformMFT, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, (void**)&sampleTransform);*/
    return new (std::nothrow) SampleTransform();
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

HRESULT negotiateInputType(IMFTransform * transform, IMFMediaType * in_media_type) {
    GUID neededInputType = GetSubtype(in_media_type);
    DebugInfo(DetectSubtype(neededInputType));
    int i = 0;
    IMFMediaType* inputType = NULL;
    HRESULT hr = S_OK;
    HRESULT getType = S_OK;
    while (SUCCEEDED(hr))
    {
        getType = transform->GetInputAvailableType(0, i, &inputType);
        i++;
        if (getType == MF_E_TRANSFORM_TYPE_NOT_SET) {
            return getType;
        }       
        THROW_ON_FAIL(hr);        
        GUID minorType = GetSubtype(inputType); 
        DebugInfo(DetectSubtype(minorType));
        if (minorType == neededInputType) {
            hr = CopyType(in_media_type, inputType);
            THROW_ON_FAIL(hr);
            //hr = transform->SetInputType(0, inputType, 0);
            THROW_ON_FAIL(hr);
            return hr;
        }
    }
    return hr;
}

HRESULT negotiateOutputType(IMFTransform * transform, GUID out_video_format, IMFMediaType * in_media_type) {
    int i = 0;
    HRESULT hr = S_OK;
    IMFMediaType* outputType = NULL;
    while (true) {
        hr = transform->GetOutputAvailableType(0, i, &outputType);        
        if (FAILED(hr)) {
            break;
        }
        i++;
        GUID minorType = GetSubtype(outputType);        
        if (minorType == out_video_format) {
            hr = CopyType(in_media_type, outputType);
            THROW_ON_FAIL(hr);
            hr = transform->SetOutputType(0, outputType, 0);
            THROW_ON_FAIL(hr);
            break;
        }
    }
    return hr;
}

IMFMediaType* ConvertToOutputType(IMFMediaType* in_mf_media_type) {
    CComPtr<IMFMediaType> out_mf_media_type;
    HRESULT hr = MFCreateMediaType(&out_mf_media_type);
    THROW_ON_FAIL(hr);
    hr = out_mf_media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    THROW_ON_FAIL(hr);
    hr = out_mf_media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    THROW_ON_FAIL(hr); 
    hr = CopyType(in_mf_media_type, out_mf_media_type);
    return out_mf_media_type.Detach();
}

IMFTransform* CreateEncoderMft(IMFMediaType * in_media_type, GUID out_type, GUID out_subtype)
{
    GUID subtype = GetSubtype(in_media_type);
    IMFTransform * pEncoder = FindEncoderTransform(subtype, out_type, out_subtype);
    HRESULT hr = S_OK;    
    DWORD inputstreamsCount;
    DWORD outputstreamsCount;

    hr = pEncoder->GetStreamCount(&inputstreamsCount, &outputstreamsCount);
    THROW_ON_FAIL(hr);
    HRESULT inputHr = NegotiateInputType(pEncoder, 0, in_media_type);
    hr = NegotiateOutputType(pEncoder, 0, out_subtype, in_media_type);
    DWORD mftStatus = 0;
    pEncoder->GetInputStatus(0, &mftStatus);
    if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus) {
        OutputDebugStringW(L"error");
    }

    THROW_ON_FAIL(hr);
    if (FAILED(inputHr)){
        hr = NegotiateInputType(pEncoder, 0, in_media_type);
        THROW_ON_FAIL(hr);
    }
    return pEncoder;
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

IMFTopologyNode* CTopoBuilder::AddEncoderIfNeed(IMFTopology * topology, IMFStreamDescriptor * pStreamDescriptor, IMFTopologyNode * output_node)
{
    CComPtr<IMFTopologyNode> transformNode;
    CComPtr<IMFTopologyNode> colorConverterNode;
    CComPtr<IMFTopologyNode> sampleTransformNode;
    HRESULT hr = S_OK;
    IMFMediaType * mediaType = GetMediaType(pStreamDescriptor);
    CComPtr<IMFTransform> sampleTransform;
    CComPtr<IMFTransform> encoder;
    CComPtr<IMFTransform> color = CreateColorConverterMFT();
   // sampleTransform = CreateSampleTransform();
   // hr = AddTransformNode(topology, sampleTransform, output_node, &sampleTransformNode);
    THROW_ON_FAIL(hr);
    GUID minorType = GetSubtype(mediaType);
    if (minorType != MEDIASUBTYPE_H264) {
        encoder = CreateEncoderMft(mediaType, MFMediaType_Video, MEDIASUBTYPE_H264);
    }
    if (encoder != NULL) {
        DebugInfo(DetectSubtype(minorType));
        //CComPtr<IMFTransform> transform;
       // transform = CreateEncoderMft(mediaType, MFVideoFormat_H264);
        
        hr = AddTransformNode(topology, encoder, output_node, &transformNode);
        THROW_ON_FAIL(hr);
        hr = AddTransformNode(topology, color, transformNode, &colorConverterNode);
        THROW_ON_FAIL(hr);
        return colorConverterNode.Detach();
    }
    else {
        return sampleTransformNode;
    }
}

//
// Initiates topology building from the file URL by first creating a media source, and then
// adding source and sink nodes for every stream found in the file.
//
HRESULT CTopoBuilder::RenderURL(PCWSTR fileUrl, HWND videoHwnd, bool addNetwork)
{
    HRESULT hr = S_OK;
    ToFile(addNetwork);
    m_videoHwnd = videoHwnd;

    // The topology can have either a rendering sink (when videoHwnd is not NULL), a 
    // network sink, or both.
    if(videoHwnd == NULL && !addNetwork)
    {
        hr = E_INVALIDARG;
        throw;
    }

    // first create the media source for the file/stream passed in.  Fail and fall out if
    // the media source creation fails (e.g. if the file format is not recognized)
    hr = CreateMediaSource(fileUrl);
    THROW_ON_FAIL(hr);

    // add a network sink if one was requested
    if(addNetwork)
    {
        //hr = CreateNetworkSink("127.0.0.1", 8080);
        THROW_ON_FAIL(hr);
    }

    // create the actual topology
    hr = CreateTopology();

    return hr;
}
HRESULT CTopoBuilder::RenderCamera(HWND videoHwnd, bool addNetwork)
{
    HRESULT hr = S_OK;
    ToFile(addNetwork);
    m_videoHwnd = videoHwnd;

    // The topology can have either a rendering sink (when videoHwnd is not NULL), a 
    // network sink, or both.
    if (videoHwnd == NULL && !addNetwork)
    {
        hr = E_INVALIDARG;
        throw;
    }

    // first create the media source for the file/stream passed in.  Fail and fall out if
    // the media source creation fails (e.g. if the file format is not recognized)
    hr = CreateMediaSource();
    THROW_ON_FAIL(hr);

    // add a network sink if one was requested
    if (addNetwork)
    {
        //hr = CreateNetworkSink(8080);
        THROW_ON_FAIL(hr);
    }

    // create the actual topology
    hr = CreateTopology();

    return hr;
}
//a
// Create a network sink that will listen for requests on the specified port.
//
HRESULT CTopoBuilder::CreateNetworkSink(PCSTR host, DWORD requestPort, IMFMediaType* output_type)
{
    HRESULT hr = S_OK;
    CComPtr<IMFPresentationDescriptor> pPresDescriptor;
    CComPtr<IMFASFProfile> pAsfProfile;
    CComQIPtr<IMFASFContentInfo> pAsfContentInfo;
    
    CComPtr<IMFActivate> pByteStreamActivate;
    CComPtr<IMFActivate> pNetSinkActivate;
    THROW_ON_NULL(m_pSource);

    // create an HTTP activator for the custom HTTP output byte stream object
    pByteStreamActivate = new (std::nothrow) CHttpOutputStreamActivate(host, requestPort);
    THROW_ON_NULL(pByteStreamActivate);
        
    // create the presentation descriptor for the source
    hr = m_pSource->CreatePresentationDescriptor(&pPresDescriptor);
    THROW_ON_FAIL(hr);

    
    // create the ASF profile from the presentation descriptor
    hr = MFCreateASFProfileFromPresentationDescriptor(pPresDescriptor, &pAsfProfile);
    if (FAILED(hr)) {
        hr = MFCreateASFProfile(&pAsfProfile);
        CComPtr<IMFASFStreamConfig> stream;
        hr = pAsfProfile->CreateStream(output_type, &stream);
        THROW_ON_FAIL(hr);
        hr = stream->SetStreamNumber(1);
        THROW_ON_FAIL(hr);
        hr = pAsfProfile->SetStream(stream);
        THROW_ON_FAIL(hr);
    }
    THROW_ON_FAIL(hr);

    // create the ContentInfo object for the ASF profile
    hr = MFCreateASFContentInfo(&pAsfContentInfo);
    THROW_ON_FAIL(hr);

    // set the profile on the content info object
    hr = pAsfContentInfo->SetProfile(pAsfProfile);
    THROW_ON_FAIL(hr);

    // create an activator object for an ASF streaming sink
    hr = MFCreateASFStreamingMediaSinkActivate(pByteStreamActivate, pAsfContentInfo, 
        &m_pNetworkSinkActivate);
    THROW_ON_FAIL(hr);
}

HRESULT CTopoBuilder::CreateFileSink(PCWSTR filePath, IMFMediaType * in_mf_media_type)
{
    HRESULT hr = S_OK;
    bool imfmediasink = true;
    CComPtr<IMFByteStream> byte_stream;
    if (imfmediasink) {
        CComPtr<IMFStreamSink> stream_sink;
        hr = MFCreateFile(
            MF_ACCESSMODE_WRITE, MF_OPENMODE_FAIL_IF_NOT_EXIST, MF_FILEFLAGS_NONE,
            filePath, &byte_stream);
        THROW_ON_FAIL(hr);
        if (GetSubtype(in_mf_media_type) != MEDIASUBTYPE_H264) {
            transform = CreateEncoderMft(in_mf_media_type, MFMediaType_Video, MEDIASUBTYPE_H264);
            THROW_ON_NULL(transform);
        }
    }
    CComPtr<IMFMediaType> out_mf_media_type = ConvertToOutputType(in_mf_media_type);
   
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
        hr = out_mf_media_type->SetBlob(MF_MT_MPEG_SEQUENCE_HEADER, g_blob, pcbBlobSize);
        THROW_ON_FAIL(hr);
    }
    // m_Sink = new (std::nothrow) CAviSink(L"c:\\users\\public\\file.avi", &hr);
    FormatReader::getInstance().Read(L"out media type for sink", out_mf_media_type);
    if (imfmediasink) {

        hr = MFCreateFMPEG4MediaSink(byte_stream, out_mf_media_type, NULL, &m_Sink);
    }
    hr = SampleGrabberCB::CreateTimingInstance(L"A", &sampleGrabber);

  

    //hr = MFCreateMPEG4MediaSink(byte_stream, out_mf_media_type, NULL, &m_Sink);    
    THROW_ON_FAIL(hr);
    


    // Stream Sink を取得する
    return hr;
}

//
// Create a media source for the specified URL string.  The URL can be a path to a stream, 
// or it can be a path to a local file.
//
HRESULT CTopoBuilder::CreateMediaSource(PCWSTR sURL)
{

    CComPtr<IUnknown> pSource;
    MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
    HRESULT hr = S_OK;
    CComPtr<IMFSourceResolver> pSourceResolver = NULL;
    // Create the source resolver.
    hr = MFCreateSourceResolver(&pSourceResolver);
    THROW_ON_FAIL(hr);

    // Use the syncrhonous source resolver to create the media source.
    hr = pSourceResolver->CreateObjectFromURL(
        sURL,                       // URL of the source.
        MF_RESOLUTION_MEDIASOURCE | 
            MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE,  
                                    // indicate that we want a source object, and 
                                    // pass in optional source search parameters
        NULL,                       // Optional property store for extra parameters
        &objectType,                // Receives the created object type.
        &pSource                    // ReceiSampleTransformves a pointer to the media source.
        );
    THROW_ON_FAIL(hr);
    
    // Get the IMFMediaSource interface from the media source.
    
    m_pSource = pSource;
    
   // BREAK_ON_NULL(m_pSource, E_NOINTERFACE);

    return hr;
}

HRESULT CTopoBuilder::CreateMediaSource()
{
    if (m_pSource != nullptr) {
        m_pSource->Shutdown();
        m_pSource.Release();
        m_pSource = nullptr;
    }
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

//
// Since we created the source, we are responsible for shutting it down.
//
HRESULT CTopoBuilder::ShutdownSource(void)
{    
    HRESULT hr = S_OK;

    if(m_pSource != NULL)
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

HRESULT CTopoBuilder::Finish(IMFMediaSession * m_pSession) {
    HRESULT hr = S_OK;
    if (m_pSession) {
        hr = m_pSession->Close();
    }
    return hr;
}

HRESULT CTopoBuilder::AfterSessionClose(IMFMediaSession * m_pSession) {
    HRESULT hr = S_OK;
    if (sampleGrabber) {        
         sampleGrabber->Stop();
        THROW_ON_FAIL(hr);
        sampleGrabber.Release();
        sampleGrabber = NULL;
    }
    if (m_Sink) {
        hr = m_Sink->Shutdown();
        THROW_ON_FAIL(hr);
        m_Sink.Release();
        m_Sink = NULL;
    }
    if (m_pSource) {
        hr = m_pSource->Shutdown();
        THROW_ON_FAIL(hr);
        m_pSource.Release();
        m_pSource = NULL;
    }
    if (m_pNetworkSinkActivate) {
        hr = m_pNetworkSinkActivate->ShutdownObject();
        THROW_ON_FAIL(hr);
        m_pNetworkSinkActivate.Release();
        m_pNetworkSinkActivate = NULL;
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
HRESULT CTopoBuilder::CreateTopology(void)
{
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
        if(FAILED(hr))
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
HRESULT CTopoBuilder::AddBranchToPartialTopology(
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



        // Create a source node for this stream.
        hr = CreateSourceStreamNode(pPresDescriptor, pStreamDescriptor, pSourceNode);
        THROW_ON_FAIL(hr);
       
        // Create the output node for the renderer.
        hr = CreateOutputNode(pPresDescriptor, pStreamDescriptor, m_videoHwnd, pSourceNode, &pOutputNode);
        THROW_ON_FAIL(hr);


        // Add the source and sink nodes to the topology.
        hr = m_pTopology->AddNode(pSourceNode);
        THROW_ON_FAIL(hr);

        hr = m_pTopology->AddNode(pOutputNode);
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
HRESULT CTopoBuilder::CreateSourceStreamNode(
    CComPtr<IMFPresentationDescriptor> pPresDescriptor,
    CComPtr<IMFStreamDescriptor> pStreamDescriptor,
    CComPtr<IMFTopologyNode> &pNode)
{
    HRESULT hr = S_OK;
    pNode = NULL;
    // Create the tee node
    hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &pNode);
    THROW_ON_FAIL(hr);
    THROW_ON_NULL(pPresDescriptor, E_POINTER);
    THROW_ON_NULL(pStreamDescriptor, E_POINTER);
    CComPtr<IMFTopologyNode> pNode1 = NULL;
    // Create the topology node, indicating that it must be a source node.
    hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode1);
    THROW_ON_FAIL(hr);

    // Associate the node with the source by passing in a pointer to the media source,
    // and indicating that it is the source
    hr = pNode1->SetUnknown(MF_TOPONODE_SOURCE, m_pSource);
    THROW_ON_FAIL(hr);

    // Set the node presentation descriptor attribute of the node by passing 
    // in a pointer to the presentation descriptor
    hr = pNode1->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPresDescriptor);
    THROW_ON_FAIL(hr);

    // Set the node stream descriptor attribute by passing in a pointer to the stream
    // descriptor
    hr = pNode1->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pStreamDescriptor);
    THROW_ON_FAIL(hr);   
    m_pTopology->AddNode(pNode1);

    hr = pNode1->ConnectOutput(0, pNode, 0);
    THROW_ON_FAIL(hr);

    CComPtr<IMFTopologyNode> grabberNode = NULL;
    hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &grabberNode);
    SampleGrabberCB* sampleGrabber2 = NULL;
    SampleGrabberCB::CreateTimingInstance(L"inner", &sampleGrabber2);
    CComPtr<IMFActivate>pSinkActivate = NULL;
    CComPtr<IMFMediaType>pType = GetMediaType(pStreamDescriptor);
    THROW_ON_FAIL(MFCreateSampleGrabberSinkActivate(pType, sampleGrabber2, &pSinkActivate));
    // To run as fast as possible, set this attribute (requires Windows 7):
    THROW_ON_FAIL(pSinkActivate->SetUINT32(MF_SAMPLEGRABBERSINK_IGNORE_CLOCK, TRUE));
    hr = grabberNode->SetObject(pSinkActivate);
    THROW_ON_FAIL(hr);
    m_pTopology->AddNode(grabberNode);

    hr = pNode->ConnectOutput(1, grabberNode, 0);
    THROW_ON_FAIL(hr);


    return hr;
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
HRESULT CTopoBuilder::CreateOutputNode(
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

		// Get the media type handler for the stream which will be used to process
	// the media types of the stream.  The handler stores the media type.
	hr = pStreamDescriptor->GetMediaTypeHandler(&pHandler);
    THROW_ON_FAIL(hr);
    hr = pHandler->GetCurrentMediaType(&in_media_type);
    THROW_ON_FAIL(hr);
	// Get the major media type (e.g. video or audio)
    hr = in_media_type->GetMajorType(&majorType);
    THROW_ON_FAIL(hr);
    FormatReader::getInstance().Read(L"in_media_type", in_media_type);
    if(m_videoHwnd != NULL)
    {
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
            hr = MFCreateVideoRendererActivate(hwndVideo, &pRendererActivate);
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
    }

    if(toFile
        && majorType == MFMediaType_Video)
    {
       
        hr = CreateFileSink(L"C:\\Users\\Public\\Encoded21.mp4", in_media_type);
        THROW_ON_FAIL(hr);

        CComPtr<IMFMediaType> out_mf_media_type = ConvertToOutputType(in_media_type);
        hr = CreateNetworkSink("localhost", 8080, out_mf_media_type);
        THROW_ON_FAIL(hr);
        CComPtr<IMFTopologyNode> pOldOutput = pOutputNode;
        pOutputNode = NULL;
        hr = CreateTeeMp4Twig(pPresDescriptor, pStreamDescriptor, pOldOutput, &pOutputNode);
        THROW_ON_FAIL(hr);
    }

    *ppOutputNode = pOutputNode.Detach();

    return hr;
}

HRESULT CTopoBuilder::CreateAsfNode(IMFMediaType * output_type, IMFTopologyNode** node)
{
    CComPtr<IMFTopologyNode> asfNode;
    CComPtr<IMFASFProfile> pAsfProfile;
    CComQIPtr<IMFASFContentInfo> pAsfContentInfo;

    CComPtr<IMFActivate> pByteStreamActivate;
    CComPtr<IMFActivate> pNetSinkActivate;
    HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &asfNode);
    THROW_ON_FAIL(hr);
    pByteStreamActivate = new (std::nothrow) TempFileStreamActivate();
    hr = MFCreateASFProfile(&pAsfProfile);
    CComPtr<IMFASFStreamConfig> stream;
    hr = pAsfProfile->CreateStream(output_type, &stream);
    THROW_ON_FAIL(hr);
    hr = stream->SetStreamNumber(1);
    THROW_ON_FAIL(hr);
    hr = pAsfProfile->SetStream(stream);
    THROW_ON_FAIL(hr);

    // create the ContentInfo object for the ASF profile
    hr = MFCreateASFContentInfo(&pAsfContentInfo);
    THROW_ON_FAIL(hr);

    // set the profile on the content info object
    hr = pAsfContentInfo->SetProfile(pAsfProfile);
    THROW_ON_FAIL(hr);

    // create an activator object for an ASF streaming sink
    hr = MFCreateASFStreamingMediaSinkActivate(pByteStreamActivate, pAsfContentInfo,
        &pNetSinkActivate);
    THROW_ON_FAIL(hr);
    hr = asfNode->SetObject(pNetSinkActivate);
    THROW_ON_FAIL(hr);
    *node = asfNode.Detach();
}

HRESULT CTopoBuilder::CreateTeeMp4Twig(IMFPresentationDescriptor* pPresDescriptor, IMFStreamDescriptor* pStreamDescriptor,
    IMFTopologyNode* pRendererNode, IMFTopologyNode** ppTeeNode)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopologyNode> output_node;
    CComPtr<IMFTopologyNode> output_node2;
    CComPtr<IMFTopologyNode> networkNode;
    CComPtr<IMFTopologyNode> asfNode;
    CComPtr<IMFTopologyNode> encoder_node;
    CComPtr<IMFTopologyNode> pTeeNode;
    CComPtr<IMFStreamSink> stream_sink;

    DWORD sink_count;
    
    hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &output_node);
    THROW_ON_FAIL(hr);
    hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &output_node2);
    THROW_ON_FAIL(hr);
    if (m_Sink) {
        hr = m_Sink->GetStreamSinkCount(&sink_count);
        THROW_ON_FAIL(hr);
        ATLASSERT(sink_count == 1);
        hr = m_Sink->GetStreamSinkByIndex(0, &stream_sink);
        THROW_ON_FAIL(hr);
        hr = output_node->SetObject(stream_sink);        
        THROW_ON_FAIL(hr);
    }
    if (sampleGrabber) {
        CComPtr<IMFActivate>pSinkActivate = NULL;
        CComPtr<IMFMediaType>pType = GetMediaType(pStreamDescriptor);
        THROW_ON_FAIL(MFCreateSampleGrabberSinkActivate(pType, sampleGrabber, &pSinkActivate));
        // To run as fast as possible, set this attribute (requires Windows 7):
        THROW_ON_FAIL(pSinkActivate->SetUINT32(MF_SAMPLEGRABBERSINK_IGNORE_CLOCK, TRUE));
        hr = output_node2->SetObject(pSinkActivate);
        THROW_ON_FAIL(hr);
   }
   
    THROW_ON_FAIL(hr);
    hr = m_pTopology->AddNode(output_node);
    THROW_ON_FAIL(hr);
    hr = m_pTopology->AddNode(output_node2);
    THROW_ON_FAIL(hr);
    // create the topology Tee node
    hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &pTeeNode);
    THROW_ON_FAIL(hr);
    // connect the first Tee node output to the network sink node
    if (m_Sink)
    {
        output_node = AddEncoderIfNeed(m_pTopology, pStreamDescriptor, output_node);
    }
    THROW_ON_FAIL(hr);
    hr = pTeeNode->ConnectOutput(0, output_node, 0);
    THROW_ON_FAIL(hr);
    hr = pTeeNode->ConnectOutput(1, output_node2, 0);
    THROW_ON_FAIL(hr);
    // if a renderer node was created and passed in, add it to the topology
    if (pRendererNode != NULL)
    {
        // add the renderer node to the topology
        hr = m_pTopology->AddNode(pRendererNode);
        THROW_ON_FAIL(hr);

        // connect the second Tee node output to the renderer sink node
        hr = pTeeNode->ConnectOutput(2, pRendererNode, 0);
        THROW_ON_FAIL(hr);
    }
    //hr = CreateTeeNetworkTwig(1, &networkNode);
    //THROW_ON_FAIL(hr);
    //networkNode = AddEncoderIfNeed(m_pTopology, pStreamDescriptor, networkNode);
   // hr = pTeeNode->ConnectOutput(3, networkNode, 0);
   // THROW_ON_FAIL(hr);
    //IMFMediaType* inner = GetMediaType(pStreamDescriptor);
    
    //IMFMediaType* out_type = ConvertToOutputType(inner);
    //hr = CreateAsfNode(out_type, &asfNode);
    THROW_ON_FAIL(hr);

    //asfNode = AddEncoderIfNeed(m_pTopology, pStreamDescriptor, asfNode);
    //hr = pTeeNode->ConnectOutput(3, asfNode, 0);
    THROW_ON_FAIL(hr);
    
    // detach the Tee node and return it as the output node
    *ppTeeNode = pTeeNode.Detach();
    return hr;
}


//
// If there is a network sink, create a Tee node and hook the network sink in parallel to
// the renderer sink in the topology, then return the Tee node.
//
HRESULT CTopoBuilder::CreateTeeNetworkTwig(DWORD stream, IMFTopologyNode** node)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopologyNode> pNetworkOutputNode;
    do
    {
        // if the network sink is not configured, just exit
        if(m_pNetworkSinkActivate == NULL)
            break;
      
        // create the output topology node for one of the streams on the network sink
        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNetworkOutputNode);
        BREAK_ON_FAIL(hr);

        // set the output stream ID on the stream sink topology node
        hr = pNetworkOutputNode->SetUINT32(MF_TOPONODE_STREAMID, stream);
        BREAK_ON_FAIL(hr);

        // associate the output network topology node with the network sink
        hr = pNetworkOutputNode->SetObject(m_pNetworkSinkActivate);
        BREAK_ON_FAIL(hr);

        // add the network output topology node to the topology
        m_pTopology->AddNode(pNetworkOutputNode);
        BREAK_ON_FAIL(hr);    

        *node = pNetworkOutputNode.Detach();
    }
    while(false);

    return hr;
}