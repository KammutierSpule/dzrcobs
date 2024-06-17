// /////////////////////////////////////////////////////////////////////////////
///	@file test_dzrcobs.cpp
///	@brief
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
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <dzrcobs/dzrcobs.h>
#include <dzrcobs/dzrcobs_decode.h>

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
static uint8_t s_dzrcobs_datatest_plainencoding[] = {
	// 1
	1, 'A',				// decoded
	2 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 0x02, 0xFC /*Encoding*/, 0x54 /*CRC8*/,	// encoded
	// 2
	4, 'A', 'B', 'C', 'D',
	5 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 'B', 'C', 'D', 0x05, 0xFC /*Encoding*/, 0x58 /*CRC8*/,	// encoded
	// 3
	4, 'A', 'B', 0x00, 'C',
	5 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 'B', 0x03, 'C', 0x02, 0xFC /*Encoding*/, 0x86 /*CRC8*/,	// encoded
	// 4
	7, 'A', 0x00, 0x00, 0x00, 'B', 'C', 'D',
	8 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 0x02, 0x01, 0x01, 'B', 'C', 'D', 0x04, 0xFC /*Encoding*/, 0xAC /*CRC8*/,	// encoded
};

// NOLINTEND
// clang-format on

// Tests
// /////////////////////////////////////////////////////////////////////////////

// NOLINTBEGIN
TEST( DZRCOBS, MacroEncodeMax )
// NOLINTEND
{
	// DZCOBS requires a minimum of 1 byte overhead,
	// and a maximum of ⌈n/126⌉ bytes for n data bytes (one byte in 126, rounded
	// up)
	CHECK_EQUAL( 1, DZRCOBS_MAX_ENCODED_SIZE( 0 ) );
	CHECK_EQUAL( 1 + 1, DZRCOBS_MAX_ENCODED_SIZE( 1 ) );
	CHECK_EQUAL( 126 + 1, DZRCOBS_MAX_ENCODED_SIZE( 126 ) );
	CHECK_EQUAL( 127 + 2, DZRCOBS_MAX_ENCODED_SIZE( 127 ) );
	CHECK_EQUAL( ( 126 * 2 ) + ( 1 * 2 ), DZRCOBS_MAX_ENCODED_SIZE( 126 * 2 ) );
	CHECK_EQUAL( ( 126 * 2 + 1 ) + ( 1 * 2 ), DZRCOBS_MAX_ENCODED_SIZE( 126 * 2 ) + 1 );
}

// NOLINTBEGIN
TEST( DZRCOBS, DecodeManual )
// NOLINTEND
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzrcobs_datatest_plainencoding;
	const uint8_t *pDatatest_end = s_dzrcobs_datatest_plainencoding + sizeof( s_dzrcobs_datatest_plainencoding );

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

		CHECK_EQUAL( encodedDataSize, DZRCOBS_MAX_ENCODED_SIZE( decodeDataSize ) + DZRCOBS_FRAME_HEADER_SIZE );

		sDZRCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= encodedData;
		decodeCtx.srcBufEncodedLen	= encodedDataSize;
		decodeCtx.dstBufDecoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = decodeDataSize; // used to test limit of the buffer

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzrcobs_decode( &decodeCtx, &decodedLen, &decodedPos, &user6bitDataRightAlgn );

		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( 0x3F, user6bitDataRightAlgn );
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
	const uint8_t *pDatatest		 = s_dzrcobs_datatest_plainencoding;
	const uint8_t *pDatatest_end = s_dzrcobs_datatest_plainencoding + sizeof( s_dzrcobs_datatest_plainencoding );

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

		CHECK_EQUAL( encodedDataSize, DZRCOBS_MAX_ENCODED_SIZE( decodeDataSize ) + DZRCOBS_FRAME_HEADER_SIZE );

		eDZRCOBS_ret ret	= DZRCOBS_RET_SUCCESS;
		size_t encodedLen = 0;
		sDZRCOBS_ctx ctx;

		ret = dzrcobs_encode_inc_begin( &ctx,
																		DZRCOBS_PLAIN,
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

	uint8_t user6bitDataRightAlgn = 0;

	sDZRCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer;
	decodeCtx.srcBufEncodedLen	= 1;
	decodeCtx.dstBufDecoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = 1;

	ret = dzrcobs_decode( &decodeCtx, &decodedLen, &decodedPos, NULL );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( &decodeCtx, &decodedLen, NULL, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( &decodeCtx, NULL, &decodedPos, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( NULL, &decodedLen, &decodedPos, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeBeginInvalidArgs )
// NOLINTEND
{
	eDZRCOBS_ret ret = DZRCOBS_RET_SUCCESS;
	sDZRCOBS_ctx ctx;

	ret = dzrcobs_encode_inc_begin( &ctx, DZRCOBS_PLAIN, buffer, 0 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "buffer length <2 must fail" );

	ret = dzrcobs_encode_inc_begin( &ctx, DZRCOBS_PLAIN, buffer, 1 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "buffer length <2 must fail" );

	ret = dzrcobs_encode_inc_begin( &ctx, DZRCOBS_PLAIN, nullptr, 3 );
	CHECK_EQUAL_TEXT( DZRCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );

	ret = dzrcobs_encode_inc_begin( nullptr, DZRCOBS_PLAIN, buffer, 3 );
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

// EOF
// /////////////////////////////////////////////////////////////////////////////
