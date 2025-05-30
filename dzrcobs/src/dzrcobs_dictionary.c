// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs_dictionary.c
///	@brief
///
///	@par  Plataform Target:	Any
/// @par  Tab Size: 2
///
/// @copyright (C) 2025 Mario Luzeiro All rights reserved.
/// @author Mario Luzeiro <mluzeiro@ua.pt>
///
/// @par  License: Distributed under the 3-Clause BSD License. See accompanying
/// file LICENSE or a copy at https://opensource.org/licenses/BSD-3-Clause
/// SPDX-License-Identifier: BSD-3-Clause
///
// /////////////////////////////////////////////////////////////////////////////

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include "dzrcobs/dzrcobs_dictionary.h"
#include <stddef.h>
#include <string.h>
#include "dzrcobs_assert.h"

eDICT_ret dzrcobs_dictionary_init( sDICT_ctx *aCtx, const char *aDictionary, size_t aDictionarySize )
{
	if( ( !aCtx ) || ( !aDictionary ) || ( aDictionarySize < 3 ) )
	{
		return DICT_RET_ERR_BAD_ARG;
	}

	memset( aCtx, 0x00, sizeof( sDICT_ctx ) );

	aCtx->minWordSize = 0xFF;

	if( dzrcobs_dictionary_isvalid( aDictionary, aDictionarySize ) != DICT_IS_VALID )
	{
		return DICT_RET_ERR_INVALID;
	}

	// Setup dictionary structure

	const char *pDictBuffer		 = aDictionary;
	const char *pDictBufferEnd = aDictionary + aDictionarySize - 1; // remove last 0
	uint8_t currentWordIndex	 = 1;
	uint8_t currentStride			 = 0;

	sDICT_wordentry *pWordEntry = NULL;

	while( pDictBuffer < pDictBufferEnd )
	{
		const uint8_t newStride = ( (uint8_t)( *pDictBuffer ) - '0' ) + 1;

		if( currentStride != newStride )
		{
			currentStride = newStride;

			if( pWordEntry == NULL )
			{
				pWordEntry = &aCtx->wordSizeTable[0];
			}
			else
			{
				DZRCOBS_ASSERT( pWordEntry->nEntries > 0 );
				pWordEntry->lastIndex = pWordEntry->nEntries - 1;
				pWordEntry++;
			}

			pWordEntry->dictionaryBegin = (const uint8_t *)pDictBuffer;
			pWordEntry->globalIndex			= currentWordIndex;
			pWordEntry->strideSize			= currentStride;
			pWordEntry->nEntries				= 0;

			const uint8_t wordSize = currentStride - 1;

			if( aCtx->minWordSize > wordSize )
			{
				aCtx->minWordSize = wordSize;
			}

			if( aCtx->maxWordSize < wordSize )
			{
				aCtx->maxWordSize = wordSize;
			}
		}

		pWordEntry->nEntries++;

		pDictBuffer += newStride;

		currentWordIndex++;
	}

	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[0].strideSize == ( 2 + 1 ) ) || ( aCtx->wordSizeTable[0].nEntries == 0 ) );
	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[1].strideSize == ( 3 + 1 ) ) || ( aCtx->wordSizeTable[1].nEntries == 0 ) );
	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[2].strideSize == ( 4 + 1 ) ) || ( aCtx->wordSizeTable[2].nEntries == 0 ) );
	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[3].strideSize == ( 5 + 1 ) ) || ( aCtx->wordSizeTable[3].nEntries == 0 ) );

	return DICT_RET_SUCCESS;
}

#define DZRCOBS_MIN_DICT_WORD_SIZE ( 2 )
#define DZRCOBS_MAX_DICT_WORD_SIZE ( 5 )
#define DZRCOBS_MAX_DICT_WORD_COUNTING ( 126 )

