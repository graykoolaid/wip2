#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>
#include "importer.h"
//#include "struct.h"

using namespace std;





class Object
{
public:

	int numMeshes;
	int alpha;
	vector<Vertex>		vertices;
	ID3D11Buffer*		vertexBuffer;
	vector<ID3D11ShaderResourceView*> texArray;
	vector<ID3D11ShaderResourceView*> NormArray;

	Object();
	void objLoad( char* filename, vector<const wchar_t *> *textures, vector<const wchar_t *> *NormTextures, ID3D11Device* devv );

private:
	HRESULT hr1;
	vector<Object> objList;
	int objDex;

	ID3D11Device *dev1;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *devcon1;           // the pointer to our Direct3D device context

	ID3D11Buffer *pVBuffer1;                // the pointer to the vertex buffer
	ID3D11Buffer *pIBuffer1;
	ID3D11Buffer *pCBuffer1;                // the pointer to the constant buffer

	ID3D11ShaderResourceView*   g_pTextureRV1;

	ID3D11ShaderResourceView**  g_pTexArray1;
	ID3D11ShaderResourceView**  g_pNormArray1;

};
#endif