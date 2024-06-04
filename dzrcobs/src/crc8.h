// /////////////////////////////////////////////////////////////////////////////
///	@file crc8_0xA6.cpp
///	@brief Implement CRC table for crc8 0xA6 polymonial
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
#ifndef _DZRCOBS_CRC8_H_
#define _DZRCOBS_CRC8_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

// Definitions
// /////////////////////////////////////////////////////////////////////////////
#ifndef DZRCOBS_CRC_TABLE
#define DZRCOBS_CRC_TABLE 1
#endif

#define DZRCOBS_CRC_INIT_VAL ( 0xFF )

// Declarations
// /////////////////////////////////////////////////////////////////////////////
#if DZRCOBS_CRC_TABLE == 1
extern const uint8_t G_CRC8_0xA6[256];
#define DZRCOBS_CRC( crc, newbyte ) G_CRC8_0xA6[(uint8_t)( (uint8_t)( crc ) ^ (uint8_t)( newbyte ) )]
#else
#error No CRC mode defined. Define: DZRCOBS_CRC_TABLE
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
