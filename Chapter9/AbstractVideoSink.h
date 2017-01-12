#pragma once
#include "Common.h"
class AbstractVideoSink {
public:    
    virtual IMFTopologyNode * GetBranch(IMFTopology * topology, IMFMediaType * in_type, GUID output_type, GUID out_subtype) = 0;
    virtual HRESULT Start() = 0;
    virtual HRESULT Shutdown() = 0;

   
protected:

    
};

