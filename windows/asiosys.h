//============================================================================
// asiosys.h
//  Jamulus ASIO definitions for the ASIOSDK
//  Always include asiosys.h to include the ASIOSDK !
//  Never include iasiodrv.h or asio.h directly
//============================================================================

#ifndef __asiosys__
#define __asiosys__

#include "windows.h"

#define ASIO_LITTLE_ENDIAN 1
#define ASIO_CPU_X86       1

#define NATIVE_INT64    0
#define IEEE754_64FLOAT 1

#define DRVERR_OK                  0
#define DRVERR                     -5000
#define DRVERR_INVALID_PARAM       DRVERR - 1
#define DRVERR_DEVICE_NOT_FOUND    DRVERR - 2
#define DRVERR_DEVICE_NOT_OPEN     DRVERR - 3
#define DRVERR_DEVICE_ALREADY_OPEN DRVERR - 4
#define DRVERR_DEVICE_CANNOT_OPEN  DRVERR - 5

// Max string lengths, buffers should be LEN + 1 for string terminating zero.
#define MAXPATHLEN    511
#define MAXDRVNAMELEN 127
#define MAXERRMSGLEN  123

#define __ASIODRIVER_FWD_DEFINED__

#include "iasiodrv.h" // will also include asio.h

inline bool ASIOError_OK ( ASIOError error ) { return ( ( error == ASE_OK ) || ( error == ASE_SUCCESS ) ); }

#endif
