// /////////////////////////////////////////////////////////////////////////////
///	@file dzrcobs_assert.h
///	@brief Assert declaration
///
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

// Includes
// /////////////////////////////////////////////////////////////////////////////

#ifndef DZRCOBS_ASSERT

#ifdef ASAP_IS_DEBUG_BUILD
#include <assert.h>
#define DZRCOBS_ASSERT( a ) assert( a )
#else
#define DZRCOBS_ASSERT( a )
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
