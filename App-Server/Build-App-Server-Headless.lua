project "App-Server-Headless"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "../App-Common/Source",

      "../Walnut/vendor/glm",

      "../Walnut/Walnut/Source",
      "../Walnut/Walnut/Platform/Headless",

      "../Walnut/vendor/spdlog/include",
      "../Walnut/vendor/yaml-cpp/include",

      -- Walnut-Networking
      "../Walnut/Walnut-Modules/Walnut-Networking/Source",
      "../Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/include"

   }

   links
   {
       "App-Common-Headless",
       "Walnut-Headless",
       "Walnut-Networking",

       "yaml-cpp",
   }

   	defines
	{
		"YAML_CPP_STATIC_DEFINE"
	}

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

      postbuildcommands 
	  {
	    '{COPY} "../%{WalnutNetworkingBinDir}/GameNetworkingSockets.dll" "%{cfg.targetdir}"',
	    '{COPY} "../%{WalnutNetworkingBinDir}/libcrypto-3-x64.dll" "%{cfg.targetdir}"',
	    '{COPY} "../%{WalnutNetworkingBinDir}/libprotobufd.dll" "%{cfg.targetdir}"',
	  }

   filter "system:linux"
      libdirs { "../Walnut/Walnut-Networking/vendor/GameNetworkingSockets/bin/Linux" }
      links { "GameNetworkingSockets" }

       defines { "WL_HEADLESS" }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      defines { "WL_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"