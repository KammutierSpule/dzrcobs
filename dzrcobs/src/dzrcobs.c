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
#include "dzrcobs/dzrcobs_dictionary.h"
#include "dzrcobs_assert.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////
eDZRCOBS_ret dzrcobs_encode_inc_plain( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );
eDZRCOBS_ret dzrcobs_encode_inc_dictionary( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );

#define DZRCOBS_PREVIOUS_CODE_BLOCK ( 0x00 )
#define DZRCOBS_PREVIOUS_CODE_DICTIONARY ( 0x01 )
#define DZRCOBS_PREVIOUS_CODE_ZERO ( 0x02 )

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

	if( ( ( aEncoding == DZRCOBS_USING_DICT_1 ) && ( aCtx->pDict[0] == NULL ) ) ||
			( ( aEncoding == DZRCOBS_USING_DICT_2 ) && ( aCtx->pDict[1] == NULL ) ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}

	aCtx->pDst		 = aDstBuf;
	aCtx->pCurDst	 = aDstBuf;
	aCtx->pDstEnd	 = aDstBuf + aDstBufSize;
	aCtx->code		 = 1;
	aCtx->crc			 = DZRCOBS_CRC_INIT_VAL;
	aCtx->encoding = aEncoding;

	aCtx->previousCode = DZRCOBS_PREVIOUS_CODE_ZERO;
	aCtx->pendingMask	 = DZRCOBS_NEXTCODE_IS_ZERO;

	aCtx->isFirstByteInTheBuffer = true;

	DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter = 0 );

	switch( aEncoding )
	{
	case DZRCOBS_PLAIN:
		aCtx->encFunc = dzrcobs_encode_inc_plain;
		break;

	case DZRCOBS_USING_DICT_1:
	case DZRCOBS_USING_DICT_2:
		aCtx->encFunc = dzrcobs_encode_inc_dictionary;
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

	// Add last tracked zero code
	if( ( aCtx->encoding == DZRCOBS_PLAIN ) || ( aCtx->previousCode != DZRCOBS_PREVIOUS_CODE_DICTIONARY ) )
	{
		DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

		const uint8_t curCode = ( aCtx->code == 1 ) ? 0x01 : ( aCtx->code | aCtx->pendingMask );

		aCtx->crc = DZRCOBS_CRC( aCtx->crc, curCode );

		*aCtx->pCurDst++ = curCode;
	}

	// Add (tail) header info
	const uint8_t encodingByte = (uint8_t)( aCtx->user6bits << 2 ) | ( (uint8_t)aCtx->encoding & 0x03 );

	DZRCOBS_ASSERT( encodingByte != 0 );

	aCtx->crc = DZRCOBS_CRC( aCtx->crc, encodingByte );

	DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );
	*aCtx->pCurDst++ = encodingByte;

	const uint8_t finalCrc = aCtx->crc;

	DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );
	*aCtx->pCurDst++ = ( finalCrc == 0x00 ) ? DZRCOBS_CRC_VALUE_WHEN_CRC_IS_ZERO : finalCrc; // Avoid zero ending CRC.

	// Calc encoded size
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

	if( ( aCtx->pCurDst + DZRCOBS_FRAME_HEADER_SIZE + maxEncodedSize ) > aCtx->pDstEnd )
	{
		return DZRCOBS_RET_ERR_OVERFLOW;
	}

	return aCtx->encFunc( aCtx, aSrcBuf, aSrcBufSize );
}

