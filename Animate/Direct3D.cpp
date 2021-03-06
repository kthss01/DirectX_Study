// Animate.cpp: 응용 프로그램의 진입점을 정의합니다.
/*
	애니메이션의 가장 대표적인 기법은 키프레임 애니메이션이다.
	우리는 위치와 회전의 키값을 만들고, 이들 키를 보간(interpolate)
	하는 애니메이션을 제작해 볼 것이다.
*/

#include "stdafx.h"
#include "Direct3D.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HWND g_hwnd;

LPDIRECT3D9 g_pD3D = NULL;	// D3D 디바이스를 생성할 D3D 객체 변수
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;	// 렌더링에 사용될 D3D 디바이스
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;	// 정점을 보관할 정점 버퍼
LPDIRECT3DINDEXBUFFER9 g_pIB = NULL; // 인덱스를 보관할 인덱스 버퍼

D3DXMATRIXA16 g_matTMParent; // 부모의 TM
D3DXMATRIXA16 g_matRParent; // 부모의 회전행렬

D3DXMATRIXA16 g_matTMChild; // 자식의 TM
D3DXMATRIXA16 g_matRChild; // 자식의 회전행렬

float g_fRot = 0.0f;

struct CUSTOMVERTEX {
	float x, y, z; // 정점의 변환된 좌표
	DWORD color; // 정점의 색깔
};

// 사용자 정점 구조체에 관한 정보를 나타내는 FVF값
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE )

struct MYINDEX {
	WORD _0, _1, _2; // 일반적으로 인덱스는 16비트의 크기를 갖는다.
};

// 애니메이션 키값을 보간하기 위한 배열
D3DXVECTOR3 g_aniPos[2]; // 위치(position) 키 값
D3DXQUATERNION g_aniRot[2]; // 회전(quaternion) 키 값

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

	// 정점에 색깔값이 있으므로, 광원 기능을 끈다.
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// 디바이스 상태 정보를 처리할 경우 여기에서 한다.
	return S_OK;
}

/*
	정점 버퍼를 생성하고 정점값을 채워 넣는다.
	정점 버퍼란 기본적으로 정점 정보를 갖고 있는 메모리 블록이다.
	정점 버퍼를 생성한 다음에는 반드시 Lock()과 Unlock()으로 포인터를 얻어내서
	정점 정보를 정점 버퍼에 써넣어야 한다.
	또한 D3D는 인덱스 버퍼도 사용 가능하다는 것을 명심하자.
	정점 버퍼나 인덱스 버퍼는 기본 시스템 메모리 외에 디바이스 메모리(비디오 카드 메모리)
	에 생성될 수 있는데, 대부분의 비디오카드에서는 이렇게 할 경우 엄청난 속도의 향상을
	얻을 수 있다.
*/
HRESULT InitVB() {
	// 상자(cube)를 렌더링하기 위해 8개의 정점 선언
	CUSTOMVERTEX vertices[] =
	{
		{ -1,  1,  1 , 0xffff0000 }, // v0
		{  1,  1,  1 , 0xff00ff00 }, // v1
		{  1,  1, -1 , 0xff0000ff }, // v2
		{ -1,  1, -1 , 0xffffff00 }, // v3

		{ -1, -1,  1 , 0xff00ffff }, // v4
		{  1, -1,  1 , 0xffff00ff }, // v5
		{  1, -1, -1 , 0xff000000 }, // v6
		{ -1, -1, -1 , 0xffffffff }, // v7
	};
	
	// 정점 버퍼 생성
	// 8개의 사용자 정점을 보관할 메모리를 할당한다.
	// FVF를 지정하여 보관할 데이터의 형식을 지정한다.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(
		8 * sizeof(CUSTOMVERTEX), 
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
		return E_FAIL;
	}

	// 정점 버퍼를 값으로 채운다.
	// 정점 버퍼의 Lock() 함수를 호출하여 포인터를 얻어온다.
	void* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}

/*
	인덱스 버퍼를 생성하고 인덱스 값을 채워 넣는다.
*/
HRESULT InitIB() {
	// 상자(cube)를 렌더링하기 위해 12개의 면 선언
	MYINDEX indices[] = {
		{ 0, 1, 2 }, { 0, 2, 3 }, // 윗면
		{ 4, 6, 5 }, { 4, 7, 6 }, // 아랫면
		{ 0, 3, 7 }, { 0, 7, 4 }, // 왼쪽면
		{ 1, 5, 6 }, { 1, 6, 2 }, // 오른쪽면
		{ 3, 2, 6 }, { 3, 6, 7 }, // 앞면
		{ 0, 4, 5 }, { 0, 5, 1 }, // 뒷면
	};

	// 인덱스 버퍼 생성
	// D3DFMT_INDEX16은 인덱스의 단위가 16비트라는 것임
	// 우리는 MYINDEX 구조체에서 WORD형으로 선언했으므로
	// D3DFMT_INDEX16을 사용한다.
	if (FAILED(g_pd3dDevice->CreateIndexBuffer(
		12 * sizeof(MYINDEX), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT,
		&g_pIB, NULL)))
		return E_FAIL;

	// 인덱스 버퍼를 값으로 채운다.
	// 인덱스 버퍼의 Lock() 함수를 호출하여 포인터를 얻어온다.
	void* pIndices;
	if (FAILED(g_pIB->Lock(0, sizeof(indices), (void**)&pIndices, 0)))
		return E_FAIL;	
	memcpy(pIndices, indices, sizeof(indices));
	g_pIB->Unlock();

	return S_OK;
}

