-- premake5.lua
workspace "Walnut-Chat-Headless"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "App-Server"

   -- Workspace-wide defines
   defines
   {
       "WL_HEADLESS"
   }

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/Build-Walnut-Headless-External.lua"

group "App"
    include "App-Common/Build-App-Common-Headless.lua"
    include "App-Server/Build-App-Server-Headless.lua"
group ""