eDZRCOBS_ret dzrcobs_encode_inc_plain( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	DZRCOBS_ASSERT( aCtx != NULL );
	DZRCOBS_ASSERT( aSrcBuf != NULL );
	DZRCOBS_ASSERT( aSrcBufSize > 0 );

	uint8_t *curDst = aCtx->pCurDst;

#ifdef ASAP_IS_DEBUG_BUILD
	size_t srcReadCounter = 0;
#endif

	uint8_t curCode = aCtx->code;

	while( aSrcBufSize )
	{
		aSrcBufSize--;

		DZRCOBS_RUN_ONDEBUG( srcReadCounter++ );

		const uint8_t byte = *aSrcBuf++;

		if( byte == 0 )
		{
			if( !aCtx->isFirstByteInTheBuffer )
			{
				DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

				aCtx->crc = DZRCOBS_CRC( aCtx->crc, curCode );
				*curDst++ = curCode;
			}
			else
			{
				aCtx->isFirstByteInTheBuffer = false;
			}

			curCode = 1;
		}
		else
		{
			DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

			aCtx->isFirstByteInTheBuffer = false;

			aCtx->crc = DZRCOBS_CRC( aCtx->crc, byte );
			*curDst++ = byte;
			curCode++;

			if( curCode == DZRCOBS_CODE_JUMP_PLAIN )
			{
				DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

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

eDZRCOBS_ret dzrcobs_encode_inc_dictionary( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	DZRCOBS_ASSERT( aCtx != NULL );
	DZRCOBS_ASSERT( aSrcBuf != NULL );
	DZRCOBS_ASSERT( aSrcBufSize > 0 );
	DZRCOBS_ASSERT( ( aCtx->encoding == DZRCOBS_USING_DICT_1 ) || ( aCtx->encoding == DZRCOBS_USING_DICT_2 ) );

	uint8_t *curDst = aCtx->pCurDst;

	uint8_t curCode = aCtx->code;

	const sDICT_ctx *pDict = aCtx->pDict[aCtx->encoding - DZRCOBS_USING_DICT_1];

	bool previously_found_a_dictionary = false;

	while( aSrcBufSize )
	{
		size_t keySizeFound = 0;

		uint8_t foundIdx = DZRCOBS_Dictionary_Search( pDict, aSrcBuf, aSrcBufSize, &keySizeFound );

		if( foundIdx )
		{
			DZRCOBS_ASSERT( keySizeFound > 0 );
			DZRCOBS_ASSERT( keySizeFound <= aSrcBufSize );

			if( !previously_found_a_dictionary )
			{
				if( !aCtx->isFirstByteInTheBuffer )
				{
					DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

					curCode = ( curCode == 1 ) ? 0x01 : ( curCode | aCtx->pendingMask );

					aCtx->crc = DZRCOBS_CRC( aCtx->crc, curCode );

					*curDst++ = curCode;
				}

				curCode = 1;
			}

			aCtx->previousCode = DZRCOBS_PREVIOUS_CODE_DICTIONARY;
			aCtx->pendingMask	 = DZRCOBS_NEXTCODE_IS_DICTIONARY;

			previously_found_a_dictionary = true;

			DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

			foundIdx -= 1; // remove base index
			const uint8_t dictEntry = DZRCOBS_DICTIONARY_BITMASK | foundIdx;

			aCtx->crc = DZRCOBS_CRC( aCtx->crc, dictEntry );

			aCtx->isFirstByteInTheBuffer = false;

			*curDst++ = dictEntry;

			// advance keyword
			aSrcBufSize -= keySizeFound;
			aSrcBuf += keySizeFound;

			continue;
		}

		// Continue with regular plain encoding
		aSrcBufSize--;

		const uint8_t byte = *aSrcBuf++;

		if( byte == 0 )
		{
			if( aCtx->previousCode != DZRCOBS_PREVIOUS_CODE_DICTIONARY )
			{
				if( !aCtx->isFirstByteInTheBuffer )
				{
					DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

					curCode = ( curCode == 1 ) ? 0x01 : ( curCode | aCtx->pendingMask );

					aCtx->crc = DZRCOBS_CRC( aCtx->crc, curCode );

					*curDst++ = curCode;

					aCtx->pendingMask = DZRCOBS_NEXTCODE_IS_ZERO;
				}

				aCtx->isFirstByteInTheBuffer = false;
			}
			previously_found_a_dictionary = false;

			curCode = 1;

			aCtx->previousCode = DZRCOBS_PREVIOUS_CODE_ZERO;
			// aCtx->pendingMask	 = DZRCOBS_NEXTCODE_IS_ZERO;
		}
		else
		{
			if( aCtx->previousCode == DZRCOBS_PREVIOUS_CODE_ZERO )
			{
				aCtx->pendingMask = DZRCOBS_NEXTCODE_IS_ZERO;
			}
			else
			{
				if( aCtx->previousCode == DZRCOBS_PREVIOUS_CODE_DICTIONARY )
				{
					aCtx->pendingMask = DZRCOBS_NEXTCODE_IS_DICTIONARY;
				}
			}

			previously_found_a_dictionary = false;

			DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

			aCtx->crc										 = DZRCOBS_CRC( aCtx->crc, byte );
			aCtx->isFirstByteInTheBuffer = false;
			aCtx->previousCode					 = DZRCOBS_PREVIOUS_CODE_BLOCK;

			*curDst++ = byte;
			curCode++;

			if( curCode == DZRCOBS_CODE_JUMP )
			{
				DZRCOBS_RUN_ONDEBUG( aCtx->writeCounter++ );

				aCtx->crc = DZRCOBS_CRC( aCtx->crc, curCode );

				aCtx->isFirstByteInTheBuffer = false;

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
