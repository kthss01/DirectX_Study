#include "stdafx.h"
#include "ZQuadTree.h"
#include "ZFLog.h"
#include "ZFrustum.h"

ZQuadTree::ZQuadTree(int cx, int cy) {
	int i;
	m_nCenter = 0;
	for (i = 0; i < 4; i++) {
		m_pChild[i] = NULL;
	}

	m_nCorner[CORNER_TL] = 0;
	m_nCorner[CORNER_TR] = cx -1;
	m_nCorner[CORNER_BL] = cx * (cy - 1);
	m_nCorner[CORNER_BR] = cx * cy - 1;
	m_nCenter = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] +
		m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR]) / 4;

	m_fRadius = 0.0f;
	m_bCulled = false;
	m_fRadius = 0.0f;
}

ZQuadTree::ZQuadTree(ZQuadTree* pParent) {
	int i;
	m_nCenter = 0;
	for (i = 0; i < 4; i++) {
		m_pChild[i] = NULL;
		m_nCorner[i] = 0;
	}

	m_bCulled = false;
	m_fRadius = 0.0f;
}

ZQuadTree::~ZQuadTree() {
	_Destroy();
}

void ZQuadTree::_Destroy() {
	for (int i = 0; i < 4; i++) DEL(m_pChild[i]);
}

void ZQuadTree::_AllInFrustum()
{
	m_bCulled = false;
	if (!m_pChild[0]) return;

	m_pChild[0]->_AllInFrustum();
	m_pChild[1]->_AllInFrustum();
	m_pChild[2]->_AllInFrustum();
	m_pChild[3]->_AllInFrustum();
}

bool ZQuadTree::_SetCorners(
	int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR) {
	m_nCorner[CORNER_TL] = nCornerTL;
	m_nCorner[CORNER_TR] = nCornerTR;
	m_nCorner[CORNER_BL] = nCornerBL;
	m_nCorner[CORNER_BR] = nCornerBR;
	m_nCenter = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] +
		m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR]) / 4;

	return true;
}

ZQuadTree* ZQuadTree::_AddChild(int nCornerTL, int nCornerTR,
	int nCornerBL, int nCornerBR) {
	ZQuadTree* pChild;

	pChild = new ZQuadTree(this);
	pChild->_SetCorners(nCornerTL, nCornerTR, nCornerBL, nCornerBR);

	return pChild;
}

bool ZQuadTree::_SubDivide() {
	int nTopEdgeCenter;
	int nBottomEdgeCenter;
	int nLeftEdgeCenter;
	int nRightEdgeCenter;
	int nCentralPoint;

	// 상단변 가운데
	nTopEdgeCenter = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR]) / 2;
	// 하단변 가운데
	nBottomEdgeCenter = (m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR]) / 2;
	// 좌측변 가운데
	nLeftEdgeCenter = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_BL]) / 2;
	// 우측변 가운데
	nRightEdgeCenter = (m_nCorner[CORNER_TR] + m_nCorner[CORNER_BR]) / 2;
	// 한가운데
	nCentralPoint = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] +
		m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR]) / 4;

	// 더 이상 분할이 불가능한가? 그렇다면 SubDivide() 종료
	if ((m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL]) <= 1)
		return false;

	// 4개의 자식노드 추가
	m_pChild[CORNER_TL] = _AddChild(
		m_nCorner[CORNER_TL], nTopEdgeCenter, nLeftEdgeCenter, nCentralPoint);
	m_pChild[CORNER_TR] = _AddChild(
		nTopEdgeCenter, m_nCorner[CORNER_TR], nCentralPoint, nRightEdgeCenter);
	m_pChild[CORNER_BL] = _AddChild(
		nLeftEdgeCenter, nCentralPoint, m_nCorner[CORNER_BL], nBottomEdgeCenter);
	m_pChild[CORNER_BR] = _AddChild(
		nCentralPoint, nRightEdgeCenter, nBottomEdgeCenter, m_nCorner[CORNER_BR]);

	return true;
}