eDICTVALID_ret dzrcobs_dictionary_isvalid( const char *aDictionary, size_t aDictionarySize )
{
	const char *pDictBufferEnd = aDictionary + aDictionarySize;

	char tmpBuffer[2][DZRCOBS_MAX_DICT_WORD_SIZE + 1];

	const char *pDictBuffer = aDictionary;

	char *pCurrentWordBuffer	= tmpBuffer[0];
	char *pPreviousWordBuffer = NULL;
	uint8_t previousWordLen		= 0;

	uint8_t wordCount					 = 0;
	uint8_t differentWordCount = 0;

	while( pDictBuffer < pDictBufferEnd )
	{
		char sizeChar = *pDictBuffer++;

		if( !( ( sizeChar >= ( DZRCOBS_MIN_DICT_WORD_SIZE + '0' ) ) &&
					 ( sizeChar <= ( DZRCOBS_MAX_DICT_WORD_SIZE + '0' ) ) ) )
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

		if( ( pPreviousWordBuffer != NULL ) && ( previousWordLen == currentWordLen ) )
		{
			if( memcmp( pPreviousWordBuffer, pCurrentWordBuffer, currentWordLen ) >= 0 )
			{
				return DICT_INVALID_NOT_SORTED;
			}
		}
		else
		{
			previousWordLen			= currentWordLen;
			pPreviousWordBuffer = tmpBuffer[1];
			differentWordCount++;

			if( differentWordCount > DICT_MAX_DIFFERENTWORDSIZES )
			{
				return DICT_INVALID_NUMBER_OF_WORDSIZES;
			}
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

	if( ( pDictBuffer + 1 ) < pDictBufferEnd )
	{
		return DICT_INVALID_EARLIER_END;
	}

	return DICT_IS_VALID;
}

uint8_t DZRCOBS_Dictionary_SearchKeyOnEntry( const uint8_t *aSearchKey, const sDICT_wordentry *aDictWordEntry )
{
	DZRCOBS_ASSERT( aSearchKey != NULL );
	DZRCOBS_ASSERT( aDictWordEntry != NULL );

	const uint8_t *dictionaryBegin = aDictWordEntry->dictionaryBegin + 1; // skip the first byte with entry size
	const size_t strideSize				 = aDictWordEntry->strideSize;
	const size_t wordSize					 = strideSize - 1;

	int16_t idxStart = 0;
	int16_t idxLast	 = aDictWordEntry->lastIndex;

	while( idxStart <= idxLast )
	{
		const size_t idxMiddle = ( (size_t)idxStart + (size_t)idxLast ) >> 1;

		const uint8_t *entry = dictionaryBegin + strideSize * idxMiddle;
		const int cmpResult	 = memcmp( aSearchKey, entry, wordSize );

		if( cmpResult > 0 )
		{
			idxStart = (int16_t)( idxMiddle + 1 );
		}
		else
		{
			if( cmpResult < 0 )
			{
				idxLast = (int16_t)( idxMiddle - 1 );
			}
			else
			{
				return (uint8_t)( idxMiddle + aDictWordEntry->globalIndex );
			}
		}
	}

	return 0;
}

uint8_t dzrcobs_dictionary_search( const sDICT_ctx *aCtx,
																	 const uint8_t *aSearchKey,
																	 size_t aSearchKeySize,
																	 size_t *aOutKeySizeFound )
{
	DZRCOBS_ASSERT( aCtx != NULL );
	DZRCOBS_ASSERT( aSearchKey != NULL );
	DZRCOBS_ASSERT( aOutKeySizeFound != NULL );

	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[0].strideSize == ( 2 + 1 ) ) || ( aCtx->wordSizeTable[0].nEntries == 0 ) );
	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[1].strideSize == ( 3 + 1 ) ) || ( aCtx->wordSizeTable[1].nEntries == 0 ) );
	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[2].strideSize == ( 4 + 1 ) ) || ( aCtx->wordSizeTable[2].nEntries == 0 ) );
	DZRCOBS_ASSERT( ( aCtx->wordSizeTable[3].strideSize == ( 5 + 1 ) ) || ( aCtx->wordSizeTable[3].nEntries == 0 ) );

	if( aSearchKeySize < aCtx->minWordSize )
	{
		return 0;
	}

	if( aSearchKeySize > aCtx->maxWordSize )
	{
		aSearchKeySize = aCtx->maxWordSize;
	}

	const size_t compareKeySize = aSearchKeySize + 1; // this is just to fake a dummy header byte

	for( uint8_t i = 0; i < DICT_MAX_DIFFERENTWORDSIZES; i++ )
	{
		const sDICT_wordentry *wordEntry = &aCtx->wordSizeTable[i];

		if( ( wordEntry->nEntries > 0 ) && ( compareKeySize >= wordEntry->strideSize ) )
		{
			const uint8_t idxFound = DZRCOBS_Dictionary_SearchKeyOnEntry( aSearchKey, wordEntry );

			if( idxFound != 0 )
			{
				*aOutKeySizeFound = wordEntry->strideSize - 1;

				return idxFound;
			}
		}
	}

	return 0;
}

const uint8_t *dzrcobs_dictionary_get( const sDICT_ctx *aCtx, uint8_t aIndex, uint8_t *aOutWordSize )
{
	DZRCOBS_ASSERT( aCtx != NULL );
	DZRCOBS_ASSERT( aIndex < 126 );
	DZRCOBS_ASSERT( aOutWordSize != NULL );

	aIndex++; // convert to start as a 1 index (for easy comparison)

	for( uint8_t i = 0; i < DICT_MAX_DIFFERENTWORDSIZES; i++ )
	{
		const sDICT_wordentry *wordEntry = &aCtx->wordSizeTable[i];

		if( ( wordEntry->nEntries > 0 ) && ( ( wordEntry->globalIndex + wordEntry->lastIndex ) >= aIndex ) )
		{
			*aOutWordSize = wordEntry->strideSize - 1;

			const size_t wordZeroIdx = aIndex - wordEntry->globalIndex;

			return wordEntry->dictionaryBegin +						 // entry base +
						 ( wordZeroIdx * wordEntry->strideSize ) // index * stride +
						 + 1;																		 // +1 to skip the word size
		}
	}

	return NULL;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
