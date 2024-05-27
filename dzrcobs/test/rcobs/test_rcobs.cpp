// /////////////////////////////////////////////////////////////////////////////
///	@file test_rcobs.cpp
///	@brief Tests for reversed COBS
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
#include <dzrcobs/rcobs.h>
#include "CppUTest/UtestMacros.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////

#define UTEST_GUARD_BYTE ( 0xEE )
#define UTEST_GUARD_SIZE ( 4 )
#define UTEST_ENCODED_DECODED_DATA_MAX_SIZE ( 256 + 1 )

// Setup
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
TEST_GROUP( RCOBS ){
	void setup()
	{
		buffer = new uint8_t[UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE];
	}

	void teardown()
	{
		delete [] buffer;
		buffer = nullptr;
	}

	uint8_t *buffer;
};
// NOLINTEND
// clang-format on

// Test data
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
static uint8_t s_rcobs_datatest[] = {
	// 1
	1, 'A',				// decoded
	2, 'A', 0x02,	// encoded
	// 2
	4, 'A', 'B', 'C', 'D',
	5, 'A', 'B', 'C', 'D', 0x05,
	// 3
	4, 'A', 'B', 0x00, 'C',
	5, 'A', 'B', 0x03, 'C', 0x02,
	// 4
	7, 'A', 0x00, 0x00, 0x00, 'B', 'C', 'D',
	8, 'A', 0x02, 0x01, 0x01, 'B', 'C', 'D', 0x04,
};
// NOLINTEND
// clang-format on

// Tests
// /////////////////////////////////////////////////////////////////////////////
TEST( RCOBS, MACRO_RCOBS_ENCODE_MAX )
{
	// COBS requires a minimum of 1 byte overhead,
	// and a maximum of ⌈n/254⌉ bytes for n data bytes (one byte in 254, rounded
	// up)
	CHECK_EQUAL( 1, RCOBS_MAX_ENCODED_SIZE( 0 ) );
	CHECK_EQUAL( 1 + 1, RCOBS_MAX_ENCODED_SIZE( 1 ) );
	CHECK_EQUAL( 254 + 1, RCOBS_MAX_ENCODED_SIZE( 254 ) );
	CHECK_EQUAL( 255 + 2, RCOBS_MAX_ENCODED_SIZE( 255 ) );
	CHECK_EQUAL( ( 254 * 2 ) + ( 1 * 2 ), RCOBS_MAX_ENCODED_SIZE( 254 * 2 ) );
	CHECK_EQUAL( ( 254 * 2 + 1 ) + ( 1 * 2 ), RCOBS_MAX_ENCODED_SIZE( 254 * 2 ) + 1 );
}

TEST( RCOBS, manual_decode )
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_rcobs_datatest;
	const uint8_t *pDatatest_end = s_rcobs_datatest + sizeof( s_rcobs_datatest );

	while( pDatatest < pDatatest_end )
	{
		const uint8_t decodeDataSize = *pDatatest++;
		const uint8_t *decodeData		 = pDatatest;
		pDatatest += decodeDataSize;

		const uint8_t encodedDataSize = *pDatatest++;
		const uint8_t *encodedData		= pDatatest;
		pDatatest += encodedDataSize;

		eRCOBS_ret ret			= RCOBS_RET_SUCCESS;
		size_t decodedLen		= 0;
		uint8_t *decodedPos = nullptr;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		CHECK_EQUAL( encodedDataSize, RCOBS_MAX_ENCODED_SIZE( decodeDataSize ) );

		ret = rcobs_decode( encodedData,
												encodedDataSize,
												buffer + UTEST_GUARD_SIZE,
												decodeDataSize, // used to test limit of the buffer
												&decodedLen,
												&decodedPos );

		CHECK_EQUAL( RCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( decodeDataSize, decodedLen );
		CHECK_EQUAL( decodedPos, ( buffer + UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( decodeData, decodedPos, decodedLen ) );
	}
}

TEST( RCOBS, manual_encode )
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_rcobs_datatest;
	const uint8_t *pDatatest_end = s_rcobs_datatest + sizeof( s_rcobs_datatest );

	while( pDatatest < pDatatest_end )
	{
		const uint8_t decodeDataSize = *pDatatest++;
		const uint8_t *decodeData		 = pDatatest;
		pDatatest += decodeDataSize;

		const uint8_t encodedDataSize = *pDatatest++;
		const uint8_t *encodedData		= pDatatest;
		pDatatest += encodedDataSize;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		CHECK_EQUAL( encodedDataSize, RCOBS_MAX_ENCODED_SIZE( decodeDataSize ) );

		eRCOBS_ret ret		= RCOBS_RET_SUCCESS;
		size_t encodedLen = 0;
		sRCOBS_ctx ctx;

		ret = rcobs_encode_inc_begin( &ctx,
																	buffer + UTEST_GUARD_SIZE,
																	encodedDataSize // Used to test the limit
		);
		CHECK_EQUAL( RCOBS_RET_SUCCESS, ret );

		ret = rcobs_encode_inc( &ctx, decodeData, decodeDataSize );
		CHECK_EQUAL( RCOBS_RET_SUCCESS, ret );

		ret = rcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( RCOBS_RET_SUCCESS, ret );

		CHECK_EQUAL( encodedDataSize, encodedLen );
		CHECK_EQUAL( 0, memcmp( encodedData, buffer + UTEST_GUARD_SIZE, encodedDataSize ) );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + encodedDataSize, &guard, UTEST_GUARD_SIZE ) );
	}
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
  