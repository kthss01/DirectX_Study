// HeightMap.cpp: 응용 프로그램의 진입점을 정의합니다.
/*
	QuadTree 구현
*/

#include "stdafx.h"
#include "Direct3D.h"
#include "ZCamera.h"
#include "ZFrustum.h"
#include "ZTerrain.h"
#include "ZFLog.h"
#include <string>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HWND g_hwnd = NULL;

LPDIRECT3D9 g_pD3D = NULL;	// D3D 디바이스를 생성할 D3D 객체 변수
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;	// 렌더링에 사용될 D3D 디바이스

D3DXMATRIXA16 g_matAni;
D3DXMATRIXA16 g_matWorld;
D3DXMATRIXA16 g_matView;
D3DXMATRIXA16 g_matProj;

DWORD g_dwMouseX = 0; // 마우스의 좌표
DWORD g_dwMouseY = 0; // 마우스의 좌표
bool g_bHideFrustum = true; // 절두체를 안 그릴 것인가?
bool g_bLockFrustum = false; // 절두체를 고정할 것인가?
bool g_bWireframe = false; // 와이어프레임으로 그릴것인가?

ZFLog*		g_pLog = NULL;
ZCamera* g_pCamera = NULL; // Camera 클래스
ZFrustum* g_pFrustum = NULL; // Frustum 클래스
ZTerrain* g_pTerrain = NULL;

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

	// 컬링 기능을 끈다.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// Z버퍼 기능을 켠다.
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	// 디바이스 상태 정보를 처리할 경우 여기에서 한다.
	return S_OK;
}

/*
	행렬 설정
*/
void InitMatrix() {
	// 월드 행렬 설정
	D3DXMatrixIdentity(&g_matWorld);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &g_matWorld);

	// 뷰 행렬 설정
	D3DXVECTOR3 vEyePt(0.0f, 100.0f, -(float)129.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &g_matView);

	// 실제 프로젝션 행렬
	D3DXMatrixPerspectiveFovLH(&g_matProj, D3DX_PI / 4, 1.0f, 1.0f,
		1000.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &g_matProj);

	// 카메라 초기화
	g_pCamera->SetView(&vEyePt, &vLookatPt, &vUpVec);
}

/*
	기하 정보 초기화
*/
HRESULT InitGeometry() {
	InitMatrix();

	// 최초의 마우스 위치 보관
	POINT pt;
	GetCursorPos(&pt);
	g_dwMouseX = pt.x;
	g_dwMouseY = pt.y;

	return S_OK;
}

HRESULT InitObjects() {
	LPSTR tex[4] = { (char*)"tile2.tga", 
		(char*)"", (char*)"", (char*)"" };
	D3DXVECTOR3 vScale;

	vScale.x = vScale.z = 1.0f; vScale.y = 0.1f;
	g_pLog = new ZFLog(ZF_LOG_TARGET_WINDOW);
	g_pCamera = new ZCamera;
	g_pFrustum = new ZFrustum;
	g_pTerrain = new ZTerrain;
	g_pTerrain->Create(g_pd3dDevice, &vScale, (char*)BMP_HEIGHTMAP, tex);

	return S_OK;
}

void DeleteObjects() {
	// 등록된 클래스 소거
	DEL(g_pTerrain);
	DEL(g_pFrustum);
	DEL(g_pLog);
	DEL(g_pCamera);
}

/*
초기화된 객체들을 소거한다.
*/
void CleanUp() {
	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
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
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pd3dDevice->SetMaterial(&mtrl);

	// 광원 설정
	D3DXVECTOR3 vecDir;	// 방향성 광원
	// directional light이 향할 빛의 방향
	D3DLIGHT9 light; // 광원 구조체
	// 구조체를 0으로 초기화 한다.
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	// 광원의 종류 (점 광원, 방향성 광원, 스포트라이트)
	light.Type = D3DLIGHT_DIRECTIONAL; 
	// 광원의 색깔과 밝기
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 0.0f;
	vecDir = D3DXVECTOR3(1, 1, 1); // 광원 고정
	// 광원 회전
	vecDir = D3DXVECTOR3(
		cosf(GetTickCount() / 350.0f), 1.0f,
		sinf(GetTickCount() / 350.0f));
	// 광원의 방향을 단위벡터로 만든다.
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
	// 광원이 다다를 수 있는 최대 거리
	light.Range = 1000.0f;
	// 디바이스에 0번 광원 설치
	g_pd3dDevice->SetLight(0, &light);
	// 0번 광원을 켠다.
	g_pd3dDevice->LightEnable(0, true);
	// 광원 설정을 켠다.
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, true);

	// 환경광원(ambient light)의 값 설정
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00909090);
}

