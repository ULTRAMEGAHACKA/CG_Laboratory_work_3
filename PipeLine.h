#pragma once
#ifndef PIPELINE_H
#define	PIPELINE_H
#include<GL/glew.h>
#include<GL/freeglut.h>
#include<glm/vec3.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <iostream>
using namespace std;

class Pipeline
{
public:
    Pipeline() {
        m_scale = { 1.0f, 1.0f, 1.0f };
        m_trans = { 0.0f, 0.0f, 0.0f };
        m_rot = { 0.0f, 0.0f, 0.0f };
    }

    void Scale(float ScaleX, float ScaleY, float ScaleZ)
    {
        m_scale = { ScaleX, ScaleY, ScaleZ };
    }

    void WorldPos(float x, float y, float z)
    {
        m_trans = { x, y, z };
    }

    void Rotate(float RotateX, float RotateY, float RotateZ)
    {
        m_rot = { RotateX, RotateY, RotateZ };
    }
    void SetPerspectiveProj(float FOV, float Width, float Height, float zNear, float zFar)
    {
        m_persProj.FOV = FOV;
        m_persProj.Width = Width;
        m_persProj.Height = Height;
        m_persProj.zNear = zNear;
        m_persProj.zFar = zFar;
    }
    
    void SetCamera(glm::vec3 pos, glm::vec3 target, glm::vec3 up) {
        m_camera.Pos = pos;
        m_camera.Target = target;
        m_camera.Up = up;
    }
    const glm::mat4* GetTrans()
    {
        InitScaleTransform();
        InitRotateTransform();
        InitTranslationMatrix();

        WVP = ScaleMat * RotateMat * TransMat;
        return &WVP;
    }
    const glm::mat4x4* GetTransWorld()
    {
        GetTrans();
        InitPerspective();
        InitCamera();
        InitCamTrans();

        World = WVP * CamTrans * Cam * Proj;
        return &World;
    }
    glm::mat4x4 InitEdm()
    {
        glm::mat4x4 edm(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
        return edm;
    }

private:
    glm::vec3 m_worldPos;
    glm::vec3 m_rotateInfo;
    glm::mat4x4 m_transformation = InitEdm();
    glm::mat4x4 worldM = InitEdm();
    struct persProj {
        float FOV;
        float Width;
        float Height;
        float zNear;
        float zFar;
    };
    struct camera {
        glm::vec3 Pos;
        glm::vec3 Target;
        glm::vec3 Up;
    };
    glm::mat4 matrix = {
        matrix[0][0] = 1.0f, matrix[0][1] = 0.0f, matrix[0][2] = 0.0f, matrix[0][3] = 0.0f,
        matrix[1][0] = 0.0f, matrix[1][1] = 1.0f, matrix[1][2] = 0.0f, matrix[1][3] = 0.0f,
        matrix[2][0] = 0.0f, matrix[2][1] = 0.0f, matrix[2][2] = 1.0f, matrix[2][3] = 0.0f,
        matrix[3][0] = 0.0f, matrix[3][1] = 0.0f, matrix[3][2] = 0.0f, matrix[3][3] = 1.0f,
    };
    glm::mat4 ScaleMat = matrix, RotateMat = matrix, TransMat = matrix, Proj = matrix, Cam = matrix, CamTrans = matrix;
    glm::vec3 m_scale, m_trans, m_rot;
    persProj m_persProj;
    camera m_camera;
    glm::mat4 WVP = matrix;
    glm::mat4 World = matrix;

    glm::vec3 cross(glm::vec3 v1, glm::vec3 v2) {
        float x = v1.y * v2.z - v1.z * v2.y;
        float y = v1.z * v2.x - v1.x * v2.z;
        float z = v1.x * v2.y - v1.y * v2.x;
        return glm::vec3(x, y, z);
    }

    void normalize(glm::vec3& v) {
        float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }

    void InitScaleTransform() {
        ScaleMat = matrix;
        ScaleMat[0][0] = m_scale.x;
        ScaleMat[1][1] = m_scale.y;
        ScaleMat[2][2] = m_scale.z;
    };

    void InitRotateTransform() {
        glm::mat4 rx, ry, rz;
        rx = matrix;
        ry = matrix;
        rz = matrix;
        const float x = glm::radians(m_rot.x);
        const float y = glm::radians(m_rot.y);
        const float z = glm::radians(m_rot.z);

        rx[1][1] = cosf(x); rx[1][2] = -sinf(x);
        rx[2][1] = sinf(x); rx[2][2] = cosf(x);

        ry[0][0] = cosf(y); ry[0][2] = -sinf(y);
        ry[2][0] = sinf(y); ry[2][2] = cosf(y);

        rz[0][0] = cosf(z); rz[0][1] = -sinf(z);
        rz[1][0] = sinf(z); rz[1][1] = cosf(z);

        RotateMat = rz * ry * rx;
    };

    void InitTranslationMatrix() {
        TransMat = matrix;
        TransMat[0][3] = m_trans.x;
        TransMat[1][3] = m_trans.y;
        TransMat[2][3] = m_trans.z;
    };

    void InitPerspective() {
        float ar = m_persProj.Width / m_persProj.Height;
        float zNear = m_persProj.zNear;
        float zFar = m_persProj.zFar;
        float zRange = zNear - zFar;
        float tanHalfFOV = tanf(glm::radians(m_persProj.FOV / 2.0));

        Proj = matrix;
        Proj[0][0] = 1 / (tanHalfFOV * ar);
        Proj[1][1] = 1 / tanHalfFOV;
        Proj[2][2] = (-zNear - zFar) / zRange;
        Proj[2][3] = 2. * zFar * zNear / zRange;
        Proj[3][2] = 1.0f;
        Proj[3][3] = 0.0f;
    };

    void InitCamera() {
        glm::vec3 n = m_camera.Target;
        glm::vec3 u = m_camera.Up;
        normalize(n);
        normalize(u);
        u = cross(u, m_camera.Target);
        glm::vec3 v = cross(n, u);
        Cam = matrix;
        Cam[0][0] = u.x; Cam[0][1] = u.y; Cam[0][2] = u.z;
        Cam[1][0] = v.x; Cam[1][1] = v.y; Cam[1][2] = v.z;
        Cam[2][0] = n.x; Cam[2][1] = n.y; Cam[2][2] = n.z;
    }

    void InitCamTrans() {
        CamTrans = matrix;
        CamTrans[0][3] = -m_camera.Pos.x;
        CamTrans[1][3] = -m_camera.Pos.y;
        CamTrans[2][3] = -m_camera.Pos.z;
    }
};

#endif	/* PIPELINE_H */
