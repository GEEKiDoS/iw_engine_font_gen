#include <vector>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <D3dx9tex.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

#include <stdio.h>
#include <codecvt>

#include <locale.h>
#include <algorithm>

#include "TextRange.h"

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

uint16_t WidetoGBK(uint16_t in)
{
	wchar_t input[2] = { in, 0 };
	char output[3];

	WideCharToMultiByte(CP_ACP, 0, input, -1, output, 3, NULL, NULL);

	if (output[1] == 0)
		return in;

	uint16_t target1 = 0;

	__asm
	{
		mov ah, output[0]
		mov al, output[1]
		mov target1, ax
	}

	return target1;
}

ImWchar* GetTextRangeCommonChinese(const  ImWchar* baseRange)
{
	ImFontGlyphRangesBuilder builder;

	builder.AddRanges(baseRange);

	ImWchar tmp = 0;

	setlocale(LC_ALL, "chs");

	wchar_t* range = (wchar_t*)textRange;

	while ((tmp = *(++range)) != L'\000')
	{
		// wprintf(L"%lc", tmp);
		builder.AddChar(tmp);
	}

	// 标点
	builder.AddChar(0xFF1F);
	builder.AddChar(0xFF01);
	builder.AddChar(0xFF0C);
	builder.AddChar(0xFF1B);
	builder.AddChar(0xFF1A);
	builder.AddChar(0x2018);
	builder.AddChar(0x2019);
	builder.AddChar(0x201C);
	builder.AddChar(0x201D);
	builder.AddChar(0xFF08);
	builder.AddChar(0xFF09);
	builder.AddChar(0x2014);
	builder.AddChar(0x2026);
	builder.AddChar(0x2013);
	builder.AddChar(0xFF0E);
	builder.AddChar(L'￥');

	static ImVector<ImWchar> output_range;

	builder.BuildRanges(&output_range);

	return output_range.Data;
}


int main()
{
	std::string font_name;
	std::cout << "Input font: ";
	std::cin >> font_name;

	int pixelSize;
	std::cout << "Input font size: ";
	std::cin >> pixelSize;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("IWFontGen"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("IW Engine Font Generator"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	ImFontConfig cfg;

	ImFont* font = io.Fonts->AddFontFromFileTTF(font_name.data(), pixelSize, &cfg, GetTextRangeCommonChinese(io.Fonts->GetGlyphRangesChineseSimplifiedCommon()));

	//
   // Our state
	ImVec4 clear_color = ImVec4(0, 0, 0, 1.00f);

	char buffer[10000];
	int baseline = 0;

	memset(buffer, 0, 10000);

	RECT rect;

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Font Generator", nullptr,
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoCollapse
			);

			GetWindowRect(hwnd, &rect);

			ImGui::SetWindowSize(ImVec2(rect.right - rect.left, rect.bottom - rect.top), 0);
			ImGui::SetWindowPos(ImVec2(0, 0), 0);

			ImGui::Text(io.Fonts->Fonts[0]->GetDebugName());
			ImGui::Text("Numb3r5:");
			ImGui::Text("01234567890");
			ImGui::Text("English Text Preview");
			ImGui::Text("the quick brown fox jumps over a lazy dog.");
			ImGui::Text("THE QUICK BROWN FOX JUMPS OVER A LAZY DOG.");
			ImGui::Text(u8"中文文本预览：");
			ImGui::Text(u8"一去二三里烟村四五家亭台六七座八九十支花");
			ImGui::Text(u8"中国智造，慧及全球");

			ImGui::InputTextMultiline("Input Test", buffer, 10000);

			ImGui::InputInt("Baseline adjust", &baseline, 1, 2, ImGuiInputTextFlags_CharsDecimal);

			if (ImGui::Button("Save Font"))
			{
				std::string where;
				std::cout << "Where: ";
				std::cin >> where;

				auto textureFile = where + ".png";

				D3DXSaveTextureToFileA(textureFile.data(), _D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, (LPDIRECT3DBASETEXTURE9)io.Fonts->TexID, 0);

				FILE* f;

				auto err = fopen_s(&f, where.data(), "w");

				fprintf(f, "{");
				fprintf(f, "\"fontName\":\"%s\",", "fonts/name");
				fprintf(f, "\"material\":\"%s\",", "fonts/name");
				fprintf(f, "\"glowMaterial\":\"%s\",", "fonts/name_glow");
				fprintf(f, "\"pixelHeight\":%d,", (int)io.Fonts->Fonts[0]->FontSize);
				fprintf(f, "\"glyphs\": [");

				std::vector<int> list;

				for (int i = 0; i < io.Fonts->Fonts[0]->Glyphs.Size; i++)
					list.push_back(i);

				std::sort(list.begin(), list.end(), [io](int a, int b)
				{
					return (io.Fonts->Fonts[0]->Glyphs[a].Codepoint < io.Fonts->Fonts[0]->Glyphs[b].Codepoint);
				});

				for (int i = 0; i < io.Fonts->Fonts[0]->Glyphs.Size; i++)
				{
					ImFontGlyph& glyph = io.Fonts->Fonts[0]->Glyphs[list[i]];

					if (glyph.Codepoint < 32)
						continue;

					fprintf(f, "{");
					fprintf(f, "\"letter\":%d,", glyph.Codepoint);
					fprintf(f, "\"dx\":%d,", (int)glyph.AdvanceX);
					fprintf(f, "\"pixelHeight\":%d,", (int)round(glyph.Y1 - glyph.Y0));
					fprintf(f, "\"pixelWidth\":%d,", (int)round(glyph.X1 - glyph.X0));
					fprintf(f, "\"x0\":%d,", (int)round(glyph.X0));
					fprintf(f, "\"y0\":%d,", (int)round(glyph.Y0 - io.Fonts->Fonts[0]->FontSize - baseline));
					fprintf(f, "\"s0\":%f,", glyph.U0);
					fprintf(f, "\"t0\":%f,", glyph.V0);
					fprintf(f, "\"s1\":%f,", glyph.U1);
					fprintf(f, "\"t1\":%f", glyph.V1);


					if (i == io.Fonts->Fonts[0]->Glyphs.Size - 1)
						fprintf(f, "}");
					else
						fprintf(f, "},");
				}
				fprintf(f, "],");
				fprintf(f, "\"glyphCount\":%d", list.size());
				fprintf(f, "}");

				fclose(f);
			}


			ImGui::Text("Font atlas preview:");

			ImGui::Image(io.Fonts->TexID, ImVec2(io.Fonts->TexWidth, io.Fonts->TexHeight));

			ImGui::End();
		}

		// Rendering
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f), (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}


	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
