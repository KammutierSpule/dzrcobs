// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs_decode.h
///	@brief Dictionary compression with Reverse-COBS dencoding declarations
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
#ifndef _DZRCOBS_DECODE_H_
#define _DZRCOBS_DECODE_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include "dzrcobs.h"
#include "dzrcobs_dictionary.h"

// clang-format off
#ifdef __cplusplus
extern "C" {
#endif
// clang-format on

// Definitions
// /////////////////////////////////////////////////////////////////////////////

typedef struct s_DZRCOB_decodectx
{
	const uint8_t *srcBufEncoded; ///< Source buffer encoded
	size_t srcBufEncodedLen;			///< Source buffer encoded data length
	uint8_t *dstBufDecoded;				///< Destiny buffer
	size_t dstBufDecodedSize;			///< Max buffer size

	///< Dictionaries that may be used on this decoding
	const sDICT_ctx *pDict[DZRCOBS_DICT_N];
} sDZRCOBS_decodectx;

/**
 * @brief Decodes a source encoded buffer. It will place the decoded data
 *        right aligned with the dstBufDecoded.
 *        Data starts dstBufDecoded[dstBufEncodedSize - aOutDecodedLen]
 *        Data ends dstBufDecoded[dstBufEncodedSize - 1]
 *        It implicit assumes that the encoded data ends with a 0,
 *        so srcBufEncoded[srcBufEncodedLen] == 0 (implicit)
 *
 * @param aDecodeCtx Struct with variables prepared to decode.
 * @param aOutDecodedLen Size of decoded data
 * @param aOutDecodedStartPos Start position of the decoded data
 * @param uint8_t *aOutUser6bitDataRightAlgn The 6 bit user data that arrived in
 * the package. Right aligned
 * (between &dstBufDecoded[0] and &dstBufDecoded[dstBufEncodedSize - 1])
 * @retval RCOBS_RET_SUCCESS if decoded is ok
 * @retval RCOBS_RET_ERR_BAD_ARG if invalid arguments are passed
 * @retval RCOBS_RET_ERR_OVERFLOW if it overflows the destiny buffer
 * @retval RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD if some invalid value (eg: 0x00)
 */
eDZRCOBS_ret dzrcobs_decode( const sDZRCOBS_decodectx *aDecodeCtx,
														 size_t *aOutDecodedLen,
														 uint8_t **aOutDecodedStartPos,
														 uint8_t *aOutUser6bitDataRightAlgn );

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
