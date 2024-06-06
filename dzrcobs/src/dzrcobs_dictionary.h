// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs_dictionary.h
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
#ifndef _DZRCOBS_DICTIONARY_H_
#define _DZRCOBS_DICTIONARY_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// clang-format off
#ifdef __cplusplus
extern "C" {
#endif
// clang-format on

// Definitions
// /////////////////////////////////////////////////////////////////////////////

#define DICT_MAX_DIFFERENTWORDSIZES ( 4 )

/// Dictionary entry for different word sizes
typedef struct s_DICT_wordentry
{
	const uint8_t *dictionaryBegin; ///< Origin pointer to the dictionary entry
	uint8_t lastIndex;							///< Number of entries-1
	uint8_t globalIndex;						///< Start index for this dictionary entry on the global dictionary
	uint8_t strideSize;							///< word size + 1, that is the size of each word entry
} sDICT_wordentry;

typedef struct s_DICT_ctx
{
	sDICT_wordentry wordSizeTable[DICT_MAX_DIFFERENTWORDSIZES];
} sDICT_ctx;

typedef enum e_DICT_ret
{
	DICT_RET_SUCCESS = 0,
	DICT_RET_ERR_BAD_ARG,
	DICT_RET_ERR_INVALID,
} eDICT_ret;

// Declarations
// /////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize a dictionary context
 *
 * @param aCtx The context to store this dictionary session
 * @param aDictionary Pointer to the dictionary string array that will be used
 * @param aDictionarySize Size of dictionary string
 * @return eDICT_ret DICT_RET_SUCCESS if all good with parameters
 */
eDICT_ret DZRCOBS_Dictionary_Init( sDICT_ctx *aCtx, const char *aDictionary, size_t aDictionarySize );

/**
 * @brief Search for a Key in the dictionary
 *
 * @param aCtx The context to be used
 * @param aSearchKey The key buffer data
 * @param aSearchKeySize The key buffer size
 * @param aOutKeySizeFound The output with the key size found (2..5)
 * @return uint8_t 0 not found, 1..126 index of the key found
 */
uint8_t DZRCOBS_Dictionary_Search( const sDICT_ctx *aCtx,
																	 const uint8_t *aSearchKey,
																	 size_t aSearchKeySize,
																	 size_t *aOutKeySizeFound );

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
