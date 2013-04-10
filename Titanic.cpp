// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
#include "Camera.h"
#include "GeometryGenerator.h"
#include "Object.h"
//#include "struct.h"

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

struct PosNormalTexTan
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT3 TangentU;
	XMFLOAT2 Tex;
};

// global declarations
IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
ID3D11InputLayout *pLayout;            // the pointer to the input layout
ID3D11VertexShader *pVS;               // the pointer to the vertex shader
ID3D11PixelShader *pPS;                // the pointer to the pixel shader
ID3D11Buffer *pVBuffer;                // the pointer to the vertex buffer
ID3D11Buffer *pIBuffer;
ID3D11Buffer *pCBuffer;                // the pointer to the constant buffer

Object* obj;

HWND hWnd;
class Camera *mCam;					   // the camera
POINT mLastMousePos;				   // the last mouse position

GeometryGenerator *geoGen;			   // the pointer to the geometry generator
GeometryGenerator::MeshData skyBoxData;			   // geometry for the sky box
GeometryGenerator::MeshData screenQuadData;		   // geometry for the screen aligned quad
ID3D10EffectShaderResourceVariable* diffuseTextureArray;
ID3D10EffectShaderResourceVariable* normalTextureArray;

ID3D11Texture2D*            depthStencil = NULL;
ID3D11DepthStencilView*     depthStencilView = NULL;
ID3D11DepthStencilView *zbuffer;    // global


// various buffer structs
struct VERTEX{FLOAT X, Y, Z; D3DXCOLOR Color;};
struct PERFRAME{D3DXCOLOR Color; FLOAT X, Y, Z;};

// function prototypes
void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void RenderFrame(void);     // renders a single frame
void CleanD3D(void);        // closes Direct3D and releases memory
void InitGraphics(void);    // creates the shape to render
void InitPipeline(void);    // loads and prepares the shaders

void OnMouseDown(WPARAM btnState, int x, int y);
void OnMouseUp(WPARAM btnState, int x, int y);
void OnMouseMove(WPARAM btnState, int x, int y);
void UpdateCamera(float dt);

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

float dt = .001;

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    RECT wr = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
                          L"WindowClass",
                          L"Our First Direct3D Program",
                          WS_OVERLAPPEDWINDOW,
                          300,
                          300,
                          wr.right - wr.left,
                          wr.bottom - wr.top,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    InitD3D(hWnd);

    // enter the main loop:

    MSG msg;

    while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
                break;
        }
		UpdateCamera(dt);
        RenderFrame();
    }

    // clean up DirectX and COM
    CleanD3D();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
		case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWnd)
{
	// initialize camera
	mCam = new Camera();
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	geoGen->CreateFullscreenQuad(screenQuadData);
	geoGen->CreateBox(25.5f, 25.5f, 25.5f, skyBoxData);




    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    scd.BufferCount = 1;                                   // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
    scd.BufferDesc.Width = SCREEN_WIDTH;                   // set the back buffer width
    scd.BufferDesc.Height = SCREEN_HEIGHT;                 // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // how swap chain is to be used
    scd.OutputWindow = hWnd;                               // the window to be used
    scd.SampleDesc.Count = 4;                              // how many multisamples
    scd.Windowed = TRUE;                                   // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching

    // create a device, device context and swap chain using the information in the scd struct
    D3D11CreateDeviceAndSwapChain(NULL,
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  D3D11_SDK_VERSION,
                                  &scd,
                                  &swapchain,
                                  &dev,
                                  NULL,
                                  &devcon);

	 
    // get the address of the back buffer
    ID3D11Texture2D *pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // use the back buffer address to create the render target
    dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
    pBackBuffer->Release();

    // set the render target as the back buffer
    devcon->OMSetRenderTargets(1, &backbuffer, NULL);


    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0.0;
	viewport.MaxDepth = 1.0;

    devcon->RSSetViewports(1, &viewport);

	HRESULT hr;

	ID3D11RasterizerState*		pState;
	D3D11_RASTERIZER_DESC		raster;
	ZeroMemory( &raster, sizeof(D3D11_RASTERIZER_DESC));

	raster.FillMode = D3D11_FILL_SOLID;
	raster.CullMode = D3D11_CULL_BACK;
	raster.FrontCounterClockwise = FALSE;
	raster.DepthBias = 0;
	raster.DepthBiasClamp = 0.0f;
	raster.SlopeScaledDepthBias = 0.0f;
	raster.DepthClipEnable = TRUE; //set for testing otherwise true
	raster.ScissorEnable = FALSE;
	raster.MultisampleEnable = FALSE;
	raster.AntialiasedLineEnable = FALSE;

	hr = dev->CreateRasterizerState (&raster, &pState);
	devcon->RSSetState( pState );

	D3D11_TEXTURE2D_DESC texd;
ZeroMemory(&texd, sizeof(texd));

texd.Width = SCREEN_WIDTH;
texd.Height = SCREEN_HEIGHT;
texd.ArraySize = 1;
texd.MipLevels = 1;
texd.SampleDesc.Count = 4;
texd.Format = DXGI_FORMAT_D32_FLOAT;
texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

ID3D11Texture2D *pDepthBuffer;
dev->CreateTexture2D(&texd, NULL, &pDepthBuffer);


D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
ZeroMemory(&dsvd, sizeof(dsvd));

dsvd.Format = DXGI_FORMAT_D32_FLOAT;
dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

dev->CreateDepthStencilView(pDepthBuffer, &dsvd, &zbuffer);
pDepthBuffer->Release();

devcon->OMSetRenderTargets(1, &backbuffer, zbuffer);
devcon->ClearDepthStencilView(zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

    InitPipeline();
    InitGraphics();
}

// this is the function that cleans up Direct3D and COM
void CleanD3D(void)
{
    swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    pLayout->Release();
    pVS->Release();
    pPS->Release();
    pVBuffer->Release();
    pCBuffer->Release();
    swapchain->Release();
    backbuffer->Release();
    dev->Release();
    devcon->Release();
}


// this is the function that creates the shape to render
void InitGraphics()
{
	PosNormalTexTan vertices[24];

	for(size_t i = 0; i < skyBoxData.Vertices.size(); i++)
    {
        vertices[i].Pos      = skyBoxData.Vertices[i].Position;
        vertices[i].Normal   = skyBoxData.Vertices[i].Normal;
        vertices[i].Tex      = skyBoxData.Vertices[i].TexC;
        vertices[i].TangentU = skyBoxData.Vertices[i].TangentU;
    }

	UINT indices[36];

	for(size_t i = 0; i < skyBoxData.Indices.size(); i++)
	{
		indices[i] = skyBoxData.Indices[35-i];
	}


    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(PosNormalTexTan) * 24;             // size is the VERTEX struct * 3
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    dev->CreateBuffer(&bd, NULL, &pVBuffer);       // create the buffer


    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, &vertices, sizeof(PosNormalTexTan) * 24);                 // copy the data
    devcon->Unmap(pVBuffer, NULL);                                      // unmap the buffer

	// create the index buffer
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(UINT) * 36;    // 3 per triangle, 12 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    dev->CreateBuffer(&bd, NULL, &pIBuffer);

    devcon->Map(pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);      // map the buffer
	memcpy(ms.pData, &indices, sizeof(UINT) * 36);                     // copy the data
    devcon->Unmap(pIBuffer, NULL);

    ID3D11ShaderResourceView* cubeMap;

    HRESULT hr = D3DX11CreateShaderResourceViewFromFile(dev, L"mountains1024.dds", 0, 0, &cubeMap, 0 );

    devcon->PSSetShaderResources(0, 1, &cubeMap);
}


