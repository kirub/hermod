workspace "hermod"
	configurations { "Debug", "Release" }
	platforms { "Win32", "x64" }
	location "../build"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x86"

	filter { "platforms:x64" }
		system "Windows"
		architecture "x86_64"
		
		
	os.execute("build_gtest.bat")

project "hermod"
	kind "SharedLib"
	language "C++"
	cppdialect "C++latest"
	targetdir "../libs/%{cfg.buildcfg}"
	objdir ("../build")
	includedirs { "../include" } 
    defines { "DLL_EXPORTS" }


	files { "../include/**.h", "../include/**.inl", "../src/**.cpp" }
	removefiles {"../tests/**"}
   
	links { "ws2_32" }

	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "On"
		targetsuffix "-d"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	  
project "tests"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++latest"
	location "../build"
	targetdir "../bin/%{cfg.buildcfg}"
	debugdir "../libs/%{cfg.buildcfg}"
	objdir ("../build")
	includedirs { "../include",  "../libs/googletest/googletest/include" } 
	libdirs { "../libs/%{cfg.buildcfg}", "../libs/googletest/build/lib/%{cfg.buildcfg}" }

	files { "../tests/**.h", "../tests/**.cpp" }
	removefiles {"../tests/main.cpp", "../tests/framework.cpp", "../tests/unit_serialization.h" , "../tests/unit_fragments.h", "../tests/framework.h"}

	links { "hermod", "gtest", "gtest_main" }

	filter "configurations:Debug"
		defines { "DEBUG", "WITH_TESTS=1" }
		symbols "On"	
		postbuildcommands {
			"{COPYFILE} ../libs/%{cfg.buildcfg}/hermod-d.dll %[%{!cfg.targetdir}]",
			"{COPYFILE} ../libs/googletest/build/bin/%{cfg.buildcfg}/gtest.dll %[%{!cfg.targetdir}]",
			"{COPYFILE} ../libs/googletest/build/bin/%{cfg.buildcfg}/gtest_main.dll %[%{!cfg.targetdir}]"
		}

	filter "configurations:Release"
		defines { "NDEBUG", "WITH_TESTS=1" }
		optimize "On"
		postbuildcommands {
			"{COPYFILE} ../libs/%{cfg.buildcfg}/hermod.dll %[%{!cfg.targetdir}]",
			"{COPYFILE} ../libs/googletest/build/bin/%{cfg.buildcfg}/gtest.dll %[%{!cfg.targetdir}]",
			"{COPYFILE} ../libs/googletest/build/bin/%{cfg.buildcfg}/gtest_main.dll %[%{!cfg.targetdir}]"
		}