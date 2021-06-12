/*
  Unity Capture
  Copyright (c) 2018 Bernhard Schelling

  Based on UnityCam
  https://github.com/mrayy/UnityCam
  Copyright (c) 2016 MHD Yamen Saraiji

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "shared.inl"
#include <chrono>
#include <string>
#include "IUnityGraphics.h"
#include "SharedImageWrapper.h"

enum
{
	RET_SUCCESS = 0,
	RET_WARNING_FRAMESKIP = 1,
	RET_WARNING_CAPTUREINACTIVE = 2,
	RET_ERROR_UNSUPPORTEDGRAPHICSDEVICE = 100,
	RET_ERROR_PARAMETER = 101,
	RET_ERROR_TOOLARGERESOLUTION = 102,
	RET_ERROR_TEXTUREFORMAT = 103,
	RET_ERROR_READTEXTURE = 104,
};

#include <d3d11.h>

static int g_GraphicsDeviceType = -1;
static ID3D11Device* g_D3D11GraphicsDevice = 0;

struct UnityCaptureInstance
{
	SharedImageMemory* Sender;
	int Width, Height;
	DXGI_FORMAT Format;
	bool UseDoubleBuffering, AlternativeBuffer;
	ID3D11Texture2D* Textures[2];
};

extern "C" __declspec(dllexport) UnityCaptureInstance* CaptureCreateInstance(int CapNum)
{
	UnityCaptureInstance* c = new UnityCaptureInstance();
	memset(c, 0, sizeof(UnityCaptureInstance));
	c->Sender = new SharedImageMemory(CapNum);
	return c;
}

extern "C" __declspec(dllexport) void CaptureDeleteInstance(UnityCaptureInstance* c)
{
	if (!c) return;
	delete c->Sender;
	if (c->Textures[0]) c->Textures[0]->Release();
	if (c->Textures[1]) c->Textures[1]->Release();
	delete c;
}

extern "C" __declspec(dllexport) int CaptureSendTexture(UnityCaptureInstance* c, void* TextureNativePtr, int Timeout, bool UseDoubleBuffering, SharedImageMemory::EResizeMode ResizeMode, SharedImageMemory::EMirrorMode MirrorMode, bool IsLinearColorSpace)
{
	if (!c || !TextureNativePtr) return RET_ERROR_PARAMETER;
	if (g_GraphicsDeviceType != kUnityGfxRendererD3D11) return RET_ERROR_UNSUPPORTEDGRAPHICSDEVICE;
	if (!c->Sender->SendIsReady()) return RET_WARNING_CAPTUREINACTIVE;

	//Get the active D3D11 context
	ID3D11DeviceContext* ctx = NULL;
	g_D3D11GraphicsDevice->GetImmediateContext(&ctx);
	if (!ctx) return RET_ERROR_UNSUPPORTEDGRAPHICSDEVICE;

	//Read the size and format info from the render texture
	ID3D11Texture2D* d3dtex = (ID3D11Texture2D*)TextureNativePtr;
	D3D11_TEXTURE2D_DESC desc = {0};
	d3dtex->GetDesc(&desc);
	if (!desc.Width || !desc.Height) return RET_ERROR_READTEXTURE;

	if (c->Width != desc.Width || c->Height != desc.Height || c->Format != desc.Format || c->UseDoubleBuffering != UseDoubleBuffering)
	{
		//Allocate a Texture2D resource which holds the texture with CPU memory access
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = desc.Width;
		textureDesc.Height = desc.Height;
		textureDesc.MipLevels = desc.MipLevels;
		textureDesc.ArraySize = 1;
		textureDesc.Format = desc.Format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.MiscFlags = 0;
		
		if (c->Textures[0]) c->Textures[0]->Release();
		g_D3D11GraphicsDevice->CreateTexture2D(&textureDesc, NULL, &c->Textures[0]);
		if (c->Textures[1]) c->Textures[1]->Release(); 
		if (UseDoubleBuffering) g_D3D11GraphicsDevice->CreateTexture2D(&textureDesc, NULL, &c->Textures[1]);
		else c->Textures[1] = NULL;
		c->Width = desc.Width;
		c->Height = desc.Height;
		c->Format = desc.Format;
		c->UseDoubleBuffering = UseDoubleBuffering;
	}

	//Handle double buffer
	if (c->UseDoubleBuffering) c->AlternativeBuffer ^= 1;
	ID3D11Texture2D* WriteTexture = c->Textures[c->UseDoubleBuffering &&  c->AlternativeBuffer ? 1 : 0];
	ID3D11Texture2D* ReadTexture  = c->Textures[c->UseDoubleBuffering && !c->AlternativeBuffer ? 1 : 0];
	//Check texture format
	SharedImageMemory::EFormat Format;
	if      (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || desc.Format == DXGI_FORMAT_R8G8B8A8_UINT || desc.Format == DXGI_FORMAT_R8G8B8A8_TYPELESS) 
		Format = SharedImageMemory::FORMAT_UINT8;
	else if (desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT || desc.Format == DXGI_FORMAT_R16G16B16A16_TYPELESS) 
		Format = (IsLinearColorSpace ? SharedImageMemory::FORMAT_FP16_LINEAR : SharedImageMemory::FORMAT_FP16_GAMMA);
	else return RET_ERROR_TEXTUREFORMAT;

	//Copy render texture to texture with CPU access and map the image data to RAM
	ctx->CopyResource(WriteTexture, d3dtex);
	D3D11_MAPPED_SUBRESOURCE mapResource;
	if (FAILED(ctx->Map(ReadTexture, 0, D3D11_MAP_READ, 0, &mapResource))) return RET_ERROR_READTEXTURE;

	//Push the captured data to the direct show filter
	SharedImageMemory::ESendResult res = c->Sender->Send(
		desc.Width, 
		desc.Height,
		mapResource.RowPitch / ((Format == SharedImageMemory::FORMAT_UINT8 ||Format == SharedImageMemory::FORMAT_UINT8_BGR)  ? 4 : 8),
		mapResource.RowPitch * desc.Height, 
		Format, 
		ResizeMode, 
		MirrorMode, 
		Timeout, 
		(const unsigned char*)mapResource.pData);

	ctx->Unmap(ReadTexture, 0);

	switch (res)
	{
		case SharedImageMemory::SENDRES_TOOLARGE:        return RET_ERROR_TOOLARGERESOLUTION;
		case SharedImageMemory::SENDRES_WARN_FRAMESKIP:  return RET_WARNING_FRAMESKIP;
	}
	return RET_SUCCESS;
}

mray::SharedImageWrapper* wrapper=0;
UnityCaptureInstance* c_glob=0;
extern "C" __declspec(dllexport) int SendTexture2(void* TextureNativePtr, 
	int Timeout, 
	bool UseDoubleBuffering,  
	bool IsLinearColorSpace)
{
	if (!wrapper) wrapper = new mray::SharedImageWrapper();
	if (!c_glob) c_glob = CaptureCreateInstance(0);

	if (g_GraphicsDeviceType != kUnityGfxRendererD3D11) return RET_ERROR_UNSUPPORTEDGRAPHICSDEVICE;
	if (!c_glob->Sender->SendIsReady()) return RET_WARNING_CAPTUREINACTIVE;

	UnityCaptureInstance* c = c_glob;
	//Get the active D3D11 context
	ID3D11DeviceContext* ctx = NULL;
	g_D3D11GraphicsDevice->GetImmediateContext(&ctx);
	if (!ctx) return RET_ERROR_UNSUPPORTEDGRAPHICSDEVICE;

	//Read the size and format info from the render texture
	ID3D11Texture2D* d3dtex = (ID3D11Texture2D*)TextureNativePtr;
	D3D11_TEXTURE2D_DESC desc = {0};
	d3dtex->GetDesc(&desc);
	if (!desc.Width || !desc.Height) return RET_ERROR_READTEXTURE;

	if (c->Width != desc.Width || c->Height != desc.Height || c->Format != desc.Format || c->UseDoubleBuffering != UseDoubleBuffering)
	{
		//Allocate a Texture2D resource which holds the texture with CPU memory access
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = desc.Width;
		textureDesc.Height = desc.Height;
		textureDesc.MipLevels = desc.MipLevels;
		textureDesc.ArraySize = 1;
		textureDesc.Format = desc.Format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.MiscFlags = 0;
		
		if (c->Textures[0]) c->Textures[0]->Release();
		g_D3D11GraphicsDevice->CreateTexture2D(&textureDesc, NULL, &c->Textures[0]);
		if (c->Textures[1]) c->Textures[1]->Release(); 
		if (UseDoubleBuffering) g_D3D11GraphicsDevice->CreateTexture2D(&textureDesc, NULL, &c->Textures[1]);
		else c->Textures[1] = NULL;
		c->Width = desc.Width;
		c->Height = desc.Height;
		c->Format = desc.Format;
		c->UseDoubleBuffering = UseDoubleBuffering;
	}

	//Handle double buffer
	if (c->UseDoubleBuffering) c->AlternativeBuffer ^= 1;
	ID3D11Texture2D* WriteTexture = c->Textures[c->UseDoubleBuffering &&  c->AlternativeBuffer ? 1 : 0];
	ID3D11Texture2D* ReadTexture  = c->Textures[c->UseDoubleBuffering && !c->AlternativeBuffer ? 1 : 0];

	//Check texture format 
	SharedImageMemory::EFormat Format;
	if(desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || 
		desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || 
		desc.Format == DXGI_FORMAT_R8G8B8A8_UINT || 
		desc.Format == DXGI_FORMAT_R8G8B8A8_TYPELESS ) 
		Format = SharedImageMemory::FORMAT_UINT8;
	else if(desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM ||
		desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
		Format = SharedImageMemory::FORMAT_UINT8_BGR;
	else if (desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT || desc.Format == DXGI_FORMAT_R16G16B16A16_TYPELESS) 
		Format = (IsLinearColorSpace ? SharedImageMemory::FORMAT_FP16_LINEAR : SharedImageMemory::FORMAT_FP16_GAMMA);
	else{
		std::string inf ="SendTexture2 ";
		inf += std::to_string(desc.Format);
		MessageBox(NULL, inf.c_str(), "Texture", MB_OK | MB_SETFOREGROUND);
		
		return RET_ERROR_TEXTUREFORMAT;
	}

	//Copy render texture to texture with CPU access and map the image data to RAM
	ctx->CopyResource(WriteTexture, d3dtex);
	D3D11_MAPPED_SUBRESOURCE mapResource;
	if (FAILED(ctx->Map(ReadTexture, 0, D3D11_MAP_READ, 0, &mapResource))) return RET_ERROR_READTEXTURE;

	//Push the captured data to the direct show filter
	SharedImageMemory::ESendResult res = c->Sender->Send(
		desc.Width, 
		desc.Height,
		mapResource.RowPitch / ((Format == SharedImageMemory::FORMAT_UINT8 || Format == SharedImageMemory::FORMAT_UINT8_BGR) ? 4 : 8),
		mapResource.RowPitch * desc.Height, 
		Format, 
		SharedImageMemory::RESIZEMODE_DISABLED , 
		SharedImageMemory::MIRRORMODE_DISABLED, 
		Timeout, 
		(const unsigned char*)mapResource.pData);

	ctx->Unmap(ReadTexture, 0);

	switch (res)
	{
		case SharedImageMemory::SENDRES_TOOLARGE:        return RET_ERROR_TOOLARGERESOLUTION;
		case SharedImageMemory::SENDRES_WARN_FRAMESKIP:  return RET_WARNING_FRAMESKIP;
	}
	return RET_SUCCESS;
}

// If exported by a plugin, this function will be called when graphics device is created, destroyed, and before and after it is reset (ie, resolution changed).
extern "C" void UNITY_INTERFACE_EXPORT UnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
	if (eventType == kUnityGfxDeviceEventInitialize || eventType == kUnityGfxDeviceEventAfterReset)
	{
		g_GraphicsDeviceType = deviceType;
		if (deviceType == kUnityGfxRendererD3D11) g_D3D11GraphicsDevice = (ID3D11Device*)device;
	}
	else g_GraphicsDeviceType = -1;
}

extern "C" __declspec(dllexport) bool UnrealSetGraphicsDevice(void* device, int eventType)
{
	if (eventType == kUnityGfxDeviceEventInitialize || eventType == kUnityGfxDeviceEventAfterReset)
	{
		g_GraphicsDeviceType = kUnityGfxRendererD3D11;
		if (g_GraphicsDeviceType == kUnityGfxRendererD3D11) g_D3D11GraphicsDevice = (ID3D11Device*)device;
		return true;
	}
	else g_GraphicsDeviceType = -1;
	return false;
}