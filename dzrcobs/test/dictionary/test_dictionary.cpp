// /////////////////////////////////////////////////////////////////////////////
///	@file test_dictionary.cpp
///	@brief Tests dictionary
///
///	@par  Plataform Target:	Tests
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

#include <CppUTest/TestHarness.h>
#include <CppUTest/UtestMacros.h>
#include <cstdint>
#include <dzrcobs/dzrcobs_dictionary.h>

// Definitions
// /////////////////////////////////////////////////////////////////////////////
extern "C"
{
uint8_t DZRCOBS_Dictionary_SearchKeyOnEntry( const uint8_t *aSearchKey, const sDICT_wordentry *aDictWordEntry );
}

// Setup
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
/// Dictionary string, descendent order, null terminated
static const char s_TEST_Dictionary[] =
	DICT_ADD_WORD(2, "\x01\x00")
	DICT_ADD_WORD(2, "\x02\x00")
	DICT_ADD_WORD(2, "\x03\x00")
	DICT_ADD_WORD(2, "\x04\x00")
	DICT_ADD_WORD(2, "\x05\x10")
	DICT_ADD_WORD(2, "\x05\x20")
	DICT_ADD_WORD(2, "\x05\x30")
	DICT_ADD_WORD(2, "\x05\x40")
	DICT_ADD_WORD(3, "\x00\x00\x00")
	DICT_ADD_WORD(3, "\x00\x00\x01")
	DICT_ADD_WORD(3, "\x00\x01\x00")
	DICT_ADD_WORD(3, "\x01\x00\x00")
  DICT_ADD_WORD(4, "\x01\x00\x00\x00")
  DICT_ADD_WORD(5, "\x01\x00\x00\x00\x00")
;
// NOLINTEND
// clang-format on

const size_t s_TEST_Dictionary_size = sizeof( s_TEST_Dictionary );

// clang-format off
// NOLINTBEGIN
TEST_GROUP( DICTIONARY ){
	void setup()
	{
	  eDICTVALID_ret dictRet = dzrcobs_dictionary_isvalid( s_TEST_Dictionary, s_TEST_Dictionary_size );
	  CHECK_EQUAL( DICT_IS_VALID, dictRet );

    eDICT_ret ret = dzrcobs_dictionary_init( &m_dictCtx, s_TEST_Dictionary, s_TEST_Dictionary_size );
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
	eDICTVALID_ret ret = dzrcobs_dictionary_isvalid( G_DZRCOBS_DefaultDictionary, G_DZRCOBS_DefaultDictionary_size );
	CHECK_EQUAL( DICT_IS_VALID, ret );
}

// NOLINTBEGIN
TEST( DICTIONARY, SearchKeyOnEntry )
{
	uint8_t ret = 0;

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x00, 0x01 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 0, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x00, 0x02 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 0, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x00, 0x03 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 0, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x00, 0x04 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 0, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x01, 0x00 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 1, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x02, 0x00 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 2, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x03, 0x00 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 3, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x04, 0x00 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 4, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x05, 0x10 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 5, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x05, 0x20 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 6, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x05, 0x30 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 7, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x05, 0x40 }, &m_dictCtx.wordSizeTable[0] );
	CHECK_EQUAL( 8, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x00, 0x00, 0x00 }, &m_dictCtx.wordSizeTable[1] );
	CHECK_EQUAL( 9, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x00, 0x00, 0x01 }, &m_dictCtx.wordSizeTable[1] );
	CHECK_EQUAL( 10, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x00, 0x01, 0x00 }, &m_dictCtx.wordSizeTable[1] );
	CHECK_EQUAL( 11, ret );

	ret = DZRCOBS_Dictionary_SearchKeyOnEntry( ( uint8_t[] ){ 0x01, 0x00, 0x00 }, &m_dictCtx.wordSizeTable[1] );
	CHECK_EQUAL( 12, ret );
}

// NOLINTEND

// NOLINTBEGIN
TEST( DICTIONARY, SearchKey )
{
	uint8_t ret = 0;
	size_t keySizeFound;

	ret = dzrcobs_dictionary_search( &m_dictCtx, ( uint8_t[] ){ 0x00, 0x01 }, 2, &keySizeFound );
	CHECK_EQUAL( 0, ret );

	ret = dzrcobs_dictionary_search( &m_dictCtx, ( uint8_t[] ){ 0x01, 0x00, 0xFF }, 3, &keySizeFound );
	CHECK_EQUAL( 1, ret );
  CHECK_EQUAL( 2, keySizeFound );
}

// NOLINTEND

// EOF
// /////////////////////////////////////////////////////////////////////////////