int ZQuadTree::_GenTriIndex(int nTris, LPVOID pIndex, 
	TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum, float fLODRatio) {
	// 컬링된 노드라면 그냥 리턴한다.
	if (m_bCulled) {
		m_bCulled = false;
		return nTris;
	}

#ifdef _USE_INDEX16
		LPWORD p = ((LPWORD)pIndex) + nTris * 3;
#else
		LPDWORD p = ((LPWORD)pIndex) + nTris * 3;
#endif

	// 현재 노드가 출력되어야 하는가?
	if (_IsVisible(pHeightMap, pFrustum->GetPos(), fLODRatio)) {
		// 만약 최하위 노드라면 부분분할(subdivide)이 불가능하므로 
		// 그냥 출력하고 리턴한다.
		if (m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL] <= 1) {
			// 좌측상단 삼각형
			*p++ = m_nCorner[0];
			*p++ = m_nCorner[1];
			*p++ = m_nCorner[2];
			nTris++;

			// 우측하단 삼각형
			*p++ = m_nCorner[2];
			*p++ = m_nCorner[1];
			*p++ = m_nCorner[3];
			nTris++;

			return nTris;
		}
		
		bool b[4] = { 0,0,0,0 };
		// 상단 이웃노드(neighbor node)가 출력 가능한가?
		if (m_pNeighbor[EDGE_UP])
			b[EDGE_UP] = m_pNeighbor[EDGE_UP]->
			_IsVisible(pHeightMap, pFrustum->GetPos(), fLODRatio);
		// 하단 이웃노드(neighbor node)가 출력 가능한가?
		if (m_pNeighbor[EDGE_DN])
			b[EDGE_DN] = m_pNeighbor[EDGE_DN]->
			_IsVisible(pHeightMap, pFrustum->GetPos(), fLODRatio);
		// 좌측 이웃노드(neighbor node)가 출력 가능한가?
		if (m_pNeighbor[EDGE_LT])
			b[EDGE_LT] = m_pNeighbor[EDGE_LT]->
			_IsVisible(pHeightMap, pFrustum->GetPos(), fLODRatio);
		// 우측 이웃노드(neighbor node)가 출력 가능한가?
		if (m_pNeighbor[EDGE_RT])
			b[EDGE_RT] = m_pNeighbor[EDGE_RT]->
			_IsVisible(pHeightMap, pFrustum->GetPos(), fLODRatio);

		// 이웃노드들이 모두 다 출력 가능하다면 현재노드와 이웃노드들이 
		// 같이 LOD이므로 부분분할이 필요 없다.
		if (b[EDGE_UP] && b[EDGE_DN] && b[EDGE_LT] && b[EDGE_RT]) {
			// 좌측상단 삼각형
			*p++ = m_nCorner[0];
			*p++ = m_nCorner[1];
			*p++ = m_nCorner[2];
			nTris++;

			// 우측하단 삼각형
			*p++ = m_nCorner[2];
			*p++ = m_nCorner[1];
			*p++ = m_nCorner[3];
			nTris++;

			return nTris;
		}
		
		int n;

		if (!b[EDGE_UP]) // 상단 부분분할이 필요한가?
		{
			n = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR]) / 2;
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_TL];
			*p++ = n;
			nTris++;
			*p++ = m_nCenter;
			*p++ = n;
			*p++ = m_nCorner[CORNER_TR];
			nTris++;
		}
		else // 상단 부분분할이 필요 없을 경우
		{
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_TL];
			*p++ = m_nCorner[CORNER_TR];
			nTris++;
		}

		if (!b[EDGE_DN]) // 하단 부분분할이 필요한가?
		{
			n = (m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR]) / 2;
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_BR];
			*p++ = n;
			nTris++;
			*p++ = m_nCenter;
			*p++ = n;
			*p++ = m_nCorner[CORNER_BL];
			nTris++;
		}
		else // 하단 부분분할이 필요 없을 경우
		{
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_BR];
			*p++ = m_nCorner[CORNER_BL];
			nTris++;
		}

		if (!b[EDGE_LT]) // 좌측 부분분할이 필요한가?
		{
			n = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_BL]) / 2;
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_BL];
			*p++ = n;
			nTris++;
			*p++ = m_nCenter;
			*p++ = n;
			*p++ = m_nCorner[CORNER_TL];
			nTris++;
		}
		else // 좌측 부분분할이 필요 없을 경우
		{
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_BL];
			*p++ = m_nCorner[CORNER_TL];
			nTris++;
		}

		if (!b[EDGE_RT]) // 우측 부분분할이 필요한가?
		{
			n = (m_nCorner[CORNER_TR] + m_nCorner[CORNER_BR]) / 2;
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_TR];
			*p++ = n;
			nTris++;
			*p++ = m_nCenter;
			*p++ = n;
			*p++ = m_nCorner[CORNER_BR];
			nTris++;
		}
		else // 우측 부분분할이 필요 없을 경우
		{
			*p++ = m_nCenter;
			*p++ = m_nCorner[CORNER_TR];
			*p++ = m_nCorner[CORNER_BR];
			nTris++;
		}

		return nTris;
	}

	// 자식 노드들 검색
	if (m_pChild[CORNER_TL]) nTris = m_pChild[CORNER_TL]->
		_GenTriIndex(nTris, pIndex, pHeightMap, pFrustum, fLODRatio);
	if (m_pChild[CORNER_TR]) nTris = m_pChild[CORNER_TR]->
		_GenTriIndex(nTris, pIndex, pHeightMap, pFrustum, fLODRatio);
	if (m_pChild[CORNER_BL]) nTris = m_pChild[CORNER_BL]->
		_GenTriIndex(nTris, pIndex, pHeightMap, pFrustum, fLODRatio);
	if (m_pChild[CORNER_BR]) nTris = m_pChild[CORNER_BR]->
		_GenTriIndex(nTris, pIndex, pHeightMap, pFrustum, fLODRatio);

	return nTris;
}

