// Skinning.cpp: 응용 프로그램의 진입점을 정의합니다.
/*
	D3D의 기본 API기능을 사용한 매트릭스 팔레트 방식 스키닝
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
LPDIRECT3DTEXTURE9 g_pTexture = NULL; // 텍스처

D3DXMATRIXA16 g_mat0; // 0번 행렬
D3DXMATRIXA16 g_mat1; // 1번 행렬

struct CUSTOMVERTEX {
	D3DXVECTOR3 position; // 정점의 변환된 좌표
	float b[3]; // 가중치
	DWORD index; // 가중치 인덱스
	DWORD color; // 정점의 색깔
	float tu, tv; // 텍스처 좌표
};

// 사용자 정점 구조체에 관한 정보를 나타내는 FVF값
// D3DFVF_XYZB4 : 4개의 블렌드 값
// D3DFVF_LASTBETA_UBYTE4: 마지막 DWORD index 값은 unsinged byte형 4개를 나타냄
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_DIFFUSE | D3DFVF_TEX1)

struct MYINDEX {
	WORD _0, _1, _2; // 일반적으로 인덱스는 16비트의 크기를 갖는다.
};

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

	// 매트릭스 팔레트 사용
	g_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, true);

	// 가중치는 4개
	g_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS);

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

	// 정점 버퍼 생성
	// 8개의 사용자 정점을 보관할 메모리를 할당한다.
	// FVF를 지정하여 보관할 데이터의 형식을 지정한다.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(
		50 * 2 * sizeof(CUSTOMVERTEX), 
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
		return E_FAIL;
	}

	// 정점 버퍼를 값으로 채운다.
	// 정점 버퍼의 Lock() 함수를 호출하여 포인터를 얻어온다.
	CUSTOMVERTEX* pVertices;
	if (FAILED(g_pVB->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;

	for (DWORD i = 0; i < 50; i++) {
		float theta = (2 * D3DX_PI * i) / (50 - 1);

		pVertices[2 * i + 0].position = 
			D3DXVECTOR3(sinf(theta), -1.0f, cosf(theta));
		pVertices[2 * i + 0].b[0] = 1.0f;
		pVertices[2 * i + 0].b[1] = 0.0f;
		pVertices[2 * i + 0].b[2] = 0.0f;
		// 0번 가중치는 0번 행렬의 영향을 1.0만큼 받음
		pVertices[2 * i + 0].index = 0x0000;
		pVertices[2 * i + 0].color = 0xffffffff;
		pVertices[2 * i + 0].tu = ((float)i) / (50 - 1);
		pVertices[2 * i + 0].tv = 1.0f;

		pVertices[2 * i + 1].position =
			D3DXVECTOR3(sinf(theta), 1.0f, cosf(theta));
		pVertices[2 * i + 1].b[0] = 0.5f;
		pVertices[2 * i + 1].b[1] = 0.5f;
		pVertices[2 * i + 1].b[2] = 0.0f;
		// 0번 가중치는 0번 행렬의 영향을 0.5만큼 받음
		pVertices[2 * i + 1].index = 0x0001;
		pVertices[2 * i + 1].color = 0xff808080;
		pVertices[2 * i + 1].tu = ((float)i) / (50 - 1);
		pVertices[2 * i + 1].tv = 0.0f;
	}

	g_pVB->Unlock();

	return S_OK;
}

/*
	인덱스 버퍼를 생성하고 인덱스 값을 채워 넣는다.
*/
HRESULT InitIB() {

	return S_OK;
}

/*
	기하 정보 초기화
*/
HRESULT InitGeometry() {
	if (FAILED(InitVB())) return E_FAIL;
	if (FAILED(InitIB())) return E_FAIL;

	if (FAILED(D3DXCreateTextureFromFile(
		g_pd3dDevice, "lake.bmp", &g_pTexture)))
		return E_FAIL;

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
	D3DXVECTOR3 vEyePt(0.0f, 2.0f, -3.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// 프로젝션 행렬 설정
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(
		&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

/*
	애니메이션 행렬 생성
*/
void Animate() {
	// 0번 행렬은 단위행렬
	D3DXMatrixIdentity(&g_mat0);

	// 0 ~ 2PI까지(0~360도) 값을 변화시킴 Fixed Point 기법 사용
	DWORD d = GetTickCount() % ((int)((D3DX_PI * 2) * 1000));
	// Y축 회전행렬
	D3DXMatrixRotationY(&g_mat1, d / 1000.0f);
}

/*
	초기화된 객체들을 소거한다.
*/
void CleanUp() {
	if (g_pTexture != NULL)
		g_pTexture->Release();

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
void DrawMesh(void) {
	g_pd3dDevice->SetStreamSource(
		0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	g_pd3dDevice->DrawPrimitive(
		D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2);
}

/*
	화면을 그린다.
*/
void Render() {
	D3DXMATRIXA16 matWorld;

	// 후변 버퍼와 Z버퍼를 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);

	// 애니메이션 행렬 설정
	Animate();

	//  렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		// 0번 매트릭스 팔레트에 단위행렬
		g_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &g_mat0);
		// 1번 매트릭스 팔레트에 회전행렬
		g_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(1), &g_mat1);
		g_pd3dDevice->SetTexture(0, g_pTexture);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,
			D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1,
			D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2,
			D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,
			D3DTOP_DISABLE);

		DrawMesh();

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
		"BasicFrame", "Skinning",
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
