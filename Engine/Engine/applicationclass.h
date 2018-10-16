////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_


/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "inputclass.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "terrainclass.h"
#include "timerclass.h"
#include "positionclass.h"
#include "fpsclass.h"
#include "cpuclass.h"
#include "fontshaderclass.h"
#include "textclass.h"
#include "terrainshaderclass.h"
#include "lightclass.h"
#include "skydomeclass.h"
#include "skydomeshaderclass.h"
#include "skyplaneclass.h"
#include "skyplaneshaderclass.h"
#include "rendertotexclass.h"
#include "perlintexshader.h"
#include "Orthoclass.h"
#include "horizontalblurshaderclass.h"
#include "verticalblurshaderclass.h"
#include "textureshaderclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ApplicationClass
////////////////////////////////////////////////////////////////////////////////
class ApplicationClass
{
public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	bool HandleInput(float);
	bool RenderGraphics();
	bool RenderSceneToTexture();
	bool DownSampleTexture();
	bool RenderHorizontalBlurToTexture();
	bool RenderVerticalBlurToTexture();
	bool UpSampleTexture();
	bool Render2DTextureScene();

private:
	// Object Classes
	InputClass* m_Input;
	D3DClass* m_Direct3D;
	CameraClass* m_Camera;
	TerrainClass* m_Terrain;
	TimerClass* m_Timer;
	PositionClass* m_Position;
	FpsClass* m_Fps;
	CpuClass* m_Cpu;
	TextClass* m_Text;
	LightClass* m_Light;
	SkyDomeClass* m_SkyDome;
	SkyPlaneClass* m_SkyPlane;

	// Shader Classes
	FontShaderClass* m_FontShader;
	TerrainShaderClass* m_TerrainShader;
	SkyDomeShaderClass* m_SkyDomeShader;
	SkyPlaneShaderClass* m_SkyPlaneShader;
	HorizontalBlurShaderClass* m_HorizontalBlurShader;
	VerticalBlurShaderClass* m_VerticalBlurShader;
	TextureShaderClass* m_TextureShader;

	// Render to Textures
	RenderToTexClass* m_RenderTexture;
	RenderToTexClass* m_DownSampleTexture;
	RenderToTexClass* m_HorizontalBlurTexture;
	RenderToTexClass* m_VerticalBlurTexture;
	RenderToTexClass* m_UpSampleTexture;

	// Ortho Meshes
	OrthoClass* m_SmallWindow;
	OrthoClass* m_FullScreenWindow;

	bool blur;
};

#endif