/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

// tr_init.c -- functions that are not called every frame

#include "../server/exe_headers.h"

#include "tr_local.h"
#include "../rd-common/tr_common.h"
#include "tr_WorldEffects.h"
#include "qcommon/MiniHeap.h"
#include <algorithm>

glconfig_t	glConfig;
glconfigExt_t glConfigExt;
glstate_t	glState;
window_t	window;
glstatic_t	gls;

cvar_t	*r_verbose;
cvar_t	*r_ignore;

cvar_t	*r_detailTextures;

cvar_t	*r_znear;
cvar_t	*r_zproj;

cvar_t	*r_skipBackEnd;

cvar_t	*r_measureOverdraw;

cvar_t	*r_inGameVideo;
cvar_t	*r_fastsky;
cvar_t	*r_drawSun;
cvar_t	*r_dynamiclight;
// rjr - removed for hacking
cvar_t	*r_dlightBacks;

cvar_t	*r_lodbias;
cvar_t	*r_lodscale;
cvar_t	*r_autolodscalevalue;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_drawfog;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_facePlaneCull;
cvar_t	*r_roofCullCeilDist; //ceiling distance cull tolerance -rww
cvar_t	*r_roofCullFloorDist; //floor distance cull tolerance -rww
cvar_t	*r_showcluster;
cvar_t	*r_nocurves;

cvar_t	*r_autoMap; //automap renderside toggle for debugging -rww
cvar_t	*r_autoMapBackAlpha; //alpha of automap bg -rww
cvar_t	*r_autoMapDisable; //don't calc it (since it's slow in debug) -rww

cvar_t	*r_dlightStyle;
cvar_t	*r_surfaceSprites;
cvar_t	*r_surfaceWeather;

cvar_t	*r_windSpeed;
cvar_t	*r_windAngle;
cvar_t	*r_windGust;
cvar_t	*r_windDampFactor;
cvar_t	*r_windPointForce;
cvar_t	*r_windPointX;
cvar_t	*r_windPointY;

cvar_t	*r_allowExtensions;

cvar_t	*r_ext_compressed_textures;
cvar_t	*r_ext_compressed_lightmaps;
cvar_t	*r_ext_preferred_tc_method;
cvar_t	*r_ext_gamma_control;
cvar_t	*r_ext_multitexture;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_texture_env_add;
cvar_t	*r_ext_texture_filter_anisotropic;
cvar_t	*r_gammaShaders;

cvar_t	*r_environmentMapping;

cvar_t	*r_DynamicGlow;
cvar_t	*r_DynamicGlowPasses;
cvar_t	*r_DynamicGlowDelta;
cvar_t	*r_DynamicGlowIntensity;
cvar_t	*r_DynamicGlowSoft;
cvar_t	*r_DynamicGlowWidth;
cvar_t	*r_DynamicGlowHeight;
cvar_t	*r_DynamicGlowScale;

cvar_t	*r_smartpicmip;

cvar_t	*r_ignoreGLErrors;
cvar_t	*r_logFile;

cvar_t	*r_primitives;
cvar_t	*r_texturebits;
cvar_t	*r_texturebitslm;

cvar_t	*r_lightmap;
cvar_t	*r_distanceCull;
cvar_t	*r_vertexLight;
cvar_t	*r_uiFullScreen;
cvar_t	*r_shadows;
cvar_t	*r_shadowRange;


cvar_t	*r_flares;
cvar_t	*r_flareSize;
cvar_t	*r_flareFade;
cvar_t	*r_flareCoeff;

cvar_t	*r_nobind;
cvar_t	*r_singleShader;
cvar_t	*r_colorMipLevels;
cvar_t	*r_picmip;
cvar_t	*r_showtris;
cvar_t	*r_showsky;
cvar_t	*r_shownormals;
cvar_t	*r_finish;
cvar_t	*r_clear;
cvar_t	*r_markcount;
cvar_t	*r_textureMode;
cvar_t	*r_offsetFactor;
cvar_t	*r_offsetUnits;
cvar_t	*r_gamma;
cvar_t	*r_intensity;
cvar_t	*r_lockpvs;
cvar_t	*r_noportals;
cvar_t	*r_portalOnly;

cvar_t	*r_subdivisions;
cvar_t	*r_lodCurveError;



cvar_t	*r_overBrightBits;
cvar_t	*r_mapOverBrightBits;

cvar_t	*r_debugSurface;
cvar_t	*r_simpleMipMaps;

cvar_t	*r_showImages;

cvar_t	*r_ambientScale;
cvar_t	*r_directedScale;
cvar_t	*r_debugLight;
cvar_t	*r_debugSort;

cvar_t	*r_marksOnTriangleMeshes;

cvar_t	*r_aspectCorrectFonts;
cvar_t	*cl_ratioFix;

// Vulkan
cvar_t	*r_defaultImage;
cvar_t	*r_device;
cvar_t	*r_stencilbits;
cvar_t	*r_ext_multisample;
cvar_t	*r_ext_supersample;
cvar_t	*r_ext_alpha_to_coverage;
cvar_t	*r_fbo;
cvar_t	*r_hdr;
cvar_t	*r_ext_max_anisotropy;
cvar_t	*r_mapGreyScale;
cvar_t	*r_greyscale;
cvar_t	*r_dither;
cvar_t	*r_presentBits;
cvar_t	*r_bloom;
cvar_t	*r_bloom_threshold;
cvar_t	*r_bloom_intensity;
cvar_t	*r_renderWidth;
cvar_t	*r_renderHeight;
cvar_t	*r_renderScale;
cvar_t	*r_ignorehwgamma;

#ifdef USE_PMLIGHT
cvar_t	*r_dlightMode;
cvar_t	*r_dlightScale;
cvar_t	*r_dlightIntensity;
#endif
cvar_t	*r_dlightSaturation;
cvar_t	*r_roundImagesDown;
cvar_t	*r_nomip;
#ifdef USE_VBO
cvar_t	*r_vbo;
#endif

// the limits apply to the sum of all scenes in a frame --
// the main view, all the 3D icons, etc
#define	DEFAULT_MAX_POLYS		600
#define	DEFAULT_MAX_POLYVERTS	3000
static cvar_t	*r_maxpolys;
static cvar_t	*r_maxpolyverts;
int		max_polys;
int		max_polyverts;

cvar_t	*r_modelpoolmegs;

/*
Ghoul2 Insert Start
*/
#ifdef _DEBUG
cvar_t	*r_noPrecacheGLA;
#endif

cvar_t	*r_noGhoul2;
cvar_t	*r_Ghoul2AnimSmooth;

cvar_t	*r_Ghoul2UnSqash;
cvar_t	*r_Ghoul2TimeBase=0;
cvar_t	*r_Ghoul2NoLerp;
cvar_t	*r_Ghoul2NoBlend;
cvar_t	*r_Ghoul2BlendMultiplier=0;
cvar_t	*r_Ghoul2UnSqashAfterSmooth;

cvar_t	*broadsword=0;
cvar_t	*broadsword_kickbones=0;
cvar_t	*broadsword_kickorigin=0;
cvar_t	*broadsword_playflop=0;
cvar_t	*broadsword_dontstopanim=0;
cvar_t	*broadsword_waitforshot=0;
cvar_t	*broadsword_smallbbox=0;
cvar_t	*broadsword_extra1=0;
cvar_t	*broadsword_extra2=0;

cvar_t	*broadsword_effcorr=0;
cvar_t	*broadsword_ragtobase=0;
cvar_t	*broadsword_dircap=0;

// More bullshit needed for the proper modular renderer --eez
cvar_t* sv_mapname;

/*
Ghoul2 Insert End
*/

cvar_t *se_language;
#ifdef JK2_MODE
cvar_t* sp_language;			// JK2
#endif
cvar_t* com_buildScript;

cvar_t *r_aviMotionJpegQuality;
cvar_t *r_screenshotJpegQuality;

