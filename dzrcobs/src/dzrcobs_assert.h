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
#ifndef _DZRCOBS_ASSERT_H_
#define _DZRCOBS_ASSERT_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////

#ifndef DZRCOBS_ASSERT

#ifdef IS_DEBUG_BUILD
#include <assert.h>
#define DZRCOBS_ASSERT( a ) assert( a )
#else
#define DZRCOBS_ASSERT( a )
#endif

#endif

#ifndef DZRCOBS_RUN_ONDEBUG

#ifdef IS_DEBUG_BUILD
#define DZRCOBS_RUN_ONDEBUG( a ) ( a )
#else
#define DZRCOBS_RUN_ONDEBUG( a )
#endif

#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
