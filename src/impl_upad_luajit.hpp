/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __IMPL_UPAD_LUAJIT_HPP__
#define __IMPL_UPAD_LUAJIT_HPP__

#ifdef UPAD_LUA_PRECOMPILE
#define USE_LUAJIT
#endif

#ifdef USE_LUAJIT

extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

#endif

#endif
