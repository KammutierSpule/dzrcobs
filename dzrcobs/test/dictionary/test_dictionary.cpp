// /////////////////////////////////////////////////////////////////////////////
///	@file test_dictionary.cpp
///	@brief Tests dictionary
///
///	@par  Plataform Target:	Tests
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

#include <CppUTest/TestHarness.h>
#include <CppUTest/UtestMacros.h>
#include <dzrcobs/dictionary.h>
#include <dzrcobs_dictionary.h>

// Definitions
// /////////////////////////////////////////////////////////////////////////////

// Setup
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
TEST_GROUP( DICTIONARY ){
	void setup()
	{
    eDICT_ret ret = DZRCOBS_Dictionary_Init( &m_dictCtx, G_DZRCOBS_DefaultDictionary, G_DZRCOBS_DefaultDictionary_size );
    CHECK_EQUAL( DICT_RET_SUCCESS, ret );
	}

	void teardown()
	{

	}

  sDICT_ctx m_dictCtx;
};
// NOLINTEND
// clang-format on

// Test data
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN

// NOLINTEND
// clang-format on

// Tests
// /////////////////////////////////////////////////////////////////////////////

// NOLINTBEGIN
TEST( DICTIONARY, InternalDictionaryValidation )
// NOLINTEND
{
	eDICTVALID_ret ret = DZRCOBS_Dictionary_IsValid( G_DZRCOBS_DefaultDictionary, G_DZRCOBS_DefaultDictionary_size );
	CHECK_EQUAL( DICT_IS_VALID, ret );
}

TEST( DICTIONARY, DictionarySearch )
// NOLINTEND
{
	eDICT_ret ret = DICT_RET_ERR_INVALID;

	ret = DZRCOBS_Dictionary_SearchAndInc( &m_dictCtx, 'a' );
	CHECK_EQUAL( DICT_RET_THE_SEARCH_CONTINUES, ret );

	ret = DZRCOBS_Dictionary_SearchAndInc( &m_dictCtx, 'n' );
	CHECK_EQUAL( DICT_RET_THE_SEARCH_CONTINUES, ret );

	ret = DZRCOBS_Dictionary_SearchAndInc( &m_dictCtx, 'd' );
	CHECK_EQUAL( DICT_RET_THE_SEARCH_CONTINUES, ret );

	ret = DZRCOBS_Dictionary_SearchAndInc( &m_dictCtx, 'z' );
	CHECK_EQUAL( DICT_RET_SEARCH_END, ret );

	uint8_t matchedWordEntry = 0;
	uint8_t matchedWordLen	 = 0;

	const bool matched = DZRCOBS_Dictionary_GetMatchedWord( &m_dictCtx, &matchedWordEntry, &matchedWordLen );

	CHECK_EQUAL( true, matched );
	CHECK( matchedWordEntry > 0 );
	CHECK( matchedWordLen == 3 );
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
