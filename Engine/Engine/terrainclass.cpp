////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "terrainclass.h"
#include <cmath>

TerrainClass::TerrainClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_heightMap = 0;
	m_terrainGeneratedToggle = false;
	m_GrassTexture = 0;
	m_SlopeTexture = 0;
	m_RockTexture = 0;
}

TerrainClass::TerrainClass(const TerrainClass& other)
{
}

TerrainClass::~TerrainClass()
{
}

bool TerrainClass::InitializeTerrain(ID3D11Device* device, int terrainWidth, int terrainHeight, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename)
{
	int index;
	float height = 0.0;
	bool result;

	// Save the dimensions of the terrain.
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	// Create the structure to hold the terrain data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if(!m_heightMap)
	{
		return false;
	}

	// Initialise the data in the height map (flat).
	for(int j=0; j<m_terrainHeight; j++)
	{
		for(int i=0; i<m_terrainWidth; i++)
		{			
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

		}
	}

	//even though we are generating a flat terrain, we still need to normalise it. 
	// Calculate the normals for the terrain data.
	result = CalculateNormals();
	if(!result)
	{
		return false;
	}

	// Calculate the texture coordinates.
	CalculateTextureCoordinates();

	// Load the textures.
	result = LoadTextures(device, grassTextureFilename, slopeTextureFilename, rockTextureFilename);
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	// Potential drop-points
	srand(time(NULL));

	GenerateLandscape(false, device);

	return true;
}

bool TerrainClass::GenerateLandscape(bool volcano, ID3D11Device* device)
{
	// Low frequency, tall perlin to create a hilly base terrain
	GenerateHeightMap(device, MOUNTAINS);

	// Particle deposition to create a large central mountain
	ParticleDeposition(device, 8256, 25);

	// Smooth the height map to get rid of sharp points
	SmoothHeightMap(device);

	// High frequency, short perlin to create small details in the terrain
	GenerateHeightMap(device, RIDGES);

	// Invert the y values above a certain point to create crater
	InvertVolcano(device);

	// Smooth again to get rid of sharp edges around crater
	SmoothHeightMap(device);

	return true;
}

void TerrainClass::Shutdown()
{
	// Release the vertex and index buffer.
	ShutdownBuffers();

	// Release the height map data.
	ShutdownHeightMap();

	// Release the textures.
	ReleaseTextures();

	return;
}

void TerrainClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}

int TerrainClass::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* TerrainClass::GetGrassTexture()
{
	return m_GrassTexture->GetTexture();
}

ID3D11ShaderResourceView* TerrainClass::GetSlopeTexture()
{
	return m_SlopeTexture->GetTexture();
}

ID3D11ShaderResourceView* TerrainClass::GetRockTexture()
{
	return m_RockTexture->GetTexture();
}

bool TerrainClass::GenerateHeightMap(ID3D11Device* device, PerlinType type)
{
	float vec2[2];
	bool result;
	int index;
	float height = 0.0;
		
	x_pos += 1.0f;
	y_pos += 1.0f;

	//loop through the terrain and set the heights how we want. This is where we generate the terrain

 	for(int j=0; j<m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			if (type == RIDGES)
			{
				vec2[0] = (i + x_pos) / 2;
				vec2[1] = (j + y_pos) / 2;

				// Noise
				m_heightMap[index].x = (float)i;
				m_heightMap[index].y += (float)perlin.noise2(vec2); // generate a random number for the height 
				m_heightMap[index].z = (float)j;
			}
			else if (type == MOUNTAINS)
			{
				vec2[0] = (i + x_pos) / 12;
				vec2[1] = (j + y_pos) / 12;

				// Noise
				m_heightMap[index].x = (float)i;
				m_heightMap[index].y += (float)perlin.noise2(vec2) * 10; // generate a random number for the height 
				m_heightMap[index].z = (float)j;
			}
		}
	}

	result = CalculateNormals();
	if(!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}
	return true;
}

