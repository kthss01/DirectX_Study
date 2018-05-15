#include "d3dUtility.h"

IDirect3DDevice9* Device = 0;

int Width = 1024;
int Height = 768;

//-----------------------------------------------------------------------------//
// Setup
//-----------------------------------------------------------------------------//

bool Setup()
{
	return true;
}

//-----------------------------------------------------------------------------//
// Cleanup
//-----------------------------------------------------------------------------//

void Cleanup()
{

}

//-----------------------------------------------------------------------------//
// Display - 화면에 표시하는 부분
//-----------------------------------------------------------------------------//

bool Display(float timeDelta)
{
	if (Device)
	{
		// 후면 버퍼를 소거하고 깊이 버퍼, 스텐실 버퍼를 각각 검은색(0x00000000)과 1.0으로 초기화한다
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

		// 후면 버퍼를 선언한다
		Device->Present(0, 0, 0, 0);
	}

	return true;
}

//-----------------------------------------------------------------------------//
// WndProc
//-----------------------------------------------------------------------------//

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) // ESC 키를 누르면 프로그램을 종료한다
			DestroyWindow(hwnd);

		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------//
// WinMain
//-----------------------------------------------------------------------------//

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
	if (!d3d::InitD3D(hInstance, Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		MessageBox(0, TEXT("InitD3D() - FAILED"), 0, 0);
		return 0;
	}

	if (!Setup())
	{
		MessageBox(0, TEXT("Setup() - FAILED"), 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display); // 메시지 루프

	// 메시지 루프를 빠져나오면 종료 작업
	Cleanup();
	Device->Release();

	return 0;
}
