#include <vector>
#include <iostream>
#include <string>

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

using namespace std::string_literals;

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

ImWchar cp1252[] = { 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F, 0x20AC, 0x20, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000, 0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178, 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF, 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF };

ImWchar* GetTextRangeEAscii()
{
	ImFontGlyphRangesBuilder builder;

	for (int i = 32; i < 256; i++)
	{
		if (cp1252[i] == 0)
			continue;

		builder.AddChar(cp1252[i]);
	}

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

	ImFont* font = io.Fonts->AddFontFromFileTTF(font_name.data(), pixelSize, &cfg, GetTextRangeEAscii());

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
			// ImGui::Text(u8"中文文本预览：");
			// ImGui::Text(u8"一去二三里烟村四五家亭台六七座八九十支花");
			// ImGui::Text(u8"中国智造，慧及全球");

			ImGui::InputTextMultiline("Input Test", buffer, 10000);

			ImGui::InputInt("Baseline adjust", &baseline, 1, 2, ImGuiInputTextFlags_CharsDecimal);

			if (ImGui::Button("Save Font"))
			{
				std::string where;
				std::cout << "Where: ";
				std::cin >> where;

				auto textureFile = where + ".dds";

				D3DXSaveTextureToFileA(textureFile.data(), _D3DXIMAGE_FILEFORMAT::D3DXIFF_DDS, (LPDIRECT3DBASETEXTURE9)io.Fonts->TexID, 0);

				FILE* f;

				auto err = fopen_s(&f, (where + ".json").data(), "w");

				fprintf(f, "{");
				fprintf(f, "\"fontName\":\"%s\",", ("fonts/"s + where).data());
				fprintf(f, "\"material\":\"%s\",", ("fonts/"s + where).data());
				fprintf(f, "\"glowMaterial\":\"%s\",", ("fonts/"s + where + "_glow"s).data());
				fprintf(f, "\"pixelHeight\":%d,", (int)io.Fonts->Fonts[0]->FontSize);
				fprintf(f, "\"glyphs\": [");

				std::vector<int> list;

				int j = 0;
				for (int i = 32; i < 256; i++)
				{
					int pc = cp1252[i];

					if (cp1252[i] == 0)
						pc = '#';

					int pos = 0;

					for (int j = 0; j < io.Fonts->Fonts[0]->Glyphs.Size; j++)
					{
						auto& g = io.Fonts->Fonts[0]->Glyphs[j];

						if (g.Codepoint == pc)
						{
							pos = j;
							break;
						}
					}

					list.push_back(pos);
				}

				//for (int i = 0; i < io.Fonts->Fonts[0]->Glyphs.Size; i++)
				//	list.push_back(i);
				//
				//std::sort(list.begin(), list.end(), [io](int a, int b)
				//	{
				//		return (WidetoGBK(io.Fonts->Fonts[0]->Glyphs[a].Codepoint) < WidetoGBK(io.Fonts->Fonts[0]->Glyphs[b].Codepoint));
				//	});

				int count = 0;

				for (int i = 32; i < 256; i++)
				{
					ImFontGlyph& glyph = io.Fonts->Fonts[0]->Glyphs[list[i - 32]];

					fprintf(f, "{");
					fprintf(f, "\"letter\":%d,", i);
					fprintf(f, "\"dx\":%d,", (int)glyph.AdvanceX);
					fprintf(f, "\"pixelHeight\":%d,", (int)round(glyph.Y1 - glyph.Y0));
					fprintf(f, "\"pixelWidth\":%d,", (int)round(glyph.X1 - glyph.X0));
					fprintf(f, "\"x0\":%d,", (int)round(glyph.X0));
					fprintf(f, "\"y0\":%d,", (int)round(glyph.Y0 - io.Fonts->Fonts[0]->FontSize - baseline));
					fprintf(f, "\"s0\":%f,", glyph.U0);
					fprintf(f, "\"t0\":%f,", glyph.V0);
					fprintf(f, "\"s1\":%f,", glyph.U1);
					fprintf(f, "\"t1\":%f", glyph.V1);


					if (i == 255)
						fprintf(f, "}");
					else
						fprintf(f, "},");

					count++;
				}
				fprintf(f, "],");
				fprintf(f, "\"glyphCount\":%d", count);
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