bool TerrainClass::ParticleDeposition(ID3D11Device* device, int center_point, int height)
{
	float vec2[2];
	bool result;
	//the toggle is just a bool that I use to make sure this is only called ONCE when you press a key
	//until you release the key and start again. We dont want to be generating the terrain 500
	//times per second. 
		int drop_points[25];
		bool isStable = false;
		bool tall = false;
		int index = 0;
		
		// Calculate the neighbours - all possible drop points
		drop_points[0] = center_point - 258;
		drop_points[1] = center_point - 257;
		drop_points[2] = center_point - 256;
		drop_points[3] = center_point - 255;
		drop_points[4] = center_point - 254;
		drop_points[5] = center_point - 130;
		drop_points[6] = center_point - 129;
		drop_points[7] = center_point - 128;
		drop_points[8] = center_point - 127;
		drop_points[9] = center_point - 126;
		drop_points[10] = center_point - 2;
		drop_points[11] = center_point - 1;
		drop_points[12] = center_point;
		drop_points[13] = center_point + 1;
		drop_points[14] = center_point + 2;
		drop_points[15] = center_point + 126;
		drop_points[16] = center_point + 127;
		drop_points[17] = center_point + 128;
		drop_points[18] = center_point + 129;
		drop_points[19] = center_point + 130;
		drop_points[20] = center_point + 254;
		drop_points[21] = center_point + 255;
		drop_points[22] = center_point + 256;
		drop_points[23] = center_point + 257;
		drop_points[24] = center_point + 258;
		
		int rando = rand() % 25;
		drop = drop_points[rando];
		
		while(!tall)
		{
			int count = 0;
			int particle_neighbours[8];

			// Check if neghbours are lower - if so, add them to the array
			if (m_heightMap[drop - 127].y < m_heightMap[drop].y)
			{
				particle_neighbours[count] = (drop - 127);
				count++;
			}
			if (m_heightMap[drop - 128].y < m_heightMap[drop].y )
			{
				particle_neighbours[count] = (drop - 128);
				count++;
			}
			if (m_heightMap[drop - 129].y < m_heightMap[drop].y)
			{
				particle_neighbours[count] = (drop - 129);
				count++;
			}
			if (m_heightMap[drop - 1].y < m_heightMap[drop].y)
			{
				particle_neighbours[count] = (drop - 1);
				count++;
			}
			if (m_heightMap[drop + 1].y < m_heightMap[drop].y)
			{
				particle_neighbours[count] = (drop + 1);
				count++;
			}
			if (m_heightMap[drop + 127].y < m_heightMap[drop].y)
			{
				particle_neighbours[count] = (drop + 127);
				count++;
			}
			if (m_heightMap[drop + 128].y < m_heightMap[drop].y)
			{
				particle_neighbours[count] = (drop + 128);
				count++;
			}
			if (m_heightMap[drop + 129].y < m_heightMap[drop].y)
			{
				particle_neighbours[count] = (drop + 129);
				count++;
			}

			// If no neighbours are lower - particle is stable, raise point
			if (count == 0)
			{
				m_heightMap[drop].y += 3;
				
				if (m_heightMap[drop].y >= height)
				{
					tall = true;
					break;
				}

				int new_rando = rand() % 25;
				drop = drop_points[new_rando];
			}
			else
			{
				drop = particle_neighbours[rand() % count];
			}
		}



		result = CalculateNormals();
		if (!result)
		{
			return false;
		}

		// Initialize the vertex and index buffer that hold the geometry for the terrain.
		result = InitializeBuffers(device);
		if (!result)
		{
			return false;
		}



	return true;
}

bool TerrainClass::SmoothHeightMap(ID3D11Device* device)
{
	float vec2[2];
	bool result;
	int index;
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			if (index - 127 >= 0)
			{
				m_heightMap[index].y += m_heightMap[index - 127].y;
				count++;
			}

			if (index - 128 >= 0)
			{
				m_heightMap[index].y += m_heightMap[index - 128].y;
				count++;
			}

			if (index - 129 >= 0)
			{
				m_heightMap[index].y += m_heightMap[index - 129].y;
				count++;
			}

			if (index - 1 >= 0)
			{
				m_heightMap[index].y += m_heightMap[index - 1].y;
				count++;
			}

			if (index + 1 <= m_terrainWidth * m_terrainHeight - 1)
			{
				m_heightMap[index].y += m_heightMap[index + 1].y;
				count++;
			}

			if (index + 127 <= m_terrainWidth * m_terrainHeight - 1)
			{
				m_heightMap[index].y += m_heightMap[index + 127].y;
				count++;
			}

			if (index + 128 <= m_terrainWidth * m_terrainHeight - 1)
			{
				m_heightMap[index].y += m_heightMap[index + 128].y;
				count++;
			}

			if (index + 129 <= m_terrainWidth * m_terrainHeight - 1)
			{
				m_heightMap[index].y += m_heightMap[index + 129].y;
				count++;
			}

			m_heightMap[index].y /= count;
			count = 1;
		}
	}


	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	return true;
}

