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

typedef struct s_DICT_ctx
{
	const char *dictionaryBegin; ///< Origin pointer to the dictionary
	const char *dictionaryEnd;	 ///< Last position pointer, 1 position outside buffer range
	const char *currentWordString;
	const char *nextWordEntry;

	/// Current last entry entry found on search. This is best full word index (1..126) found so far.
	uint8_t matchedWordEntry;
	uint8_t matchedWordLen;
	uint8_t currentWordLen;
} sDICT_ctx;

typedef enum e_DICT_ret
{
	DICT_RET_SUCCESS = 0,
	DICT_RET_SEARCH_END,
	DICT_RET_THE_SEARCH_CONTINUES,
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
 * @brief Reset search parameters and start from beginning of the dictionary
 *
 * @param aCtx The context
 * @retval DICT_RET_SUCCESS if all ok.
 * @retval DICT_RET_ERR_BAD_ARG if something wrong.
 */
eDICT_ret DZRCOBS_Dictionary_ResetSearch( sDICT_ctx *aCtx );

/**
 * @brief Search for a char in the dictionary and increment the internal search
 *
 * @param aCtx
 * @param aNextChar The char to be tested
 * @retval DICT_RET_THE_SEARCH_CONTINUES The aNextChar matches, the search can
 * continue.
 * @retval DICT_RET_SEARCH_END The aNextChar does not match and there is no need
 * to continue to search more. User must call other functions and reset search.
 * this function cannot be called anymore after this return.
 * @retval DICT_RET_ERR_BAD_ARG In case something wrong with the arguments.
 */
eDICT_ret DZRCOBS_Dictionary_SearchAndInc( sDICT_ctx *aCtx, char aNextChar );

/**
 * @brief Get the matched word entry position and the word len if there was a
 * match. if no previous full match word, return false and don't set the values
 *
 * @param aOutmatchedWordEntry
 * @param aOutmatchedWordLen
 * @return true if there was a previous match word. out arguments were set.
 * @return false if no previous matched word. out arguments untouched.
 */
bool DZRCOBS_Dictionary_GetMatchedWord( const sDICT_ctx *aCtx,
																				uint8_t *aOutmatchedWordEntry,
																				uint8_t *aOutmatchedWordLen );

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
