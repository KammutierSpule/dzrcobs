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
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
/// Dictionary string, descendent order, null terminated
static const char s_TEST_Dictionary1[] =
	DICT_ADD_WORD(2, "\x01\x01")
	DICT_ADD_WORD(3, "\x02\x00\x02")
	DICT_ADD_WORD(4, "\x03\x00\x00\x03")
	DICT_ADD_WORD(5, "\x04\x00\x00\x00\x04")
;
// NOLINTEND
// clang-format on

const size_t s_TEST_Dictionary1_size = sizeof( s_TEST_Dictionary1 );

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
	// 0
	1, 'A',				// decoded
	2 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 0x02, 0xFC /*Encoding*/, 0x54 /*CRC8*/,	// encoded
	// 1
	4, 'A', 'B', 'C', 'D',
	5 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 'B', 'C', 'D', 0x05, 0xFC /*Encoding*/, 0x58 /*CRC8*/,	// encoded
	// 2
	4, 'A', 'B', 0x00, 'C',
	5 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 'B', 0x03, 'C', 0x02, 0xFC /*Encoding*/, 0x86 /*CRC8*/,	// encoded
	// 3
	7, 'A', 0x00, 0x00, 0x00, 'B', 'C', 'D',
	8 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 0x02, 0x01, 0x01, 'B', 'C', 'D', 0x04, 0xFC /*Encoding*/, 0xAC /*CRC8*/,	// encoded
	// 4
	1, 0x00,				// decoded
	1 + DZRCOBS_FRAME_HEADER_SIZE, 0x01, 0xFC /*Encoding*/, 0x6E /*CRC8*/,	// encoded
};

#define TEST_USERBITS (0x3F)

