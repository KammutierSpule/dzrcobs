// /////////////////////////////////////////////////////////////////////////////
///	@file test_dzrcobs.cpp
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
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <dzrcobs/dzrcobs.h>
#include <dzrcobs/rcobs.h>
#include "CppUTest/UtestMacros.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////

#define UTEST_GUARD_BYTE ( 0xEE )
#define UTEST_GUARD_SIZE ( 4 )
#define UTEST_ENCODED_DECODED_DATA_MAX_SIZE ( 1024 )

// Setup
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
TEST_GROUP( DZRCOBS ){
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
static uint8_t s_dzrcobs_datatest[] = {
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

// NOLINTBEGIN
TEST( DZRCOBS, MACRO_DZRCOBS_ENCODE_MAX )
// NOLINTEND
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

// NOLINTBEGIN
TEST( DZRCOBS, DecodeManual )
// NOLINTEND
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzrcobs_datatest;
	const uint8_t *pDatatest_end = s_dzrcobs_datatest + sizeof( s_dzrcobs_datatest );

	while( pDatatest < pDatatest_end )
	{
		const uint8_t decodeDataSize = *pDatatest++;
		const uint8_t *decodeData		 = pDatatest;
		pDatatest += decodeDataSize;

		const uint8_t encodedDataSize = *pDatatest++;
		const uint8_t *encodedData		= pDatatest;
		pDatatest += encodedDataSize;

		eDZRCOBS_ret ret		= DZRCOBS_RET_SUCCESS;
		size_t decodedLen		= 0;
		uint8_t *decodedPos = nullptr;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		CHECK_EQUAL( encodedDataSize, RCOBS_MAX_ENCODED_SIZE( decodeDataSize ) );

		ret = dzrcobs_decode( encodedData,
													encodedDataSize,
													buffer + UTEST_GUARD_SIZE,
													decodeDataSize, // used to test limit of the buffer
													&decodedLen,
													&decodedPos );

		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( decodeDataSize, decodedLen );
		CHECK_EQUAL( decodedPos, ( buffer + UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( decodeData, decodedPos, decodedLen ) );
	}
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeManual )
// NOLINTEND
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzrcobs_datatest;
	const uint8_t *pDatatest_end = s_dzrcobs_datatest + sizeof( s_dzrcobs_datatest );

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

		eDZRCOBS_ret ret	= DZRCOBS_RET_SUCCESS;
		size_t encodedLen = 0;
		sDZRCOBS_ctx ctx;

		ret = dzrcobs_encode_inc_begin( &ctx,
																		buffer + UTEST_GUARD_SIZE,
																		encodedDataSize // Used to test the limit
		);
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		ret = dzrcobs_encode_inc( &ctx, decodeData, decodeDataSize );
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		ret = dzrcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		CHECK_EQUAL( encodedDataSize, encodedLen );
		CHECK_EQUAL( 0, memcmp( encodedData, buffer + UTEST_GUARD_SIZE, encodedDataSize ) );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + encodedDataSize, &guard, UTEST_GUARD_SIZE ) );
	}
}

