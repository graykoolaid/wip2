#include "Object.h"

Object::Object()
{

}
//---------------------------------------------------------------------------------------
// Character Loader here
//---------------------------------------------------------------------------------------
void Object::objLoad( char* filename, vector<const wchar_t *> *textures, vector<const wchar_t *> *NormTextures, ID3D11Device* devv )
{
	dev1 = devv;
	vector<XMFLOAT3> vertexices;
	vector<XMFLOAT3> normexices;
	vector<XMFLOAT3> texexices;
	vector<WORD> indexices;
	vector<int> texNo;
	int tempTexCount;

	vector<Vertex> verts;

//	Object tempO;
//	objList.push_back( tempO );
//	objDex = objList.size() - 1;
//	tempTexCount = Import( filename, &vertices );
	numMeshes = Import( filename, &vertexes );
	//numMeshes -= 1;
	//vertices = verts[0];
	
//	tempO.indices = indexices;
//	tempO.vertices  = tempO;
	ID3D11Buffer* tempVB;
	// Load the Vertex Buffer

	for( int i = 0; i < numMeshes; i++ )
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( Vertex ) * vertexes[i].size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &vertexes[i][0];
		hr1 = dev1->CreateBuffer( &bd, &InitData, &tempVB );
		if( FAILED( hr1 ) )
			return;

		vertexBuffer.push_back( tempVB );
	}
	// Save the Vertex Buffer for easy access
	//vertexBuffer = pVBuffer1;

	//// Load the Indice Buffer
	//bd.Usage = D3D10_USAGE_DEFAULT;
	//bd.ByteWidth = sizeof( int ) * indices_vec.size();
	//bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	//bd.CPUAccessFlags = 0;
	//bd.MiscFlags = 0;
	//InitData.pSysMem = &indices_vec[0];
	//hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
	//if( FAILED( hr ) )
	//	return;

	// Save the Index Buffer for easy access
//	indexBuffer = g_pIndexBuffer;

	for( int i = 0; i < textures->size(); i++ )
	{
		hr1 = D3DX11CreateShaderResourceViewFromFile( dev1, textures[0][i], NULL, NULL, &g_pTextureRV1, NULL );
		texArray.push_back( g_pTextureRV1 );
		hr1 = D3DX11CreateShaderResourceViewFromFile( dev1, NormTextures[0][i], NULL, NULL, &g_pTextureRV1, NULL );
		NormArray.push_back( g_pTextureRV1 );
	}
	
	//numMeshes = 1;//tempTexCount;
	alpha = 0;
	textures->resize(0);
	NormTextures->resize(0);
	return;
}

void Object::render( ID3D11DeviceContext * devcon)
{
        UINT stride = sizeof(Vertex);
        UINT offset = 0;

		for( int i = 0; i < numMeshes; i++ )
		{
			devcon->IASetInputLayout(objLayout);
			devcon->PSSetShaderResources(0, 1, &texArray[i] );
			devcon->PSSetShaderResources(1, 1, &NormArray[i] );

			devcon->IASetVertexBuffers(0, 1, &vertexBuffer[i], &stride, &offset);

			//devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);

			// select which primtive type we are using
			devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// draw the vertex buffer to the back buffer
		   // devcon->DrawIndexed(36, 0, 0);

			devcon->Draw( vertexes[i].size(),0);
		}
}