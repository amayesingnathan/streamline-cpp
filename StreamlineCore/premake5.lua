project "StreamlineCore"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
		
    targetdir 	("%{wks.location}/bin/%{prj.name}/" .. outputDir)
    objdir 		("%{wks.location}/obj/%{prj.name}/" .. outputDir)
	
	pchheader "pch.h"
	pchsource "src/pch.cpp"

    files 
    { 
        "src/**.h", 
        "src/**.cpp",
        "dependencies/ImGuizmo/ImGuizmo.h",
        "dependencies/ImGuizmo/ImGuizmo.cpp",
    }
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

    includedirs
    {
        "%{IncludeDir.StreamlineCore}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glad}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.ImGuizmo}",
    }

	links
	{
        "glad",
        "glfw",
        "imgui",
	}
	
	filter "files:dependencies/ImGuizmo/**.cpp"
		flags { "NoPCH" }
	
    filter "system:windows"
        kind "StaticLib"
        systemversion "latest"
        links
        {
            "opengl32.lib",
        }
		
	filter "system:linux"
        kind "SharedLib"
        pic "On"
        systemversion "latest"

    filter "configurations:Debug"
		runtime "Debug"
        symbols "on"
    filter "configurations:Release"
		runtime "Release"
        optimize "on"