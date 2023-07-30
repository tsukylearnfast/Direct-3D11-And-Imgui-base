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