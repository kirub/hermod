workspace "hermod"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x86"

	filter { "platforms:Win64" }
		system "Windows"
		architecture "x86_64"

project "hermod"
	kind "SharedLib"
	language "C++"
	cppdialect "C++latest"
	targetdir "libs/%{cfg.buildcfg}"
	objdir ("build")
	includedirs { "include" } 
    defines { "DLL_EXPORTS" }


	files { "include/**.h", "src/**.cpp" }
	removefiles {"tests/**"}
   
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
	targetdir "bin/%{cfg.buildcfg}"
	debugdir "libs/%{cfg.buildcfg}"
	objdir ("build")
	includedirs { "include" } 
	libdirs { "libs/%{cfg.buildcfg}" }

	files { "tests/**.h", "tests/**.cpp" }

	links { "hermod" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"	
		postbuildcommands {
			"{COPYFILE} libs/%{cfg.buildcfg}/hermod-d.dll %[%{!cfg.targetdir}]"
		}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		postbuildcommands {
			"{COPYFILE} libs/%{cfg.buildcfg}/hermod.dll %[%{!cfg.targetdir}]"
		}