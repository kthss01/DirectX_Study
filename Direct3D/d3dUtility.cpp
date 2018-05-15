#include "d3dUtility.h"

bool d3d::InitD3D(HINSTANCE hInstance, int width, int height, bool windowed, D3DDEVTYPE deviceType, IDirect3DDevice9** device)
{
	//-------------------------------------------------------------------//
	// 메인 윈도우 작성
	//-------------------------------------------------------------------//
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = TEXT("D3DSample");

	if (!RegisterClass(&wc))
	{
		MessageBox(0, TEXT("RegisterClass() - FAILED"), 0, 0);
		return false;
	}

	HWND hwnd = 0;
	hwnd = CreateWindow(TEXT("D3DSample"), TEXT("D3DSample - 1. CreateDevice"), WS_EX_TOPMOST,
		0, 0, width, height, 0, 0, hInstance, 0);

	if (!hwnd)
	{
		MessageBox(0, TEXT("CreateWindow() - FAILED"), 0, 0);
		return false;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	//-------------------------------------------------------------------//
	// Init D3D
	//-------------------------------------------------------------------//

	HRESULT hr = 0;

	// ----------------------------------------- //
	// 1. IDirect3D9 객체 생성
	// ----------------------------------------- //

	IDirect3D9* d3d9 = 0;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9)
	{
		MessageBox(0, TEXT("Direct3DCreate9() - FAILED"), 0, 0);
		return false;
	}

	// ----------------------------------------- //
	// 2. 하드웨어 버텍스 프로세싱 확인
	// ----------------------------------------- //

	// 기본 디스플레이 어댑터의 특성으로 D3DCAPS9 구조체를 채운다
	D3DCAPS9 caps;

	d3d9->GetDeviceCaps(
		D3DADAPTER_DEFAULT, // 기본 디스플레이 어댑터를 표기
		deviceType, // 장치 타입을 지정(보통 D3DDEVTYPE_HAL)
		&caps); // D3DCAPS9 구조체를 리턴

	int vp = 0;

	// 하드웨어 버텍스 프로세싱을 이용할 수 있는가
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING; // 하드웨어 버텍스 프로세싱으로 vp에 저장
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING; // 소프트웨어 버텍스 프로세싱으로 vp에 저장

	d3d9->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, deviceType, D3DFMT_A8R8G8B8,
		false, D3DMULTISAMPLE_16_SAMPLES, NULL);

	// ----------------------------------------- //
	// 3. D3DPRESENT_PARAMETERS 구조체 채우기
	// ----------------------------------------- //

	D3DPRESENT_PARAMETERS d3dpp;

	d3dpp.BackBufferWidth = width; // 후면 버퍼 너비
	d3dpp.BackBufferHeight = height; // 후면 버퍼 높이
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8; // 후면 버퍼의 픽셀 포맷(32비트 픽셀 포맷)
	d3dpp.BackBufferCount = 1; // 이용할 후면 버퍼의 수
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE; // 멀티 샘플링 타입
	d3dpp.MultiSampleQuality = 0; // 멀티 샘플링 레벨
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // 플리핑 체인의 버퍼가 교환되는 방법을 지정
	d3dpp.hDeviceWindow = hwnd; // 윈도우 핸들
	d3dpp.Windowed = windowed; // 창(true), 전체화면(false) 모드
	d3dpp.EnableAutoDepthStencil = true; // Direct3D가 자동으로 깊이 버퍼, 스텐실 버퍼를 만들고 관리한다(true)
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; // 깊이 버퍼, 스텐실 버퍼 포맷
	d3dpp.Flags = 0; // 몇 가지 부가적인 특성들
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT; // 화면 주사율
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // 

	// ----------------------------------------- //
	// 4. 장치 생성
	// ----------------------------------------- //

	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, // 메인 그래픽 장치
		deviceType, // 장치 타입
		hwnd, // 장치와 연결된 윈도우
		vp, // 버텍스 프로세싱 타입
		&d3dpp, // 시연 인자
		device); // 생성된 장치

	// 장치 생성에 실패했을 경우
	if (FAILED(hr))
	{
		// 16-bit 깊이 버퍼로 다시한번 시도한다
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, deviceType, hwnd, vp, &d3dpp, device);

		if (FAILED(hr))
		{
			d3d9->Release(); // done with d3d9 object
			MessageBox(0, TEXT("CreateDevice() - FAILED"), 0, 0);
			return false;
		}
	}

	d3d9->Release(); // done with d3d9 object

	return true;
}

//-----------------------------------------------------------------------------//
// EnterMsgLoop - 메시지 루프
//-----------------------------------------------------------------------------//

int d3d::EnterMsgLoop(bool(&ptr_display) (float timeDelta))
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	static float lastTime = (float)timeGetTime();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			float currTime = (float)timeGetTime();
			float timeDelta = (currTime - lastTime) * 0.001f;

			ptr_display(timeDelta); // 디스플레이 함수를 호출한다

			lastTime = currTime;
		}
	}
	return msg.wParam;
}