// Encoded is using s_TEST_Dictionary1
static uint8_t s_dzrcobs_datatest_dictionary[] = {
	// 0
	2, 0x01, 0x01,				// decoded
	1 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x94 /*CRC8*/,	// encoded
	// 1
	4, 0x01, 0x01, 0x01, 0x01,				// decoded
	2 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x44 /*CRC8*/,	// encoded
	// 2
	5, 0x12, 0x01, 0x01, 0x01, 0x01,				// decoded
	4 + DZRCOBS_FRAME_HEADER_SIZE, 0x12, 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 0x80 + 0, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xB2 /*CRC8*/,	// encoded
	// 3
	6, 0x12, 0x01, 0x01, 0x23, 0x01, 0x01,				// decoded
	6 + DZRCOBS_FRAME_HEADER_SIZE, 0x12, 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 0x80 + 0, 0x23, 0x02 | DZRCOBS_NEXTCODE_IS_DICTIONARY, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x1E /*CRC8*/,	// encoded
	// 4
	7, 0x12, 0x01, 0x01, 0x23, 0x02, 0x00, 0x02,				// decoded
	6 + DZRCOBS_FRAME_HEADER_SIZE, 0x12, 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 0x80 + 0, 0x23, 0x02 | DZRCOBS_NEXTCODE_IS_DICTIONARY, 0x80 + 1, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x66 /*CRC8*/,	// encoded
	// 5
	7, 0x12, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02,				// decoded
	5 + DZRCOBS_FRAME_HEADER_SIZE, 0x12, 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 0x80 + 0, 0x01, 0x80 + 1, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x9E /*CRC8*/,	// encoded
	// 6
	1, 0x00,				// decoded
	1 + DZRCOBS_FRAME_HEADER_SIZE, 0x01 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xC8 /*CRC8*/,	// encoded
	// 7
	2, 0x00, 0x00,	// decoded
	2 + DZRCOBS_FRAME_HEADER_SIZE, 0x01 | DZRCOBS_NEXTCODE_IS_ZERO, 0x01 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xFE /*CRC8*/,	// encoded
	// 8
	9, 0x12, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02, 0x12, 0x00,		// decoded
	8 + DZRCOBS_FRAME_HEADER_SIZE, 0x12, 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 0x80 + 0, 0x01, 0x80 + 1, 0x12, 0x02 | DZRCOBS_NEXTCODE_IS_DICTIONARY, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xE8 /*CRC8*/,	// encoded
	// 9
	4, 0x01, 0x01, 0x12, 0x00,		// decoded
	4 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x12, 0x02 | DZRCOBS_NEXTCODE_IS_DICTIONARY, 0x01 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xD2 /*CRC8*/,	// encoded
	// 10
	4, 'A', 'B', 0x00, 'C',		// decoded
	5 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 'B', 0x03, 'C', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x20 /*CRC8*/,	// encoded
	// 11
	5, 'A', 0x00, 'B', 0x00, 'C',		// decoded
	6 + DZRCOBS_FRAME_HEADER_SIZE, 'A', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 'B', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 'C', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xC0 /*CRC8*/,	// encoded
	// 12
	4, 0x01, 0x01, 0x00, 'C',		// decoded
	3 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 'C', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x2E /*CRC8*/,	// encoded
	// 13
	3, 0x01, 0x01, 'C',		// decoded
	3 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 'C', 0x02 | DZRCOBS_NEXTCODE_IS_DICTIONARY, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x3C /*CRC8*/,	// encoded
	// 14
	6, 0x01, 0x01, 0x00, 'A', 0x00, 'B',		// decoded
	5 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 'A', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO,'B', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xC0 /*CRC8*/,	// encoded
	// 15
	8, 0x01, 0x01, 0x00, 'A', 0x01, 0x01, 0x00, 'B',		// decoded
	6 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 'A', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, 0x80 + 0, 'B', 0x02 | DZRCOBS_NEXTCODE_IS_ZERO, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x96 /*CRC8*/,	// encoded
	// 16
	5, 0x01, 0x01, 0x00, 0x01, 0x01,		// decoded
	3 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x01, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xEE /*CRC8*/,	// encoded
	// 17
	4, 0x01, 0x01, 'C', 0x00,		// decoded
	4 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 'C', 0x02 | DZRCOBS_NEXTCODE_IS_DICTIONARY, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x1C /*CRC8*/,	// encoded
	// 18
	6, 0x01, 0x01, 'C', 0x01, 0x01, 0x00,		// decoded
	5 + DZRCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 'C', 0x02 | DZRCOBS_NEXTCODE_IS_DICTIONARY, 0x80 + 0, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x26 /*CRC8*/,	// encoded
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

		sDZRCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= encodedData;
		decodeCtx.srcBufEncodedLen	= encodedDataSize;
		decodeCtx.dstBufDecoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = std::max<uint8_t>( decodeDataSize, 4 ); // used to test limit of the buffer

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzrcobs_decode( &decodeCtx, &decodedLen, &decodedPos, &user6bitDataRightAlgn );

		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( 0x3F, user6bitDataRightAlgn );
		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( decodeDataSize, decodedLen );
		CHECK_COMPARE( decodedPos, >=, ( buffer + UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( decodeData, decodedPos, decodedLen ) );
		idx++;
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

		eDZRCOBS_ret ret	= DZRCOBS_RET_SUCCESS;
		size_t encodedLen = 0;
		sDZRCOBS_ctx ctx;

		ret = dzrcobs_encode_inc_begin( &ctx,
																		DZRCOBS_PLAIN,
																		buffer + UTEST_GUARD_SIZE,
																		std::max<uint8_t>( encodedDataSize, 4 ) // used to test limit of the buffer
		);
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		ctx.user6bits = TEST_USERBITS;

		ret = dzrcobs_encode_inc( &ctx, decodeData, decodeDataSize );
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		ret = dzrcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		CHECK_EQUAL( encodedDataSize, encodedLen );
		CHECK_EQUAL( 0, memcmp( encodedData, buffer + UTEST_GUARD_SIZE, encodedDataSize ) );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + encodedDataSize, &guard, UTEST_GUARD_SIZE ) );
		idx++;
	}
}

