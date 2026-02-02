---@diagnostic disable: undefined-global, undefined-field
require"vendor/premake-ecc/ecc"

workspace("imgsurf")
    configurations({ "debug", "release" })
    platforms({"linux", "windows"})
    location("build")
    architecture("x86_64")

project("imagesurf library")
    language("C")
    cdialect("C23")
    warnings("Extra")
    targetname("imgsurf")
    kind("StaticLib")

    filter("configurations:debug")
        defines{"DEBUG"}
        runtime("debug")
        symbols("On")
        optimize("Off")

    filter("configurations:release")
        defines{"NDEBUG"}
        staticruntime("off")
        runtime("release")
        symbols("Off")
        optimize("Speed")

    filter("platforms:Linux")
        system("Linux")
        defines("BUILD_LINUX")
        targetdir("bin/imgsurf_linux/%{cfg.buildcfg}")
        objdir("obj/imgsurf/%{cfg.buildcfg}")
        files({ "./src/linux_imgsurf*",
                "./include/linux_imgsurf*",
                "./src/imgsurf_*",
                "./include/imgsurf_*" })
        includedirs({ "./include/", "/usr/include/"})
        linkoptions("-fuse-ld=mold")
        toolset("clang")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/imgsurf_win64/%{cfg.buildcfg}")
        objdir("obj/")
        files({ "./src/win32_imgsurf*",
                "./include/win32_imgsurf*",
                "./src/imgsurf_*",
                "./include/imgsurf_*" })
        includedirs({ "./include/"})
        buildoptions{"/wd4068", "/wd4100"}
        ignoredefaultlibraries({ "MSVCRT" })

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address", "-fno-omit-frame-pointer", "-static-libasan"})
        linkoptions({"-gfull",  "-O0", "-fsanitize=address", "-fno-omit-frame-pointer", "-static-libasan"})

project("imagesurf unit tests")
    language("C")
    cdialect("C23")
    warnings("Extra")
    targetname("imgsurftest")
    kind("ConsoleApp")

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
        targetdir("bin/imgsurftest_linux/%{cfg.buildcfg}")
        objdir("obj/")
        files({ "./src/linux_imgsurf*",
                "./include/linux_imgsurf*",
                "./src/imgsurf*",
                "./include/imgsurf*" })
        includedirs({ "./include/", "/usr/include/"})
        buildoptions({"-g"})
        linkoptions({"-fuse-ld=mold"})
        toolset("clang")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/imgsurftest_win64/%{cfg.buildcfg}")
        objdir("obj/")
        files({ "./src/win32_imgsurf*",
                "./include/win32_imgsurf*",
                "./src/imgsurf*",
                "./include/imgsurf*" })
        includedirs({ "./include/"})
        buildoptions{"/wd4068", "/wd4100"}
        ignoredefaultlibraries({ "MSVCRT" })

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address", "-fno-omit-frame-pointer", "-static-libasan"})
        linkoptions({"-gfull",  "-O0", "-fsanitize=address", "-fno-omit-frame-pointer", "-static-libasan"})
