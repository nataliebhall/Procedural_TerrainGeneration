////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <stdio.h>
#include "perlin_noise.h"
#include "raytriangle.h"
#include "quickVect.h"
#include <time.h>
#include "textureclass.h"

const int TEXTURE_REPEAT = 32;

////////////////////////////////////////////////////////////////////////////////
// Class name: TerrainClass
////////////////////////////////////////////////////////////////////////////////
class TerrainClass
{
private:
	enum PerlinType
	{
		MOUNTAINS,
		RIDGES,
		RESET
	};

	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	    D3DXVECTOR3 normal;
	};

	struct HeightMapType 
	{ 
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType 
	{ 
		float x, y, z;
	};

public:
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool InitializeTerrain(ID3D11Device*, int terrainWidth, int terrainHeight, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename,
		WCHAR* rockTextureFilename);
	bool GenerateLandscape(bool volcano, ID3D11Device* device);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device* device, PerlinType type);
	bool ParticleDeposition(ID3D11Device* device, int point, int height);
	bool SmoothHeightMap(ID3D11Device* device);
	bool InvertVolcano(ID3D11Device* device);
	bool CollisionDetection(ID3D11Device* device, bool keydown, D3DXVECTOR3 camera);
	int  GetIndexCount();
	bool GetMove() { return can_move; }
	void CalculateTextureCoordinates();
	bool LoadTextures(ID3D11Device*, WCHAR*, WCHAR*, WCHAR*);
	void ReleaseTextures();

	ID3D11ShaderResourceView* GetGrassTexture();
	ID3D11ShaderResourceView* GetSlopeTexture();
	ID3D11ShaderResourceView* GetRockTexture();

private:
	bool LoadHeightMap(char*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	
private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	HeightMapType* m_heightMap;

	TextureClass *m_GrassTexture, *m_SlopeTexture, *m_RockTexture;

	perlin_noise perlin;

	int min = -10; 
	int max = 10;
	int count = 1;
	float x_pos = 1.0f;
	float y_pos = 1.0f;
	bool can_move = true;
	int drop = 0;

};

#endif