#pragma once

class ZFrustum;

class ZQuadTree {
	// ����Ʈ���� �����Ǵ� 4���� �ڳʰ��� ���� �����
	enum CornerType { CORNER_TL, CORNER_TR, CORNER_BL, CORNER_BR };

	// ����Ʈ���� �������Ұ��� ����
	enum QuadLocation {
		FRUSTUM_OUT = 0, // �������ҿ��� ���� ���
		FRUSTUM_PARTIALLY_IN = 1, // �������ҿ� �κ�����
		FRUSTUM_COMPLETELY_IN = 2, // �������ҿ� ��������
		FRUSTUM_UNKNOWN = -1 // ��
	};
private:
	ZQuadTree * m_pChild[4]; // QuadTree�� 4���� �ڽĳ��

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

	// ���� ��尡 ����� ������ ����ΰ�?
	bool _IsVisible() { 
		return (m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL] <= 1); 
	}

	// ����� �������� �ε����� �����Ѵ�.
	int _GenTriIndex(int nTriangles, LPVOID pIndex);

	// �޸𸮿��� ����Ʈ���� �����Ѵ�.
	void _Destroy();

	// ���� ��尡 �������ҿ� ���ԵǴ°�?
	int _IsInFrustum(TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum);

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
	int GenerateIndex(LPVOID pIndex, TERRAINVERTEX* pHeightMap, ZFrustum* pFrustum);

};