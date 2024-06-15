// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs.cpp
///	@brief
///
///	@par  Plataform Target:	Any
/// @par  Tab Size: 2
///
/// @copyright (C) 2024 Mario Luzeiro All rights reserved.
/// @author Mario Luzeiro <mluzeiro@ua.pt>
///
/// @par  License: Distributed under the 3-Clause BSD License. See accompanying
/// file LICENSE or a copy at https://opensource.org/licenses/BSD-3-Clause
/// SPDX-License-Identifier: BSD-3-Clause
///
// /////////////////////////////////////////////////////////////////////////////

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <dzrcobs/dzrcobs.h>
#include <stdbool.h>
#include "crc8.h"
#include "dzrcobs_assert.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////
#define DZRCOBS_CODE_JUMP ( 0x7F )

eDZRCOBS_ret dzrcobs_encode_inc_plain( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );
eDZRCOBS_ret dzrcobs_encode_inc_dictionary( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );

// Implementation
// /////////////////////////////////////////////////////////////////////////////

eDZRCOBS_ret dzrcobs_encode_set_dictionary( sDZRCOBS_ctx *aCtx,
																						const sDICT_ctx *aDictCtx,
																						eDZRCOBS_encoding aDictEncoding )
{
	if( ( !aCtx ) || ( !aDictCtx ) ||
			( !( ( aDictEncoding == DZRCOBS_USING_DICT_1 ) || ( aDictEncoding == DZRCOBS_USING_DICT_2 ) ) ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}

	const uint8_t idx = (uint8_t)( aDictEncoding - DZRCOBS_USING_DICT_1 );
	aCtx->pDict[idx]	= aDictCtx;

	return DZRCOBS_RET_SUCCESS;
}

eDZRCOBS_ret dzrcobs_encode_inc_begin( sDZRCOBS_ctx *aCtx,
																			 eDZRCOBS_encoding aEncoding,
																			 uint8_t *aDstBuf,
																			 size_t aDstBufSize )
{
	if( ( !aCtx ) || ( !aDstBuf ) || ( aDstBufSize < 2 ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}

#if DZRCOBS_USE_DICT == 1
	if( ( ( aEncoding == DZRCOBS_USING_DICT_1 ) && ( aCtx->pDict[0] == NULL ) ) ||
			( ( aEncoding == DZRCOBS_USING_DICT_2 ) && ( aCtx->pDict[1] == NULL ) ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}
#endif

	aCtx->pDst		 = aDstBuf;
	aCtx->pCurDst	 = aDstBuf;
	aCtx->pDstEnd	 = aDstBuf + aDstBufSize;
	aCtx->code		 = 1;
	aCtx->crc			 = DZRCOBS_CRC_INIT_VAL;
	aCtx->encoding = aEncoding;

#ifdef ASAP_IS_DEBUG_BUILD
	aCtx->writeCounter = 0;
#endif

	switch( aEncoding )
	{
	case DZRCOBS_PLAIN:
		aCtx->encFunc = dzrcobs_encode_inc_plain;
		break;

	case DZRCOBS_USING_DICT_1:
	case DZRCOBS_USING_DICT_2:
		// aCtx->encFunc = dzrcobs_encode_inc_dictionary;
		break;
	case DZRCOBS_RESERVED:
	default:
		aCtx->encFunc = NULL;
		break;
	}

	DZRCOBS_ASSERT( aCtx->encFunc != NULL );

	return DZRCOBS_RET_SUCCESS;
}

eDZRCOBS_ret dzrcobs_encode_inc_end( sDZRCOBS_ctx *aCtx, size_t *aOutSizeEncoded )
{
	if( ( !aCtx ) || ( !aOutSizeEncoded ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}

	if( ( aCtx->pCurDst + 1 ) > aCtx->pDstEnd )
	{
		return DZRCOBS_RET_ERR_OVERFLOW;
	}

#ifdef ASAP_IS_DEBUG_BUILD
	aCtx->writeCounter += 3;
#endif

	aCtx->crc = DZRCOBS_CRC( aCtx->crc, aCtx->code );

	*aCtx->pCurDst++ = aCtx->code;

	const uint8_t encodingByte = (uint8_t)( aCtx->user6bits << 2 ) | ( (uint8_t)aCtx->encoding & 0x03 );

	DZRCOBS_ASSERT( encodingByte != 0 );

	aCtx->crc = DZRCOBS_CRC( aCtx->crc, encodingByte );

	*aCtx->pCurDst++ = encodingByte;

	const uint8_t finalCrc = aCtx->crc;

	*aCtx->pCurDst++ = ( finalCrc == 0x00 ) ? 0xFF : finalCrc; // Avoid zero ending CRC.

	*aOutSizeEncoded = (size_t)( aCtx->pCurDst - aCtx->pDst );

	aCtx->encFunc = NULL;

	return DZRCOBS_RET_SUCCESS;
}

eDZRCOBS_ret dzrcobs_encode_inc( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	if( ( !aCtx ) || ( !aSrcBuf ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}

	if( aCtx->encFunc == NULL )
	{
		return DZRCOBS_RET_ERR_NOTINITIALIZED;
	}

	if( aSrcBufSize == 0 )
	{
		return DZRCOBS_RET_SUCCESS;
	}

	const size_t maxEncodedSize = DZRCOBS_MAX_ENCODED_SIZE( aSrcBufSize );

	if( ( aCtx->pCurDst + 2 + maxEncodedSize ) > aCtx->pDstEnd )
	{
		return DZRCOBS_RET_ERR_OVERFLOW;
	}

	return aCtx->encFunc( aCtx, aSrcBuf, aSrcBufSize );
}

eDZRCOBS_ret dzrcobs_encode_inc_plain( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	uint8_t *curDst = aCtx->pCurDst;

#ifdef ASAP_IS_DEBUG_BUILD
	size_t srcReadCounter = 0;
#endif

	uint8_t curCode = aCtx->code;

	while( aSrcBufSize )
	{
		aSrcBufSize--;

#ifdef ASAP_IS_DEBUG_BUILD
		srcReadCounter++;
#endif

		const uint8_t byte = *aSrcBuf++;

		if( byte == 0 )
		{
#ifdef ASAP_IS_DEBUG_BUILD
			aCtx->writeCounter++;
#endif
			aCtx->crc = DZRCOBS_CRC( aCtx->crc, curCode );

			*curDst++ = curCode;
			curCode		= 1;
		}
		else
		{
#ifdef ASAP_IS_DEBUG_BUILD
			aCtx->writeCounter++;
#endif

			aCtx->crc = DZRCOBS_CRC( aCtx->crc, byte );
			*curDst++ = byte;
			curCode++;

			if( curCode == DZRCOBS_CODE_JUMP )
			{
#ifdef ASAP_IS_DEBUG_BUILD
				aCtx->writeCounter++;
#endif

				aCtx->crc = DZRCOBS_CRC( aCtx->crc, curCode );
				*curDst++ = curCode;
				curCode		= 1;
			}
		}
	}

	aCtx->code		= curCode;
	aCtx->pCurDst = curDst;

	return DZRCOBS_RET_SUCCESS;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