/*
	상태 정보 출력
*/
void LogStatus() {
	g_pLog->Log((char*)"Wireframe:%d", g_bWireframe);
	g_pLog->Log((char*)"HideFrustum:%d", g_bHideFrustum);
	g_pLog->Log((char*)"LockFrustum:%d", g_bLockFrustum);
}

/*
	FPS(Frame Per Second) 출력
*/
void LogFPS() {
	static DWORD nTick = 0;
	static DWORD nFPS = 0;

	// 1초가 지났는가?
	if (GetTickCount() - nTick > 1000) {
		nTick = GetTickCount();
		// FPS값 출력
		g_pLog->Log((char*)"FPS:%d", nFPS);
		nFPS = 0;

		//// 카메라의 위치값 출력
		//D3DXVECTOR3* pv;
		//pv = g_pCamera->GetEye();
		//g_pLog->Log((char*)"EYE:[%f,%f,%f]", pv->x, pv->y, pv->z);

		LogStatus(); // 상태 정보를 여기서 출력 (1초에 한 번)

		return;
	}
	nFPS++;
}

/*
	마우스 입력 처리
*/
void ProcessMouse() {
	POINT pt;
	// 마우스의 민감도, 이 값이 커질수록 많이 움직인다.
	float fDelta = 0.001f;

	GetCursorPos(&pt);
	int dx = pt.x - g_dwMouseX; // 마우스의 변화값
	int dy = pt.y - g_dwMouseY; // 마우스의 변화값

	// 마우스의 Y축 회전값은 3D world의 X축 회전값
	g_pCamera->RotateLocalX(dy * fDelta);
	// 마우스의 X축 회전값은 3D world의 Y축 회전값
	g_pCamera->RotateLocalY(dx * fDelta);
	// 카메라 행렬을 얻는다.
	D3DXMATRIXA16* pmatView = g_pCamera->GetViewMatrix();
	// 카메라 행렬 셋팅
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);

	// 마우스를 윈도우의 중앙으로 초기화
	//SetCursor(NULL); // 마우스 나타지 않게됨
	RECT rc;
	GetClientRect(g_hwnd, &rc);
	pt.x = (rc.right - rc.left) / 2;
	pt.y = (rc.bottom - rc.top) / 2;
	ClientToScreen(g_hwnd, &pt);
	SetCursorPos(pt.x, pt.y);
	g_dwMouseX = pt.x;
	g_dwMouseY = pt.y;
}

/*
	키보드 입력 처리
*/
void ProcessKey() {
	if (GetAsyncKeyState('A')) g_pCamera->MoveLocalZ(0.5f);
	if (GetAsyncKeyState('Z')) g_pCamera->MoveLocalZ(-0.5f);
}

/*
	입력 처리
*/
void ProcessInputs() {
	ProcessMouse();
	ProcessKey();
}

/*
	애니메이션 행렬 생성
*/
void Animate() {
	D3DXMatrixIdentity(&g_matAni);

	SetupLights();
	ProcessInputs();

	D3DXMATRIXA16 m;
	D3DXMATRIXA16 *pView;
	// 카메라 클래스로부터 행렬 정보를 얻는다.
	pView = g_pCamera->GetViewMatrix(); 
	// 월드 좌표를 얻기 위해서 View * Proj 행렬을 계산한다.
	m = *pView * g_matProj; 
	// View*Proj 행렬로 절두체를 만든다.
	if (!g_bLockFrustum) g_pFrustum->Make(&m);

	LogFPS();
}

/*
	화면을 그린다.
*/
void Render() {
	// 후변 버퍼와 Z버퍼를 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(200, 200, 200), 1.0f, 0);

	// 애니메이션 행렬 설정
	Animate();

	//  렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		g_pTerrain->Draw();
		if (!g_bHideFrustum) g_pFrustum->Draw(g_pd3dDevice);

		// 렌더링 종료
		g_pd3dDevice->EndScene();
	}

	// 후면 버퍼를 보이는 화면으로
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
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
		CleanUp();
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostMessage(hWnd, WM_DESTROY, 0, 0L);
			break;
		case '1':
			g_bWireframe = !g_bWireframe;
			break;
		case '2':
			g_bLockFrustum = !g_bLockFrustum;
			g_bHideFrustum = !g_bLockFrustum;
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
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

	// 윈도우 클래스 등록
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
					GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					"BasicFrame", NULL };
	RegisterClassEx(&wc);

	// 윈도우 생성
	HWND hWnd = CreateWindow(
		"BasicFrame", WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW, 100, 100, WINDOW_W, WINDOW_H,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	g_hwnd = hWnd;

	// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd))) {
		InitObjects();
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

	DeleteObjects();
	UnregisterClass("BasicFrame", wc.hInstance);
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
