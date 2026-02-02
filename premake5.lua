---@diagnostic disable: undefined-global, undefined-field
require"vendor/premake-ecc/ecc"

workspace("imgsurf")
    configurations({ "debug", "release" })
    platforms({"linux", "windows"})
    location("build")
    architecture("x86_64")

project("imagesurf")
    language("C")
    cdialect("C23")
    warnings("Extra")
    targetname("imgsurf")
    kind("StaticLib")

    filter("configurations:debug")
        defines{"DEBUG"}
        staticruntime("off")
        runtime("debug")
        symbols("On")

    filter("configurations:release")
        defines{"NDEBUG"}
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        targetdir("bin/Linux_%{cfg.buildcfg}")
        objdir("obj/Linux_%{cfg.buildcfg}")
        files({ "./src/linux_imgsurf*",
                "./include/linux_imgsurf*",
                "./src/imgsurf_*",
                "./include/imgsurf_*" })
        includedirs({ "./include/", "/usr/include/"})
        linkoptions{"-fuse-ld=mold"}

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/Win64_%{cfg.buildcfg}")
        objdir("obj/Win64_%{cfg.buildcfg}")
        files({ "./src/win32_imgsurf*",
                "./include/win32_imgsurf*",
                "./src/imgsurf_*",
                "./include/imgsurf_*" })
        includedirs({ "./include/"})
        buildoptions{"/wd4068", "/wd4100"}
        ignoredefaultlibraries({ "MSVCRT" })
