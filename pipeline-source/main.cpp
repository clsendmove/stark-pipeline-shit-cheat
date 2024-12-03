
#include "../pipeline-include/typedefs.hpp"
#include "../pipeline-include/console/console.h"
#include "../pipeline-source/factory/factory.hpp"
#include "crashlog.hpp"


DWORD api pipeline(void* args) {
    console.spawn("pipeline");
    while (!GetModuleHandleA("GameUI.dll"))
		Sleep(15000);

    factory->startup();

    while (!GetAsyncKeyState(VK_F11) & 1) {
        Sleep(1000);
    }
    factory->shutdown();
    console.kill();
	FreeLibraryAndExitThread(static_cast<HMODULE>(args), EXIT_SUCCESS);
}

DWORD api exit() {
    factory->shutdown();
    return 0;
}

i32 api DllMain(HMODULE inst, DWORD reason, void* arg) {
    switch (reason) {
    case DLL_PROCESS_ATTACH: {
        Crashlog::Setup();

        auto thread = CreateThread(nullptr, 0, pipeline, inst, 0, nullptr);
        if (!thread)
            return 0;
        CloseHandle(thread);
        return 1;
    }
    case DLL_PROCESS_DETACH: {
        exit();
        return 1;
    }
    default:
        return 1;
    }
}