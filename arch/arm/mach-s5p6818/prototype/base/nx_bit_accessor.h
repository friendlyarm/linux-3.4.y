//------------------------------------------------------------------------------

//	   2012/07/23 Tony    first description
//------------------------------------------------------------------------------
#ifndef _NX_BIT_ACCESSOR_H_
#define _NX_BIT_ACCESSOR_H_

#include "nx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

U32 NX_BIT_SetBitRange32( U32 OldValue, U32 BitValue, U32 MSB, U32 LSB );
U32 NX_BIT_SetBit32( U32 OldValue, U32 BitValue, U32 BitNumber );
U16 NX_BIT_SetBitRange16( U16 OldValue, U32 BitValue, U32 MSB, U32 LSB );
U16 NX_BIT_SetBit16( U16 OldValue, U32 BitValue, U32 BitNumber );
U32 NX_BIT_GetBitRange32( U32 Value, U32 MSB, U32 LSB );
CBOOL NX_BIT_GetBit32( U32 Value, U32 BitNumber );
U16 NX_BIT_GetBitRange16( U16 Value, U32 MSB, U32 LSB );
CBOOL NX_BIT_GetBit16( U16 Value, U32 BitNumber );

#ifdef	__cplusplus
}
#endif

#endif  // _NX_BIT_ACCESSOR_H_
