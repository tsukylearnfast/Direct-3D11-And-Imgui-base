#pragma once
#include <Windows.h>
#include <d3d11.h>

#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui.h"
#include <dwmapi.h>



// IMPORTANT GO TO LINKER THEN SYSYTEM AND PUT SUBSYSTEM WINDOWS:
// Include d3d11.lib
// Add ImGui


class DX11ImGuiWindow
{
private:
	int width;
	int height;
	LPCWSTR className;
	LPCWSTR windowName;
	HWND window;
	ID3D11Device* device{ nullptr };
	ID3D11DeviceContext* device_context{ nullptr };
	WNDCLASSEXW wc{};
	IDXGISwapChain* swap_chain{ nullptr };
	ID3D11RenderTargetView* render_target_view{ nullptr };

public:

	INT APIENTRY SetupD3D(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show);
	DX11ImGuiWindow(LPCWSTR className, LPCWSTR windowName , int width , int height)
	{
		this->className = className;
		this->windowName = windowName;
		this->width = width;
		this->height = height;
	}
	void InitImGui();
	void SendMSG(bool running);
	void RenderImGuiNewFrame();
	void RenderImGui();
	void CleanupImGui(); 
	void CleanupD3D(); 
	void CenterWindow();
};

inline POINTS position = { };
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
		return 0L;
	}
	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return 0L;
	}

	switch (message)
	{
	case WM_SYSCOMMAND: {
		if ((w_param & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;
	case WM_LBUTTONDOWN: {
		position = MAKEPOINTS(l_param); // set click points
	}return 0;
	case WM_MOUSEMOVE: {
		if (w_param == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(l_param);
			auto rect = ::RECT{ };

			GetWindowRect(window, &rect);

			rect.left += points.x - position.x;
			rect.top += points.y - position.y;

			if (position.x >= 0 &&
				position.x <=  1920 && 
				position.y >= 0 && position.y <= 19)
				SetWindowPos(
					window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}
	return DefWindowProc(window, message, w_param, l_param);
}

void DX11ImGuiWindow::CenterWindow()
{
	HWND hWnd = GetActiveWindow();
	RECT rc;
	GetWindowRect(hWnd, &rc);

	int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

	SetWindowPos(hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

INT APIENTRY DX11ImGuiWindow::SetupD3D(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show)
{
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = instance;
	wc.lpszClassName = L"T";

	RegisterClassExW(&wc);


	window = CreateWindowExW(
		WS_EX_APPWINDOW,
		wc.lpszClassName,
		L"T",
		WS_POPUP,
		0, 0,
		this->width, this->height,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);




	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2U;
	sd.OutputWindow = window;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	constexpr D3D_FEATURE_LEVEL levels[2]{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};



	D3D_FEATURE_LEVEL level{};

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		levels,
		2U,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&level,
		&device_context
	);

	ID3D11Texture2D* back_buffer{ nullptr };
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	if (back_buffer) {
		device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
		back_buffer->Release();
	}
	else {
		return 1;
	}

	ShowWindow(window, cmd_show);
	UpdateWindow(window);
}

void DX11ImGuiWindow::InitImGui()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, device_context);

}

void DX11ImGuiWindow::SendMSG(bool running)
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT) {
			running = false;
		}
	}

}
void DX11ImGuiWindow::RenderImGuiNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
}
void DX11ImGuiWindow::RenderImGui()
{
	ImGui::Render();



	constexpr float color[4]{ 0.f,0.f,0.f,0.f };
	device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
	device_context->ClearRenderTargetView(render_target_view, color);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swap_chain->Present(0U, 0U);
}
void DX11ImGuiWindow::CleanupImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();
}
void DX11ImGuiWindow::CleanupD3D()
{
	if (swap_chain) {
		swap_chain->Release();
	}
	if (device_context) {
		device_context->Release();
	}
	if (device) {
		device->Release();
	}
	if (render_target_view) {
		render_target_view->Release();
	}

	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

/*

#include "../header/DX11ImGuiWindow.h"


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE h, PSTR pstr, INT cmd_show)
{

	DX11ImGuiWindow obj{ L"T",L"T", 500 ,500 };
	obj.SetupD3D(instance, h, pstr, cmd_show);

	obj.InitImGui();

	bool running = true;
	bool ImGuiRunning = true;




	obj.CenterWindow();

	while (running)
	{
		obj.SendMSG(&running);

		obj.RenderImGuiNewFrame();

		// RENDER HERE
		ImGui::SetNextWindowSize({ 500,500 }); // need to be the same size as the obj window size
		ImGui::SetNextWindowPos({ 0,0 });
		ImGui::Begin("-", &ImGuiRunning);

		ImGui::End();


		obj.RenderImGui();
	}

	obj.CleanupImGui();
	obj.CleanupD3D();

}
*/