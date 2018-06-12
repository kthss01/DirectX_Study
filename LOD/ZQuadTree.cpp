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

	// ��ܺ� ���
	nTopEdgeCenter = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR]) / 2;
	// �ϴܺ� ���
	nBottomEdgeCenter = (m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR]) / 2;
	// ������ ���
	nLeftEdgeCenter = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_BL]) / 2;
	// ������ ���
	nRightEdgeCenter = (m_nCorner[CORNER_TR] + m_nCorner[CORNER_BR]) / 2;
	// �Ѱ��
	nCentralPoint = (m_nCorner[CORNER_TL] + m_nCorner[CORNER_TR] +
		m_nCorner[CORNER_BL] + m_nCorner[CORNER_BR]) / 4;

	// �� �̻� ������ �Ұ����Ѱ�? �׷��ٸ� SubDivide() ����
	if ((m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL]) <= 1)
		return false;

	// 4���� �ڽĳ�� �߰�
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
	// �ø��� ����� �׳� �����Ѵ�.
	if (m_bCulled) {
		m_bCulled = false;
		return nTris;
	}

	if (_IsVisible(pHeightMap, pFrustum->GetPos(), fLODRatio)) {
#ifdef _USE_INDEX16
		LPWORD p = ((LPWORD)pIndex) + nTris * 3;
#else
		LPDWORD p = ((LPWORD)pIndex) + nTris * 3;
#endif
		// ������� �ﰢ��
		*p++ = m_nCorner[0];
		*p++ = m_nCorner[1];
		*p++ = m_nCorner[2];
		nTris++;

		// �����ϴ� �ﰢ��
		*p++ = m_nCorner[2];
		*p++ = m_nCorner[1];
		*p++ = m_nCorner[3];
		nTris++;

		return nTris;
	}

	// �ڽ� ���� �˻�
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

	// ��豸 �ȿ� �ִ°�?
	//if (m_fRadius == 0.0f) g_pLog->Log(
	//	(char*)"Index : [%d], Radius : [%f]", m_nCenter, m_fRadius);
	bInSphere = pFrustum->IsInSphere(
		(D3DXVECTOR3*)(pHeightMap + m_nCenter), m_fRadius);
	// ��豸 �ȿ� ������ �� ������ ����ü �׽�Ʈ ����
	if (!bInSphere) return FRUSTUM_OUT; 

	// ����Ʈ���� 4���� ��� ����ü �׽�Ʈ
	b[0] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[0]));
	b[1] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[1]));
	b[2] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[2]));
	b[3] = pFrustum->IsIn((D3DXVECTOR3*)(pHeightMap + m_nCorner[3]));

	// 4�� ��� ����ü �ȿ� ����
	if ((b[0] + b[1] + b[2] + b[3]) == 4) return FRUSTUM_COMPLETELY_IN;

	// �Ϻκ��� ����ü�� �ִ� ���
	return FRUSTUM_PARTIALLY_IN;
}

void ZQuadTree::_FrustumCull(TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum) {
	int ret;

	ret = _IsInFrustum(pHeightMap, pFrustum);

	switch (ret)
	{
	case FRUSTUM_COMPLETELY_IN:
		// ����ü�� ������ ���Ե�. ������� �˻� �ʿ� ����
		m_bCulled = false;
		return;
	case FRUSTUM_PARTIALLY_IN:
		// ����ü�� �Ϻ� ���Ե�. ������� �˻� �ʿ���
		m_bCulled = false;
		break;
	case FRUSTUM_OUT:
		// ����ü�� ������ ���. ������� �˻� �ʿ� ����
		m_bCulled = true;
		return;
	}

	if (m_pChild[0]) m_pChild[0]->_FrustumCull(pHeightMap, pFrustum);
	if (m_pChild[1]) m_pChild[1]->_FrustumCull(pHeightMap, pFrustum);
	if (m_pChild[2]) m_pChild[2]->_FrustumCull(pHeightMap, pFrustum);
	if (m_pChild[3]) m_pChild[3]->_FrustumCull(pHeightMap, pFrustum);
}

bool ZQuadTree::Build(TERRAINVERTEX* pHeightMap) {
	if (_SubDivide()) {
		// ���� ��ܰ� ���� �ϴ��� �Ÿ��� ���Ѵ�.
		D3DXVECTOR3 v =
			*((D3DXVECTOR3*)(pHeightMap + m_nCorner[CORNER_TL]))
			- *((D3DXVECTOR3*)(pHeightMap + m_nCorner[CORNER_BR]));

		// v�� �Ÿ����� �� ��带 ���δ� ��豸�� �����̹Ƿ�,
		// 2�� ������ �������� ���Ѵ�.
		m_fRadius = D3DXVec3Length(&v) / 2.0f;

		m_pChild[CORNER_TL]->Build(pHeightMap);
		m_pChild[CORNER_TR]->Build(pHeightMap);
		m_pChild[CORNER_BL]->Build(pHeightMap);
		m_pChild[CORNER_BR]->Build(pHeightMap);
	}

	return true;
}

int ZQuadTree::GenerateIndex(LPVOID pIndex, 
	TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum, float fLODRatio) {
	_FrustumCull(pHeightMap, pFrustum);
	return _GenTriIndex(0, pIndex, pHeightMap, pFrustum, fLODRatio);
}