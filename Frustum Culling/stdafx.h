// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>
// 여기 없는거 같음
#include <mmsystem.h>					// TimeGetTime() 함수를 사용하기 위해서 포함하는 헤더

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define WINDOW_W 500
#define WINDOW_H 500
#define WINDOW_TITLE "HeightMap+Camera+Frustum"
#define BMP_HEIGHTMAP "map128.bmp"

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
