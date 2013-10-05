//---------------------------------------------------------------------------------
//
//  Little Color Management System
//  Copyright (c) 1998-2011 Marti Maria Saguer
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//---------------------------------------------------------------------------------
//

#include "lcms2_extras.h"
#include "lcms2_internal.h"

// No gamut check, Cach�, 16 bits, 4 bytes
#define FUNCTION_NAME CachedXFORM4
#define CACHED
#define INBYTES 4
#include "cmsxform.h"

// No gamut check, Cach�, 16 bits, 8 bytes
#define FUNCTION_NAME CachedXFORM8
#define CACHED
#define INBYTES 8
#include "cmsxform.h"

// Special one for common case.
#define FUNCTION_NAME CachedXFORM3to1
#define CACHED
#define INBYTES 6
#define UNPACK(T,D,S,Z)                                   \
do {                                                      \
       (D)[0] = FROM_8_TO_16(*(S)); (S)++;     /* R */    \
       (D)[1] = FROM_8_TO_16(*(S)); (S)++;     /* G */    \
       (D)[2] = FROM_8_TO_16(*(S)); (S)++;     /* B */    \
} while (0)
#define PACK(T,S,D,Z)            \
do {                             \
    *(D)++ = FROM_16_TO_8(*(S)); \
} while (0);
#include "cmsxform.h"

static
cmsBool cmsExtrasTransformFactory(_cmsTransformFn     *xform,
                                  void               **UserData,
                                  _cmsFreeUserDataFn  *FreePrivateDataFn,
                                  cmsPipeline        **Lut,
                                  cmsUInt32Number     *InputFormat,
                                  cmsUInt32Number     *OutputFormat,
                                  cmsUInt32Number     *dwFlags)
{
    if ((*InputFormat & (cmsFLAGS_NULLTRANSFORM | cmsFLAGS_GAMUTCHECK | cmsFLAGS_NOCACHE)) == 0 &&
        (T_FLOAT(*InputFormat) == 0)) {
        if ((*InputFormat & ~COLORSPACE_SH(31)) == (CHANNELS_SH(3)|BYTES_SH(1)) &&
            (*OutputFormat & ~COLORSPACE_SH(31)) == (CHANNELS_SH(1)|BYTES_SH(1))) {
            *xform = CachedXFORM3to1;
            *dwFlags |= cmsFLAGS_CAN_CHANGE_FORMATTER;
	    return 1;
	} else {
            int inwords = T_CHANNELS(*InputFormat);
            if (inwords <= 2) {
                *xform = CachedXFORM4;
                *dwFlags |= cmsFLAGS_CAN_CHANGE_FORMATTER;
                return 1;
            } else if (inwords <= 4) {
                *xform = CachedXFORM8;
                *dwFlags |= cmsFLAGS_CAN_CHANGE_FORMATTER;
                return 1;
            }
        }
    }
    return 0;
}

static cmsPluginTransform extraTransforms =
{
    {
        cmsPluginMagicNumber,
        LCMS_VERSION,
        cmsPluginTransformSig,
        NULL,
    },
    &cmsExtrasTransformFactory
};

cmsBool cmsRegisterExtraTransforms(void)
{
    return cmsPlugin(&extraTransforms);
}
