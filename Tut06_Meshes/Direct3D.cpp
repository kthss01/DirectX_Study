// Tut06_Meshes.cpp: 응용 프로그램의 진입점을 정의합니다.
/*
	보다 멋진 기하 정보를 출력하기 위해서는 전문적인 3D모델러가 만든 모델을
	파일로 읽어들이는 것이 일반적이다. 다행스럽게도 D3DX에는 강력한 X파일
	처리 기능이 있어서 정점 버퍼, 인덱스 버퍼 생성 등의 많은 부분을 대신해 준다.
	이번 예제는 D3DXMESH를 사용하여 파일을 읽어서 이 파일과 연관된 재질과
	텍스처를 함께 사용하는 것을 알아보자.

	이번에 소개되지는 않지만 나중에 사용하게 될 강력한 기능 중에 하나가
	FVF를 지정하여 새로운 메시를 복제(clone) 하는 것이다. 이 기능을 사용하여
	텍스처 좌표나 법선벡터 등을 기존메시에 추가한 새로운 메시를 생성할 수 있다.
	(나중에 Cg, HLSL 등을 공부할 때 많이 사용될 것이다.)
*/

#include "stdafx.h"
#include "Direct3D.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

LPDIRECT3D9 g_pD3D = NULL;	// D3D 디바이스를 생성할 D3D 객체 변수
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;	// 렌더링에 사용될 D3D 디바이스
LPD3DXMESH g_pMesh = NULL;	// 메시 객체
D3DMATERIAL9* g_pMeshMaterials = NULL; // 메시에서 사용할 재질
LPDIRECT3DTEXTURE9* g_pMeshTextures = NULL;	// 메시에서 사용할 텍스처
DWORD g_dwNumMaterials = 0L; // 메시에서 사용중인 재질의 개수

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

	// Z버퍼 기능을 켠다.
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	// 주변 광원값을 최대 밝기로
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

	// 디바이스 상태 정보를 처리할 경우 여기에서 한다.
	return S_OK;
}

/*
	기하 정보 초기화
	메시읽기, 재질과 텍스처 배열 생성
*/
HRESULT InitGeometry() {
	// 재질을 임시로 보관할 버퍼 선언
	LPD3DXBUFFER pD3DXMtrlBuffer;

	// Tiger.x 파일을 메시로 읽어들인다. 이때 재질 정보도 함께 읽는다.
	if (FAILED(D3DXLoadMeshFromX("Tiger.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL, &pD3DXMtrlBuffer, NULL,
		&g_dwNumMaterials, &g_pMesh))) {
		// 현재 폴더에 파일이 없으면 상위폴더 검색
		if (FAILED(D3DXLoadMeshFromX("..\\Tiger.x",
			D3DXMESH_SYSTEMMEM, g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials, &g_pMesh))) {
			MessageBox(NULL, "Could not find tiger.x", "Direct3D.exe",
				MB_OK);
			return E_FAIL;
		}
	}

	// 재질 정보와 텍스처 정보를 따로 뽑아낸다.
	D3DXMATERIAL* d3dxMaterials = 
		(D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	// 재질 개수만큼 재질 구조체 배열 생성
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	// 재질 개수만큼 텍스처 배열 생성
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];

	for (DWORD i = 0; i < g_dwNumMaterials; i++) {
		// 재질 정보 복사
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// 주변 광원 정보를 Diffuse 정보로
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlen(d3dxMaterials[i].pTextureFilename) > 0) {
			// 텍스처를 파일에서 로드한다.
			if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&g_pMeshTextures[i]))) {
				// 텍스처가 현재 폴더에 없으면 상위폴더 검색
				const TCHAR* strPrefix = TEXT("..\\");
				const int lenPrefix = lstrlen(strPrefix);
				TCHAR strTexture[MAX_PATH];
				lstrcpyn(strTexture, strPrefix, MAX_PATH);
				lstrcpyn(strTexture + lenPrefix,
					d3dxMaterials[i].pTextureFilename, MAX_PATH - lenPrefix);
				if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice,
					strTexture, &g_pMeshTextures[i]))) {
					MessageBox(NULL, "Could not find texture map",
						"Direct3D.exe", MB_OK);
				}
			}
		}
	}

	// 임시로 생성한 재질 버퍼 소거
	pD3DXMtrlBuffer->Release();

	return S_OK;
}

/*
	초기화된 객체들을 소거한다.
*/
void ClenUp() {
	if (g_pMeshMaterials != NULL)
		delete[] g_pMeshMaterials;

	if (g_pMeshTextures != NULL) {
		for (DWORD i = 0; i < g_dwNumMaterials; i++) {
			if (g_pMeshTextures[i])
				g_pMeshTextures[i]->Release();
		}
		delete[] g_pMeshTextures;
	}

	if (g_pMesh != NULL)
		g_pMesh->Release();

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
	D3DXMatrixRotationY(&matWorld, timeGetTime() / 1000.0f);
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
	화면을 그린다.
*/
void Render() {
	// 후변 버퍼와 Z버퍼를 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	//  렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {

		// 월드, 뷰, 프로젝션 행렬을 설정한다.
		SetUpMatrices();

		// 메시는 재질이 다른 메시별로 부분집합을 이루고 있다.
		// 이들을 루프를 수행해서 모두 그려준다.
		for (DWORD i = 0; i < g_dwNumMaterials; i++) {
			// 부분집합 메시의 재질과 텍스처 생성
			g_pd3dDevice->SetMaterial(&g_pMeshMaterials[i]);
			g_pd3dDevice->SetTexture(0, g_pMeshTextures[i]);

			// 부분집합 메시 출력
			g_pMesh->DrawSubset(i);
		}

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
					"D3D Tutorial", NULL };
	RegisterClassEx(&wc);

	// 윈도우 생성
	HWND hWnd = CreateWindow(
		"D3D Tutorial", "D3D Tutorial 06: Meshes",
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
	UnregisterClass("D3D Tutorial", wc.hInstance);
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
