#pragma once

#include <iostream>

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "glad/glad.h"
#include "stb_image.h"

#define X glm::vec3(1.f,.0f,.0f)
#define Y glm::vec3(0.f,1.f,.0f)
#define Z glm::vec3(0.f,.0f,1.0f)
#define X1 glm::vec4(1.f,.0f,.0f,1.0f)
#define Y1 glm::vec4(0.f,1.f,.0f,1.0f)
#define Z1 glm::vec4(0.f,.0f,1.0f,1.0f)

using namespace std;

class pointMass {
    public:
        int id;
        glm::vec3 position;
        glm::vec3 speed;
        float mass;
        pointMass(int id,glm::vec3 position, glm::vec3 speed, float mass);
        void update(glm::vec3* dynamicPos);
};