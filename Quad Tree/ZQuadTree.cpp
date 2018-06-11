#include "stdafx.h"
#include "ZQuadTree.h"

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
}

ZQuadTree::ZQuadTree(ZQuadTree* pParent) {
	int i;
	m_nCenter = 0;
	for (i = 0; i < 4; i++) {
		m_pChild[i] = NULL;
		m_nCorner[i] = 0;
	}
}

ZQuadTree::~ZQuadTree() {
	_Destroy();
}

void ZQuadTree::_Destroy() {
	for (int i = 0; i < 4; i++) DEL(m_pChild[i]);
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

int ZQuadTree::_GenTriIndex(int nTris, LPVOID pIndex) {
	if (_IsVisible()) {
#ifdef _USE_INDEX16
		LPWORD p = ((LPWORD)pIndex) + nTris * 3;
#else
		LPDWORD p = ((LPWORD)pIndex) + nTris * 3;
#endif
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

	if (m_pChild[CORNER_TL]) nTris = m_pChild[CORNER_TL]->_GenTriIndex(nTris, pIndex);
	if (m_pChild[CORNER_TR]) nTris = m_pChild[CORNER_TR]->_GenTriIndex(nTris, pIndex);
	if (m_pChild[CORNER_BL]) nTris = m_pChild[CORNER_BL]->_GenTriIndex(nTris, pIndex);
	if (m_pChild[CORNER_BR]) nTris = m_pChild[CORNER_BR]->_GenTriIndex(nTris, pIndex);

	return nTris;
}

bool ZQuadTree::Build() {
	if (_SubDivide()) {
		m_pChild[CORNER_TL]->Build();
		m_pChild[CORNER_TR]->Build();
		m_pChild[CORNER_BL]->Build();
		m_pChild[CORNER_BR]->Build();
	}

	return true;
}

int ZQuadTree::GenerateIndex(LPVOID pIndex) {
	return _GenTriIndex(0, pIndex);
}