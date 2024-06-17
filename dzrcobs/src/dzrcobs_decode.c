// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs_decode.cpp
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
#include <dzrcobs/dzrcobs_decode.h>
#include <stdbool.h>
#include "crc8.h"
#include "dzrcobs/dzrcobs.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////
#define DZRCOBS_CODE_JUMP ( 0x7F )

// Implementation
// /////////////////////////////////////////////////////////////////////////////
eDZRCOBS_ret dzrcobs_decode( const sDZRCOBS_decodectx *aDecodeCtx,
														 size_t *aOutDecodedLen,
														 uint8_t **aOutDecodedStartPos,
														 uint8_t *aOutUser6bitDataRightAlgn )
{
	if( ( !aDecodeCtx ) || ( !aDecodeCtx->srcBufEncoded ) || ( !aDecodeCtx->dstBufDecoded ) || ( !aOutDecodedLen ) ||
			( !aOutDecodedStartPos ) || ( aDecodeCtx->dstBufDecodedSize == 0 ) || ( aDecodeCtx->srcBufEncodedLen < 4 ) )
	{
		return DZRCOBS_RET_ERR_BAD_ARG;
	}

	const uint8_t *pBeginEncoded = aDecodeCtx->srcBufEncoded;
	const uint8_t *pReadEncoded	 = aDecodeCtx->srcBufEncoded + aDecodeCtx->srcBufEncodedLen - 1;

	const uint8_t *pBeginDecoded	= aDecodeCtx->dstBufDecoded;
	uint8_t *pWriteDecodedInitial = aDecodeCtx->dstBufDecoded + aDecodeCtx->dstBufDecodedSize;
	uint8_t *pWriteDecoded				= pWriteDecodedInitial; // starts out of buffer, will be decremented latter

#ifdef ASAP_IS_DEBUG_BUILD
	size_t totalWrite = 0;
	size_t totalRead	= 0;
#endif

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

	const uint8_t receivedUserEncoding = *pReadEncoded--;

	if( receivedUserEncoding == 0 )
	{
		return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
	}

	const eDZRCOBS_encoding encoding = (eDZRCOBS_encoding)( receivedUserEncoding & 0x03 );

	if( encoding == DZRCOBS_RESERVED )
	{
		return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
	}

	while( pReadEncoded >= pBeginEncoded )
	{
		uint8_t code = *pReadEncoded;

		if( code == 0 )
		{
			return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
		}

		if( ( code != DZRCOBS_CODE_JUMP ) &&					// Only adds if the new code is not skipping
				( pWriteDecoded != pWriteDecodedInitial ) // Only adds if this is not the first run
		)
		{
			pWriteDecoded--;
			*pWriteDecoded = 0;

#ifdef ASAP_IS_DEBUG_BUILD
			totalWrite++;
#endif
		}

		code--;
		if( ( pWriteDecoded - code ) < pBeginDecoded )
		{
			return DZRCOBS_RET_ERR_OVERFLOW;
		}

		pReadEncoded--;

#ifdef ASAP_IS_DEBUG_BUILD
		totalRead++;
#endif

		while( code )
		{
			code--;

			const uint8_t byte = *pReadEncoded--;

#ifdef ASAP_IS_DEBUG_BUILD
			totalRead++;
#endif

			if( byte == 0 )
			{
				return DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
			}

			pWriteDecoded--;
			*pWriteDecoded = byte;

#ifdef ASAP_IS_DEBUG_BUILD
			totalWrite++;
#endif
		}
	}

	*aOutDecodedStartPos = pWriteDecoded;

	*aOutDecodedLen = aDecodeCtx->dstBufDecodedSize - (size_t)( pWriteDecoded - aDecodeCtx->dstBufDecoded );

	*aOutUser6bitDataRightAlgn = ( receivedUserEncoding >> 2 ) & 0x3F;

	return DZRCOBS_RET_SUCCESS;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
