// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs_dictionary.c
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
#include "dzrcobs_dictionary.h"
#include <string.h>
#include "dzrcobs/dictionary.h"

#ifndef DZRCOBS_ASSERT

#ifdef ASAP_IS_DEBUG_BUILD
#include <assert.h>
#define DZRCOBS_ASSERT( a ) assert( a )
#else
#define DZRCOBS_ASSERT( a )
#endif

#endif

static void DZRCOBS_Dictionary_UpdateVariables( sDICT_ctx *aCtx )
{
	const char sizeChar = aCtx->currentWordString[0];

	DZRCOBS_ASSERT( ( sizeChar >= '1' ) && ( sizeChar <= '9' ) );

	aCtx->currentWordLen = (uint8_t)( sizeChar - '0' );

	aCtx->currentWordString++;

	// End of the current word string / start of the next entry with the len
	// information
	aCtx->nextWordEntry = aCtx->currentWordString + aCtx->currentWordLen;
}

eDICT_ret DZRCOBS_Dictionary_ResetSearch( sDICT_ctx *aCtx )
{
	if( !aCtx )
	{
		return DICT_RET_ERR_BAD_ARG;
	}

	aCtx->matchedWordEntry	= 0; // 0 means not found
	aCtx->currentWordString = aCtx->dictionaryBegin;
	aCtx->matchedWordEntry	= 0;
	aCtx->matchedWordLen		= 0;

	DZRCOBS_Dictionary_UpdateVariables( aCtx );

	return DICT_RET_SUCCESS;
}

eDICT_ret DZRCOBS_Dictionary_Init( sDICT_ctx *aCtx, const char *aDictionary, size_t aDictionarySize )
{
	if( ( !aCtx ) || ( !aDictionary ) || ( aDictionarySize < 3 ) )
	{
		return DICT_RET_ERR_BAD_ARG;
	}

	aCtx->dictionaryBegin = aDictionary;
	aCtx->dictionaryEnd		= aDictionary + aDictionarySize;

	return DZRCOBS_Dictionary_ResetSearch( aCtx );
}

eDICT_ret DZRCOBS_Dictionary_SearchAndInc( sDICT_ctx *aCtx, char aNextChar )
{
	if( !aCtx )
	{
		return DICT_RET_ERR_BAD_ARG;
	}

	if( *aCtx->currentWordString == aNextChar )
	{
		aCtx->currentWordString++;

		const bool reachedTheEndOfTheWorld = aCtx->currentWordString == aCtx->nextWordEntry;

		if( reachedTheEndOfTheWorld )
		{
			aCtx->matchedWordEntry++;
			aCtx->matchedWordLen = aCtx->currentWordLen;

			const bool reachedTheEndOfTheDict = aCtx->currentWordString == aCtx->dictionaryEnd;

			if( reachedTheEndOfTheDict )
			{
				return DICT_RET_SEARCH_END;
			}

			DZRCOBS_Dictionary_UpdateVariables( aCtx );
		}

		return DICT_RET_THE_SEARCH_CONTINUES;
	}

	// Since sorted dictionary is used, this means that there are no more words
	// that may match.
	return DICT_RET_SEARCH_END;
}

bool DZRCOBS_Dictionary_GetMatchedWord( const sDICT_ctx *aCtx,
																				uint8_t *aOutmatchedWordEntry,
																				uint8_t *aOutmatchedWordLen )
{
	DZRCOBS_ASSERT( aCtx );
	DZRCOBS_ASSERT( aOutmatchedWordEntry );
	DZRCOBS_ASSERT( aOutmatchedWordLen );

	if( ( !aCtx ) || ( !aOutmatchedWordEntry ) || ( !aOutmatchedWordLen ) )
	{
		return false;
	}

	if( aCtx->matchedWordEntry )
	{
		*aOutmatchedWordEntry = aCtx->matchedWordEntry;
		*aOutmatchedWordLen		= aCtx->matchedWordLen;

		return true;
	}

	return false;
}

#define DZRCOBS_MAX_DICT_WORD_SIZE ( 9 )
#define DZRCOBS_MAX_DICT_WORD_COUNTING ( 126 )

eDICTVALID_ret DZRCOBS_Dictionary_IsValid( const char *aDictionary, size_t aDictionarySize )
{
	const char *dictionaryEnd = aDictionary + aDictionarySize;

	char tmpBuffer[2][DZRCOBS_MAX_DICT_WORD_SIZE + 1];

	const char *pDictBuffer = aDictionary;

	char *pCurrentWordBuffer	= tmpBuffer[0];
	char *pPreviousWordBuffer = NULL;

	uint8_t wordCount = 0;

	while( pDictBuffer < dictionaryEnd )
	{
		char sizeChar = *pDictBuffer++;

		if( !( ( sizeChar >= '1' ) && ( sizeChar <= '9' ) ) )
		{
			return DICT_INVALID_WORDSIZE;
		}

		const uint8_t currentWordLen = (uint8_t)( sizeChar - '0' );

		memcpy( pCurrentWordBuffer, pDictBuffer, currentWordLen );

		// advance to the next
		pDictBuffer += currentWordLen;
		wordCount++;

		if( wordCount > DZRCOBS_MAX_DICT_WORD_COUNTING )
		{
			return DICT_INVALID_WORDCOUNTING;
		}

		if( pPreviousWordBuffer != NULL )
		{
			if( memcmp( pPreviousWordBuffer, pCurrentWordBuffer, currentWordLen ) > 0 )
			{
				return DICT_INVALID_NOT_SORTED;
			}
		}
		else
		{
			pPreviousWordBuffer = tmpBuffer[1];
		}

		if( *pDictBuffer == 0 )
		{
			break;
		}

		// Swap
		char *tmpPtr				= pPreviousWordBuffer;
		pPreviousWordBuffer = pCurrentWordBuffer;
		pCurrentWordBuffer	= tmpPtr;
	}

	if( *pDictBuffer != 0 )
	{
		return DICT_INVALID_OUTOFBOUNDS;
	}

	if( wordCount == 0 )
	{
		return DICT_INVALID_WORDCOUNTING;
	}

	if( ( pDictBuffer + 1 ) < dictionaryEnd )
	{
		return DICT_INVALID_EARLIER_END;
	}

	return DICT_IS_VALID;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
