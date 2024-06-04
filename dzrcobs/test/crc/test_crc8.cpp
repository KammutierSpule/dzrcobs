// /////////////////////////////////////////////////////////////////////////////
///	@file test_crc.cpp
///	@brief Tests for CRC8
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
#include <../src/crc8.h>
#include <CppUTest/TestHarness.h>
#include <CppUTest/UtestMacros.h>
#include <cstdint>

// Definitions
// /////////////////////////////////////////////////////////////////////////////

// Setup
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
TEST_GROUP( DZRCOBS_CRC ){
	void setup()
	{
	}

	void teardown()
	{
	}
};
// NOLINTEND
// clang-format on

// Tests
// /////////////////////////////////////////////////////////////////////////////

// http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

// NOLINTBEGIN
TEST( DZRCOBS_CRC, CRC8_0x00_0xFF_0x00_0xFF )
// NOLINTEND
{
	uint8_t crc = DZRCOBS_CRC_INIT_VAL;

	crc = DZRCOBS_CRC( crc, 0x00 );
	CHECK_EQUAL( 0x4A, crc );

	crc = DZRCOBS_CRC( crc, 0xFF );
	CHECK_EQUAL( 0x5A, crc );

	crc = DZRCOBS_CRC( crc, 0x00 );
	CHECK_EQUAL( 0x7E, crc );

	crc = DZRCOBS_CRC( crc, 0xFF );
	CHECK_EQUAL( 0x9A, crc );

	crc = DZRCOBS_CRC( crc, 0x9A );
	CHECK_EQUAL( 0x00, crc );
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
