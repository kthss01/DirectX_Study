#pragma once

class ZCamera {
private:
	D3DXVECTOR3 m_vEye; // 카메라의 현재 위치
	D3DXVECTOR3 m_vLookat; // 카메라의 시선 위치
	D3DXVECTOR3 m_vUp; // 카메라의 상방 벡터
	D3DXVECTOR3 m_vView; // 카메라가 향하는 단위방향 벡터
	D3DXVECTOR3 m_vCross; // 카메라의 측면벡터 cross(view, up)
	D3DXMATRIXA16 m_matView; // 카메라 행렬
	D3DXMATRIXA16 m_matBill; // 빌보드행렬(카메라의 역행렬)
public:
	ZCamera(); // 생성자

	// 카메라행렬을 얻어낸다.
	D3DXMATRIXA16* GetViewMatrix() { return &m_matView; }
	// 빌보드행렬을 얻어낸다.
	D3DXMATRIXA16* GetBillMatrix() { return &m_matBill; }
	// 카메라 행렬을 생성하기 위한 기본 벡터값들을 설정한다.
	D3DXMATRIXA16* SetView(D3DXVECTOR3* pvEye, 
		D3DXVECTOR3* pvLookat, D3DXVECTOR3* pvUp);

	// 카메라의 위치값을 설정한다.
	void SetEye(D3DXVECTOR3* pv) { m_vEye = *pv; }
	// 카메라의 위치값을 얻어낸다.
	D3DXVECTOR3* GetEye() { return &m_vEye; }
	// 카메라의 시선값을 설정한다.
	void SetLookat(D3DXVECTOR3* pv) { m_vLookat = *pv; }
	D3DXVECTOR3* GetLookat() { return &m_vLookat; }
	// 카메라의 상방벡터 값을 설정한다.
	void SetUp(D3DXVECTOR3* pv) { m_vUp = *pv; }
	// 카메라의 상박벡터 값을 얻어낸다.
	D3DXVECTOR3* GetUp() { return &m_vUp; }
	// 카메라가 향하는 단위방향 벡터값을 얻어낸다.
	D3DXVECTOR3* GetView() { return &m_vView; }
	// 값을 갱신한다.
	void Flush() { SetView(&m_vEye, &m_vLookat, &m_vUp); }

	// 카메라 좌표계의 x축으로 angle만큼 회전한다.
	D3DXMATRIXA16* RotateLocalX(float angle);
	// 카메라 좌표계의 y축으로 angle만큼 회전한다.
	D3DXMATRIXA16* RotateLocalY(float angle);
	//D3DXMATRIXA16* RotateLocalZ(float angle);
	// 월드 좌표게의 *pv값의 위치로 이동한다.
	D3DXMATRIXA16* MoveTo(D3DXVECTOR3* pv);
	// 카메라 좌표계의 x축 방향으로 dist만큼 전진한다. (후진은 -dist)
	D3DXMATRIXA16* MoveLocalX(float dist);
	// 카메라 좌표계의 y축 방향으로 dist만큼 전진한다. (후진은 -dist)
	D3DXMATRIXA16* MoveLocalY(float dist);
	// 카메라 좌표계의 z축 방향으로 dist만큼 전진한다. (후진은 -dist)
	D3DXMATRIXA16* MoveLocalZ(float dist);
};