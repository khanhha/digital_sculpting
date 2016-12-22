// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <GTEngineDEF.h>

namespace gte
{

// Types of primitives supported by index buffers.
enum GTE_IMPEXP IPType
{
    IP_NONE                     = 0xFFFFFFFF,
    IP_POLYPOINT                = 0x00000001,
    IP_POLYSEGMENT_DISJOINT     = 0x00000002,
    IP_POLYSEGMENT_CONTIGUOUS   = 0x00000004,
    IP_TRIMESH                  = 0x00000008,
    IP_TRISTRIP                 = 0x00000010,

    IP_HAS_POINTS    = IP_POLYPOINT,
    IP_HAS_SEGMENTS  = IP_POLYSEGMENT_DISJOINT | IP_POLYSEGMENT_CONTIGUOUS,
    IP_HAS_TRIANGLES = IP_TRIMESH | IP_TRISTRIP,

    IP_NUM_TYPES = 5 // IP_POLYPOINT through IP_TRISTRIP
};

}
