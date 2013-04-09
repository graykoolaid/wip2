cbuffer ConstantBuffer
{
    float4x4 matFinal;
}

TextureCube cubeMap;

struct VOut
{
    float4 posH : SV_POSITION;
    float3 posL : POSITION;
};

struct Vin
{
	float4 Pos		: POSITION;
	float4 Normal	: NORMAL;
	float2 Tex		: TEXCOORD;
	int TexNum	    : TEXNUM;
	float4 Tangent	: TANGENT;
	float4 BiNormal	: BINORMAL;
};

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

//VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float3 tangent : TANGENT, float2 texcoord : TEXCOORD )
VOut VShader( 	
	float4 Pos		: POSITION,
	float4 Normal	: NORMAL,
	float2 Tex		: TEXCOORD,
	int TexNum	    : TEXNUM,
	float4 Tangent	: TANGENT,
	float4 BiNormal	: BINORMAL  )
{
    VOut output;
	
  //  output.posH = mul(matFinal, float4( position, 1.0f)).xyww;    // transform the vertex from 3D to 2D
    output.posH = mul(matFinal, float4( Pos, 1.0f)).xyww;    // transform the vertex from 3D to 2D
   // output.posL = position;
    output.posL = Pos;

    return output;
}


float4 PShader(VOut input) : SV_TARGET
{
    return cubeMap.Sample(samTriLinearSam, input.posL);
}