// Vulkan
#include "vk_local.h"
Vk_Instance vk;
Vk_World	vk_world;

#if 0
#if !defined(__APPLE__)
PFNGLSTENCILOPSEPARATEPROC qglStencilOpSeparate;
#endif

PFNGLACTIVETEXTUREARBPROC qglActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC qglClientActiveTextureARB;
PFNGLMULTITEXCOORD2FARBPROC qglMultiTexCoord2fARB;
#if !defined(__APPLE__)
PFNGLTEXIMAGE3DPROC qglTexImage3D;
PFNGLTEXSUBIMAGE3DPROC qglTexSubImage3D;
#endif

PFNGLCOMBINERPARAMETERFVNVPROC qglCombinerParameterfvNV;
PFNGLCOMBINERPARAMETERIVNVPROC qglCombinerParameterivNV;
PFNGLCOMBINERPARAMETERFNVPROC qglCombinerParameterfNV;
PFNGLCOMBINERPARAMETERINVPROC qglCombinerParameteriNV;
PFNGLCOMBINERINPUTNVPROC qglCombinerInputNV;
PFNGLCOMBINEROUTPUTNVPROC qglCombinerOutputNV;

PFNGLFINALCOMBINERINPUTNVPROC qglFinalCombinerInputNV;
PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC qglGetCombinerInputParameterfvNV;
PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC qglGetCombinerInputParameterivNV;
PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC qglGetCombinerOutputParameterfvNV;
PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC qglGetCombinerOutputParameterivNV;
PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC qglGetFinalCombinerInputParameterfvNV;
PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC qglGetFinalCombinerInputParameterivNV;

PFNGLPROGRAMSTRINGARBPROC qglProgramStringARB;
PFNGLBINDPROGRAMARBPROC qglBindProgramARB;
PFNGLDELETEPROGRAMSARBPROC qglDeleteProgramsARB;
PFNGLGENPROGRAMSARBPROC qglGenProgramsARB;
PFNGLPROGRAMENVPARAMETER4DARBPROC qglProgramEnvParameter4dARB;
PFNGLPROGRAMENVPARAMETER4DVARBPROC qglProgramEnvParameter4dvARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC qglProgramEnvParameter4fARB;
PFNGLPROGRAMENVPARAMETER4FVARBPROC qglProgramEnvParameter4fvARB;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC qglProgramLocalParameter4dARB;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC qglProgramLocalParameter4dvARB;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC qglProgramLocalParameter4fARB;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC qglProgramLocalParameter4fvARB;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC qglGetProgramEnvParameterdvARB;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC qglGetProgramEnvParameterfvARB;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC qglGetProgramLocalParameterdvARB;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC qglGetProgramLocalParameterfvARB;
PFNGLGETPROGRAMIVARBPROC qglGetProgramivARB;
PFNGLGETPROGRAMSTRINGARBPROC qglGetProgramStringARB;
PFNGLISPROGRAMARBPROC qglIsProgramARB;

PFNGLLOCKARRAYSEXTPROC qglLockArraysEXT;
PFNGLUNLOCKARRAYSEXTPROC qglUnlockArraysEXT;
#endif

bool g_bTextureRectangleHack = false;

void RE_SetLightStyle( int style, int color );
void RE_GetBModelVerts( int bmodelIndex, vec3_t *verts, vec3_t normal );

void R_Set2DRatio( void ) {
	if (cl_ratioFix->integer)
		tr.widthRatioCoef = ((float)(SCREEN_WIDTH * gls.windowHeight) / (float)(SCREEN_HEIGHT * gls.windowWidth));
	else
		tr.widthRatioCoef = 1.0f;

	if (tr.widthRatioCoef > 1)
		tr.widthRatioCoef = 1.0f;
}

/*
==============================================================================

						SCREEN SHOTS

==============================================================================
*/

/*
==================
RB_ReadPixels

Reads an image but takes care of alignment issues for reading RGB images.

Reads a minimum offset for where the RGB data starts in the image from
integer stored at pointer offset. When the function has returned the actual
offset was written back to address offset. This address will always have an
alignment of packAlign to ensure efficient copying.

Stores the length of padding after a line of pixels to address padlen

Return value must be freed with Hunk_FreeTempMemory()
==================
*/

byte *RB_ReadPixels( int x, int y, int width, int height, size_t *offset, int *padlen, int lineAlign )
{
	byte *buffer, *bufstart;
	int bufAlign, linelen;
	int packAlign = 1;

	linelen = width * 3;

	bufAlign = MAX(packAlign, 16); // for SIMD

	// Allocate a few more bytes so that we can choose an alignment we like
	//buffer = ri.Hunk_AllocateTempMemory(padwidth * height + *offset + bufAlign - 1);
	buffer = (byte*)R_Malloc(width * height + *offset + bufAlign - 1, TAG_TEMP_WORKSPACE, qfalse);
	bufstart = (byte*)PADP((intptr_t)buffer + *offset, bufAlign);

	vk_read_pixels(bufstart, width, height);

	*offset = bufstart - buffer;
	*padlen = PAD(linelen, packAlign) - linelen;

	return buffer;
}

/*
==================
R_TakeScreenshot
==================
*/
void R_TakeScreenshot( int x, int y, int width, int height, char *fileName ) {
	byte *allbuf, *buffer;
	byte *srcptr, *destptr;
	byte *endline, *endmem;
	byte temp;

	int linelen, padlen;
	size_t offset = 18, memcount;

	allbuf = RB_ReadPixels(x, y, width, height, &offset, &padlen, 0);
	buffer = allbuf + offset - 18;

	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;	// pixel size

	// swap rgb to bgr and remove padding from line endings
	linelen = width * 3;

	srcptr = destptr = allbuf + offset;
	endmem = srcptr + (linelen + padlen) * height;

	while(srcptr < endmem)
	{
		endline = srcptr + linelen;

		while(srcptr < endline)
		{
			temp = srcptr[0];
			*destptr++ = srcptr[2];
			*destptr++ = srcptr[1];
			*destptr++ = temp;

			srcptr += 3;
		}

		// Skip the pad
		srcptr += padlen;
	}

	memcount = linelen * height;

	// gamma correct
	if(glConfig.deviceSupportsGamma && !glConfigExt.doGammaCorrectionWithShaders)
		R_GammaCorrect(allbuf + offset, memcount);

	ri.FS_WriteFile(fileName, buffer, memcount + 18);

	R_Free(allbuf);
}

/*
==================
R_TakeScreenshotPNG
==================
*/
void R_TakeScreenshotPNG( int x, int y, int width, int height, char *fileName ) {
	byte *buffer=NULL;
	size_t offset=0;
	int padlen=0;

	buffer = RB_ReadPixels( x, y, width, height, &offset, &padlen, 0);
	RE_SavePNG( fileName, buffer, width, height, 3 );
	R_Free( buffer );
}

/*
==================
R_TakeScreenshotJPEG
==================
*/
void R_TakeScreenshotJPEG( int x, int y, int width, int height, char *fileName ) {
	byte *buffer;
	size_t offset = 0, memcount;
	int padlen;

	buffer = RB_ReadPixels(x, y, width, height, &offset, &padlen, 0);
	memcount = (width * 3 + padlen) * height;

	// gamma correct
	if(glConfig.deviceSupportsGamma && !glConfigExt.doGammaCorrectionWithShaders)
		R_GammaCorrect(buffer + offset, memcount);

	RE_SaveJPG(fileName, r_screenshotJpegQuality->integer, width, height, buffer + offset, padlen);
	R_Free( buffer );
}

/*
==================
R_ScreenshotFilename
==================
*/
void R_ScreenshotFilename( char *buf, int bufSize, const char *ext ) {
	time_t rawtime;
	char timeStr[32] = {0}; // should really only reach ~19 chars

	time( &rawtime );
	strftime( timeStr, sizeof( timeStr ), "%Y-%m-%d_%H-%M-%S", localtime( &rawtime ) ); // or gmtime

	Com_sprintf( buf, bufSize, "screenshots/shot%s.%s", timeStr, ext );
}