bool TerrainClass::InvertVolcano(ID3D11Device* device)
{
	bool result;
	int index;
	float max_y = 20.0f;

	// Invert the top of the volcano
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			if (m_heightMap[index].y >= max_y)
			{
				m_heightMap[index].y = max_y - ((m_heightMap[index].y - max_y) * 1.5);
			}
		}
	}

	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	return true;
}

bool TerrainClass::CollisionDetection(ID3D11Device* device, bool keydown, D3DXVECTOR3 cameraPos)
{
	bool result;
	//the toggle is just a bool that I use to make sure this is only called ONCE when you press a key
	//until you release the key and start again. We dont want to be generating the terrain 500
	//times per second. 
	if (keydown && (!m_terrainGeneratedToggle))
	{
		int index;
		float height = 0.0;
		can_move = true;

		int x = cameraPos.x;
		int y = cameraPos.y;
		int z = cameraPos.z;

		quickVect orig(x, y, z);

		quickVect point_1(m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth))].x,
							m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth))].y, 
							m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth))].z);

		quickVect point_2(m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth)) + 1].x,
							m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth)) + 1].y,
							m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth)) + 1].z);

		quickVect point_3(m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth)) + m_terrainHeight].x,
							m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth)) + m_terrainHeight].y,
							m_heightMap[(int)(orig.x + (orig.z * m_terrainWidth)) + m_terrainHeight].z);

		quickVect dir(0, -1, 0);

		quickVect intersection_point;

		raytriangle raytri;

		if (raytri.RayIntersectsTriangle(orig, dir, point_1, point_2, point_3, intersection_point))
		{
			if (cameraPos.y < (intersection_point.y + 4))
			{
				can_move = false;
			}
			else
			{
				can_move = true;
			}
		}
		else
		{
			can_move = true;
		}

		result = CalculateNormals();
		if (!result)
		{
			return false;
		}

		// Initialize the vertex and index buffer that hold the geometry for the terrain.
		result = InitializeBuffers(device);
		if (!result)
		{
			return false;
		}

		m_terrainGeneratedToggle = true;
	}
	else
	{
		m_terrainGeneratedToggle = false;
	}
	return true;
}

bool TerrainClass::LoadHeightMap(char* filename)
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;


	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_terrainWidth = bitmapInfoHeader.biWidth;
	m_terrainHeight = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_terrainWidth * m_terrainHeight * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if(!m_heightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k=0;

	// Read the image data into the height map.
	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			height = bitmapImage[k];
			
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

			k+=3;
		}
	}

	// Release the bitmap image data.
	delete [] bitmapImage;
	bitmapImage = 0;

	return true;
}

void TerrainClass::NormalizeHeightMap()
{
	int i, j;


	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			m_heightMap[(m_terrainHeight * j) + i].y /= 15.0f;
		}
	}

	return;
}

