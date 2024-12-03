#include "factory.hpp"

void c_factory::startup() {
 

    if (GetModuleHandleA("dxvk_d3d9.dll")) {
        //g_console->print(icon_type::CON_ERROR, "Failed to load!");
        MessageBoxA(nullptr,
            "You are currently running with graphics options that pipeline does not support.\nCheck your "
            "launch arguments for -vulkan and try again.",
            "pipeline-error", MB_ICONERROR);
        failed = true;
        return;
    }



    Sleep(350);

   

    hook_manager->startup_hooks();
    
    configpath = std::filesystem::current_path().string() + ("\\pipeline");
    if (!std::filesystem::exists(configpath))
    {
        std::filesystem::create_directory(configpath);
    }


    print_on_inject(); // todo
   //surface->play_sound("hl1/fvox/activated.wav");


   
}

void c_factory::shutdown()
{
    if (failed)
        return;

   // ctx->menu_open = false;
    config->gui.is_open = false;
    Sleep(250);
    //g_hooks->shutdown_hooks();
    hook_manager->shutdown_hooks();
}

bool c_factory::should_unload()
{
	return false;
}

void c_factory::print_on_inject()
{
}

