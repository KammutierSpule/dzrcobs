// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs_decode.cpp
///	@brief
///
///	@par  Plataform Target:	Any
/// @par  Tab Size: 2
///
/// @copyright (C) 2025 Mario Luzeiro All rights reserved.
/// @author Mario Luzeiro <mluzeiro@ua.pt>
///
/// @par  License: Distributed under the 3-Clause BSD License. See accompanying
/// file LICENSE or a copy at https://opensource.org/licenses/BSD-3-Clause
/// SPDX-License-Identifier: BSD-3-Clause
///
// /////////////////////////////////////////////////////////////////////////////

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <dzrcobs/dzrcobs_decode.h>
#include <stdbool.h>
#include "crc8.h"
#include "dzrcobs/dzrcobs.h"
#include "dzrcobs_assert.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////

// Implementation
// /////////////////////////////////////////////////////////////////////////////
eDZRCOBS_ret dzrcobs_decode( const sDZRCOBS_decodectx *aDecodeCtx,
														 size_t *aOutDecodedLen,
														 uint8_t **aOutDecodedStartPos,
														 uint8_t *aOutUser6bitDataRightAlgn )
{
	if( ( !aDecodeCtx ) || ( !aDecodeCtx->srcBufEncoded ) || ( !aDecodeCtx->dstBufDecoded ) || ( !aOutDecodedLen ) ||
			( !aOutDecodedStartPos ) || ( aDecodeCtx->dstBufDecodedSize == 0 ) || ( aDecodeCtx->srcBufEncodedLen < 3 ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}

	const uint8_t *pBeginEncoded = aDecodeCtx->srcBufEncoded;
	const uint8_t *pReadEncoded	 = aDecodeCtx->srcBufEncoded + aDecodeCtx->srcBufEncodedLen - 1;

	const uint8_t *pBeginDecoded	= aDecodeCtx->dstBufDecoded;
	uint8_t *pWriteDecodedInitial = aDecodeCtx->dstBufDecoded + aDecodeCtx->dstBufDecodedSize;
	uint8_t *pWriteDecoded				= pWriteDecodedInitial; // starts out of buffer, will be decremented latter

#ifdef IS_DEBUG_BUILD
	size_t totalWrite = 0;
	size_t totalRead	= 0;
#endif

	DZRCOBS_RUN_ONDEBUG( totalRead++ );

	const uint8_t receivedCRC8 = *pReadEncoded--;

	if( receivedCRC8 == 0 )
	{
		return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
	}

	uint8_t crc = DZRCOBS_CRC_INIT_VAL;

	const uint8_t *pData		= pBeginEncoded;
	const uint8_t *pDataEnd = aDecodeCtx->srcBufEncoded + aDecodeCtx->srcBufEncodedLen - 1; // -1 removed CRC

	while( pData < pDataEnd )
	{
		crc = DZRCOBS_CRC( crc, *pData++ );
	}

	if( ( ( crc != 0 ) && ( crc != receivedCRC8 ) ) ||
			( ( crc == 0 ) && ( receivedCRC8 != DZRCOBS_CRC_VALUE_WHEN_CRC_IS_ZERO ) ) )
	{
		return DZRCOBS_RET_ERR_CRC;
	}

	DZRCOBS_RUN_ONDEBUG( totalRead++ );

	const uint8_t receivedUserEncoding = *pReadEncoded--;

	if( receivedUserEncoding == 0 )
	{
		return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
	}

	// Get and validate encoding type
	const eDZRCOBS_encoding encoding = (eDZRCOBS_encoding)( receivedUserEncoding & 0x03 );

	const sDICT_ctx *pDict	= NULL;
	uint8_t jumpCodeBitmask = 0;

	switch( encoding )
	{
	case DZRCOBS_PLAIN:
		jumpCodeBitmask = DZRCOBS_CODE_JUMP_PLAIN;
		break;
	case DZRCOBS_USING_DICT_1:
	case DZRCOBS_USING_DICT_2:
		jumpCodeBitmask = DZRCOBS_CODE_JUMP;

		pDict = aDecodeCtx->pDict[encoding - DZRCOBS_USING_DICT_1];
		if( pDict == NULL )
		{
			return DZRCOBS_RET_ERR_NO_DICTIONARY_TO_DECODE;
		}
		break;

	case DZRCOBS_RESERVED:
	default:
		return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
		break;
	}

	bool is_end_of_code_a_zero = false;

	while( pReadEncoded >= pBeginEncoded )
	{
		DZRCOBS_RUN_ONDEBUG( totalRead++ );
		uint8_t code = *pReadEncoded--;

		if( ( encoding == DZRCOBS_PLAIN ) || ( code < DZRCOBS_DICTIONARY_BITMASK ) )
		{
			const bool is_code_jump_delimiter = ( ( code & jumpCodeBitmask ) == jumpCodeBitmask );

			if( !is_code_jump_delimiter )
			{
				is_end_of_code_a_zero = ( encoding == DZRCOBS_PLAIN ) ? true : ( ( code & DZRCOBS_NEXTCODE_BITMASK ) == 0 );
			}

			code &= jumpCodeBitmask;

			if( code == 0 )
			{
				return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
			}

			code--;
			if( ( pWriteDecoded - code ) < pBeginDecoded )
			{
				return DZRCOBS_RET_ERR_OVERFLOW;
			}

			if( code == 0 )
			{
				const bool is_read_data_remain = ( pReadEncoded >= pBeginEncoded );
				if( !is_read_data_remain )
				{
					break;
				}

				DZRCOBS_RUN_ONDEBUG( totalWrite++ );

				pWriteDecoded--;
				*pWriteDecoded = 0;
			}
			else
			{
				while( code )
				{
					code--;

					const uint8_t byte = *pReadEncoded--;
					DZRCOBS_RUN_ONDEBUG( totalRead++ );

					if( byte == 0 )
					{
						return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
					}

					pWriteDecoded--;
					*pWriteDecoded = byte;

					DZRCOBS_RUN_ONDEBUG( totalWrite++ );
				}

				if( is_end_of_code_a_zero )
				{
					const bool is_still_data_to_read = ( pReadEncoded >= pBeginEncoded );

					if( is_still_data_to_read )
					{
						const bool is_next_byte_a_jump_code = *pReadEncoded == jumpCodeBitmask;
						if( !is_next_byte_a_jump_code )
						{
							if( ( pWriteDecoded - 1 ) < pBeginDecoded )
							{
								return DZRCOBS_RET_ERR_OVERFLOW;
							}

							pWriteDecoded--;
							*pWriteDecoded = 0;
							DZRCOBS_RUN_ONDEBUG( totalWrite++ );
						}
					}
				}
			}
		}
		else
		{
			const uint8_t dictIdx = ( code & ~DZRCOBS_DICTIONARY_BITMASK );

			uint8_t wordSize = 0;

			const uint8_t *word = dzrcobs_dictionary_get( pDict, dictIdx, &wordSize );

			if( word == NULL )
			{
				return DZRCOBS_RET_ERR_WORD_NOT_FOUND_ON_DICTIONARY;
			}

			if( ( pWriteDecoded - wordSize ) < pBeginDecoded )
			{
				return DZRCOBS_RET_ERR_OVERFLOW;
			}

			const uint8_t *wordEnd = word + wordSize;
			wordEnd--;

			while( wordSize )
			{
				wordSize--;
				DZRCOBS_RUN_ONDEBUG( totalWrite++ );

				pWriteDecoded--;
				*pWriteDecoded = *wordEnd--;
			}
		}
	}

	*aOutDecodedStartPos = pWriteDecoded;

	*aOutDecodedLen = aDecodeCtx->dstBufDecodedSize - (size_t)( pWriteDecoded - aDecodeCtx->dstBufDecoded );

	*aOutUser6bitDataRightAlgn = ( receivedUserEncoding >> 2 ) & 0x3F;

	return DZRCOBS_RET_SUCCESS;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
