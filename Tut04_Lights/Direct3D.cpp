// Tut04_Lights.cpp: 응용 프로그램의 진입점을 정의합니다.
/*
	조명을 사용하면 훨씬 더 극적인 연출이 가능하다.
	조명을 사용하기 위해서는 광원이나 재질을 생성해야 한다. 또한 기하 정보에
	법선벡터 정보를 포함하고 있어야 한다.
	광원은 위치, 색깔, 방향 등의 정보를 바탕으로 생성된다.
*/

#include "stdafx.h"
#include "Direct3D.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

LPDIRECT3D9				g_pD3D		 = NULL;	// D3D 디바이스를 생성할 D3D 객체 변수
LPDIRECT3DDEVICE9		g_pd3dDevice = NULL;	// 렌더링에 사용될 D3D 디바이스
LPDIRECT3DVERTEXBUFFER9	g_pVB		 = NULL;	// 정점을 보관할 정점 버퍼

// 사용자 정점을 정의할 구조체
struct CUSTOMVERTEX {
	D3DXVECTOR3 position;	// 정점의 3차원 좌표
	D3DXVECTOR3 normal;		// 정점의 법선벡터
};

// 사용자 정점 구조체에 관한 정보를 나타내는 FVF 값
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL)

/*
	Direct3D 초기화
*/
HRESULT InitD3D(HWND hWnd) {
	// 디바이스를 생성하기 위한 D3D 객체 생성
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// 디바이스 생성을 위한 구조체
	// 복잡한 오브젝트를 그릴 것이므로 이번에는 Z버퍼가 필요하다.
	D3DPRESENT_PARAMETERS d3dpp; 
	// 반드시 ZeroMemory() 함수로 미리 구조체를 깨끗이 지워야한다.
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;	// 창모드로 생성
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// 가장 효율적인 SWAP 효과
	// 현재 바탕화면 모드에 맞춰서 후면 버퍼 생성
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

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

	// 컬링 기능을 끈다. 삼각형의 앞면, 뒷면을 모두 렌더링한다.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Z버퍼 기능을 켠다.
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	// 디바이스 상태 정보를 처리할 경우 여기에서 한다.
	return S_OK;
}

/*
	기하 정보 초기화
*/
HRESULT InitGeometry() {
	// 정점 버퍼 생성
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(
		50 * 2 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL))) {
		return E_FAIL;
	}

	// 알고리즘을 사용해서 실린더(위 아래가 터진 원통)를 만든다.
	CUSTOMVERTEX * pVertices;
	if (FAILED(g_pVB->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	for (DWORD i = 0; i < 50; i++) {
		FLOAT theta = (2 * D3DX_PI * i) / (50 - 1);
		// 실린더의 아래쪽 원통의 좌표
		pVertices[2 * i + 0].position =
			D3DXVECTOR3(sinf(theta), -1.0f, cosf(theta));
		// 실린더의 아래쪽 원통의 법선벡터
		pVertices[2 * i + 0].normal =
			D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
		// 실린더의 위쪽 원통의 좌표
		pVertices[2 * i + 1].position =
			D3DXVECTOR3(sinf(theta), 1.0f, cosf(theta));
		// 실린더의 위쪽 원통의 법선벡터
		pVertices[2 + i + 1].normal =
			D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
	}
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
	행렬 설정
	행렬은 세 개가 있고, 각각 월드, 뷰, 프로젝션 행렬이다.
*/
VOID SetUpMatrices() {
	// 월드 행렬
	D3DXMATRIXA16 matWorld;
	// 월드 행렬을 단위행렬로 설정
	D3DXMatrixIdentity(&matWorld);
	// X축을 중심으로 회전행렬 생성
	D3DXMatrixRotationX(&matWorld, timeGetTime() / 500.0f);
	// 디바이스에 월드 행렬 설정
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);


	// 뷰 행렬 설정
	// 1. 눈의 위치 (0, 3.0, -5)
	D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);	
	// 2. 눈이 바라보는 위치(0, 0, 0)
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);	
	// 3. 천정 방향을 나타내는 상방벡터(0, 1, 0);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	// 1, 2, 3의 값으로 뷰 행렬 생성
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// 생성한 뷰 행렬을 디바이스에 설정
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);	

	// 프로젝션 행렬을 설정 
	// 시야각(FOV = Field Of View)과 종횡비(aspect ratio), 
	// 클리핑 평면의 값이 필요하다.
	D3DXMATRIXA16 matProj;
	// matProj		: 값이 설정된 행렬
	// D3DX_PI/4	: FOV(D3DX_PI/4 = 45도)
	// 1.0f			: 종횡비
	// 1.0f			: 근접 클리핑 평면(near clipping plane)
	// 100.0f		: 원거리 클리핑 평면(far clipping plane)
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	// 생성한 프로젝션 행렬을 디바이스에 설정
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

/*
	광원 설정
*/
void SetupLights() {
	// 재질(material) 설정
	// 재질은 디바이스에 단 하나만 설정될 수 있다.
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 0.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pd3dDevice->SetMaterial(&mtrl);

	// 광원 설정
	// 방향성 광원(directional light)이 향할 빛의 방향
	D3DXVECTOR3 vecDir;
	// 광원 구조체
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));	// 구조체를 0으로 지운다.
	// 광원의 종류(점 광원, 방향성 광원, 스포트라이트
	light.Type = D3DLIGHT_DIRECTIONAL;	

	// 광원의 색깔과 밝기
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	// 광원의 방향
	vecDir = D3DXVECTOR3(
		cosf(timeGetTime() / 350.0f), 1.0f, 
		sinf(timeGetTime() / 350.0f));
	// 광원의 방향을 단위벡터로 만든다.
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
	
	// 디바이스에 0번 광원 설치
	g_pd3dDevice->SetLight(0, &light);
	// 0번 광원을 켠다.
	g_pd3dDevice->LightEnable(0, true);
	// 광원 설정을 켠다.
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, true);

	// 환경 광원(ambient light)의 값 설정
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00202020);
}

/*
	화면을 그린다.
*/
void Render() {
	// 후변 버퍼와 Z버퍼를 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	//  렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		// 광원과 재질 설정
		SetupLights();

		// 월드, 뷰, 프로젝션 행렬을 설정한다.
		SetUpMatrices();

		// 정점 버퍼의 내용을 그린다.
		// 1. 정점 정보가 담겨있는 정점 버퍼를 출력 스트림으로 할당한다.
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		// 2. D3D에 정점셰이더 정보를 지정한다. 
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		// 3. 기하 정보를 출력하기 위한 DrawPrimitive() 함수 호출
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2);

		// 렌더링 종료
		g_pd3dDevice->EndScene();
	}

	g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

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
		L"D3D Tutorial", L"D3D Tutorial 04: Lights",
		WS_OVERLAPPEDWINDOW, 100, 100, 500, 500,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd))) {

		// 장면에 사용될 기하 정보 초기화
		if (SUCCEEDED(InitGeometry())) {
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
