-- premake5.lua
workspace "Walnut-Chat"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "WalnutApp"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/Build-Walnut-External.lua"

group "App"
    include "App-Common/Build-App-Common.lua"
    include "App-Client/Build-App-Client.lua"
    include "App-Server/Build-App-Server.lua"
group ""