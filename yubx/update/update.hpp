// YuB-X Version: 2.0.8
// Roblox Version: version-765338e04cf54fde
// Dump Time:      2025-07-09 22:43:32

#pragma once
#include <cstdint>
#include <string>
#include <Windows.h>
#include <iostream>

#include "shuffles-encryptions.hpp"

struct lua_State;
#define xreb(x) (x + (uintptr_t)(GetModuleHandle(nullptr)))

namespace update
{
    namespace roblox
    {
        const uintptr_t print = xreb(0x14AD380);
        const uintptr_t OpcodeLookupTable = xreb(0x5159b40);

        const uintptr_t task_scheduler = xreb(0x63FA88C);
        const uintptr_t fake_datamodel = xreb(0x682B928);
        const uintptr_t luad_throw = xreb(0x2653370);
        const uintptr_t task_defer = xreb(0xFFA6A0);
    }

    namespace lua
    {
        const uintptr_t luao_nilobject = xreb(0x4740F38);
        const uintptr_t luau_execute = xreb(0x2685CE0);
        const uintptr_t luah_dummynode = xreb(0x4740958);
    }

    namespace lua_state
    {
        const uintptr_t GetGlobalState = xreb(0xDD2460);
    }

    namespace fflags
    {
        const uintptr_t EnableLoadModule = xreb(0x5B44AC8);
        const uintptr_t DebugCheckRenderThreading = xreb(0x5B6E3D0);
        const uintptr_t RenderDebugCheckThreading2 = xreb(0x5B99730);
        const uintptr_t DisableCorescriptLoadstring = xreb(0x5B44AA8);
        const uintptr_t LockViolationInstanceCrash = xreb(0x5B4F670);
        const uintptr_t LockViolationScriptCrash = xreb(0x5B447E0);
        const uintptr_t WndProcessCheck = xreb(0x5B33A30);
        const uintptr_t LuaStepIntervalMsOverrideEnabled = xreb(0x5B47200);
        const uintptr_t GetFastFlag = xreb(0x38992F0);
        const uintptr_t SetFastFlag = xreb(0x3899D80);
    }

    namespace offsets
    {
        namespace datamodel
        {
            const uintptr_t fake_datamodel_to_datamodel = 0x1B8;
            const uintptr_t script_context = 0x3B0;
            const uintptr_t place_id = 0x1A0;
        }

        namespace luastate
        {
            const uintptr_t global_state = 0x140;
            const uintptr_t decrypt_state = 0x88;
        }

        namespace instance
        {
            const uintptr_t name = 0x78;
            const uintptr_t children = 0x80;
        }
    }
}

namespace roblox
{
    using print_func_t = int(__fastcall*)(int, const char*, ...);
    inline print_func_t r_print = reinterpret_cast<print_func_t>(update::roblox::print);

    using luad_throw_t = void(__fastcall*)(lua_State*, int);
    inline luad_throw_t luad_throw = reinterpret_cast<luad_throw_t>(update::roblox::luad_throw);

    using task_defer_t = uintptr_t(__fastcall*)(int64_t);
    inline task_defer_t task_defer = reinterpret_cast<task_defer_t>(update::roblox::task_defer);

    inline auto GetState = (uintptr_t(__fastcall*)(int64_t, uint64_t*, uint64_t*))update::lua_state::GetGlobalState;
}