// NOLINTBEGIN
TEST( DZRCOBS, EncodeDictionaryManual )
// NOLINTEND
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzrcobs_datatest_dictionary;
	const uint8_t *pDatatest_end = s_dzrcobs_datatest_dictionary + sizeof( s_dzrcobs_datatest_dictionary );

	sDICT_ctx m_dictCtx;

	eDICT_ret ret = DZRCOBS_Dictionary_Init( &m_dictCtx, s_TEST_Dictionary1, s_TEST_Dictionary1_size );
	CHECK_EQUAL( DICT_RET_SUCCESS, ret );

	sDZRCOBS_ctx ctx;
	memset( &ctx, 0, sizeof( sDZRCOBS_ctx ) );

	dzrcobs_encode_set_dictionary( &ctx, &m_dictCtx, DZRCOBS_USING_DICT_1 );
	ctx.user6bits = TEST_USERBITS;

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

		CHECK_TRUE( encodedDataSize <= DZRCOBS_MAX_ENCODED_SIZE( decodeDataSize ) + DZRCOBS_FRAME_HEADER_SIZE );

		eDZRCOBS_ret ret	= DZRCOBS_RET_SUCCESS;
		size_t encodedLen = 0;

		ret = dzrcobs_encode_inc_begin(
		 &ctx,
		 DZRCOBS_USING_DICT_1,
		 buffer + UTEST_GUARD_SIZE,
		 std::max<uint8_t>( DZRCOBS_MAX_ENCODED_SIZE( decodeDataSize ) + DZRCOBS_FRAME_HEADER_SIZE,
												4 ) // Used to test the limit
		);
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		ret = dzrcobs_encode_inc( &ctx, decodeData, decodeDataSize );
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		ret = dzrcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );

		printf( "idx: %lu, encodedDataSize: %u, encodedLen: %lu\n", idx, encodedDataSize, encodedLen );
		printf( "deco:" );

		for( size_t i = 0; i < decodeDataSize; i++ )
		{
			printf( " 0x%02X, ", decodeData[i] );
		}
		printf( "\n" );
		printf( "good:" );

		for( size_t i = 0; i < encodedDataSize; i++ )
		{
			printf( " 0x%02X, ", encodedData[i] );
		}
		printf( "\n" );
		printf( "goot:" );

		for( size_t i = 0; i < encodedLen; i++ )
		{
			printf( " 0x%02X, ", buffer[i + UTEST_GUARD_SIZE] );
		}
		printf( "\n" );

		CHECK_EQUAL( encodedDataSize, encodedLen );
		CHECK_EQUAL( 0, memcmp( encodedData, buffer + UTEST_GUARD_SIZE, encodedDataSize ) );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + encodedDataSize, &guard, UTEST_GUARD_SIZE ) );
		idx++;
	}
}

// NOLINTBEGIN
TEST( DZRCOBS, DecodedictionaryManual )
// NOLINTEND
{
	sDICT_ctx m_dictCtx;

	eDICT_ret ret = DZRCOBS_Dictionary_Init( &m_dictCtx, s_TEST_Dictionary1, s_TEST_Dictionary1_size );
	CHECK_EQUAL( DICT_RET_SUCCESS, ret );

	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzrcobs_datatest_dictionary;
	const uint8_t *pDatatest_end = s_dzrcobs_datatest_dictionary + sizeof( s_dzrcobs_datatest_dictionary );

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

		sDZRCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= encodedData;
		decodeCtx.srcBufEncodedLen	= encodedDataSize;
		decodeCtx.dstBufDecoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = decodeDataSize; // used to test limit of the buffer
		decodeCtx.pDict[0]					= &m_dictCtx;

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzrcobs_decode( &decodeCtx, &decodedLen, &decodedPos, &user6bitDataRightAlgn );

		CHECK_EQUAL( DZRCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( 0x3F, user6bitDataRightAlgn );
		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( decodeDataSize, decodedLen );
		CHECK_COMPARE( decodedPos, >=, ( buffer + UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( decodeData, decodedPos, decodedLen ) );
		idx++;
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

	ret = dzrcobs_decode( &decodeCtx, &decodedLen, &decodedPos, nullptr );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( &decodeCtx, &decodedLen, nullptr, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( &decodeCtx, nullptr, &decodedPos, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZRCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzrcobs_decode( nullptr, &decodedLen, &decodedPos, &user6bitDataRightAlgn );
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
