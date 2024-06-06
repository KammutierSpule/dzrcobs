// /////////////////////////////////////////////////////////////////////////////
///	@file dictionary.h
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
#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

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

// Helper to construct a entry on the dictionary
#define DICT_ADD_WORD( len, word ) #len word

typedef enum e_DICTVALID_ret
{
	DICT_IS_VALID = 0,
	DICT_INVALID_NOT_SORTED,
	DICT_INVALID_OUTOFBOUNDS,
	DICT_INVALID_WORDCOUNTING,
  DICT_INVALID_WORDSIZE,
  DICT_INVALID_EARLIER_END,
  DICT_INVALID_NUMBER_OF_WORDSIZES,
} eDICTVALID_ret;


// Declarations
// /////////////////////////////////////////////////////////////////////////////

/**
 * @brief Perform some verifications on dictionary to evaluate if it is valid.
 *        Checks size validity of the content.
 *
 * @param aDictionary
 * @param aDictionarySize
 * @retval DICT_IS_VALID Dictionary is well formed.
 * @retval DICT_INVALID_NOT_SORTED Is not sorted
 * @retval DICT_INVALID_OUTOFBOUNDS Out of bounds
 *         (internal word sizes does not match the dictionary size)
 * @retval DICT_INVALID_WORDCOUNTING Word counting exceeded.
 * @retval DICT_INVALID_WORDSIZE If there is an invalid word size
 */
eDICTVALID_ret DZRCOBS_Dictionary_IsValid( const char *aDictionary, size_t aDictionarySize );

// External declaration of default dictionary
extern const char G_DZRCOBS_DefaultDictionary[];
extern const size_t G_DZRCOBS_DefaultDictionary_size;

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