/*
====================
R_LevelShot

levelshots are specialized 256*256 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
#define LEVELSHOTSIZE 256
static void R_LevelShot( void ) {
	char		checkname[MAX_OSPATH];
	byte		*buffer;
	byte		*source, *allsource;
	byte		*src, *dst;
	size_t		offset = 0;
	int			padlen;
	int			x, y;
	int			r, g, b;
	float		xScale, yScale;
	int			xx, yy;

	Com_sprintf( checkname, sizeof(checkname), "levelshots/%s.tga", tr.world->baseName );

	allsource = RB_ReadPixels(0, 0, gls.captureWidth, gls.captureHeight, &offset, &padlen, 0);
	source = allsource + offset;

	buffer = (byte*)R_Malloc(LEVELSHOTSIZE * LEVELSHOTSIZE * 3 + 18, TAG_TEMP_WORKSPACE, qfalse);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = LEVELSHOTSIZE & 255;
	buffer[13] = LEVELSHOTSIZE >> 8;
	buffer[14] = LEVELSHOTSIZE & 255;
	buffer[15] = LEVELSHOTSIZE >> 8;
	buffer[16] = 24;	// pixel size

	// resample from source
	xScale = glConfig.vidWidth / (4.0*LEVELSHOTSIZE);
	yScale = glConfig.vidHeight / (3.0*LEVELSHOTSIZE);
	for ( y = 0 ; y < LEVELSHOTSIZE ; y++ ) {
		for ( x = 0 ; x < LEVELSHOTSIZE ; x++ ) {
			r = g = b = 0;
			for ( yy = 0 ; yy < 3 ; yy++ ) {
				for ( xx = 0 ; xx < 4 ; xx++ ) {
					src = source + 3 * ( glConfig.vidWidth * (int)( (y*3+yy)*yScale ) + (int)( (x*4+xx)*xScale ) );
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * ( y * LEVELSHOTSIZE + x );
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}

	// gamma correct
	if ( ( tr.overbrightBits > 0 ) && glConfig.deviceSupportsGamma && !glConfigExt.doGammaCorrectionWithShaders ) {
		R_GammaCorrect( buffer + 18, LEVELSHOTSIZE * LEVELSHOTSIZE * 3 );
	}

	ri.FS_WriteFile( checkname, buffer, LEVELSHOTSIZE * LEVELSHOTSIZE*3 + 18 );

	R_Free( buffer );
	R_Free( allsource );

	vk_debug("[skipnotify]Wrote %s\n", checkname );
}

void R_ScreenShot_f ( void ) {
	char checkname[MAX_OSPATH] = {0};
	qboolean silent = qfalse;
	int			typeMask;
	const char *ext;

	if (ri.VK_IsMinimized() && !R_CanMinimize()) {
		ri.Printf(PRINT_WARNING, "WARNING: unable to take screenshot when minimized because FBO is not available/enabled.\n");
		return;
	}

	if ( !strcmp( ri.Cmd_Argv(1), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if (Q_stricmp(ri.Cmd_Argv(0), "screenshot_tga") == 0) {
		typeMask = SCREENSHOT_TGA;
		ext = "tga";
	}
	else if (Q_stricmp(ri.Cmd_Argv(0), "screenshot_png") == 0) {
		typeMask = SCREENSHOT_PNG;
		ext = "png";
	}
	else {
		typeMask = SCREENSHOT_JPG;
		ext = "jpg";
	}

	// check if already scheduled
	if (backEnd.screenshotMask & typeMask)
		return;

	if ( !strcmp( ri.Cmd_Argv(1), "silent" ) )
		silent = qtrue;

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_sprintf( checkname, sizeof( checkname ), "screenshots/%s.%s", ri.Cmd_Argv( 1 ), ext );
	}
	else {
		// timestamp the filename
		R_ScreenshotFilename( checkname, sizeof( checkname ), ext );

		if ( ri.FS_FileExists( checkname ) ) {
			vk_debug("ScreenShot: Couldn't create a file\n" );
			return;
 		}
	}

	// we will make the screenshot right at the end of RE_EndFrame()
	backEnd.screenshotMask |= typeMask;
	if (typeMask == SCREENSHOT_JPG) {
		backEnd.screenShotJPGsilent = silent;
		Q_strncpyz(backEnd.screenshotJPG, checkname, sizeof(backEnd.screenshotJPG));
	}
	else if (typeMask == SCREENSHOT_PNG) {
		backEnd.screenShotPNGsilent = silent;
		Q_strncpyz(backEnd.screenshotPNG, checkname, sizeof(backEnd.screenshotPNG));
	}
	else {
		backEnd.screenShotTGAsilent = silent;
		Q_strncpyz(backEnd.screenshotTGA, checkname, sizeof(backEnd.screenshotTGA));
	}
}

typedef struct consoleCommand_s {
	const char	*cmd;
	xcommand_t	func;
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "imagelist",			R_ImageList_f },
	{ "shaderlist",			R_ShaderList_f },
	{ "skinlist",			R_SkinList_f },
	{ "fontlist",			R_FontList_f },
	{ "screenshot",			R_ScreenShot_f },
	{ "screenshot_png",		R_ScreenShot_f },
	{ "screenshot_tga",		R_ScreenShot_f },
	{ "gfxinfo",			GfxInfo_f },
	{ "r_we",				R_WorldEffect_f },
	{ "imagecacheinfo",		RE_RegisterImages_Info_f },
	{ "modellist",			R_Modellist_f },
	{ "modelcacheinfo",		RE_RegisterModels_Info_f },
	{ "r_cleardecals",		RE_ClearDecals },
	{ "vkinfo",				vk_info_f }
};

static const size_t numCommands = ARRAY_LEN( commands );

#ifdef _DEBUG
#define MIN_PRIMITIVES -1
#else
#define MIN_PRIMITIVES 0
#endif
#define MAX_PRIMITIVES 3

/*
===============
R_Register
===============
*/
void R_Register( void )
{
	//FIXME: lol badness
	se_language = ri.Cvar_Get("se_language", "english", CVAR_ARCHIVE | CVAR_NORESTART, "");
	//
	// latched and archived variables
	//
	r_allowExtensions					= ri.Cvar_Get( "r_allowExtensions",					"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_compressed_textures			= ri.Cvar_Get( "r_ext_compress_textures",			"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_compressed_lightmaps			= ri.Cvar_Get( "r_ext_compress_lightmaps",			"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_preferred_tc_method			= ri.Cvar_Get( "r_ext_preferred_tc_method",			"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_gamma_control					= ri.Cvar_Get( "r_ext_gamma_control",				"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_multitexture					= ri.Cvar_Get( "r_ext_multitexture",				"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_compiled_vertex_array			= ri.Cvar_Get( "r_ext_compiled_vertex_array",		"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_texture_env_add				= ri.Cvar_Get( "r_ext_texture_env_add",				"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_ext_texture_filter_anisotropic	= ri.Cvar_Get( "r_ext_texture_filter_anisotropic",	"16",						CVAR_ARCHIVE_ND, "" );
	r_gammaShaders						= ri.Cvar_Get( "r_gammaShaders",					"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "Set gamma using pixel shaders inside the game window only." );
	r_environmentMapping				= ri.Cvar_Get( "r_environmentMapping",				"1",						CVAR_ARCHIVE_ND, "" );
	r_DynamicGlow						= ri.Cvar_Get( "r_DynamicGlow",						"0",						CVAR_ARCHIVE_ND, "" );
	r_DynamicGlowPasses					= ri.Cvar_Get( "r_DynamicGlowPasses",				"5",						CVAR_ARCHIVE_ND, "" );
	r_DynamicGlowDelta					= ri.Cvar_Get( "r_DynamicGlowDelta",				"0.8f",						CVAR_ARCHIVE_ND, "" );
	r_DynamicGlowIntensity				= ri.Cvar_Get( "r_DynamicGlowIntensity",			"1.13f",					CVAR_ARCHIVE_ND, "" );
	r_DynamicGlowSoft					= ri.Cvar_Get( "r_DynamicGlowSoft",					"1",						CVAR_ARCHIVE_ND, "" );
	r_DynamicGlowWidth					= ri.Cvar_Get( "r_DynamicGlowWidth",				"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_DynamicGlowHeight					= ri.Cvar_Get( "r_DynamicGlowHeight",				"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_DynamicGlowScale					= ri.Cvar_Get( "r_DynamicGlowScale",				"0.25",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_picmip							= ri.Cvar_Get( "r_picmip",							"0",						CVAR_ARCHIVE|CVAR_LATCH, "" );
	ri.Cvar_CheckRange( r_picmip, 0, 16, qtrue );
	r_smartpicmip						= ri.Cvar_Get( "r_smartpicmip",						"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "Applies r_picmip setting to map textures only." );
	r_colorMipLevels					= ri.Cvar_Get( "r_colorMipLevels",					"0",						CVAR_LATCH, "" );
	r_detailTextures					= ri.Cvar_Get( "r_detailtextures",					"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_texturebits						= ri.Cvar_Get( "r_texturebits",						"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_texturebitslm						= ri.Cvar_Get( "r_texturebitslm",					"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_overBrightBits					= ri.Cvar_Get( "r_overBrightBits",					"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_mapOverBrightBits					= ri.Cvar_Get( "r_mapOverBrightBits",				"0",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_simpleMipMaps						= ri.Cvar_Get( "r_simpleMipMaps",					"1",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	r_vertexLight						= ri.Cvar_Get( "r_vertexLight",						"0",						CVAR_ARCHIVE|CVAR_LATCH, "" );
	r_uiFullScreen						= ri.Cvar_Get( "r_uifullscreen",					"0",						0, "" );
	r_subdivisions						= ri.Cvar_Get( "r_subdivisions",					"4",						CVAR_ARCHIVE_ND|CVAR_LATCH, "" );
	ri.Cvar_CheckRange( r_subdivisions, 0, 80, qfalse );

	r_fullbright						= ri.Cvar_Get( "r_fullbright",						"0",						CVAR_ARCHIVE_ND, "" );
	r_intensity							= ri.Cvar_Get( "r_intensity",						"1",						CVAR_LATCH, "" );
	r_singleShader						= ri.Cvar_Get( "r_singleShader",					"0",						CVAR_CHEAT|CVAR_LATCH, "" );
	r_lodCurveError						= ri.Cvar_Get( "r_lodCurveError",					"250",						CVAR_ARCHIVE_ND, "" );
	r_lodbias							= ri.Cvar_Get( "r_lodbias",							"0",						CVAR_ARCHIVE_ND, "" );
	r_autolodscalevalue					= ri.Cvar_Get( "r_autolodscalevalue",				"0",						CVAR_ROM, "" );

	r_flares							= ri.Cvar_Get( "r_flares",							"1",						CVAR_ARCHIVE_ND, "" );
	r_flareSize							= ri.Cvar_Get( "r_flareSize",						"40",						CVAR_ARCHIVE_ND, "" );
	r_flareFade							= ri.Cvar_Get( "r_flareFade",						"10",						CVAR_ARCHIVE_ND, "" );
	r_flareCoeff						= ri.Cvar_Get( "r_flareCoeff",						"150",						CVAR_ARCHIVE_ND, "" );
	ri.Cvar_CheckRange(r_flareCoeff, 0.1f, 250, qfalse);

	r_znear								= ri.Cvar_Get( "r_znear",							"4",						CVAR_ARCHIVE_ND, "" );
	ri.Cvar_CheckRange( r_znear, 0.001f, 10, qfalse );
	r_zproj								= ri.Cvar_Get("r_zproj",							"64",						CVAR_ARCHIVE_ND, "" );
	r_ignoreGLErrors					= ri.Cvar_Get( "r_ignoreGLErrors",					"1",						CVAR_ARCHIVE_ND, "" );
	r_fastsky							= ri.Cvar_Get( "r_fastsky",							"0",						CVAR_ARCHIVE_ND, "" );
	r_inGameVideo						= ri.Cvar_Get( "r_inGameVideo",						"1",						CVAR_ARCHIVE_ND, "" );
	r_drawSun							= ri.Cvar_Get( "r_drawSun",							"0",						CVAR_ARCHIVE_ND, "" );
	r_dynamiclight						= ri.Cvar_Get( "r_dynamiclight",					"1",						CVAR_ARCHIVE, "" );
	// rjr - removed for hacking
	r_dlightBacks						= ri.Cvar_Get( "r_dlightBacks",						"1",						CVAR_ARCHIVE_ND, "dlight non-facing surfaces for continuity" );
	r_finish							= ri.Cvar_Get( "r_finish",							"0",						CVAR_ARCHIVE_ND, "" );
	r_textureMode						= ri.Cvar_Get( "r_textureMode",						"GL_LINEAR_MIPMAP_LINEAR",	CVAR_ARCHIVE, "" );
	r_markcount							= ri.Cvar_Get( "r_markcount",						"100",						CVAR_ARCHIVE_ND, "" );
	r_gamma								= ri.Cvar_Get( "r_gamma",							"1",						CVAR_ARCHIVE_ND, "" );
	r_facePlaneCull						= ri.Cvar_Get( "r_facePlaneCull",					"1",						CVAR_ARCHIVE_ND, "" );
	r_roofCullCeilDist					= ri.Cvar_Get( "r_roofCullCeilDist",				"256",						CVAR_CHEAT, "" ); //attempted smart method of culling out upwards facing surfaces on roofs for automap shots -rww
	r_roofCullFloorDist					= ri.Cvar_Get( "r_roofCeilFloorDist",				"128",						CVAR_CHEAT, "" ); //attempted smart method of culling out upwards facing surfaces on roofs for automap shots -rww
	r_primitives						= ri.Cvar_Get( "r_primitives",						"0",						CVAR_ARCHIVE_ND, "" );
	ri.Cvar_CheckRange( r_primitives, MIN_PRIMITIVES, MAX_PRIMITIVES, qtrue );
	r_ambientScale						= ri.Cvar_Get( "r_ambientScale",					"0.6",						0, "" );
	r_directedScale						= ri.Cvar_Get( "r_directedScale",					"1",						0, "" );
	r_autoMap							= ri.Cvar_Get( "r_autoMap",							"0",						CVAR_ARCHIVE_ND, "" ); //automap renderside toggle for debugging -rww
	r_autoMapBackAlpha					= ri.Cvar_Get( "r_autoMapBackAlpha",				"0",						0, "" ); //alpha of automap bg -rww
	r_autoMapDisable					= ri.Cvar_Get( "r_autoMapDisable",					"1",						0, "" );
	r_showImages						= ri.Cvar_Get( "r_showImages",						"0",						CVAR_CHEAT, "" );
	r_debugLight						= ri.Cvar_Get( "r_debuglight",						"0",						CVAR_TEMP, "" );
	r_debugSort							= ri.Cvar_Get( "r_debugSort",						"0",						CVAR_CHEAT, "" );
	r_dlightStyle						= ri.Cvar_Get( "r_dlightStyle",						"1",						CVAR_TEMP, "" );
	r_surfaceSprites					= ri.Cvar_Get( "r_surfaceSprites",					"1",						CVAR_ARCHIVE_ND, "" );
	r_surfaceWeather					= ri.Cvar_Get( "r_surfaceWeather",					"0",						CVAR_TEMP, "" );
	r_windSpeed							= ri.Cvar_Get( "r_windSpeed",						"0",						0, "" );
	r_windAngle							= ri.Cvar_Get( "r_windAngle",						"0",						0, "" );
	r_windGust							= ri.Cvar_Get( "r_windGust",						"0",						0, "" );
	r_windDampFactor					= ri.Cvar_Get( "r_windDampFactor",					"0.1",						0, "" );
	r_windPointForce					= ri.Cvar_Get( "r_windPointForce",					"0",						0, "" );
	r_windPointX						= ri.Cvar_Get( "r_windPointX",						"0",						0, "" );
	r_windPointY						= ri.Cvar_Get( "r_windPointY",						"0",						0, "" );
	r_nocurves							= ri.Cvar_Get( "r_nocurves",						"0",						CVAR_CHEAT, "" );
	r_drawworld							= ri.Cvar_Get( "r_drawworld",						"1",						CVAR_CHEAT, "" );
	r_drawfog							= ri.Cvar_Get("r_drawfog",							"2",						CVAR_ARCHIVE_ND, "Fog mode\n"
		" 0 - disabled\n"
		" 1 - legacy fog\n"
		" 2 - fog collapse\n");
	r_lightmap							= ri.Cvar_Get( "r_lightmap",						"0",						CVAR_ARCHIVE_ND, "" );
	r_distanceCull						= ri.Cvar_Get( "r_distanceCull",					"0",						CVAR_ARCHIVE_ND, "" );
	r_portalOnly						= ri.Cvar_Get( "r_portalOnly",						"0",						CVAR_CHEAT, "" );
	r_skipBackEnd						= ri.Cvar_Get( "r_skipBackEnd",						"0",						CVAR_CHEAT, "" );
	r_measureOverdraw					= ri.Cvar_Get( "r_measureOverdraw",					"0",						0, "" );
	r_lodscale							= ri.Cvar_Get( "r_lodscale",						"5",						CVAR_ARCHIVE_ND, "" );
	r_norefresh							= ri.Cvar_Get( "r_norefresh",						"0",						CVAR_CHEAT, "" );
	r_drawentities						= ri.Cvar_Get( "r_drawentities",					"1",						CVAR_CHEAT, "" );
	r_ignore							= ri.Cvar_Get( "r_ignore",							"1",						CVAR_CHEAT, "" );
	r_nocull							= ri.Cvar_Get( "r_nocull",							"0",						CVAR_CHEAT, "" );
	r_novis								= ri.Cvar_Get( "r_novis",							"0",						CVAR_CHEAT, "" );
	r_showcluster						= ri.Cvar_Get( "r_showcluster",						"0",						CVAR_CHEAT, "" );
	r_speeds							= ri.Cvar_Get( "r_speeds",							"0",						CVAR_CHEAT, "" );
	r_verbose							= ri.Cvar_Get( "r_verbose",							"0",						CVAR_CHEAT, "" );
	r_logFile							= ri.Cvar_Get( "r_logFile",							"0",						CVAR_CHEAT, "" );
	r_debugSurface						= ri.Cvar_Get( "r_debugSurface",					"0",						CVAR_CHEAT, "" );
	r_nobind							= ri.Cvar_Get( "r_nobind",							"0",						CVAR_CHEAT, "" );
	r_showtris							= ri.Cvar_Get( "r_showtris",						"0",						0, "" );
	r_showsky							= ri.Cvar_Get( "r_showsky",							"0",						CVAR_CHEAT, "" );
	r_shownormals						= ri.Cvar_Get( "r_shownormals",						"0",						CVAR_CHEAT, "" );
	r_clear								= ri.Cvar_Get( "r_clear",							"0",						CVAR_CHEAT, "" );
	r_offsetFactor						= ri.Cvar_Get( "r_offsetfactor",					"-1",						CVAR_CHEAT, "" );
	r_offsetUnits						= ri.Cvar_Get( "r_offsetunits",						"-2",						CVAR_CHEAT, "" );
	r_lockpvs							= ri.Cvar_Get( "r_lockpvs",							"0",						CVAR_CHEAT, "" );
	r_noportals							= ri.Cvar_Get( "r_noportals",						"0",						0, "" );
	r_shadows							= ri.Cvar_Get( "cg_shadows",						"1",						0, "" );
	r_shadowRange						= ri.Cvar_Get( "r_shadowRange",						"1000",						0, "" );
	r_marksOnTriangleMeshes				= ri.Cvar_Get( "r_marksOnTriangleMeshes",			"0",						CVAR_ARCHIVE_ND, "" );
	r_aspectCorrectFonts				= ri.Cvar_Get( "r_aspectCorrectFonts",				"0",						CVAR_ARCHIVE, "" );
	cl_ratioFix							= ri.Cvar_Get( "cl_ratioFix",						"1",						CVAR_ARCHIVE, "" );
	r_maxpolys							= ri.Cvar_Get( "r_maxpolys",						XSTRING( DEFAULT_MAX_POLYS ),		0, "" );
	r_maxpolyverts						= ri.Cvar_Get( "r_maxpolyverts",					XSTRING( DEFAULT_MAX_POLYVERTS ),	0, "" );

	// Vulkan
	r_defaultImage						= ri.Cvar_Get("r_defaultImage",						"",							CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	r_device							= ri.Cvar_Get("r_device",							"-1",						CVAR_ARCHIVE_ND | CVAR_LATCH, "Select physical device to render:\n" \
		" 0+ - use explicit device index\n" \
		" -1 - first discrete GPU\n" \
		" -2 - first integrated GPU");
	ri.Cvar_CheckRange(r_device, -2, 8, qtrue);
	r_device->modified					= qfalse;

	r_stencilbits						= ri.Cvar_Get("r_stencilbits",						"8",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	r_ext_multisample					= ri.Cvar_Get("r_ext_multisample",					"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_ext_multisample, 0, 64, qtrue);
	r_ext_supersample					= ri.Cvar_Get("r_ext_supersample",					"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_ext_supersample, 0, 1, qtrue);
	r_ext_alpha_to_coverage				= ri.Cvar_Get("r_ext_alpha_to_coverage",			"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_ext_alpha_to_coverage, 0, 1, qtrue);
	r_fbo								= ri.Cvar_Get("r_fbo",								"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	r_hdr								= ri.Cvar_Get("r_hdr",								"1",						CVAR_ARCHIVE | CVAR_LATCH, "");
	r_mapGreyScale						= ri.Cvar_Get("r_mapGreyScale",						"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_mapGreyScale, -1, 1, qfalse);
	r_ext_max_anisotropy				= ri.Cvar_Get("r_ext_max_anisotropy",				"2",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_ext_max_anisotropy, 1, 16, qtrue);
	r_greyscale							= ri.Cvar_Get("r_greyscale",						"0",						CVAR_ARCHIVE_ND, "");
	ri.Cvar_CheckRange(r_greyscale, -1, 1, qfalse);
	r_dither							= ri.Cvar_Get("r_dither",							"0",						CVAR_ARCHIVE_ND, "Set dithering mode:\n 0 - disabled\n 1 - ordered\nRequires " S_COLOR_CYAN "\\r_fbo 1");
	ri.Cvar_CheckRange(r_dither, 0, 1, qtrue);
	r_presentBits						= ri.Cvar_Get("r_presentBits",						"24",						CVAR_ARCHIVE_ND | CVAR_LATCH, "Select color bits used for presentation surfaces\nRequires " S_COLOR_CYAN "\\r_fbo 1");
	ri.Cvar_CheckRange(r_presentBits, 16, 30, qtrue);
	r_bloom								= ri.Cvar_Get("r_bloom",							"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_bloom, 0, 1, qtrue);
	r_bloom_threshold					= ri.Cvar_Get("r_bloom_threshold",					"0.05",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_bloom_threshold, 0.01f, 1, qfalse);
	r_bloom_intensity					= ri.Cvar_Get("r_bloom_intensity",					"0.15",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	ri.Cvar_CheckRange(r_bloom_intensity, 0.01f, 2, qfalse);
#ifdef USE_PMLIGHT
	r_dlightMode						= ri.Cvar_Get("r_dlightMode",						"2",						CVAR_ARCHIVE, "");
	ri.Cvar_CheckRange(r_dlightMode, 0, 2, qtrue);
	r_dlightScale						= ri.Cvar_Get("r_dlightScale",						"0.8",						CVAR_ARCHIVE_ND, "");
	ri.Cvar_CheckRange(r_dlightScale, 0.1f, 1, qfalse);
	r_dlightIntensity					= ri.Cvar_Get("r_dlightIntensity",					"1.0",						CVAR_ARCHIVE_ND, "");
	ri.Cvar_CheckRange(r_dlightIntensity, 0.1f, 1, qfalse);
#endif

	r_dlightSaturation					= ri.Cvar_Get("r_dlightSaturation",					"1",						CVAR_ARCHIVE_ND, "");
	ri.Cvar_CheckRange(r_dlightSaturation, 0, 1, qfalse);

	r_roundImagesDown					= ri.Cvar_Get("r_roundImagesDown",					"1",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	r_nomip								= ri.Cvar_Get("r_nomip",							"0",						CVAR_ARCHIVE | CVAR_LATCH, "Apply picmip only on worldspawn textures");
	ri.Cvar_CheckRange(r_nomip, 0, 1, qtrue);
#ifdef USE_VBO
	r_vbo								= ri.Cvar_Get("r_vbo",								"1",						CVAR_ARCHIVE | CVAR_LATCH, "");
#endif
	r_renderWidth						= ri.Cvar_Get("r_renderWidth",						"800",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	r_renderHeight						= ri.Cvar_Get("r_renderHeight",						"600",						CVAR_ARCHIVE_ND | CVAR_LATCH, "");
	r_renderScale						= ri.Cvar_Get("r_renderScale",						"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "Scaling mode to be used with custom render resolution:\n"
		" 0 - disabled\n"
		" 1 - nearest filtering, stretch to full size\n"
		" 2 - nearest filtering, preserve aspect ratio (black bars on sides)\n"
		" 3 - linear filtering, stretch to full size\n"
		" 4 - linear filtering, preserve aspect ratio (black bars on sides)\n");
	ri.Cvar_CheckRange(r_renderScale, 0, 4, qtrue);
	r_ignorehwgamma						= ri.Cvar_Get("r_ignorehwgamma",					"0",						CVAR_ARCHIVE_ND | CVAR_LATCH, "Overrides hardware gamma capabilities");
	ri.Cvar_CheckRange(r_ignorehwgamma, 0, 1, qtrue);


/*
Ghoul2 Insert Start
*/
#ifdef _DEBUG
	r_noPrecacheGLA						= ri.Cvar_Get( "r_noPrecacheGLA",					"0",						CVAR_CHEAT, "" );
#endif
	r_noGhoul2							= ri.Cvar_Get( "r_noghoul2",						"0",						CVAR_CHEAT, "" );
	r_Ghoul2AnimSmooth					= ri.Cvar_Get( "r_ghoul2animsmooth",				"0.3",						0, "" );
	r_Ghoul2UnSqashAfterSmooth			= ri.Cvar_Get( "r_ghoul2unsqashaftersmooth",		"1",						0, "" );
	broadsword							= ri.Cvar_Get( "broadsword",						"0",						CVAR_ARCHIVE_ND, "" );
	broadsword_kickbones				= ri.Cvar_Get( "broadsword_kickbones",				"1",						0, "" );
	broadsword_kickorigin				= ri.Cvar_Get( "broadsword_kickorigin",				"1",						0, "" );
	broadsword_dontstopanim				= ri.Cvar_Get( "broadsword_dontstopanim",			"0",						0, "" );
	broadsword_waitforshot				= ri.Cvar_Get( "broadsword_waitforshot",			"0",						0, "" );
	broadsword_playflop					= ri.Cvar_Get( "broadsword_playflop",				"1",						0, "" );
	broadsword_smallbbox				= ri.Cvar_Get( "broadsword_smallbbox",				"0",						0, "" );
	broadsword_extra1					= ri.Cvar_Get( "broadsword_extra1",					"0",						0, "" );
	broadsword_extra2					= ri.Cvar_Get( "broadsword_extra2",					"0",						0, "" );
	broadsword_effcorr					= ri.Cvar_Get( "broadsword_effcorr",				"1",						0, "" );
	broadsword_ragtobase				= ri.Cvar_Get( "broadsword_ragtobase",				"2",						0, "" );
	broadsword_dircap					= ri.Cvar_Get( "broadsword_dircap",					"64",						0, "" );

	sv_mapname							= ri.Cvar_Get( "mapname",							"nomap",					CVAR_SERVERINFO | CVAR_ROM, "" );
/*
Ghoul2 Insert End
*/
#ifdef JK2_MODE
	sp_language = ri.Cvar_Get("sp_language", va("%d", SP_LANGUAGE_ENGLISH), CVAR_ARCHIVE | CVAR_NORESTART, "");
#endif
	com_buildScript = ri.Cvar_Get("com_buildScript", "0", 0, "");

	r_modelpoolmegs = ri.Cvar_Get("r_modelpoolmegs", "20", CVAR_ARCHIVE, "" );
	if (ri.LowPhysicalMemory() )
		ri.Cvar_Set("r_modelpoolmegs", "0");

	r_aviMotionJpegQuality				= ri.Cvar_Get( "r_aviMotionJpegQuality",			"100",						CVAR_ARCHIVE_ND, "" );
	r_screenshotJpegQuality				= ri.Cvar_Get( "r_screenshotJpegQuality",			"100",						CVAR_ARCHIVE_ND, "" );

	ri.Cvar_CheckRange( r_aviMotionJpegQuality, 10, 100, qtrue );
	ri.Cvar_CheckRange( r_screenshotJpegQuality, 10, 100, qtrue );

	for ( size_t i = 0; i < numCommands; i++ )
		ri.Cmd_AddCommand( commands[i].cmd, commands[i].func, "" );
}

// need to do this hackery so ghoul2 doesn't crash the game because of ITS hackery...
//
void R_ClearStuffToStopGhoul2CrashingThings(void)
{
	memset(&tr, 0, sizeof(tr));
}

/*
===============
R_Init
===============
*/
extern void R_InitWorldEffects( void ); //tr_WorldEffects.cpp
void R_Init( void ) {
	int i;
	byte *ptr;

	vk_debug("----- R_Init -----\n" );
	ri.Printf(PRINT_ALL, "----- R_Init -----\n");
	// clear all our internal state
	Com_Memset( &tr, 0, sizeof( tr ) );
	Com_Memset( &backEnd, 0, sizeof( backEnd ) );
	Com_Memset( &tess, 0, sizeof( tess ) );
	//Com_Memset( &glState, 0, sizeof( glState ) );

#ifndef FINAL_BUILD
	if ( (intptr_t)tess.xyz & 15 ) {
		ri.Printf(PRINT_WARNING, "tess.xyz not 16 byte aligned\n");
	}
#endif
	//
	// init function tables
	//
	for (i = 0; i < FUNCTABLE_SIZE; i++) {
		if (i == 0) {
			tr.sinTable[i] = EPSILON;
		}
		else if (i == (FUNCTABLE_SIZE - 1)) {
			tr.sinTable[i] = -EPSILON;
		}
		else {
			tr.sinTable[i] = sin(DEG2RAD(i * 360.0f / ((float)(FUNCTABLE_SIZE - 1))));
		}
		tr.squareTable[i] = (i < FUNCTABLE_SIZE / 2) ? 1.0f : -1.0f;
		if (i == 0) {
			tr.sawToothTable[i] = EPSILON;
		}
		else {
			tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		}
		tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];
		if (i < FUNCTABLE_SIZE / 2) {
			if (i < FUNCTABLE_SIZE / 4) {
				if (i == 0) {
					tr.triangleTable[i] = EPSILON;
				}
				else {
					tr.triangleTable[i] = (float)i / (FUNCTABLE_SIZE / 4);
				}
			}
			else {
				tr.triangleTable[i] = 1.0f - tr.triangleTable[i - FUNCTABLE_SIZE / 4];
			}
		}
		else {
			tr.triangleTable[i] = -tr.triangleTable[i - FUNCTABLE_SIZE / 2];
		}
	}

	R_InitFogTable();
	R_ImageLoader_Init();
	R_NoiseInit();
	R_Register();

	max_polys = Q_min( r_maxpolys->integer, DEFAULT_MAX_POLYS );
	max_polyverts = Q_min( r_maxpolyverts->integer, DEFAULT_MAX_POLYVERTS );

	ptr = (byte *)R_Hunk_Alloc( sizeof( *backEndData ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts, qtrue);
	backEndData = (backEndData_t *) ptr;
	backEndData->polys = (srfPoly_t *) ((char *) ptr + sizeof( *backEndData ));
	backEndData->polyVerts = (polyVert_t *) ((char *) ptr + sizeof( *backEndData ) + sizeof(srfPoly_t) * max_polys);

	R_InitNextFrame();

	for(i = 0; i < MAX_LIGHT_STYLES; i++)
	{
		RE_SetLightStyle(i, -1);
	}

	vk_create_window();		// Vulkan

	R_Set2DRatio();
	R_InitImages();	

	vk_create_pipelines();	// Vulkan
	vk_set_fastsky_color();

	R_InitShaders(qfalse);
	R_InitSkins();
	R_InitFonts();
	R_ModelInit();
	R_InitDecals();
	R_InitWorldEffects();
	RestoreGhoul2InfoArray();

	vk_debug("----- finished R_Init -----\n" );
}


// This need some tlc
/*
===============
RE_Shutdown
===============
*/
void RE_Shutdown( qboolean destroyWindow, qboolean restarting ) {
	vk_debug("RE_Shutdown( %i, %i )\n", destroyWindow, restarting);

	for (size_t i = 0; i < numCommands; i++)
		ri.Cmd_RemoveCommand(commands[i].cmd);

	R_ShutdownWorldEffects();
	R_ShutdownFonts();

	// contains vulkan resources/state, reinitialized on a map change.
	if (tr.registered) {

		if (destroyWindow){
			vk_delete_textures();

			if (restarting)
				SaveGhoul2InfoArray();
		}

		vk_release_resources();
	}

	if (destroyWindow) {
		vk_shutdown();
		Com_Memset(&vk_world, 0, sizeof(vk_world));
		Com_Memset(&glState, 0, sizeof(glState));

		if (destroyWindow && !restarting) {
			ri.VK_destroyWindow();
			Com_Memset(&glConfig, 0, sizeof(glConfig));
		}
	}

	tr.registered = qfalse;
	tr.inited = qfalse;
}

/*
=============
RE_EndRegistration

Touch all images to make sure they are resident
=============
*/
void RE_EndRegistration( void ) {
	vk_wait_idle();

	// command buffer is not in recording state at this stage
	// so we can't issue RB_ShowImages() here.
	// moved to RB_SwapBuffers
}

void RE_GetLightStyle( int style, color4ub_t color )
{
	if (style >= MAX_LIGHT_STYLES)
	{
	    Com_Error( ERR_FATAL, "RE_GetLightStyle: %d is out of range", (int)style );
		return;
	}

	byteAlias_t *baDest = (byteAlias_t *)&color, 
				*baSource = (byteAlias_t *)&styleColors[style];
	baDest->i = baSource->i;
}

void RE_SetLightStyle( int style, int color )
{
	if (style >= MAX_LIGHT_STYLES)
	{
	    Com_Error( ERR_FATAL, "RE_SetLightStyle: %d is out of range", (int)style );
		return;
	}

	byteAlias_t *ba = (byteAlias_t *)&styleColors[style];
	if ( ba->i != color) {
		ba->i = color;
	}
}

static void SetRangedFog( float range ) { tr.rangedFog = range; }

extern qboolean gG2_GBMNoReconstruct;
extern qboolean gG2_GBMUseSPMethod;
static void G2API_BoltMatrixReconstruction( qboolean reconstruct ) { gG2_GBMNoReconstruct = (qboolean)!reconstruct; }
static void G2API_BoltMatrixSPMethod( qboolean spMethod ) { gG2_GBMUseSPMethod = spMethod; }

extern void R_SVModelInit( void ); //tr_model.cpp


void RE_SetRangedFog(float range)
{
	tr.rangedFog = range;
}

// STUBS, REPLACEME
void stub_RE_GetBModelVerts(int bModel, vec3_t* vec, float* normal) {}
void stub_RE_WorldEffectCommand(const char* cmd) {}
void stub_RE_AddWeatherZone(vec3_t mins, vec3_t maxs) {}
qboolean stub_RE_ProcessDissolve(void) { return qfalse; }
qboolean stub_RE_InitDissolve(qboolean bForceCircularExtroWipe) { return qfalse; }
bool stub_R_IsShaking(vec3_t pos) { return qfalse; }
void stub_R_InitWorldEffects(void) {}
bool stub_R_GetWindVector(vec3_t windVector, vec3_t atpoint) { return qfalse; }
bool stub_R_GetWindGusting(vec3_t atpoint) { return qfalse; }
bool stub_R_IsOutside(vec3_t pos) { return qfalse; }
float stub_R_IsOutsideCausingPain(vec3_t pos) { return qfalse; }
float stub_R_GetChanceOfSaberFizz() { return qfalse; }
bool stub_R_SetTempGlobalFogColor(vec3_t color) { return qfalse; }
void stub_RE_GetScreenShot(byte* buffer, int w, int h) {}

float tr_distortionAlpha = 1.0f; //opaque
float tr_distortionStretch = 0.0f; //no stretch override
qboolean tr_distortionPrePost = qfalse; //capture before postrender phase?
qboolean tr_distortionNegate = qfalse; //negative blend mode
float* stub_get_tr_distortionAlpha(void) { return &tr_distortionAlpha; }
float* stub_get_tr_distortionStretch(void) { return &tr_distortionStretch; }
qboolean* stub_get_tr_distortionPrePost(void) { return &tr_distortionPrePost; }
qboolean* stub_get_tr_distortionNegate(void) { return &tr_distortionNegate; }

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
extern void G2API_AnimateG2Models(CGhoul2Info_v& ghoul2, int AcurrentTime, CRagDollUpdateParams* params);
extern qboolean G2API_GetRagBonePos(CGhoul2Info_v& ghoul2, const char* boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale);
extern qboolean G2API_RagEffectorKick(CGhoul2Info_v& ghoul2, const char* boneName, vec3_t velocity);
extern qboolean G2API_RagForceSolve(CGhoul2Info_v& ghoul2, qboolean force);
extern qboolean G2API_SetBoneIKState(CGhoul2Info_v& ghoul2, int time, const char* boneName, int ikState, sharedSetBoneIKStateParams_t* params);
extern qboolean G2API_IKMove(CGhoul2Info_v& ghoul2, int time, sharedIKMoveParams_t* params);
extern qboolean G2API_RagEffectorGoal(CGhoul2Info_v& ghoul2, const char* boneName, vec3_t pos);
extern qboolean G2API_RagPCJGradientSpeed(CGhoul2Info_v& ghoul2, const char* boneName, const float speed);
extern qboolean G2API_RagPCJConstraint(CGhoul2Info_v& ghoul2, const char* boneName, vec3_t min, vec3_t max);
extern void G2API_SetRagDoll(CGhoul2Info_v& ghoul2, CRagDollParams* parms);
#ifdef G2_PERFORMANCE_ANALYSIS
extern void G2Time_ResetTimers(void);
extern void G2Time_ReportTimers(void);
#endif
extern IGhoul2InfoArray &TheGhoul2InfoArray();

extern "C" {
Q_EXPORT refexport_t* QDECL GetRefAPI( int apiVersion, refimport_t *rimp ) {
	static refexport_t re;

	assert( rimp );
	ri = *rimp;

	memset( &re, 0, sizeof( re ) );

	if ( apiVersion != REF_API_VERSION ) {
		vk_debug("Mismatched REF_API_VERSION: expected %i, got %i\n", REF_API_VERSION, apiVersion );
		return NULL;
	}

	// the RE_ functions are Renderer Entry points

#define REX(x)	re.x = RE_##x

	REX(Shutdown);

	REX(BeginRegistration);
	REX(RegisterModel);
	REX(RegisterSkin);
	REX(GetAnimationCFG);
	REX(RegisterShader);
	REX(RegisterShaderNoMip);
	re.LoadWorld = RE_LoadWorldMap;
	re.R_LoadImage = R_LoadImage;

	REX(RegisterMedia_LevelLoadBegin);
	REX(RegisterMedia_LevelLoadEnd);
	REX(RegisterMedia_GetLevel);
	REX(RegisterImages_LevelLoadEnd);
	REX(RegisterModels_LevelLoadEnd);

	REX(SetWorldVisData);

	REX(EndRegistration);

	REX(ClearScene);
	REX(AddRefEntityToScene);
	REX(GetLighting);
	REX(AddPolyToScene);
	REX(AddLightToScene);
	REX(RenderScene);
	REX(GetLighting);

	REX(SetColor);
	re.DrawStretchPic = RE_StretchPic;
	re.DrawRotatePic = RE_RotatePic;
	re.DrawRotatePic2 = RE_RotatePic2;
	REX(LAGoggles);
	REX(Scissor);

	re.DrawStretchRaw = RE_StretchRaw;
	REX(UploadCinematic);

	REX(BeginFrame);
	REX(EndFrame);

	re.ProcessDissolve = stub_RE_ProcessDissolve;
	re.InitDissolve = stub_RE_InitDissolve;

	re.GetScreenShot = stub_RE_GetScreenShot;
#ifdef JK2_MODE
	REX(SaveJPGToBuffer);
	re.LoadJPGFromBuffer = LoadJPGFromBuffer;

	REX(TempRawImage_ReadFromFile);
	REX(TempRawImage_CleanUp);
#endif

	re.MarkFragments = R_MarkFragments;
	re.LerpTag = R_LerpTag;
	re.ModelBounds = R_ModelBounds;
	REX(GetLightStyle);
	REX(SetLightStyle);
	re.GetBModelVerts = stub_RE_GetBModelVerts;
	re.WorldEffectCommand = stub_RE_WorldEffectCommand;
	//REX(GetModelBounds);  //Not used by game code, do we really need it?

	re.SVModelInit = R_SVModelInit;

	REX(RegisterFont);
	REX(Font_HeightPixels);
	REX(Font_StrLenPixels);
	REX(Font_DrawString);
	REX(Font_StrLenChars);
	re.FontRatioFix = RE_FontRatioFix;
	re.Language_IsAsian = Language_IsAsian;
	re.Language_UsesSpaces = Language_UsesSpaces;
	re.AnyLanguage_ReadCharFromString = AnyLanguage_ReadCharFromString;
#ifdef JK2_MODE
	re.AnyLanguage_ReadCharFromString2 = AnyLanguage_ReadCharFromString_JK2;
#endif

	re.R_InitWorldEffects = R_InitWorldEffects;
	re.R_ClearStuffToStopGhoul2CrashingThings = R_ClearStuffToStopGhoul2CrashingThings;
	re.R_inPVS = R_inPVS;

	re.tr_distortionAlpha = stub_get_tr_distortionAlpha;
	re.tr_distortionStretch = stub_get_tr_distortionStretch;
	re.tr_distortionPrePost = stub_get_tr_distortionPrePost;
	re.tr_distortionNegate = stub_get_tr_distortionNegate;

	re.GetWindVector = stub_R_GetWindVector;
	re.GetWindGusting = stub_R_GetWindGusting;
	re.IsOutside = stub_R_IsOutside;
	re.IsOutsideCausingPain = stub_R_IsOutsideCausingPain;
	re.GetChanceOfSaberFizz = stub_R_GetChanceOfSaberFizz;
	re.IsShaking = stub_R_IsShaking;
	re.AddWeatherZone = stub_RE_AddWeatherZone;
	re.SetTempGlobalFogColor = stub_R_SetTempGlobalFogColor;

	REX(SetRangedFog);

	re.TheGhoul2InfoArray = TheGhoul2InfoArray;

#define G2EX(x)	re.G2API_##x = G2API_##x

	G2EX(AddBolt);
	G2EX(AddBoltSurfNum);
	G2EX(AddSurface);
	G2EX(AnimateG2Models);
	G2EX(AttachEnt);
	G2EX(AttachG2Model);
	G2EX(CollisionDetect);
	G2EX(CleanGhoul2Models);
	G2EX(CopyGhoul2Instance);
	G2EX(DetachEnt);
	G2EX(DetachG2Model);
	G2EX(GetAnimFileName);
	G2EX(GetAnimFileNameIndex);
	G2EX(GetAnimFileInternalNameIndex);
	G2EX(GetAnimIndex);
	G2EX(GetAnimRange);
	G2EX(GetAnimRangeIndex);
	G2EX(GetBoneAnim);
	G2EX(GetBoneAnimIndex);
	G2EX(GetBoneIndex);
	G2EX(GetBoltMatrix);
	G2EX(GetGhoul2ModelFlags);
	G2EX(GetGLAName);
	G2EX(GetParentSurface);
	G2EX(GetRagBonePos);
	G2EX(GetSurfaceIndex);
	G2EX(GetSurfaceName);
	G2EX(GetSurfaceRenderStatus);
	G2EX(GetTime);
	G2EX(GiveMeVectorFromMatrix);
	G2EX(HaveWeGhoul2Models);
	G2EX(IKMove);
	G2EX(InitGhoul2Model);
	G2EX(IsPaused);
	G2EX(ListBones);
	G2EX(ListSurfaces);
	G2EX(LoadGhoul2Models);
	G2EX(LoadSaveCodeDestructGhoul2Info);
	G2EX(PauseBoneAnim);
	G2EX(PauseBoneAnimIndex);
	G2EX(PrecacheGhoul2Model);
	G2EX(RagEffectorGoal);
	G2EX(RagEffectorKick);
	G2EX(RagForceSolve);
	G2EX(RagPCJConstraint);
	G2EX(RagPCJGradientSpeed);
	G2EX(RemoveBolt);
	G2EX(RemoveBone);
	G2EX(RemoveGhoul2Model);
	G2EX(RemoveSurface);
	G2EX(SaveGhoul2Models);
	G2EX(SetAnimIndex);
	G2EX(SetBoneAnim);
	G2EX(SetBoneAnimIndex);
	G2EX(SetBoneAngles);
	G2EX(SetBoneAnglesIndex);
	G2EX(SetBoneAnglesMatrix);
	G2EX(SetBoneIKState);
	G2EX(SetGhoul2ModelFlags);
	G2EX(SetGhoul2ModelIndexes);
	G2EX(SetLodBias);
	//G2EX(SetModelIndexes);
	G2EX(SetNewOrigin);
	G2EX(SetRagDoll);
	G2EX(SetRootSurface);
	G2EX(SetShader);
	G2EX(SetSkin);
	G2EX(SetSurfaceOnOff);
	G2EX(SetTime);
	G2EX(StopBoneAnim);
	G2EX(StopBoneAnimIndex);
	G2EX(StopBoneAngles);
	G2EX(StopBoneAnglesIndex);
#ifdef _G2_GORE
	G2EX(AddSkinGore);
	G2EX(ClearSkinGore);
#endif

#ifdef G2_PERFORMANCE_ANALYSIS
	re.G2Time_ReportTimers = G2Time_ReportTimers;
	re.G2Time_ResetTimers = G2Time_ResetTimers;
#endif

	//Swap_Init();

	return &re;
}

} //extern "C"