int ZQuadTree::_IsInFrustum(TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum) {
	bool b[4];
	bool bInSphere;

	// 경계구 안에 있는가?
	//if (m_fRadius == 0.0f) g_pLog->Log(
	//	(char*)"Index : [%d], Radius : [%f]", m_nCenter, m_fRadius);
	bInSphere = pFrustum->IsInSphere(
		(D3DXVECTOR3*)(pHeightMap + m_nCenter), m_fRadius);
	// 경계구 안에 없으면 점 단위의 절두체 테스트 생략
	if (!bInSphere) return FRUSTUM_OUT; 

	// 쿼드트리의 4군데 경계 절두체 테스트
	b[0] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[0]));
	b[1] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[1]));
	b[2] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[2]));
	b[3] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[3]));

	// 4개 모두 절두체 안에 있음
	if ((b[0] + b[1] + b[2] + b[3]) == 4) return FRUSTUM_COMPLETELY_IN;

	// 일부분이 절두체에 있는 경우
	return FRUSTUM_PARTIALLY_IN;
}

void ZQuadTree::_BuildNeighborNode(ZQuadTree * pRoot, TERRAINVERTEX * pHeightMap, int cx)
{
	int n;
	int _0, _1, _2, _3;

	for (int i = 0; i < 4; i++) {
		_0 = m_nCorner[0];
		_1 = m_nCorner[1];
		_2 = m_nCorner[2];
		_3 = m_nCorner[3];
		// 이웃노드의 4개 코너값을 얻는다.
		n = _GetNodeIndex(i, cx, _0, _1, _2, _3);
		// 코너값으로 이웃노드의 포인터를 얻어온다.
		if (n >= 0) m_pNeighbor[i] = pRoot->_FindNode(pHeightMap,
			_0, _1, _2, _3);
	}

	// 자식노드로 재귀호출
	if (m_pChild[0]) {
		m_pChild[0]->_BuildNeighborNode(pRoot, pHeightMap, cx);
		m_pChild[1]->_BuildNeighborNode(pRoot, pHeightMap, cx);
		m_pChild[2]->_BuildNeighborNode(pRoot, pHeightMap, cx);
		m_pChild[3]->_BuildNeighborNode(pRoot, pHeightMap, cx);
	}
}

