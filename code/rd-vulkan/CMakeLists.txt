#============================================================================
# Copyright (C) 2013 - 2018, OpenJK contributors
#
# This file is part of the OpenJK source code.
#
# OpenJK is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#============================================================================

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

if(BuildSPRdVulkan)

	set(SPRDVulkanRendererIncludeDirectories ${SPDir})
	set(SPRDVulkanRendererIncludeDirectories
		${SharedDir}
		${SPRDVulkanRendererIncludeDirectories}
		"${SPDir}/rd-vulkan"
		"${GSLIncludeDirectory}"
		)

	# Modular Renderer crap (let's hope this works..)

	# Defines
	set(SPRDVulkanDefines ${SharedDefines} "RENDERER" "_JK2EXE")

	# Files

	# GHOUL 2
	set(SPRDVulkanG2Files
		"${SPDir}/ghoul2/g2.h"
		"${SPDir}/ghoul2/ghoul2_gore.h"
		"${SPDir}/rd-vulkan/G2_API.cpp"
		"${SPDir}/rd-vulkan/G2_bolts.cpp"
		"${SPDir}/rd-vulkan/G2_bones.cpp"
		"${SPDir}/rd-vulkan/G2_misc.cpp"
		"${SPDir}/rd-vulkan/G2_surfaces.cpp")
	source_group("ghoul2" FILES ${SPRDVulkanG2Files})
	set(SPRDVulkanFiles ${SPRDVulkanFiles} ${SPRDVulkanG2Files})

	# Transparently use either bundled or system libjpeg.
	list(APPEND SPRDVulkanRendererIncludeDirectories ${JPEG_INCLUDE_DIR})
	list(APPEND SPRDVulkanRendererLibraries          ${JPEG_LIBRARIES})

	# Transparently use either bundled or system libpng.  Order is important --
	# libpng used zlib, so it must come before it on the linker command line.
	list(APPEND SPRDVulkanRendererIncludeDirectories ${PNG_INCLUDE_DIRS})
	list(APPEND SPRDVulkanRendererLibraries          ${PNG_LIBRARIES})

	# Transparently use either bundled or system zlib.
	list(APPEND SPRDVulkanRendererIncludeDirectories ${ZLIB_INCLUDE_DIR})
	list(APPEND SPRDVulkanRendererLibraries          ${ZLIB_LIBRARIES})

	# Transparently use our bundled minizip.
	list(APPEND SPRDVulkanRendererIncludeDirectories ${MINIZIP_INCLUDE_DIRS})
	list(APPEND SPRDVulkanRendererLibraries          ${MINIZIP_LIBRARIES})

	# Transparantly make use of all renderer directories
    list(APPEND SPRDVulkanRendererIncludeDirectories "${SPDir}/rd-common")
    list(APPEND SPRDVulkanRendererIncludeDirectories "${SPDir}/rd-vulkan")

	# Misc files
	set(SPRDVulkanCommonFiles
		"${SPDir}/qcommon/matcomp.cpp"
		"${SPDir}/qcommon/q_shared.cpp"
		"${SPDir}/qcommon/q_shared.h"
		"${SPDir}/qcommon/ojk_i_saved_game.h"
		"${SPDir}/qcommon/ojk_saved_game_class_archivers.h"
		"${SPDir}/qcommon/ojk_saved_game_helper.h"
		"${SPDir}/qcommon/ojk_saved_game_helper_fwd.h"
		"${SPDir}/qcommon/ojk_scope_guard.h"
		${SharedCommonFiles}
		)
	source_group("common" FILES ${SPRDVulkanCommonFiles})
	set(SPRDVulkanFiles ${SPRDVulkanFiles} ${SPRDVulkanCommonFiles})

	# Misc files
	set(SPRDVulkanCommonSafeFiles
		${SharedCommonSafeFiles}
		)
	source_group("common/safe" FILES ${SPRDVulkanCommonSafeFiles})
	set(SPRDVulkanFiles ${SPRDVulkanFiles} ${SPRDVulkanCommonSafeFiles})

	# main files
	set(SPRDVulkanMainFiles
		"${SPDir}/rd-vulkan/tr_backend.cpp"
		"${SPDir}/rd-vulkan/tr_bsp.cpp"
		"${SPDir}/rd-vulkan/tr_cmds.cpp"
		"${SPDir}/rd-vulkan/tr_curve.cpp"
		"${SPDir}/rd-vulkan/tr_decals.cpp"
		"${SPDir}/rd-vulkan/tr_ghoul2.cpp"
		"${SPDir}/rd-vulkan/tr_image.cpp"
		"${SPDir}/rd-vulkan/tr_init.cpp"
		"${SPDir}/rd-vulkan/tr_light.cpp"
		"${SPDir}/rd-vulkan/tr_local.h"
		"${SPDir}/rd-vulkan/tr_main.cpp"
		"${SPDir}/rd-vulkan/tr_marks.cpp"
		"${SPDir}/rd-vulkan/tr_mesh.cpp"
		"${SPDir}/rd-vulkan/tr_model.cpp"
		"${SPDir}/rd-vulkan/tr_quicksprite.cpp"
		"${SPDir}/rd-vulkan/tr_quicksprite.h"
		"${SPDir}/rd-vulkan/tr_scene.cpp"
		"${SPDir}/rd-vulkan/tr_shade.cpp"
		"${SPDir}/rd-vulkan/tr_shader.cpp"
		"${SPDir}/rd-vulkan/tr_shade_calc.cpp"
		"${SPDir}/rd-vulkan/tr_shadows.cpp"
		"${SPDir}/rd-vulkan/tr_skin.cpp"
		"${SPDir}/rd-vulkan/tr_sky.cpp"
		"${SPDir}/rd-vulkan/tr_subs.cpp"
		"${SPDir}/rd-vulkan/tr_surface.cpp"
		"${SPDir}/rd-vulkan/tr_surfacesprites.cpp"
		"${SPDir}/rd-vulkan/tr_world.cpp"
		"${SPDir}/rd-vulkan/tr_WorldEffects.cpp"
		"${SPDir}/rd-vulkan/tr_WorldEffects.h"
		"${SPDir}/rd-vulkan/vk_attachments.cpp"
		"${SPDir}/rd-vulkan/vk_bloom.cpp"
		"${SPDir}/rd-vulkan/vk_cmd.cpp"
		"${SPDir}/rd-vulkan/vk_debug.cpp"
		"${SPDir}/rd-vulkan/vk_flares.cpp"
		"${SPDir}/rd-vulkan/vk_frame.cpp"
		"${SPDir}/rd-vulkan/vk_image.cpp"
		"${SPDir}/rd-vulkan/vk_image_process.cpp"
		"${SPDir}/rd-vulkan/vk_info.cpp"
		"${SPDir}/rd-vulkan/vk_init.cpp"
		"${SPDir}/rd-vulkan/vk_instance.cpp"
		"${SPDir}/rd-vulkan/vk_local.h"
		"${SPDir}/rd-vulkan/vk_pipelines.cpp"
		"${SPDir}/rd-vulkan/vk_shaders.cpp"
		"${SPDir}/rd-vulkan/vk_shade_geometry.cpp"
		"${SPDir}/rd-vulkan/vk_swapchain.cpp"
		"${SPDir}/rd-vulkan/vk_vbo.cpp"
		)
	source_group("renderer" FILES ${SPRDVulkanMainFiles})
	set(SPRDVulkanFiles ${SPRDVulkanFiles} ${SPRDVulkanMainFiles})

	set(SPRDVulkanRdCommonFiles
		"${SPDir}/rd-common/mdx_format.h"
		"${SPDir}/rd-common/tr_common.h"
		"${SPDir}/rd-common/tr_font.cpp"
		"${SPDir}/rd-common/tr_font.h"
		"${SPDir}/rd-common/stb_image.h"
		"${SPDir}/rd-common/tr_image_load.cpp"
		"${SPDir}/rd-common/tr_image_stb.cpp"
		"${SPDir}/rd-common/tr_image_jpg.cpp"
		"${SPDir}/rd-common/tr_image_tga.cpp"
		"${SPDir}/rd-common/tr_image_png.cpp"
		"${SPDir}/rd-common/tr_noise.cpp"
		"${SPDir}/rd-common/tr_public.h"
		"${SPDir}/rd-common/tr_types.h")
	source_group("rd-common" FILES ${SPRDVulkanRdCommonFiles})
	set(SPRDVulkanFiles ${SPRDVulkanFiles} ${SPRDVulkanRdCommonFiles})

	set(SPRDVulkanVulkanAssets
		"${SPDir}/rd-vulkan/vulkan/GLSL.std.450.h"
		"${SPDir}/rd-vulkan/vulkan/libspirv.h"
		"${SPDir}/rd-vulkan/vulkan/spirv.h"
		"${SPDir}/rd-vulkan/vulkan/spirv.hpp"
		"${SPDir}/rd-vulkan/vulkan/spirv.hpp11"
		"${SPDir}/rd-vulkan/vulkan/spirv.json"
		"${SPDir}/rd-vulkan/vulkan/spirv.lua"
		"${SPDir}/rd-vulkan/vulkan/spirv.py"
		"${SPDir}/rd-vulkan/vulkan/vk_dispatch_table_helper.h"
		"${SPDir}/rd-vulkan/vulkan/vk_enum_string_helper.h"
		"${SPDir}/rd-vulkan/vulkan/vk_icd.h"
		"${SPDir}/rd-vulkan/vulkan/vk_layer.h"
		"${SPDir}/rd-vulkan/vulkan/vk_layer_dispatch_table.h"
		"${SPDir}/rd-vulkan/vulkan/vk_platform.h"
		"${SPDir}/rd-vulkan/vulkan/vk_sdk_platform.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan.hpp"
		"${SPDir}/rd-vulkan/vulkan/vulkan_android.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_core.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_ios.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_macos.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_mir.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_vi.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_wayland.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_win32.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_xcb.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_xlib.h"
		"${SPDir}/rd-vulkan/vulkan/vulkan_xlib_xrandr.h")
	source_group("renderer\\vulkan" FILES ${SPRDVulkanVulkanAssets})
	set(SPRDVulkanFiles ${SPRDVulkanFiles} ${SPRDVulkanVulkanAssets})

	#find_package(OpenGL REQUIRED)
	#set(SPRDVulkanRendererIncludeDirectories ${SPRDVulkanRendererIncludeDirectories} ${OPENGL_INCLUDE_DIR})
	#set(SPRDVulkanRendererLibraries ${SPRDVulkanRendererLibraries} ${OPENGL_LIBRARIES})

	set(SPRDVulkanRendererIncludeDirectories ${SPRDVulkanRendererIncludeDirectories} ${OpenJKLibDir})

	function(add_sp_renderer_project ProjectName Label EngineName InstallDir Component)
		add_library(${ProjectName} SHARED ${SPRDVulkanFiles})
		if(NOT MSVC)
			# remove "lib" prefix for .so/.dylib files
			set_target_properties(${ProjectName} PROPERTIES PREFIX "")
		endif()

		if(WIN32)
			install(TARGETS ${ProjectName}
				RUNTIME
				DESTINATION ${InstallDir}
				COMPONENT ${Component})
		else(WIN32)
			if(MakeApplicationBundles AND (BuildSPEngine))
				install(TARGETS ${ProjectName}
					LIBRARY
					DESTINATION "${InstallDir}/${EngineName}.app/Contents/MacOS/"
					COMPONENT ${Component})
			else()
				install(TARGETS ${ProjectName}
					LIBRARY
					DESTINATION ${InstallDir}
					COMPONENT ${Component})
			endif()
		endif()

		set_target_properties(${ProjectName} PROPERTIES COMPILE_DEFINITIONS "${SPRDVulkanDefines}")

		# Hide symbols not explicitly marked public.
		set_property(TARGET ${ProjectName} APPEND PROPERTY COMPILE_OPTIONS ${OPENJK_VISIBILITY_FLAGS})

		set_target_properties(${ProjectName} PROPERTIES INCLUDE_DIRECTORIES "${SPRDVulkanRendererIncludeDirectories}")
		set_target_properties(${ProjectName} PROPERTIES PROJECT_LABEL ${Label})

		target_link_libraries(${ProjectName} ${SPRDVulkanRendererLibraries})
	endfunction(add_sp_renderer_project)

	if(BuildSPRdVulkan)
		add_sp_renderer_project(${SPRDVulkanRenderer} "SP Vulkan Renderer" ${SPEngine} ${JKAInstallDir} ${JKASPClientComponent})
	endif(BuildSPRdVulkan)

endif(BuildSPRdVulkan)