#pragma once
#include"glm//glm.hpp"
#include"Graffiti/Scene/Model.h"
namespace Graffiti {

	enum class CameraMoveDir
	{
		FOWARD,
		BACKWARD,
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	class Camera
	{
	public:

		Camera(glm::vec3 _position=glm::vec3(6,6,6), glm::vec3 _target = glm::vec3(0, 0, 0), glm::vec3 _upVector=glm::vec3(0,1,0), float fov = 45.0, float aspect = 15.0 / 13.0, float znear = 0.1, float zfar = 1000000.0);
		~Camera() {};
		void UpdateProjectionMatrix(float fov, float aspect, float znear, float zfar);

		void ResetViewMatrix(glm::vec3 _position, glm::vec3 _target, glm::vec3 _upVector);

        //���ݰ�Χ�м���۲�Ŀ��
        void SetTarget(BoundingBox& b);
		// ��������
		void ProcessKeyboard(CameraMoveDir direction, float deltaTime);
		void ProcessMouseMovement(float xoffset, float yoffset);


		
		glm::mat4 GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		glm::vec3 GetCameraPosition() const { return m_Position; }
	
	private:
        void UpdateViewMatrix();

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Target;
		glm::vec3 m_Up{ 0.0f, 1.0f, 0.0f };
		glm::vec3 m_Right{ 1.0f, 0.0f, 0.0f };
		glm::vec3 m_Position = { 0,0,0 };

        float m_Distance;         // �����Ŀ��ľ���
        float m_Azimuth;          // ��λ�ǣ�ˮƽ��ת��
        float m_Elevation;        // ���ǣ���ֱ��ת��
        float m_Sensitivity=0.3;      // ���������


		float m_MovementSpeed = 3.0f;

	};

}