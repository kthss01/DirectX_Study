// Tut03_Matrices.cpp: 응용 프로그램의 진입점을 정의합니다.
/*
	디바이스와 정점을 생성하는 방법을 알았다.
	3차원 정점을 자유자재로 다루기 위해서는 4x4 크기의 행렬을 사용해야 한다.
	기본적인 행렬 변환에는 이동(translations), 회전(rotations), 크기(scaling)가 있다.
	이때 월드 행렬이 사용된다.
	다시 월드 좌표계의 기하 정보를 카메라 좌표계로 변환한다. 
	이때 사용되는 것이 뷰 행렬이다.
	다시 이 기하 정보를 2차원 상의 평면(viewport)에 투영해야 
	윈도우에 그려질 수 있다.
	즉, 월드 -> 뷰 -> 프로젝션의 행렬 변환을 거친 뒤에 비로소 그려질 수 있는 것이다.
	(물론, 이후 클리핑 등의 처리가 추가로 이루어진다.)

	OpenGL에서는 행렬 연산 함수를 직접 작성해야 하겠지만,
	D3D에서는 D3DX라는 유틸리티 함수들이 여러 개 존재한다.
	여기서는 D3DX 계열 함수를 사용할 것이다.
*/

#include "stdafx.h"
#include "Direct3D.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

LPDIRECT3D9				g_pD3D = NULL;			// D3D 디바이스를 생성할 D3D 객체 변수
LPDIRECT3DDEVICE9		g_pd3dDevice = NULL;	// 렌더링에 사용될 D3D 디바이스
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;			// 정점을 보관할 정점 버퍼

// 사용자 정점을 정의할 구조체
struct CUSTOMVERTEX {
	// rhw 값이 있으면 변환이 완료된 정점이다.
	FLOAT x, y, z, rhw;	// 정점의 변환된 좌표
	DWORD color;		// 정점의 색깔
};

// 사용자 정점 구조체에 관한 정보를 나타내는 FVF 값
// 구조체는 X, Y, Z, RHW 값과 Diffuse 색깔값으로 이루어져 있음을 알 수 있다.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

/*
	Direct3D 초기화
*/
HRESULT InitD3D(HWND hWnd) {
	// 디바이스를 생성하기 위한 D3D 객체 생성
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp; // 디바이스 생성을 위한 구조체
	// 반드시 ZeroMemory() 함수로 미리 구조체를 깨끗이 지워야한다.
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;	// 창모드로 생성
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// 가장 효율적인 SWAP 효과
	// 현재 바탕화면 모드에 맞춰서 후면 버퍼 생성
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// 디바이스를 다음과 같은 설정으로 생성한다.
	// 1. 디폴트 비디오카드를 사용한다 (대부분은 비디오카드가 1개이다).
	// 2. HAL 디바이스를 생성한다(HW 가속장치를 사용하겠다는 의미).
	// 3. 정점 처리는 모든 카드에서 지원하는 SW 처리로 생성한다
	// (HW로 생성할 경우 더욱 높은 성능을 낸다).
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}

	// 디바이스 상태 정보를 처리할 경우 여기에서 한다.
	return S_OK;
}

/*
	정점 버퍼를 생성하고 정점값을 채워 넣는다.
	정점 버퍼란 기본적으로 정점 정보를 갖고 있는 메모리 블록이다.
	정점 버퍼를 생성한 다음에는 반드시 Lock()과 Unlock()으로 포인터를 얻어내서
	정점 정보를 정점 버퍼에 써넣어야 한다.
	또한 D3D는 인덱스 버퍼도 사용 가능하다는 것을 명심하자.
	정점 버퍼나 인덱스 버퍼는 기본 시스템 메모리 외에
	디바이스 메모리 (비디오 카드 메모리)에 생성될 수 있는데,
	대부분의 비디오카드에서는 이렇게 할 경우 엄청난 속도의 향상을 얻을 수 있다.
*/
HRESULT InitVB() {
	// 삼각형을 렌더링하기 위해 세 개의 정점을 선언
	CUSTOMVERTEX vertices[] = {
		{ 150.0f, 50.0f, 0.5f, 1.0f, 0xffff0000, },	// x, y, z, rhw, color
		{ 250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00, },
		{ 50.0f, 250.0f, 0.5f, 1.0f, 0xff00ffff, },
	};

	// 정점 버퍼 생성
	// 3개의 사용자 정점을 보관할 메모리를 할당한다.
	// FVF를 지정하여 보관할 데이터의 형식을 지정한다.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(
		3 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL))) {
		return E_FAIL;
	}

	// 정점 버퍼를 값으로 채운다.
	// 정점 버퍼의 Lock() 함수를 호출하여 포인터를 얻어온다.
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}

/*
	초기화된 객체들을 소거한다.
*/
void ClenUp() {
	if (g_pVB != NULL)
		g_pVB->Release();

	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}

/*
	화면을 그린다.
*/
void Render() {
	if (NULL == g_pd3dDevice)
		return;

	// 후면 버퍼를 파란색(0, 0, 255)으로 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	//  렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		// 실제 렌더링 명령들이 나열될 곳

		// 정점 버퍼의 삼각형을 그린다.
		// 1. 정점 정보가 담겨있는 정점 버퍼를 출력 스트림으로 할당한다.
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		// 2. D3D에 정점셰이더 정보를 지정한다. 
		// 대부분의 경우에는 FVF만 지정한다.
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		// 3. 기하 정보를 출력하기 위한 DrawPrimitive() 함수 호출
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 1);

		// 렌더링 종료
		g_pd3dDevice->EndScene();
	}

	// 후면 버퍼를 보이는 화면으로
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	/*
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_TUT01CREATEDEVICE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TUT01CREATEDEVICE));

	MSG msg;

	// 기본 메시지 루프입니다.
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
	*/

	// 윈도우 클래스 등록
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
					GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					L"D3D Tutorial", NULL };
	RegisterClassEx(&wc);

	// 윈도우 생성
	HWND hWnd = CreateWindow(
		L"D3D Tutorial", L"D3D Tutorial 02: Vertices",
		WS_OVERLAPPEDWINDOW, 100, 100, 500, 500,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd))) {

		// 정점 버퍼 초기화
		if (SUCCEEDED(InitVB())) {
			// 윈도우 출력
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			// 메시지 루프
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT) {

				// 메시지 큐에 메시지가 있으면 메시지 처리
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
					// 처리할 메시지가 없으면 Render() 함수 호출
					Render();
			}
		}
	}

	// 등록된 클래스 소거
	UnregisterClass(L"D3D Tutorial", wc.hInstance);
	return 0;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TUT01CREATEDEVICE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TUT01CREATEDEVICE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		ClenUp();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
