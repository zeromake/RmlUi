add_rules("mode.debug", "mode.release")

option("backend")
    set_default("")
    set_showmenu(true)
option_end()

if is_plat("windows") then
    add_cxflags("/utf-8")
    set_languages("c++17")
else
    set_languages("c++14")
end

local tutorials = {
    "template",
    "drag",
}

local backends = {
    Win32_GL2 = {
        "Backends/RmlUi_Platform_Win32.cpp",
        "Backends/RmlUi_Renderer_GL2.cpp",
        "Backends/RmlUi_Backend_Win32_GL2.cpp",
    },
    Win32_VK = {
        "Backends/RmlUi_Platform_Win32.cpp",
        "Backends/RmlUi_Renderer_VK.cpp",
        "Backends/RmlUi_Backend_Win32_VK.cpp",
    },
    X11_GL2 = {
        "Backends/RmlUi_Platform_X11.cpp",
        "Backends/RmlUi_Renderer_GL2.cpp",
        "Backends/RmlUi_Backend_X11_GL2.cpp",
    },
    SDL_GL2 = {
        "Backends/RmlUi_Platform_SDL.cpp",
        "Backends/RmlUi_Renderer_GL2.cpp",
        "Backends/RmlUi_Backend_SDL_GL2.cpp"
    },
    SDL_GL3 = {
        "Backends/RmlUi_Platform_SDL.cpp",
        "Backends/RmlUi_Renderer_GL3.cpp",
        "Backends/RmlUi_Backend_SDL_GL3.cpp"
    },
    SDL_VK = {
        "Backends/RmlUi_Platform_SDL.cpp",
        "Backends/RmlUi_Renderer_VK.cpp",
        "Backends/RmlUi_Backend_SDL_VK.cpp"
    },
    SDL_SDLrenderer = {
        "Backends/RmlUi_Renderer_SDL.cpp",
        "Backends/RmlUi_Platform_SDL.cpp",
        "Backends/RmlUi_Backend_SDL_SDLrenderer.cpp"
    },
    SFML_GL2 = {
        "Backends/RmlUi_Platform_SFML.cpp",
        "Backends/RmlUi_Renderer_GL2.cpp",
        "Backends/RmlUi_Backend_SFML_GL2.cpp"
    },
    GLFW_GL2 = {
        "Backends/RmlUi_Platform_GLFW.cpp",
        "Backends/RmlUi_Renderer_GL2.cpp",
        "Backends/RmlUi_Backend_GLFW_GL2.cpp"
    },
    GLFW_GL3 = {
        "Backends/RmlUi_Platform_GLFW.cpp",
        "Backends/RmlUi_Renderer_GL3.cpp",
        "Backends/RmlUi_Backend_GLFW_GL3.cpp"
    },
    GLFW_VK = {
        "Backends/RmlUi_Platform_GLFW.cpp",
        "Backends/RmlUi_Renderer_VK.cpp",
        "Backends/RmlUi_Backend_GLFW_VK.cpp"
    }
}

local backend = get_config("backend")
if backend == "" then
    if is_plat("windows", "mingw") then
        backend = "Win32_GL2"
    elseif is_plat("macosx") then
        backend = "SDL_SDLrenderer"
    elseif is_plat("linux") then
        backend = "X11_GL2"
    end
end

add_requires("freetype", "sdl2", "sdl2_image", "glfw3", "lua", "lunasvg")

add_includedirs("Include")
add_includedirs("Backends")

add_defines("RMLUI_ENABLE_SVG_PLUGIN=1")

target("rmlui_core")
    set_kind("$(kind)")
    add_packages("freetype")
    add_files("Source/Core/*.cpp")
    add_files("Source/Core/Elements/*.cpp")
    add_files("Source/Core/Layout/*.cpp")
    add_files("Source/Core/FontEngineDefault/*.cpp")

target("rmlui_debugger")
    set_kind("$(kind)")
    add_files("Source/Debugger/*.cpp")

target("rmlui_backend")
    set_kind("$(kind)")
    add_packages("sdl2", "sdl2_image")
    for _, f in ipairs(backends[backend]) do
        add_files(f)
    end

target("rmlui_plugin_lua")
    set_kind("$(kind)")
    add_packages("lua")
    add_files("Source/Lua/**.cpp")

target("rmlui_plugin_lottie")
    set_kind("$(kind)")
    add_files("Source/Lottie/*.cpp")

target("rmlui_plugin_svg")
    set_kind("$(kind)")
    add_packages("lunasvg")
    add_files("Source/SVG/*.cpp")

target("rmlui_shell")
    set_kind("$(kind)")
    add_includedirs("Samples/shell/include")
    add_files("Samples/shell/src/*.cpp")


local samples = {
    "treeview",
    "customlog",
    "drag",
    "loaddocument",
    "transform",
    "bitmapfont",
    "animation",
    "benchmark",
    "demo",
    "databinding",
}
for _, sample in ipairs(samples) do
    target("rmlui_sample_"..sample)
        set_default(false)
        add_includedirs("Samples/shell/include")
        add_files("Samples/basic/"..sample.."/src/*.cpp")
        add_deps("rmlui_core", "rmlui_debugger", "rmlui_backend", "rmlui_plugin_svg", "rmlui_shell")
        set_rundir("$(projectdir)/Samples")
    target_end()
end
