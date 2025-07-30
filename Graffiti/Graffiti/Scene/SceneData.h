#pragma once

#include"Graffiti/base/core.h"


namespace Graffiti {
    struct PointLight {
        glm::vec4 position = glm::vec4(3.0f, 5.0f, 3.0f, 1.0f); // ���Դλ�ã�����ǰ��

        glm::vec4 ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f); // �����ԵĻ�����
        glm::vec4 diffuse = glm::vec4(1.8f, 1.65f, 1.5f, 1.0f); // ������ůɫ��
        glm::vec4 specular = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f); // �߹�������ʺϽ���/����


    };
    // ƽ�йⶨ��
    struct DirectionalLight {
        glm::vec3 direction;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };
	struct SceneData
	{
		glm::mat4 ViewProjectionMatrix;
	};

    struct PushconstData{
        glm::mat4 transform = glm::mat4(1.0f);
        int  type;
        int letsize = 0;
    };

   
}