void InitAnimation() {
	g_aniPos[0] = D3DXVECTOR3(0, 0, 0); // 위치 키(0, 0, 0)
	g_aniPos[1] = D3DXVECTOR3(5, 5, 5); // 위치 키(5, 5, 5)

	float Yaw = D3DX_PI * 90.0f / 180.0f; // Y축 90도 회전
	float Pitch = 0;
	float Roll = 0;
	D3DXQuaternionRotationYawPitchRoll(&g_aniRot[0], Yaw, Pitch, Roll);
	// 사원수 키 (Y축 90도)

	Yaw = 0;
	Pitch = D3DX_PI * 90.0f / 180.0f; // X축 90도 회전
	Roll = 0;
	D3DXQuaternionRotationYawPitchRoll(&g_aniRot[1], Yaw, Pitch, Roll);
	// 사원수 키 (X축 90도)
}

/*
	기하 정보 초기화
*/
HRESULT InitGeometry() {
	if (FAILED(InitVB())) return E_FAIL;
	if (FAILED(InitIB())) return E_FAIL;

	InitAnimation();

	return S_OK;
}

/*
	카메라 행렬 설정
*/
void SetupCamera() {
	// 월드 행렬 설정
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	// 뷰 행렬 설정
	D3DXVECTOR3 vEyePt(0.0f, 10.0f, -20.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// 프로젝션 행렬 설정
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

// 선형보간(Linear Interpolation) 함수
float Linear(float v0, float v1, float t) {
	return v0 * (1.0f - t) + v1 * t;
	// 다음 줄로 바꿔도 된다.
	// return v0 + t * ( v1 - v0 );
}

bool aniForward = true;

/*
	애니메이션 행렬 생성
*/
void Animate() {
	static float t = 0;
	float x, y, z;
	D3DXQUATERNION quat;

	if (t > 1.0f) {
		aniForward = false;
	}
	if (t < 0.0f) {
		aniForward = true;
	}

	// 위치값의 선형보간
	x = Linear(g_aniPos[0].x, g_aniPos[1].x, t);
	y = Linear(g_aniPos[0].y, g_aniPos[1].y, t);
	z = Linear(g_aniPos[0].z, g_aniPos[1].z, t);
	D3DXMatrixTranslation(&g_matTMParent, x, y, z); // 이동행렬을 구한다

	// 위의 4줄은 다음의 3줄로 바꿀 수 있다.
	//D3DXVECTOR3 v;
	//D3DXVec3Lerp(&v, &g_aniPos[0], &g_aniPos[1], t);
	//D3DXMatrixTranslation(&g_matTMParent, v.x, v.y, v.z);

	// 회전값의 구면선형보간
	D3DXQuaternionSlerp(&quat, &g_aniRot[0], &g_aniRot[1], t);
	// 사원수를 회전행렬값으로 변환
	D3DXMatrixRotationQuaternion(&g_matRParent, &quat);

	if (aniForward)
		t += 0.005f;
	else
		t -= 0.005f;
	
	// 자식메시의 Z축 회전행렬
	D3DXMatrixRotationZ(&g_matRChild, GetTickCount() / 500.0f);
	// 자식메시는 원점으로부터 (3, 3, 3) 거리에 있음
	D3DXMatrixTranslation(&g_matTMChild, 3, 3, 3);
}

/*
	초기화된 객체들을 소거한다.
*/
void CleanUp() {
	if (g_pIB != NULL)
		g_pIB->Release();

	if (g_pVB != NULL)
		g_pVB->Release();

	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}

/*
	메시 그리기
*/
void DrawMesh(D3DXMATRIXA16* pMat) {
	g_pd3dDevice->SetTransform(D3DTS_WORLD, pMat);
	g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	g_pd3dDevice->SetIndices(g_pIB);
	g_pd3dDevice->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
}

/*
	화면을 그린다.
*/
void Render() {
	D3DXMATRIXA16 matWorld;

	// 후변 버퍼와 Z버퍼를 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// 애니메이션 행렬 설정
	Animate();

	//  렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		matWorld = g_matRParent * g_matTMParent;
		DrawMesh(&matWorld); // 부모 상자 그리기

		matWorld = g_matRChild * g_matTMChild * 
			g_matRParent * g_matTMParent;
		// 바로 위의 행과 같은 결과
		//matWorld = g_matRChild * g_matTMChild * matWorld; 
		DrawMesh(&matWorld); // 자식 상자 그리기

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
		"BasicFrame", "Keyframe Animation",
		WS_OVERLAPPEDWINDOW, 100, 100, WINDOW_W, WINDOW_H,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	g_hwnd = hWnd;

	// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd))) {

		if (SUCCEEDED(InitGeometry())) {
			// 카메라 행렬 설정
			SetupCamera();

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

#define ROT_DELTA 0.1f

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		CleanUp();
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_LEFT) g_fRot -= ROT_DELTA;
		if (wParam == VK_RIGHT) g_fRot += ROT_DELTA;
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
