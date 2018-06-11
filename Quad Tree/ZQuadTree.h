#pragma once

class ZQuadTree {
	// 쿼드트리에 보관되는 4개의 코너값에 대한 상수값
	enum CornerType { CORNER_TL, CORNER_TR, CORNER_BL, CORNER_BR };

private:
	ZQuadTree * m_pChild[4]; // QuadTree의 4개의 자식노드

	int m_nCenter; // QuadTree에 보관할 첫번째 값
	int m_nCorner[4]; // QuadTree에 보관할 두번째 값

private:
	// 자식 노드를 추가한다.
	ZQuadTree * _AddChild(
		int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// 4개의 코너값을 세팅한다.
	bool _SetCorners(
		int nCornerTL, int nCornerTR, int nCornerBL, int nCornerBR);

	// Quadtree를 4ㅐ의 하위 트리로 부분분할(subdivide)한다.
	bool _SubDivide();

	// 현재 노드가 출력이 가능한 노드인가?
	bool _IsVisible() { 
		return (m_nCorner[CORNER_TR] - m_nCorner[CORNER_TL] <= 1); 
	}

	// 출력할 폴리곤의 인덱스를 생성한다.
	int _GenTriIndex(int nTriangles, LPVOID pIndex);

	// 메모리에서 쿼드트리르 삭제한다.
	void _Destroy();
public:
	// 최초 루트노드 생성자
	ZQuadTree(int cx, int cy);

	// 하위 자식노드 생성자
	ZQuadTree(ZQuadTree* pParent);

	~ZQuadTree();

	// QuadTree를 구축한다.
	bool Build();

	// 삼각형의 인덱스를 만들고, 출력할 삼각형의 개수를 반환한다.
	int GenerateIndex(LPVOID pIB);

};