bool ZQuadTree::_BuildQuadTree(TERRAINVERTEX * pHeightMap)
{
	if (_SubDivide()) {
		// 좌측 상단과 우측 하단의 거리를 구한다.
		D3DXVECTOR3 v =
			*((D3DXVECTOR3*)(pHeightMap + m_nCorner[CORNER_TL]))
			- *((D3DXVECTOR3*)(pHeightMap + m_nCorner[CORNER_BR]));
		// v의 거리값이 이 노드를 감싸는 경계구의 지름이므로,
		// 2로 나누어 반지름을 구한다.
		m_fRadius = D3DXVec3Length(&v) / 2.0f;
		m_pChild[CORNER_TL]->_BuildQuadTree(pHeightMap);
		m_pChild[CORNER_TR]->_BuildQuadTree(pHeightMap);
		m_pChild[CORNER_BL]->_BuildQuadTree(pHeightMap);
		m_pChild[CORNER_BR]->_BuildQuadTree(pHeightMap);
	}

	return true;
}

ZQuadTree* ZQuadTree::_FindNode(TERRAINVERTEX * pHeightMap, int _0, int _1, int _2, int _3)
{
	ZQuadTree* p = NULL;
	// 일치하는 노드라면 노드값을 리턴
	if ((m_nCorner[0] == _0) && (m_nCorner[1] == _1) &&
		(m_nCorner[2] == _2) && (m_nCorner[3] == _3))
		return this;

	// 자식노드가 있는가?
	if (m_pChild[0]) {
		RECT rc;
		POINT pt;
		int n = (_0 + _1 + _2 + _3) / 4;

		// 현재 맵상에서의 위치
		pt.x = (int)pHeightMap[n].p.x;
		pt.y = (int)pHeightMap[n].p.z;
		// 4개의 코너값을 기준으로 자식노드의 맵 점유범위를 얻는다.
		SetRect(&rc,
			(int)pHeightMap[m_pChild[0]->m_nCorner[CORNER_TL]].p.x,
			(int)pHeightMap[m_pChild[0]->m_nCorner[CORNER_TL]].p.z,
			(int)pHeightMap[m_pChild[0]->m_nCorner[CORNER_BR]].p.x,
			(int)pHeightMap[m_pChild[0]->m_nCorner[CORNER_BR]].p.z);

		// pt 값이 점유범위 안에 있다면 자식노드로 들어간다.
		if (PtInRect(&rc, pt))
			return m_pChild[0]->_FindNode(pHeightMap, _0, _1, _2, _3);

		// 4개의 코너값을 기준으로 자식노드의 맵 점유범위를 얻는다.
		SetRect(&rc,
			(int)pHeightMap[m_pChild[1]->m_nCorner[CORNER_TL]].p.x,
			(int)pHeightMap[m_pChild[1]->m_nCorner[CORNER_TL]].p.z,
			(int)pHeightMap[m_pChild[1]->m_nCorner[CORNER_BR]].p.x,
			(int)pHeightMap[m_pChild[1]->m_nCorner[CORNER_BR]].p.z);

		// pt 값이 점유범위 안에 있다면 자식노드로 들어간다.
		if (PtInRect(&rc, pt))
			return m_pChild[1]->_FindNode(pHeightMap, _0, _1, _2, _3);

		// 4개의 코너값을 기준으로 자식노드의 맵 점유범위를 얻는다.
		SetRect(&rc,
			(int)pHeightMap[m_pChild[2]->m_nCorner[CORNER_TL]].p.x,
			(int)pHeightMap[m_pChild[2]->m_nCorner[CORNER_TL]].p.z,
			(int)pHeightMap[m_pChild[2]->m_nCorner[CORNER_BR]].p.x,
			(int)pHeightMap[m_pChild[2]->m_nCorner[CORNER_BR]].p.z);

		// pt 값이 점유범위 안에 있다면 자식노드로 들어간다.
		if (PtInRect(&rc, pt))
			return m_pChild[2]->_FindNode(pHeightMap, _0, _1, _2, _3);

		// 4개의 코너값을 기준으로 자식노드의 맵 점유범위를 얻는다.
		SetRect(&rc,
			(int)pHeightMap[m_pChild[3]->m_nCorner[CORNER_TL]].p.x,
			(int)pHeightMap[m_pChild[3]->m_nCorner[CORNER_TL]].p.z,
			(int)pHeightMap[m_pChild[3]->m_nCorner[CORNER_BR]].p.x,
			(int)pHeightMap[m_pChild[3]->m_nCorner[CORNER_BR]].p.z);

		// pt 값이 점유범위 안에 있다면 자식노드로 들어간다.
		if (PtInRect(&rc, pt))
			return m_pChild[3]->_FindNode(pHeightMap, _0, _1, _2, _3);
	}

	return NULL;
}

