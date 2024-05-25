// /////////////////////////////////////////////////////////////////////////////
///	@file rcobs.hpp
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
#ifndef _RCOBS_H_
#define _RCOBS_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Definitions
// /////////////////////////////////////////////////////////////////////////////

typedef struct s_RCOB_ctx
{
	void *pCurDst;	///< Current destiny pointer
	void *pLastPos; ///< Last position pointer
} sRCOBS_ctx;

typedef enum eRCOBS_ret
{
	RCOBS_RET_SUCCESS = 0,
	RCOBS_RET_ERR_BAD_ARG,
	RCOBS_RET_ERR_OVERFLOW,
	RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD,
} eRCOBS_ret;

// Declarations
// /////////////////////////////////////////////////////////////////////////////

/**
 * @brief Begin an incremental encoding of data
 *
 * @param aCtx Context to be initialized
 * @param aDstBuf Destiny buffer
 * @param aDstBufSize Max buffer size
 * @return eRCOBS_ret
 */
eRCOBS_ret rcobs_encode_inc_begin( sRCOBS_ctx *aCtx, void *aDstBuf, size_t aDstBufSize );

/**
 * @brief Add the data to encoding
 *
 * @param aCtx Context in use
 * @param aSrcBuf Source buffer
 * @param aSrcBufSize Size of source buffer
 * @return eRCOBS_ret
 */
eRCOBS_ret rcobs_encode_inc( sRCOBS_ctx *aCtx, const void *aSrcBuf, size_t aSrcBufSize );

/**
 * @brief Finalize the encoding. It adds a 0 in the end of buffer
 *
 * @param aCtx Context in use
 * @param aOutSizeEncoded Size of encoded data (last 0 included)
 * @return eRCOBS_ret
 */
eRCOBS_ret rcobs_encode_inc_end( sRCOBS_ctx *aCtx, size_t *aOutSizeEncoded );

/**
 * @brief Decodes a source encoded buffer. It will place the decoded data
 *        left aligned with the aDstBufDecoded.
 *        Data starts aDstBufDecoded[aDstBufEncodedSize - aOutDecodedLen]
 *        Data ends aDstBufDecoded[aDstBufEncodedSize - 1]
 *        It implicit assumes that the encoded data ends with a 0,
 *        so aSrcBufEncoded[aSrcBufEncodedLen] == 0 (implicit)
 *
 * @param aSrcBufEncoded Source buffer encoded
 * @param aSrcBufEncodedLen Source buffer encoded data length
 * @param aDstBufDecoded Destiny buffer
 * @param aDstBufDecodedSize Max buffer size
 * @param aOutDecodedLen Size of decoded data
 * @param aOutDecodedStartPos Start position of the decoded data
 * (between &aDstBufDecoded[0] and &aDstBufDecoded[aDstBufEncodedSize - 1])
 * @retval RCOBS_RET_SUCCESS if decoded is ok
 * @retval RCOBS_RET_ERR_BAD_ARG if invalid arguments are passed
 * @retval RCOBS_RET_ERR_OVERFLOW if it overflows the destiny buffer
 * @retval RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD if some invalid value (eg: 0x00)
 */
eRCOBS_ret rcobs_decode( const uint8_t *aSrcBufEncoded,
												 size_t aSrcBufEncodedLen,
												 uint8_t *aDstBufDecoded,
												 size_t aDstBufDecodedSize,
												 size_t *aOutDecodedLen,
												 uint8_t **aOutDecodedStartPos );

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
