#pragma once
#include "TopoBuilder.h"
#include "mftransform.h"
#include <MMSystem.h>
#include "SampleTransform.h"
#include <shlwapi.h>
#include <Wmcodecdsp.h>

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
            hr = copyTypeParameters(in_media_type, inputType);
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
            hr = copyTypeParameters(in_media_type, outputType);
           
            THROW_ON_FAIL(hr);
            THROW_ON_FAIL(hr);
            hr = transform->SetOutputType(0, outputType, 0);
            THROW_ON_FAIL(hr);
            break;
        }
    }
    return hr;
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
        OutputDebugStringW(L"error");
    }
    THROW_ON_FAIL(hr);
    if (FAILED(inputHr)){
        hr = negotiateInputType(pEncoder, in_media_type);
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

IMFTopologyNode* AddEncoderIfNeed(IMFTopology * topology, IMFStreamDescriptor * pStreamDescriptor, IMFTopologyNode * output_node)
{
    CComPtr<IMFTopologyNode> transformNode;
    CComPtr<IMFTopologyNode> colorConverterNode;
    CComPtr<IMFTopologyNode> sampleTransformNode;
    HRESULT hr = S_OK;
    IMFMediaType * mediaType = GetMediaType(pStreamDescriptor);
    CComPtr<IMFTransform> sampleTransform;
    sampleTransform = CreateSampleTransform();
    hr = AddTransformNode(topology, sampleTransform, output_node, &sampleTransformNode);
    THROW_ON_FAIL(hr);
    GUID minorType = GetVideoSubtype(mediaType);
    if (minorType != MFVideoFormat_H264) {
        DetectSubtype(minorType);
        CComPtr<IMFTransform> color;
        CComPtr<IMFTransform> transform;
        color = CreateColorConverterMFT();
        transform = CreateVideoEncoderMFT(mediaType, MFVideoFormat_H264);     
        
        hr = AddTransformNode(topology, transform, sampleTransformNode, &transformNode);
        THROW_ON_FAIL(hr);
        /*hr = AddTransformNode(topology, color, transformNode, &colorConverterNode);
        THROW_ON_FAIL(hr);*/
        return transformNode.Detach();
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
        //hr = CreateNetworkSink(8080);
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
//
// Create a network sink that will listen for requests on the specified port.
//
HRESULT CTopoBuilder::CreateNetworkSink(DWORD requestPort)
{
    HRESULT hr = S_OK;
    CComPtr<IMFPresentationDescriptor> pPresDescriptor;
    CComPtr<IMFASFProfile> pAsfProfile;
    CComQIPtr<IMFASFContentInfo> pAsfContentInfo;
    
    CComPtr<IMFActivate> pByteStreamActivate;
    CComPtr<IMFActivate> pNetSinkActivate;
    THROW_ON_NULL(m_pSource);

    // create an HTTP activator for the custom HTTP output byte stream object
    pByteStreamActivate = new (std::nothrow) CHttpOutputStreamActivate(requestPort);
    THROW_ON_NULL(pByteStreamActivate);
        
    // create the presentation descriptor for the source
    hr = m_pSource->CreatePresentationDescriptor(&pPresDescriptor);
    THROW_ON_FAIL(hr);

    // create the ASF profile from the presentation descriptor
    hr = MFCreateASFProfileFromPresentationDescriptor(pPresDescriptor, &pAsfProfile);
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
    
    CComPtr<IMFMediaType> out_mf_media_type;
    CComPtr<IMFByteStream> byte_stream;
    CComPtr<IMFStreamSink> stream_sink;
    hr = MFCreateFile(
        MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE,
        filePath, &byte_stream);
    THROW_ON_FAIL(hr);
    hr = MFCreateMediaType(&out_mf_media_type);
    THROW_ON_FAIL(hr);
    hr = out_mf_media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    THROW_ON_FAIL(hr);
    hr = out_mf_media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    THROW_ON_FAIL(hr);
    hr = copyTypeParameters(in_mf_media_type, out_mf_media_type);
    THROW_ON_FAIL(hr);
    hr = MFCreateMPEG4MediaSink(byte_stream, out_mf_media_type, NULL, &m_Sink);
       
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
        &pSource                    // Receives a pointer to the media source.
        );
    THROW_ON_FAIL(hr);
    
    // Get the IMFMediaSource interface from the media source.
    
    m_pSource = pSource;

    
   // BREAK_ON_NULL(m_pSource, E_NOINTERFACE);

    return hr;
}

HRESULT CTopoBuilder::CreateMediaSource()
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
    if (m_pSource) {
        hr = m_pSource->Shutdown();
        THROW_ON_FAIL(hr);
        m_pSource.Release();
    }
    if (m_Sink) {
        hr = m_Sink->Shutdown();
        THROW_ON_FAIL(hr);
        m_Sink.Release();
        m_Sink = NULL;
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

    if(toFile && 
        majorType == MFMediaType_Video)
    {
        THROW_ON_FAIL(hr);
        hr = CreateFileSink(L"C:\\Users\\Public\\Encoded2.mp4", in_media_type);
        THROW_ON_FAIL(hr);
        CComPtr<IMFTopologyNode> pOldOutput = pOutputNode;
        pOutputNode = NULL;
        hr = CreateTeeMp4Twig(pPresDescriptor, pStreamDescriptor, pOldOutput, &pOutputNode);
        THROW_ON_FAIL(hr);
    }

    *ppOutputNode = pOutputNode.Detach();

    return hr;
}

HRESULT CTopoBuilder::CreateTeeMp4Twig(IMFPresentationDescriptor* pPresDescriptor, IMFStreamDescriptor* pStreamDescriptor,
    IMFTopologyNode* pRendererNode, IMFTopologyNode** ppTeeNode)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopologyNode> output_node;
    CComPtr<IMFTopologyNode> encoder_node;
    CComPtr<IMFTopologyNode> pTeeNode;
    CComPtr<IMFStreamSink> stream_sink;

    DWORD sink_count;
    
    hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &output_node);
    THROW_ON_FAIL(hr);
    hr = m_Sink->GetStreamSinkCount(&sink_count);
    THROW_ON_FAIL(hr);
    ATLASSERT(sink_count == 1);
    hr = m_Sink->GetStreamSinkByIndex(0, &stream_sink);
    THROW_ON_FAIL(hr);
    hr = output_node->SetObject(stream_sink);
    THROW_ON_FAIL(hr);
    hr = m_pTopology->AddNode(output_node);
    THROW_ON_FAIL(hr);
    // create the topology Tee node
    hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &pTeeNode);
    THROW_ON_FAIL(hr);
    // connect the first Tee node output to the network sink node
    output_node = AddEncoderIfNeed(m_pTopology, pStreamDescriptor, output_node);
    THROW_ON_FAIL(hr);
    hr = pTeeNode->ConnectOutput(0, output_node, 0);
    THROW_ON_FAIL(hr);

    // if a renderer node was created and passed in, add it to the topology
    if (pRendererNode != NULL)
    {
        // add the renderer node to the topology
        hr = m_pTopology->AddNode(pRendererNode);
        THROW_ON_FAIL(hr);

        // connect the second Tee node output to the renderer sink node
        hr = pTeeNode->ConnectOutput(1, pRendererNode, 0);
        THROW_ON_FAIL(hr);
    }

    // detach the Tee node and return it as the output node
    *ppTeeNode = pTeeNode.Detach();
    return hr;
}


