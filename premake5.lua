---@diagnostic disable: undefined-global, undefined-field
require"vendor/premake-ecc/ecc"

workspace("imgsurf")
    configurations({ "debug", "release" })
    platforms({"linux", "windows"})
    location("build")
    architecture("x86_64")

project("imagesurf library")
    language("C")
    cdialect("C99")
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
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/imgsurf")
        files({ "./src/linux_imgsurf*",
                "./include/linux_imgsurf*",
                "./src/imgsurf_*",
                "./include/imgsurf_*" })
        includedirs({ "./include/", "/usr/include/"})
        buildoptions({"-Wextra", "-Wall", "-Wpedantic"})
        linkoptions("-fuse-ld=mold")
        toolset("clang")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/")
        files({ "./src/win32_imgsurf*",
                "./include/win32_imgsurf*",
                "./src/imgsurf_*",
                "./include/imgsurf_*" })
        includedirs({ "./include/"})

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                      "-static-libasan"})
        linkoptions({"-gfull", "-O0", "-fsanitize=address,leak,undefined", "-fno-omit-frame-pointer",
                     "-static-libasan"})

    filter({"platforms:Windows", "configurations:debug"})
        editandcontinue("Off")
        debugformat("c7")
        buildoptions({"/fsanitize=address"})

    filter({"platforms:Windows", "configurations:release"})
        linkoptions({"/NODEFAULTLIB:MSVCRTD"})

project("imagesurf unit tests")
    language("C")
    cdialect("C99")
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
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/imgsurftest/")
        files({ "./src/linux_imgsurftest*",
                "./include/linux_imgsurftest*",
                "./src/imgsurftest*",
                "./include/imgsurftest*" })
        includedirs({ "./include/", "/usr/include/"})
        libdirs("./bin/%{cfg.buildcfg}/")
        buildoptions({"-Wextra", "-Wall", "-Wpedantic"})
        links("imgsurf:static")
        linkoptions({"-fuse-ld=mold"})
        toolset("clang")

    filter("platforms:Windows")
        system("Windows")
        defines("BUILD_WINDOWS")
        targetdir("bin/%{cfg.buildcfg}")
        objdir("obj/")
        files({ "./src/win32_imgsurf*",
                "./include/win32_imgsurf*",
                "./src/imgsurf*",
                "./include/imgsurf*" })
        includedirs({"./include/"})
        libdirs("./bin/%{cfg.buildcfg}/")
        links("imgsurf.lib")

    filter({"platforms:Linux", "configurations:debug"})
        buildoptions({"-gfull", "-O0", "-fsanitize=address", "-fno-omit-frame-pointer", "-static-libasan"})
        linkoptions({"-gfull",  "-O0", "-fsanitize=address", "-fno-omit-frame-pointer", "-static-libasan"})

    filter({"platforms:Windows", "configurations:debug"})
        editandcontinue("Off")
        debugformat("c7")
        buildoptions({"/fsanitize=address"})

    filter({"platforms:Windows", "configurations:release"})
        linkoptions({"/NODEFAULTLIB:MSVCRTD"})
