// Billboard.cpp: 응용 프로그램의 진입점을 정의합니다.
/*
	Billboard
*/

#include "stdafx.h"
#include "Direct3D.h"
#include "ZCamera.h"
#include "ZWater.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HWND g_hwnd = NULL;

LPDIRECT3D9 g_pD3D = NULL;	// D3D 디바이스를 생성할 D3D 객체 변수
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;	// 렌더링에 사용될 D3D 디바이스
LPDIRECT3DTEXTURE9 g_pTexBillboard[4] = { NULL, NULL, NULL, NULL };

D3DXMATRIXA16 g_matAni;
D3DXMATRIXA16 g_matWorld;
D3DXMATRIXA16 g_matView;
D3DXMATRIXA16 g_matProj;

DWORD g_dwMouseX = 0; // 마우스의 좌표
DWORD g_dwMouseY = 0; // 마우스의 좌표
bool g_bBillboard = true; // 빌보드처리를 할것인가
bool g_bWireframe = false; // 와이어프레임으로 그릴것인가

ZCamera* g_pCamera = NULL; // 카메라 클래스
ZWater* g_pWater = NULL;

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

	// 뷰 행렬을 설정
	D3DXVECTOR3 vEyePt(0.0f, 5.0f, (float)-3.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 5.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &g_matAni);

	// 실제 프로젝션 행렬
	D3DXMatrixPerspectiveFovLH(
		&g_matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &g_matProj);

	// 프리스텀 컬링용 프로젝션 행렬
	D3DXMatrixPerspectiveFovLH(
		&g_matProj, D3DX_PI / 4, 1.0f, 1.0f, 200.0f);

	g_pCamera->SetView(&vEyePt, &vLookatPt, &vUpVec);

}

/*
	기하정보 초기화
*/
HRESULT InitGeometry() {
	InitMatrix();
	// 빌보드로 사용할 텍스처 이미지
	D3DXCreateTextureFromFile(g_pd3dDevice, "tree01S.dds", &g_pTexBillboard[0]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "tree02S.dds", &g_pTexBillboard[1]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "tree35S.dds", &g_pTexBillboard[2]);
	D3DXCreateTextureFromFile(g_pd3dDevice, "SGA Logo.png", &g_pTexBillboard[3]);

	// 최초의 마우스 위치 보관
	POINT pt;
	GetCursorPos(&pt);
	g_dwMouseX = pt.x;
	g_dwMouseY = pt.y;

	return S_OK;
}

HRESULT InitObjecs() {
	g_pCamera = new ZCamera;
	g_pWater = new ZWater;
	g_pWater->Create(g_pd3dDevice, 64, 64, 100);

	return S_OK;
}

void DeleteObject() {
	SAFE_DELETE(g_pWater);
	SAFE_DELETE(g_pCamera);
}

void Cleanup() {
	for (int i = 0; i < 4; i++) 
		SAFE_RELEASE(g_pTexBillboard[i]);
	SAFE_RELEASE(g_pd3dDevice);
	SAFE_RELEASE(g_pD3D);
}

/*
	광원 설정
*/
void SetupLights() {
	// 재질(material)설정
	// 재질은 디바이스에 단 하나만 설정될 수 있다.
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pd3dDevice->SetMaterial(&mtrl);

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);

	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);
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
	애니메이션 설정
*/
void Animate() {
	D3DXMatrixIdentity(&g_matAni);

	SetupLights();
	ProcessInputs();
}

void DrawBillboard() {
	// 알파채널을 사용해서 투명 텍스처 효과를 낸다
	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	struct MYVERTEX {
		enum { FVF = D3DFVF_XYZ | D3DFVF_TEX1 };
		float px, py, pz;
		float tu, tv;
	};

	// 빌보드 정점
	MYVERTEX vtx[4] = {
		{ -1,  0, 0, 0, 1 },
		{ -1,  4, 0, 0, 0 },
		{ 1,  0, 0, 1, 1 },
		{ 1,  4, 0, 1, 0 }
	};
	D3DXMATRIXA16 matBillboard;
	D3DXMatrixIdentity(&matBillboard);

	// 0번 텍스처에 빌보드 텍스처를 올린다
	g_pd3dDevice->SetTexture(1, NULL);
	g_pd3dDevice->SetFVF(MYVERTEX::FVF);

	if (g_bBillboard) {
		// Y축 회전행렬은 _11, _13, _31, _33번 행렬에 회전값이 들어간다
		// 카메라의 Y축 회전행렬값을 읽어서 역행렬을 만들면 X, Z축이 고정된
		// Y축 회전 빌보드 행렬을 만들 수 있다.
		matBillboard._11 = g_pCamera->GetViewMatrix()->_11;
		matBillboard._13 = g_pCamera->GetViewMatrix()->_13;
		matBillboard._31 = g_pCamera->GetViewMatrix()->_31;
		matBillboard._33 = g_pCamera->GetViewMatrix()->_33;
		D3DXMatrixInverse(&matBillboard, NULL, &matBillboard);
	}

	// 빌보드의 좌표를 바꿔가며 찍는다
	for (int z = 0; z <= 40; z += 5) {
		for (int x = 0; x <= 40; x += 5) {
			matBillboard._41 = x - 20;
			matBillboard._42 = 0;
			matBillboard._43 = z - 20;
			g_pd3dDevice->SetTexture(0, g_pTexBillboard[(x + z) % 4]);
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matBillboard);
			g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vtx, sizeof(MYVERTEX));
		}
	}

	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &g_matWorld);
}

/*
	화면을 그린다.
*/
void Render() {
	D3DXMATRIXA16 matWorld;

	// 후변 버퍼와 Z버퍼를 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(200, 200, 200), 1.0f, 0);
	g_pd3dDevice->SetRenderState(D3DRS_FILLMODE,
		g_bWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

	// 애니메이션 행렬 설정
	Animate();

	//  렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {

		g_pWater->Draw();

		DrawBillboard();

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

	// 윈도우 클래스 등록
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
					GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					"BasicFrame", NULL };
	RegisterClassEx(&wc);

	// 윈도우 생성
	HWND hWnd = CreateWindow(
		"BasicFrame", "Billboard",
		WS_OVERLAPPEDWINDOW, 100, 100, 500, 500,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	g_hwnd = hWnd;

	srand(GetTickCount());

	// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd))) {
		if (SUCCEEDED(InitObjecs())) {
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
	}

	// 등록된 클래스 소거
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
		Cleanup();
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
			g_bBillboard = !g_bBillboard;
		}
		break;
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