int ZQuadTree::_GetNodeIndex(int ed, int cx, int & _0, int & _1, int & _2, int & _3)
{
	int n, _a, _b, _c, _d, gap;
	_a = _0;
	_b = _1;
	_c = _2;
	_d = _3;
	gap = _b - _a; // 현재 노드의 좌우폭값

	switch (ed)
	{
	case EDGE_UP: // 위쪽 방향 이웃노드의 인덱스
		_0 = _a - cx * gap;
		_1 = _b - cx * gap;
		_2 = _a;
		_3 = _b;
		break;
	case EDGE_DN: // 아래 방향 이웃노드의 인덱스
		_0 = _c;
		_1 = _d;
		_2 = _c + cx * gap;
		_3 = _d + cx * gap;
		break;
	case EDGE_LT: // 좌측 방향 이웃노드의 인덱스
		_0 = _a - gap;
		_1 = _a;
		_2 = _c - gap;
		_3 = _c;
		break;
	case EDGE_RT: // 우측 방향 이웃노드의 인덱스
		_0 = _b;
		_1 = _b + gap;
		_2 = _d;
		_3 = _d + gap;
		if ((_0 / cx) != (_a / cx)) return -1;
		if ((_1 / cx) != (_b / cx)) return -1;
		break;
	}

	n = (_0 + _1 + _2 + _3) / 4; // 가운데 인덱스
	if (!IS_IN_RANGE(n, 0, cx * cx - 1)) return -1;

	return n;
}

void ZQuadTree::_FrustumCull(TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum) {
	int ret;

	ret = _IsInFrustum(pHeightMap, pFrustum);

	switch (ret)
	{
	case FRUSTUM_COMPLETELY_IN:
		// 절두체에 완전히 포함됨. 하위노드 검색 필요 없음
		_AllInFrustum();
		return;
	case FRUSTUM_PARTIALLY_IN:
		// 절두체에 일부 포함됨. 하위노드 검색 필요함
		m_bCulled = false;
		break;
	case FRUSTUM_OUT:
		// 절두체에 완전히 벗어남. 하위노드 검색 필요 없음
		m_bCulled = true;
		return;
	}

	if (m_pChild[0]) m_pChild[0]->_FrustumCull(pHeightMap, pFrustum);
	if (m_pChild[1]) m_pChild[1]->_FrustumCull(pHeightMap, pFrustum);
	if (m_pChild[2]) m_pChild[2]->_FrustumCull(pHeightMap, pFrustum);
	if (m_pChild[3]) m_pChild[3]->_FrustumCull(pHeightMap, pFrustum);
}

bool ZQuadTree::Build(TERRAINVERTEX* pHeightMap) {
	// 쿼드트리 구축
	_BuildQuadTree(pHeightMap);

	// 이웃노드 구축
	_BuildNeighborNode(this, pHeightMap, m_nCorner[CORNER_TR] + 1);

	return true;
}

int ZQuadTree::GenerateIndex(LPVOID pIndex, 
	TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum, float fLODRatio) {
	_FrustumCull(pHeightMap, pFrustum);
	return _GenTriIndex(0, pIndex, pHeightMap, pFrustum, fLODRatio);
}