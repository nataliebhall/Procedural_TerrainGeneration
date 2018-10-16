////////////////////////////////////////////////////////////////////////////////
// Filename: skydomeclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "skydomeclass.h"

//The class constructor initializes the private member pointers to null.
SkyDomeClass::SkyDomeClass()
{
	m_model = 0;
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}


SkyDomeClass::SkyDomeClass(const SkyDomeClass& other)
{
}


SkyDomeClass::~SkyDomeClass()
{
}

//The Initialize function first loads the sky dome model into the m_model structure.After that InitializeBuffers is called which loads the sky dome model into a vertex and index buffer that can be rendered by the video card.And finally we set the two colors of the sky dome.The apex color is the color at the top of the sky dome.The center color is the color at the horizon of the sky dome.In this tutorial everything below the horizon is set the be the same color as the horizon.So basically the gradient only goes from the top of the sky dome to the horizon.
bool SkyDomeClass::Initialize(ID3D11Device* device)
{
	bool result;


	// Load in the sky dome model.
	result = LoadSkyDomeModel("../Engine/data/skydome.txt");
	if (!result)
	{
		return false;
	}

	// Load the sky dome into a vertex and index buffer for rendering.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	// Set the color at the top of the sky dome.
	m_apexColor = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);

	// Set the color at the center of the sky dome.
	//red 0.419608 green 0.137255 blue 0.556863
	m_centerColor = D3DXVECTOR4(0.0f, 0.0f, 0.25f, 1.0f);

	return true;
}

//The Shutdown function releases the sky dome model and the vertex and index buffers.
void SkyDomeClass::Shutdown()
{
	// Release the vertex and index buffer that were used for rendering the sky dome.
	ReleaseBuffers();

	// Release the sky dome model.
	ReleaseSkyDomeModel();

	return;
}

//The Render function calls the RenderBuffers function to put the sky dome geometry on the graphics pipeline for rendering.
void SkyDomeClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Render the sky dome.
	RenderBuffers(deviceContext);

	return;
}

//GetIndexCount returns the index count and is used for rendering the sky dome.
int SkyDomeClass::GetIndexCount()
{
	return m_indexCount;
}

//GetApexColor returns the color of the sky dome at the very top.
D3DXVECTOR4 SkyDomeClass::GetApexColor()
{
	return m_apexColor;
}

//GetCenterColor returns the color of the sky dome at the horizon(or 0.0f to be exact).
D3DXVECTOR4 SkyDomeClass::GetCenterColor()
{
	return m_centerColor;
}

//The LoadSkyDomeModel function loads in the sky dome model from our file format which was created by converting the.obj formatted sphere model.
bool SkyDomeClass::LoadSkyDomeModel(char* filename)
{
	ifstream fin;
	char input;
	int i;


	// Open the model file.
	fin.open(filename);

	// If it could not open the file then exit.
	if (fin.fail())
	{
		return false;
	}

	// Read up to the value of vertex count.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the model using the vertex count that was read in.
	m_model = new ModelType[m_vertexCount];
	if (!m_model)
	{
		return false;
	}

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for (i = 0; i<m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	// Close the model file.
	fin.close();

	return true;
}

//The ReleaseSkyDomeModel function releases the sky dome model structure.
void SkyDomeClass::ReleaseSkyDomeModel()
{
	if (m_model)
	{
		delete[] m_model;
		m_model = 0;
	}

	return;
}

//The InitializeBuffers function loads the sky dome model structure into the vertex and index buffer.
bool SkyDomeClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;


	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// Load the vertex array and index array with data.
	for (i = 0; i<m_vertexCount; i++)
	{
		vertices[i].position = D3DXVECTOR3(m_model[i].x, m_model[i].y, m_model[i].z);
		indices[i] = i;
	}

	// Set up the description of the vertex buffer.
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

	// Now finally create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the index buffer.
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
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

//The ReleaseBuffers function releases the vertex and index buffer that were used to render the sky dome.
void SkyDomeClass::ReleaseBuffers()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

//RenderBuffers puts the sky dome geometry on the graphics pipe line for rendering.
void SkyDomeClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
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