bool TerrainClass::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	VectorType* normals;


	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new VectorType[(m_terrainHeight-1) * (m_terrainWidth-1)];
	if(!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for(j=0; j<(m_terrainHeight-1); j++)
	{
		for(i=0; i<(m_terrainWidth-1); i++)
		{
			index1 = (j * m_terrainHeight) + i;
			index2 = (j * m_terrainHeight) + (i+1);
			index3 = ((j+1) * m_terrainHeight) + i;

			// Get three vertices from the face.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;
		
			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;
		
			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainHeight-1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if(((i-1) >= 0) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if((i < (m_terrainWidth-1)) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if(((i-1) >= 0) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if((i < (m_terrainWidth-1)) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}
			
			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));
			
			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete [] normals;
	normals = 0;

	return true;
}

void TerrainClass::ShutdownHeightMap()
{
	if(m_heightMap)
	{
		delete [] m_heightMap;
		m_heightMap = 0;
	}

	return;
}

bool TerrainClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index1, index2, index3, index4;
	float tu, tv;

	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;

	// Load the vertex and index array with the terrain data.
	for (j = 0; j<(m_terrainHeight - 1); j++)
	{
		for (i = 0; i<(m_terrainWidth - 1); i++)
		{
			index1 = (m_terrainHeight * j) + i;          // Bottom left.
			index2 = (m_terrainHeight * j) + (i + 1);      // Bottom right.
			index3 = (m_terrainHeight * (j + 1)) + i;      // Upper left.
			index4 = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.

															 // Upper left.
			tv = m_heightMap[index3].tv;

			// Modify the texture coordinates to cover the top edge.
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].texture = D3DXVECTOR2(m_heightMap[index3].tu, tv);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
			indices[index] = index;
			index++;

			// Upper right.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// Modify the texture coordinates to cover the top and right edge.
			if (tu == 0.0f) { tu = 1.0f; }
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].texture = D3DXVECTOR2(tu, tv);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].texture = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].texture = D3DXVECTOR2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// Upper right.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// Modify the texture coordinates to cover the top and right edge.
			if (tu == 0.0f) { tu = 1.0f; }
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].texture = D3DXVECTOR2(tu, tv);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// Bottom right.
			tu = m_heightMap[index2].tu;

			// Modify the texture coordinates to cover the right edge.
			if (tu == 0.0f) { tu = 1.0f; }

			vertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].texture = D3DXVECTOR2(tu, m_heightMap[index2].tv);
			vertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
			indices[index] = index;
			index++;
		}
	}

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void TerrainClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

void TerrainClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

void TerrainClass::CalculateTextureCoordinates()
{
	int incrementCount, i, j, tuCount, tvCount;
	float incrementValue, tuCoordinate, tvCoordinate;


	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)m_terrainWidth;

	// Calculate how many times to repeat the texture.
	incrementCount = m_terrainWidth / TEXTURE_REPEAT;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
	for (j = 0; j<m_terrainHeight; j++)
	{
		for (i = 0; i<m_terrainWidth; i++)
		{
			// Store the texture coordinate in the height map.
			m_heightMap[(m_terrainHeight * j) + i].tu = tuCoordinate;
			m_heightMap[(m_terrainHeight * j) + i].tv = tvCoordinate;

			// Increment the tu texture coordinate by the increment value and increment the index by one.
			tuCoordinate += incrementValue;
			tuCount++;

			// Check if at the far right end of the texture and if so then start at the beginning again.
			if (tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		// Increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		// Check if at the top of the texture and if so then start at the bottom again.
		if (tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}

	return;
}

bool TerrainClass::LoadTextures(ID3D11Device* device, WCHAR* grassTextureFilename, WCHAR* slopeTextureFilename, WCHAR* rockTextureFilename)
{
	bool result;


	// Create the grass texture object.
	m_GrassTexture = new TextureClass;
	if (!m_GrassTexture)
	{
		return false;
	}

	// Initialize the grass texture object.
	result = m_GrassTexture->Initialize(device, grassTextureFilename);
	if (!result)
	{
		return false;
	}

	// Create the slope texture object.
	m_SlopeTexture = new TextureClass;
	if (!m_SlopeTexture)
	{
		return false;
	}

	// Initialize the slope texture object.
	result = m_SlopeTexture->Initialize(device, slopeTextureFilename);
	if (!result)
	{
		return false;
	}

	// Create the rock texture object.
	m_RockTexture = new TextureClass;
	if (!m_RockTexture)
	{
		return false;
	}

	// Initialize the rock texture object.
	result = m_RockTexture->Initialize(device, rockTextureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

void TerrainClass::ReleaseTextures()
{
	// Release the texture objects.
	if (m_GrassTexture)
	{
		m_GrassTexture->Shutdown();
		delete m_GrassTexture;
		m_GrassTexture = 0;
	}

	if (m_SlopeTexture)
	{
		m_SlopeTexture->Shutdown();
		delete m_SlopeTexture;
		m_SlopeTexture = 0;
	}

	if (m_RockTexture)
	{
		m_RockTexture->Shutdown();
		delete m_RockTexture;
		m_RockTexture = 0;
	}

	return;
}
