// /////////////////////////////////////////////////////////////////////////////
///	@file rcobs.cpp
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
#include <dzrcobs/rcobs.h>
#include <stdbool.h>
#include <stdio.h>

// Implementation
// /////////////////////////////////////////////////////////////////////////////

eRCOBS_ret rcobs_encode_inc_begin( sRCOBS_ctx *aCtx, uint8_t *aDstBuf, size_t aDstBufSize )
{
	if( ( !aCtx ) || ( !aDstBuf ) || ( aDstBufSize < 2 ) )
	{
		return RCOBS_RET_ERR_BAD_ARG;
	}

	aCtx->pDst		= aDstBuf;
	aCtx->pCurDst = aDstBuf;
	aCtx->pDstEnd = aDstBuf + aDstBufSize;
	aCtx->code		= 1;

#ifdef ASAP_IS_DEBUG_BUILD
	aCtx->writeCounter = 0;
#endif

	return RCOBS_RET_SUCCESS;
}

eRCOBS_ret rcobs_encode_inc_end( sRCOBS_ctx *aCtx, size_t *aOutSizeEncoded )
{
	if( ( !aCtx ) || ( !aOutSizeEncoded ) )
	{
		return RCOBS_RET_ERR_BAD_ARG;
	}

	if( ( aCtx->pCurDst + 1 ) > aCtx->pDstEnd )
	{
		return RCOBS_RET_ERR_OVERFLOW;
	}

#ifdef ASAP_IS_DEBUG_BUILD
	aCtx->writeCounter++;
#endif

	*aCtx->pCurDst++ = aCtx->code;

	*aOutSizeEncoded = (size_t)( aCtx->pCurDst - aCtx->pDst );

	return RCOBS_RET_SUCCESS;
}

eRCOBS_ret rcobs_encode_inc( sRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	if( ( !aCtx ) || ( !aSrcBuf ) )
	{
		return RCOBS_RET_ERR_BAD_ARG;
	}

	if( aSrcBufSize == 0 )
	{
		return RCOBS_RET_SUCCESS;
	}

	const size_t maxEncodedSize = RCOBS_MAX_ENCODED_SIZE( aSrcBufSize );

	uint8_t *curDst = aCtx->pCurDst;

	if( ( curDst + maxEncodedSize ) > aCtx->pDstEnd )
	{
		return RCOBS_RET_ERR_OVERFLOW;
	}

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

			*curDst++ = curCode;
			curCode		= 1;
		}
		else
		{
#ifdef ASAP_IS_DEBUG_BUILD
			aCtx->writeCounter++;
#endif

			*curDst++ = byte;
			curCode++;

			if( curCode == 0xFF )
			{
#ifdef ASAP_IS_DEBUG_BUILD
				aCtx->writeCounter++;
#endif

				*curDst++ = curCode;
				curCode		= 1;
			}
		}
	}

	aCtx->code		= curCode;
	aCtx->pCurDst = curDst;

	return RCOBS_RET_SUCCESS;
}

eRCOBS_ret rcobs_decode( const uint8_t *aSrcBufEncoded,
												 size_t aSrcBufEncodedLen,
												 uint8_t *aDstBufDecoded,
												 size_t aDstBufDecodedSize,
												 size_t *aOutDecodedLen,
												 uint8_t **aOutDecodedStartPos )
{
	if( ( !aSrcBufEncoded ) || ( !aDstBufDecoded ) || ( !aOutDecodedLen ) || ( !aOutDecodedStartPos ) ||
			( aDstBufDecodedSize == 0 ) || ( aSrcBufEncodedLen < 2 ) )
	{
		return RCOBS_RET_ERR_BAD_ARG;
	}

	const uint8_t *pBeginEncoded = aSrcBufEncoded;
	const uint8_t *pReadEncoded	 = aSrcBufEncoded + aSrcBufEncodedLen - 1;

	const uint8_t *pBeginDecoded	= aDstBufDecoded;
	uint8_t *pWriteDecodedInitial = aDstBufDecoded + aDstBufDecodedSize;
	uint8_t *pWriteDecoded				= pWriteDecodedInitial; // starts out of buffer, will be decremented latter

#ifdef ASAP_IS_DEBUG_BUILD
	size_t totalWrite = 0;
	size_t totalRead	= 0;
#endif

	while( pReadEncoded >= pBeginEncoded )
	{
		uint8_t code = *pReadEncoded;

		if( code == 0 )
		{
			return RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
		}

		if( ( code != 0xFF ) &&												// Only adds if the new code is not skipping
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
			return RCOBS_RET_ERR_OVERFLOW;
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
				return RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
			}

			pWriteDecoded--;
			*pWriteDecoded = byte;

#ifdef ASAP_IS_DEBUG_BUILD
			totalWrite++;
#endif
		}
	}

	*aOutDecodedStartPos = pWriteDecoded;

	*aOutDecodedLen = aDstBufDecodedSize - (size_t)( pWriteDecoded - aDstBufDecoded );

	return RCOBS_RET_SUCCESS;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
