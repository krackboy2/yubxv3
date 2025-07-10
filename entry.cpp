#include "Windows.h"
#include "thread"
#include <environment/environment.hpp>
#include <roblox/task_scheduler/task_scheduler.hpp>
#include <execution/execution.hpp>
#include <filesystem>
#include <fstream>
std::filesystem::path appdata = getenv("LOCALAPPDATA");
std::filesystem::path library = appdata / ("yubx");
std::filesystem::path autoexec = library / ("autoexec");
void disable_fflags()
{
    *reinterpret_cast<BYTE*>(update::fflags::EnableLoadModule) = TRUE;
    *reinterpret_cast<BYTE*>(update::fflags::DebugCheckRenderThreading) = FALSE;
    *reinterpret_cast<BYTE*>(update::fflags::RenderDebugCheckThreading2) = FALSE;
    *reinterpret_cast<BYTE*>(update::fflags::DisableCorescriptLoadstring) = FALSE;
    *reinterpret_cast<BYTE*>(update::fflags::LockViolationInstanceCrash) = FALSE;
    *reinterpret_cast<BYTE*>(update::fflags::LockViolationScriptCrash) = FALSE;
}
void patch_detection(void* targetFunc, size_t size) {
    DWORD op;
    VirtualProtect(targetFunc, size, PAGE_EXECUTE_READWRITE, &op);

    uint8_t* code = reinterpret_cast<uint8_t*>(targetFunc);
    for (size_t i = 0; i < size; ++i) {
        if (code[i] == 0xCC) {
            code[i] = 0x90;
        }
    }

    VirtualProtect(targetFunc, size, op, &op);
}
void executions() {
    if (!std::filesystem::is_directory(library))
        std::filesystem::create_directory(library);

    std::filesystem::path executeFile = library / ("execute.lua");

    if (!std::filesystem::exists(executeFile)) {
        FILE* f = fopen(executeFile.string().c_str(), ("w"));
        if (f)
            fclose(f);
    }

    std::filesystem::file_time_type lastFileTime = std::filesystem::last_write_time(executeFile);

    while (true) {
        std::filesystem::file_time_type ftime = std::filesystem::last_write_time(executeFile);
        if (ftime != lastFileTime) {
            lastFileTime = ftime;

            FILE* file = fopen(executeFile.string().c_str(), ("rb"));
            if (!file) continue;

            fseek(file, 0, SEEK_END);
            size_t fileSize = ftell(file);
            rewind(file);

            std::string script(fileSize, '\0');
            size_t bytesread = fread(&script[0], 1, fileSize, file);
            fclose(file);

            if (bytesread <= 2) continue;

            execution::execute_script(globals::yubx_state, script);
        }

        if (!std::filesystem::is_directory(autoexec))
            std::filesystem::create_directory(autoexec);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
std::atomic<uintptr_t> laststate{ 0 };
std::atomic<uintptr_t> lastplaceid{ 0 };
std::atomic<bool> teleportMonitoringActive{ true };
uintptr_t globalstates() {
    auto scriptcontext = task_scheduler::get_script_context();
    uintptr_t gstate = scriptcontext + update::offsets::luastate::global_state;
    return gstate;
}
void tphandler() {
    try {
        uintptr_t datamodel = task_scheduler::get_datamodel();
        uintptr_t cur_placeid = *(uintptr_t*)(datamodel + update::offsets::datamodel::place_id);

        lastplaceid.store(cur_placeid);
        laststate.store(globalstates());

        bool teleportedAway = false;

        while (teleportMonitoringActive.load()) {
            datamodel = task_scheduler::get_datamodel();
            cur_placeid = *(uintptr_t*)(datamodel + update::offsets::datamodel::place_id);

            if (cur_placeid == 0 || globalstates == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                continue;
            }

            if (cur_placeid != lastplaceid.load() || globalstates() != laststate.load()) {
                teleportedAway = true;
                lastplaceid.store(cur_placeid);
                laststate.store(globalstates());
            }
            else if (teleportedAway && cur_placeid == lastplaceid.load() && globalstates() == laststate.load()) {
                teleportedAway = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                disable_fflags();
                globals::global_state = (lua_State*)(task_scheduler::get_lua_state());
                globals::yubx_state = lua_newthread(globals::global_state);
                task_scheduler::set_thread_capabilities(globals::yubx_state, 8, max_caps);
                luaL_sandboxthread(globals::yubx_state);
                environment::initialize(globals::yubx_state);
                roblox::r_print(1, "yubx is ready for script execution!");
                if (std::filesystem::exists(autoexec) && std::filesystem::is_directory(autoexec)) {
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(autoexec)) {
                        if (std::filesystem::is_regular_file(entry)) {
                            std::ifstream file(entry.path());
                            if (file) {
                                std::string script((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                                execution::execute_script(globals::yubx_state, script);
                            }
                            else {}
                        }
                    }
                }
                else {}
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    catch (const std::exception& ex) { MessageBoxA(NULL, "uncaught error in 'tphandler'!", "Roblox", NULL); }
}

void enabletp() {
    patch_detection((void*)update::roblox::print, sizeof(update::roblox::print));
    uintptr_t datamodel = task_scheduler::get_datamodel();
    uintptr_t placeid = *(uintptr_t*)(datamodel + update::offsets::datamodel::place_id);
    if (!std::filesystem::is_directory(library))
        std::filesystem::create_directory(library);
    if (!std::filesystem::is_directory(autoexec))
        std::filesystem::create_directory(autoexec);
    if (placeid) {
        disable_fflags();
        globals::global_state = (lua_State*)(task_scheduler::get_lua_state());
        globals::yubx_state = lua_newthread(globals::global_state);
        task_scheduler::set_thread_capabilities(globals::yubx_state, 8, max_caps);
        luaL_sandboxthread(globals::yubx_state);
        environment::initialize(globals::yubx_state);
        roblox::r_print(1, "yubx is ready for script execution!");
    }
    std::thread(tphandler).detach();
    std::thread(executions).detach();
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID res)
{
    if (reason == DLL_PROCESS_ATTACH)
        std::thread(enabletp).detach();
    return TRUE;
}