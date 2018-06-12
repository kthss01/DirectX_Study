#pragma once

class ZFrustum;

class ZQuadTree {
	// ����Ʈ���� �����Ǵ� 4���� �ڳʰ��� ���� �����
	enum CornerType { CORNER_TL, CORNER_TR, CORNER_BL, CORNER_BR };

	// �̿���� ó���� �����
	enum { EDGE_UP, EDGE_DN, EDGE_LT, EDGE_RT };

	// ����Ʈ���� �������Ұ��� ����
	enum QuadLocation {
		FRUSTUM_OUT = 0, // �������ҿ��� ���� ���
		FRUSTUM_PARTIALLY_IN = 1, // �������ҿ� �κ�����
		FRUSTUM_COMPLETELY_IN = 2, // �������ҿ� ��������
		FRUSTUM_UNKNOWN = -1 // ��
	};
private:
	ZQuadTree * m_pChild[4]; // QuadTree�� 4���� �ڽĳ��

	ZQuadTree * m_pParent; // Triangle Crack(Popping)�� ���� ���� ���
	ZQuadTree * m_pNeighbor[4]; // Triangle Crack(Popping)�� ���� ���� ���

	int m_nCenter; // QuadTree�� ������ ù��° ��
	int m_nCorner[4]; // QuadTree�� ������ �ι�° ��

	bool m_bCulled; // �������ҿ��� �ø��� ����ΰ�?
	float m_fRadius; // ��带 ���δ� ��豸(bounding sphere)�� ������
private:
	// �ڽ� ��带 �߰��Ѵ�.
	ZQuadTree * _AddChild(
		int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// 4���� �ڳʰ��� �����Ѵ�.
	bool _SetCorners(
		int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// Quadtree�� 4���� ���� Ʈ���� �κк���(subdivide)�Ѵ�.
	bool _SubDivide();

	float _GetDistance(D3DXVECTOR3* pv1, D3DXVECTOR3* pv2) {
		return D3DXVec3Length(&(*pv2 - *pv1));
	}

	// ī�޶�� ���� ������ �Ÿ����� �������� LOD���� ���Ѵ�.
	int _GetLODLevel(TERRAINVERTEX* pHeightMap, D3DXVECTOR3* pCamera,
		float fLODRatio) {
		float d = _GetDistance((D3DXVECTOR3*)(pHeightMap + m_nCenter), pCamera);
		return max((int)(d * fLODRatio), 1);
	}

	// ���� ��尡 LOD������� �� �� ����� ������ ����ΰ�?
	bool _IsVisible(TERRAINVERTEX* pHeightMap, D3DXVECTOR3* pCamera,
		float fLODRatio) { 
		return ((m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL]) <=
			_GetLODLevel(pHeightMap, pCamera, fLODRatio));
	}

	// ����� �������� �ε����� �����Ѵ�.
	int _GenTriIndex(int nTriangles, LPVOID pIndex, 
		TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum, float fLODRatio);

	// �޸𸮿��� ����Ʈ���� �����Ѵ�.
	void _Destroy();

	// ��� �ڽĳ���� m_bCulled ���� false�� �Ѵ�.
	void _AllInFrustum();

	// ���� ��尡 �������ҿ� ���ԵǴ°�?
	int _IsInFrustum(TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum);

	// �̿���带 ����� (�ﰢ�� ������ ������)
	void _BuildNeighborNode(ZQuadTree* pRoot, TERRAINVERTEX*
		pHeightMap, int cx);

	// ����Ʈ���� �����(Build() �Լ����� �Ҹ���.)
	bool _BuildQuadTree(TERRAINVERTEX* pHeightMap);
	
	// ����Ʈ���� �˻��ؼ� 4�� �ڳʰ��� ��ġ�ϴ� ��带 ã�´�.
	ZQuadTree* _FindNode(TERRAINVERTEX* pHeightMap, int _0, int _1,
		int _2, int _3);

	// 4�� ����(���, �ϴ�, ����, ����)�� �̿���� �ε����� ���Ѵ�.
	int _GetNodeIndex(int ed, int cx, int& _0, int& _1, int& _2, int& _3);

	// _IsInFrustum() �Լ��� ����� ���� �������� �ø� ����
	void _FrustumCull(TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum);
public:
	// ���� ��Ʈ��� ������
	ZQuadTree(int cx, int cy);

	// ���� �ڽĳ�� ������
	ZQuadTree(ZQuadTree* pParent);

	~ZQuadTree();

	// QuadTree�� �����Ѵ�.
	bool Build(TERRAINVERTEX* pHeightMap);

	// �ﰢ���� �ε����� �����, ����� �ﰢ���� ������ ��ȯ�Ѵ�.
	int GenerateIndex(LPVOID pIndex, TERRAINVERTEX* pHeightMap, 
		ZFrustum* pFrustum, float fLODRatio);

};