// NOLINTBEGIN
TEST( DZRCOBS, DecodeInvalidArgs )
// NOLINTEND
{
	eDZRCOBS_ret ret		= DZRCOBS_RET_SUCCESS;
	size_t decodedLen		= 0;
	uint8_t *decodedPos = nullptr;

	ret = dzrcobs_decode( buffer, 0, buffer + UTEST_GUARD_SIZE, 1, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( buffer, 1, buffer + UTEST_GUARD_SIZE, 1, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( nullptr, 2, buffer + UTEST_GUARD_SIZE, 1, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( buffer, 2, nullptr, 1, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( buffer, 2, buffer + UTEST_GUARD_SIZE, 1, nullptr, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( buffer, 2, buffer + UTEST_GUARD_SIZE, 1, &decodedLen, nullptr );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );
}

// NOLINTBEGIN
TEST( DZRCOBS, DecodeInvalidPayload )
// NOLINTEND
{
	eDZRCOBS_ret ret		= DZRCOBS_RET_SUCCESS;
	size_t decodedLen		= 0;
	uint8_t *decodedPos = nullptr;

	buffer[UTEST_GUARD_SIZE + 0] = 0;
	buffer[UTEST_GUARD_SIZE + 1] = 0;
	ret = dzrcobs_decode( buffer + UTEST_GUARD_SIZE, 2, buffer + UTEST_GUARD_SIZE, 1, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD, ret );

	buffer[UTEST_GUARD_SIZE + 0] = 0;
	buffer[UTEST_GUARD_SIZE + 1] = 1;
	ret = dzrcobs_decode( buffer + UTEST_GUARD_SIZE, 2, buffer + UTEST_GUARD_SIZE, 1, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD, ret );

	buffer[UTEST_GUARD_SIZE + 0] = 0;
	buffer[UTEST_GUARD_SIZE + 1] = 1;
	buffer[UTEST_GUARD_SIZE + 2] = 3;
	ret = dzrcobs_decode( buffer + UTEST_GUARD_SIZE, 3, buffer + UTEST_GUARD_SIZE, 2, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ENCODED_PAYLOAD, ret );

	buffer[UTEST_GUARD_SIZE + 0] = 1;
	buffer[UTEST_GUARD_SIZE + 1] = 1;
	buffer[UTEST_GUARD_SIZE + 2] = 4;
	ret = dzrcobs_decode( buffer + UTEST_GUARD_SIZE, 3, buffer + UTEST_GUARD_SIZE, 2, &decodedLen, &decodedPos );
	CHECK_EQUAL( DZRCOBS_RET_ERR_OVERFLOW, ret );
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeBeginInvalidArgs )
// NOLINTEND
{
	eDZRCOBS_ret ret = DZRCOBS_RET_SUCCESS;
	sDZRCOBS_ctx ctx;

	ret = dzrcobs_encode_inc_begin( &ctx, buffer, 0 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "buffer length <2 must fail" );

	ret = dzrcobs_encode_inc_begin( &ctx, buffer, 1 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "buffer length <2 must fail" );

	ret = dzrcobs_encode_inc_begin( &ctx, nullptr, 3 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );

	ret = dzrcobs_encode_inc_begin( nullptr, buffer, 3 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeIncInvalidArgs )
// NOLINTEND
{
	eDZRCOBS_ret ret = DZRCOBS_RET_SUCCESS;
	sDZRCOBS_ctx ctx;

	ret = dzrcobs_encode_inc( nullptr, buffer, 1 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );

	ret = dzrcobs_encode_inc( &ctx, nullptr, 1 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeEndInvalidArgs )
// NOLINTEND
{
	eDZRCOBS_ret ret = DZRCOBS_RET_SUCCESS;
	sDZRCOBS_ctx ctx;
	size_t sizeEncoded = 0;

	ret = dzrcobs_encode_inc_end( &ctx, nullptr );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );

	ret = dzrcobs_encode_inc_end( nullptr, &sizeEncoded );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeOverflow )
// NOLINTEND
{
	eDZRCOBS_ret ret = DZRCOBS_RET_SUCCESS;
	sDZRCOBS_ctx ctx;
	size_t sizeEncoded = 0;

	ret = dzrcobs_encode_inc_begin( &ctx, buffer + UTEST_GUARD_SIZE, 2 );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	uint8_t testBuffer[2] = { 0 };

	ret = dzrcobs_encode_inc( &ctx, testBuffer, 2 );
	CHECK_EQUAL( DZRCOBS_RET_ERR_OVERFLOW, ret );
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeFirstOKThenOverflow )
// NOLINTEND
{
	eDZRCOBS_ret ret = DZRCOBS_RET_SUCCESS;
	sDZRCOBS_ctx ctx;
	size_t sizeEncoded = 0;

	ret = dzrcobs_encode_inc_begin( &ctx, buffer + UTEST_GUARD_SIZE, 2 );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	uint8_t testBuffer[2] = { 0 };

	ret = dzrcobs_encode_inc( &ctx, testBuffer, 0 );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	ret = dzrcobs_encode_inc( &ctx, testBuffer, 1 );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	ret = dzrcobs_encode_inc( &ctx, testBuffer, 1 );
	CHECK_EQUAL( DZRCOBS_RET_ERR_OVERFLOW, ret );
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeLongBuffer )
// NOLINTEND
{
	eDZRCOBS_ret ret = DZRCOBS_RET_SUCCESS;
	sDZRCOBS_ctx ctx;
	size_t sizeEncoded = 0;

	static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																( UTEST_GUARD_BYTE << 0 );

	memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

	ret = dzrcobs_encode_inc_begin( &ctx, buffer + UTEST_GUARD_SIZE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	const size_t sizeToEncode = 512;

	uint8_t *decodedData = new uint8_t[UTEST_GUARD_SIZE + sizeToEncode + UTEST_GUARD_SIZE];

	for( size_t i = 0; i < sizeToEncode; i++ )
	{
		decodedData[i] = rand();
	}

	ret = dzrcobs_encode_inc( &ctx, decodedData + ( sizeToEncode / 2 ) * 0, sizeToEncode / 2 );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	ret = dzrcobs_encode_inc( &ctx, decodedData + ( sizeToEncode / 2 ) * 1, sizeToEncode / 2 );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	ret = dzrcobs_encode_inc_end( &ctx, &sizeEncoded );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

	uint8_t *decodedBuffer		= new uint8_t[UTEST_GUARD_SIZE + ( sizeToEncode + 1 ) + UTEST_GUARD_SIZE];
	uint8_t *decodedBufferPos = nullptr;

	size_t sizeDecoded = 0;
	ret								 = dzrcobs_decode( buffer + UTEST_GUARD_SIZE,
												 sizeEncoded,
												 decodedBuffer,
												 sizeToEncode,
												 &sizeDecoded,
												 &decodedBufferPos );
	CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

	CHECK_EQUAL( sizeToEncode, sizeDecoded );
	CHECK_EQUAL( decodedBuffer, decodedBufferPos );

	for( size_t i = 0; i < sizeToEncode; i++ )
	{
		if( decodedData[i] != decodedBufferPos[i] )
		{
			printf( "Failed at %lu, 0x%02X != 0x%02X\n", i, decodedData[i], decodedBufferPos[i] );
			break;
		}
	}
	CHECK_EQUAL( 0, memcmp( decodedData, decodedBufferPos, sizeDecoded ) );

	delete[] decodedBuffer;
	delete[] decodedData;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