// this function loads and prepares the shaders
void InitPipeline()
{
    // load and compile the two shaders
    ID3DBlob *VS, *PS;
    D3DX11CompileFromFile(L"shaders.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile(L"shaders.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

    // encapsulate both shaders into shader objects
    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

    // set the shader objects
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);

    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
		{ "TEXNUM",   0, DXGI_FORMAT_R32_FLOAT,		  0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    dev->CreateInputLayout(ied, 6, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    devcon->IASetInputLayout(pLayout);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    dev->CreateBuffer(&bd, NULL, &pCBuffer);
    devcon->VSSetConstantBuffers(0, 1, &pCBuffer);


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Here is where I load my shit. textures get reset to size = 0 in charLoad function
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	vector<const wchar_t *> textures;
	vector<const wchar_t *> normalMap;

	textures.push_back( L"../assets/Textures/CommandoArmor_DM.dds" );
	textures.push_back( L"../assets/Textures/Commando_DM.dds" );
	normalMap.push_back( L"../assets/Textures/CommandoArmor_NM.dds" );
	normalMap.push_back( L"../assets/Textures/Commando_NM.dds" );

	obj = new Object();
	obj->objLoad( "../assets/Models/bigbadman.fbx", &textures, &normalMap, dev );

    devcon->PSSetShaderResources(0, obj->texArray.size(), &obj->texArray[0] );
    devcon->PSSetShaderResources(3, obj->NormArray.size(), &obj->NormArray[0] );

}


/////////////////////////////////////
// CAMERA STUFF
/////////////////////////////////////

void UpdateCamera(float dt)
{
	float speed = 2.0f;

    ShowCursor(true);

    if( GetAsyncKeyState(0x10) & 0x8000 )
        speed = 100.0f;

    if( GetAsyncKeyState('W') & 0x8000 )
        mCam->Walk(speed*dt);

    if( GetAsyncKeyState('S') & 0x8000 )
        mCam->Walk(-speed*dt);

    if( GetAsyncKeyState('A') & 0x8000 )
        mCam->Strafe(-speed*dt);

    if( GetAsyncKeyState('D') & 0x8000 )
        mCam->Strafe(speed*dt);

	mCam->UpdateViewMatrix();
}

void OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(hWnd);
}


void OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}


void OnMouseMove(WPARAM btnState, int x, int y)
{
    if( (btnState & MK_LBUTTON) != 0 )
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        mCam->Pitch(dy);
        mCam->RotateY(dx);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

//////////////////////////////////
// END CAMERA STUFF
//////////////////////////////////

// this is the function used to render a single frame
void RenderFrame(void)
{
    XMMATRIX matRotate, matView, matProjection, matFinal;

    static float Time = 0.0f; Time += 0.001f;


    // set the new values for the constant buffer
	devcon->UpdateSubresource(pCBuffer, 0, 0, mCam->ViewProj().m, 0, 0);


    // clear the back buffer to a deep blue
	devcon->ClearDepthStencilView( zbuffer, D3D11_CLEAR_DEPTH, 1.0f, 0 );
    devcon->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
//	devcon->OMSetRenderTargets( 1, &backbuffer, depthStencilView );

        // select which vertex buffer to display
        UINT stride = sizeof(Vertex);
//        UINT stride = sizeof(VERTEX);
        UINT offset = 0;
        devcon->IASetVertexBuffers(0, 1, &obj->vertexBuffer, &stride, &offset);
//        devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

		//devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);

        // select which primtive type we are using
        devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // draw the vertex buffer to the back buffer
       // devcon->DrawIndexed(36, 0, 0);
		devcon->Draw(obj->vertices.size(),0);

    // switch the back buffer and the front buffer
    swapchain->Present(0, 0);
}