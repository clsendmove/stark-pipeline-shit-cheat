#include "../hooks.hpp"

#include <safetyhook.hpp>
#include "drawsystem/drawsystem.hpp"
#include <wrl.h>
#include <valve/sdk/shared.hpp>
std::once_flag init;
SafetyHookInline dx{};
template <typename T> using com_ptr = Microsoft::WRL::ComPtr<T>;


HRESULT api hooks::directx::present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND window,
    const RGNDATA* dirty) {
    std::call_once(init, [&] {
        D3DDEVICE_CREATION_PARAMETERS params;
        device->GetCreationParameters(&params);
        drawsystem->initialize(device, params.hFocusWindow);

        engine_client->get_screen_size(tf2.screenwidth, tf2.screenheight);
        });

    drawsystem->start_paint(device);
    {
        auto* list = ImGui::GetBackgroundDrawList();
        draw->paint_traverse(list);
        if (GetAsyncKeyState(VK_INSERT) & 1)
            config->gui.is_open = !config->gui.is_open;

        if (config->gui.is_open) {
            ImGui::Begin("test");
            ImGui::End();
        }
    }
    drawsystem->finish_paint(device);

    return dx.stdcall<HRESULT>(device, src, dest, window, dirty);
}

void hooks::directx::startup() {
    com_ptr<IDirect3D9>   d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    auto                  window = FindWindowA("Valve001", nullptr);
    D3DPRESENT_PARAMETERS params{
        .BackBufferFormat = D3DFMT_UNKNOWN,
        .SwapEffect = D3DSWAPEFFECT_DISCARD,
        .Windowed = TRUE,
        .EnableAutoDepthStencil = TRUE,
        .AutoDepthStencilFormat = D3DFMT_D16,
        .PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT,
    };

    com_ptr<IDirect3DDevice9> d3d_device{};

    d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params,
        &d3d_device);
    dx = safetyhook::create_inline(memory::vfunc_ptr(d3d_device.Get(), 17), reinterpret_cast<void*>(present));
    console.print("hooked present");

}

void hooks::directx::shutdown() {
    dx = {};
}