#pragma once

#ifndef IMPORTER_H
#define IMPORTER_H

#include <fbxsdk.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>
#include <direct.h>
#include "struct.h"
using namespace std;

//struct Vertex
//{
//	XMFLOAT3 Pos;
//	XMFLOAT3 Normal;
//	XMFLOAT2 Tex;
//	int 	 texNum;
//	XMFLOAT3 Tangent;
//	XMFLOAT3 BiNormal;
//};

int Import( char* filename, vector<Vertex>* vert );
void LoadScene(char* filename);
void ProcessScene();
void ProcessNode( FbxNode* node, int attributeType);
void ProcessMesh( FbxNode* node );
FbxVector2 GetTexCoords( FbxMesh* mesh, int layerIndex, int polygonIndex, int polygonVertexIndex, int vertexIndex );

#endif