//
// If there is a network sink, create a Tee node and hook the network sink in parallel to
// the renderer sink in the topology, then return the Tee node.
//
HRESULT CTopoBuilder::CreateTeeNetworkTwig(IMFStreamDescriptor* pStreamDescriptor, 
    IMFTopologyNode* pRendererNode, IMFTopologyNode** ppTeeNode)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopologyNode> pNetworkOutputNode;
    CComPtr<IMFTopologyNode> pTeeNode;
    DWORD streamId = 0;

    do
    {
        BREAK_ON_NULL(ppTeeNode, E_POINTER);

        // if the network sink is not configured, just exit
        if(m_pNetworkSinkActivate == NULL)
            break;

        // get the stream ID
        hr = pStreamDescriptor->GetStreamIdentifier(&streamId);
        BREAK_ON_FAIL(hr);

        // create the output topology node for one of the streams on the network sink
        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNetworkOutputNode);
        BREAK_ON_FAIL(hr);

        // set the output stream ID on the stream sink topology node
        hr = pNetworkOutputNode->SetUINT32(MF_TOPONODE_STREAMID, streamId);
        BREAK_ON_FAIL(hr);

        // associate the output network topology node with the network sink
        hr = pNetworkOutputNode->SetObject(m_pNetworkSinkActivate);
        BREAK_ON_FAIL(hr);

        // add the network output topology node to the topology
        m_pTopology->AddNode(pNetworkOutputNode);
        BREAK_ON_FAIL(hr);
        
        
        // create the topology Tee node
        hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &pTeeNode);
        BREAK_ON_FAIL(hr);

        // connect the first Tee node output to the network sink node
        hr = pTeeNode->ConnectOutput(0, pNetworkOutputNode, 0);
        BREAK_ON_FAIL(hr);

        // if a renderer node was created and passed in, add it to the topology
        if(pRendererNode != NULL)
        {
            // add the renderer node to the topology
            hr = m_pTopology->AddNode(pRendererNode);
            BREAK_ON_FAIL(hr);

            // connect the second Tee node output to the renderer sink node
            hr = pTeeNode->ConnectOutput(1, pRendererNode, 0);
            BREAK_ON_FAIL(hr);
        }

        // detach the Tee node and return it as the output node
        *ppTeeNode = pTeeNode.Detach();
    }
    while(false);

    return hr;
}