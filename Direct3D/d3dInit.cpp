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
// Display - ȭ�鿡 ǥ���ϴ� �κ�
//-----------------------------------------------------------------------------//

bool Display(float timeDelta)
{
	if (Device)
	{
		// �ĸ� ���۸� �Ұ��ϰ� ���� ����, ���ٽ� ���۸� ���� ������(0x00000000)�� 1.0���� �ʱ�ȭ�Ѵ�
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

		// �ĸ� ���۸� �����Ѵ�
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
		if (wParam == VK_ESCAPE) // ESC Ű�� ������ ���α׷��� �����Ѵ�
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

	d3d::EnterMsgLoop(Display); // �޽��� ����

	// �޽��� ������ ���������� ���� �۾�
	Cleanup();
	Device->Release();

	return 0;
}
