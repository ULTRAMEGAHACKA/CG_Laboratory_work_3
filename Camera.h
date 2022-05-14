#pragma once

#include<GL/glew.h>
#include<GL/freeglut.h>
#include<glm/vec3.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>
using namespace std;

#ifndef CAMERA_H
#define	CAMERA_H

class Camera
{
public:

    Camera();

    Camera(int WindowWidth, int WindowHeight);

    Camera(int WindowWidth, int WindowHeight, const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up);

    bool OnKeyboard(int Key);

    void OnMouse(int x, int y);

    void OnRender();

    const glm::vec3& GetPos() const
    {
        return m_pos;
    }

    const glm::vec3& GetTarget() const
    {
        return m_target;
    }

    const glm::vec3& GetUp() const
    {
        return m_up;
    }

private:
    //Camera(int WindowWidth, int WindowHeight);
    void Init();
    //void Update();

    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;

    glm::vec2 m_mousePos;

    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;

};

#endif	/* CAMERA_H */

