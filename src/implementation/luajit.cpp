// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
