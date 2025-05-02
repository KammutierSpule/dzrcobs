// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs.h
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
#ifndef _DZRCOBS_H_
#define _DZRCOBS_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdint.h>

#if DZRCOBS_USE_DICT == 1
#include "dzrcobs_dictionary.h"
#endif

// clang-format off
#ifdef __cplusplus
extern "C" {
#endif
// clang-format on

// Definitions
// /////////////////////////////////////////////////////////////////////////////

#define DZRCOBS_FRAME_HEADER_SIZE ( 2 )

typedef enum e_DZRCOBS_ret
{
	DZRCOBS_RET_SUCCESS = 0,
	DZRCOBS_RET_ERR_BAD_ARG,
	DZRCOBS_RET_ERR_OVERFLOW,
	DZRCOBS_RET_ERR_NOTINITIALIZED,
	DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD,
	DZRCOBS_RET_ERR_CRC,
	DZRCOBS_RET_ERR_NO_DICTIONARY_TO_DECODE,
	DZRCOBS_RET_ERR_WORD_NOT_FOUND_ON_DICTIONARY,
} eDZRCOBS_ret;

typedef enum e_DZRCOBS_encoding
{
	DZRCOBS_PLAIN				 = 0, ///< No compression
	DZRCOBS_USING_DICT_1 = 1, ///< Compression using dictionary 1
	DZRCOBS_USING_DICT_2 = 2, ///< Compression using dictionary 2
	DZRCOBS_RESERVED		 = 3, ///< For future uses
} eDZRCOBS_encoding;

typedef struct s_DZRCOB_ctx sDZRCOBS_ctx;

typedef eDZRCOBS_ret ( *dzrcobs_encode_inc_funcPtr )( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );

struct s_DZRCOB_ctx
{
	uint8_t *pDst;		///< Initial destiny pointer
	uint8_t *pCurDst; ///< Current destiny pointer
	uint8_t *pDstEnd; ///< Last position pointer, 1 position outside buffer range
	uint8_t code;			///< Current code

#if DZRCOBS_USE_DICT == 1
	const sDICT_ctx *pDict[DZRCOBS_DICT_N];
#endif

	dzrcobs_encode_inc_funcPtr encFunc;

	eDZRCOBS_encoding encoding;

	uint8_t crc;
	uint8_t user6bits; ///< user application 6 bits, cannot be 0, so must be 1..63, right aligned
	bool isPreviousCodeDictionaryOrZero;

#if DZRCOBS_USE_DICT == 1
	bool isFirstByteInTheBuffer;
#endif

#ifdef ASAP_IS_DEBUG_BUILD
	size_t writeCounter; ///< Current destiny counter, for debug
#endif
};

#define DZRCOBS_ONE_BYTE_OVERHEAD_EVERY ( 126 )
#define Z_DZRCOBS_DIV_ROUND_UP( n, d ) ( ( ( n ) + ( d ) - 1 ) / ( d ) )
#define DZRCOBS_MAX_OVERHEAD( size ) Z_DZRCOBS_DIV_ROUND_UP( ( size ), DZRCOBS_ONE_BYTE_OVERHEAD_EVERY )
#define DZRCOBS_MAX_ENCODED_SIZE( size ) ( ( size ) + DZRCOBS_MAX_OVERHEAD( ( size ) ) + ( ( size ) == 0 ) )

#define DZRCOBS_CRC_VALUE_WHEN_CRC_IS_ZERO (0xFF)

#define DZRCOBS_DICTIONARY_BITMASK (0x80)

// Declarations
// /////////////////////////////////////////////////////////////////////////////

#if DZRCOBS_USE_DICT == 1
/**
 * @brief Set the pointer to an existent created dictionary context
 *
 * @param aCtx The encoding context.
 * @param aDictCtx The dictionary context already created
 * @param aDictEncoding must be DZRCOBS_USING_DICT_1 or DZRCOBS_USING_DICT_2
 * @return eDZRCOBS_ret
 */
eDZRCOBS_ret dzrcobs_encode_set_dictionary( sDZRCOBS_ctx *aCtx,
																						const sDICT_ctx *aDictCtx,
																						eDZRCOBS_encoding aDictEncoding );
#endif

/**
 * @brief Begin an incremental encoding of data
 *
 * @param aCtx Context to be initialized
 * @param aEncoding The desired encoding for this frame
 * @param aDstBuf Destiny buffer
 * @param aDstBufSize Max buffer size
 * @return eRCOBS_ret
 */
eDZRCOBS_ret dzrcobs_encode_inc_begin( sDZRCOBS_ctx *aCtx,
																			 eDZRCOBS_encoding aEncoding,
																			 uint8_t *aDstBuf,
																			 size_t aDstBufSize );

/**
 * @brief Add the data to encoding
 *
 * @param aCtx Context in use
 * @param aSrcBuf Source buffer
 * @param aSrcBufSize Size of source buffer
 * @return eRCOBS_ret
 */
eDZRCOBS_ret dzrcobs_encode_inc( sDZRCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );

/**
 * @brief Finalize the encoding. It adds a 0 in the end of buffer
 *
 * @param aCtx Context in use
 * @param aOutSizeEncoded Size of encoded data (last 0 included)
 * @return eRCOBS_ret
 */
eDZRCOBS_ret dzrcobs_encode_inc_end( sDZRCOBS_ctx *aCtx, size_t *aOutSizeEncoded );

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
