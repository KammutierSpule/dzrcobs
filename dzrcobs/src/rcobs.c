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

// Implementation
// /////////////////////////////////////////////////////////////////////////////

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

	const uint8_t *pBeginDecoded = aDstBufDecoded;
	uint8_t *pWriteDecoded = aDstBufDecoded + aDstBufDecodedSize; // starts out of buffer, will be decremented latter

	while( pReadEncoded >= pBeginEncoded )
	{
		uint8_t code = *pReadEncoded;

		if( code == 0 )
		{
			return RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
		}

		code--;
		if( ( pWriteDecoded - code ) < pBeginDecoded )
		{
			return RCOBS_RET_ERR_OVERFLOW;
		}

		pReadEncoded--;

		while( code )
		{
			code--;

			const uint8_t byte = *pReadEncoded--;

			if( byte == 0 )
			{
				return RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
			}

			pWriteDecoded--;
			*pWriteDecoded = byte;
		}

		if( ( pReadEncoded >= pBeginEncoded ) && ( ( pWriteDecoded - 1 ) >= pBeginDecoded ) )
		{
			pWriteDecoded--;
			*pWriteDecoded = 0;
		}
	}

	*aOutDecodedStartPos = pWriteDecoded;

	*aOutDecodedLen = aDstBufDecodedSize - (size_t)( pWriteDecoded - aDstBufDecoded );

	return RCOBS_RET_SUCCESS;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
