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

		// printf( "idx: %lu\r\n", idx++ );

		eRCOBS_ret ret			= RCOBS_RET_SUCCESS;
		size_t decodedLen		= 0;
		uint8_t *decodedPos = nullptr;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

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

TEST( RCOBS, dummyFail )
{
	// CHECK( false );
}