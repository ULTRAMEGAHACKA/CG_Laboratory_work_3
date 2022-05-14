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
    Pipeline()
    {
        m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
        m_worldPos = glm::vec3(0.0f, 0.0f, 0.0f);
        m_rotateInfo = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    void Scale(float ScaleX, float ScaleY, float ScaleZ)
    {
        m_scale.x = ScaleX;
        m_scale.y = ScaleY;
        m_scale.z = ScaleZ;
    }

    void WorldPos(float x, float y, float z)
    {
        m_worldPos.x = x;
        m_worldPos.y = y;
        m_worldPos.z = z;
    }

    void Rotate(float RotateX, float RotateY, float RotateZ)
    {
        m_rotateInfo.x = RotateX;
        m_rotateInfo.y = RotateY;
        m_rotateInfo.z = RotateZ;
    }
    void SetPerspectiveProj(float FOV, float Width, float Height, float zNear, float zFar)
    {
        m_persProj.FOV = FOV;
        m_persProj.Width = Width;
        m_persProj.Height = Height;
        m_persProj.zNear = zNear;
        m_persProj.zFar = zFar;
    }
    void Normalize(glm::vec3 vector)
    {
        const float Length = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
        vector.x /= Length;
        vector.y /= Length;
        vector.z /= Length;
    }
    glm::vec3 Cross(const glm::vec3& v1, glm::vec3 v2) const
    {
        const float _x = v2.y * v1.z - v2.z * v1.y;
        const float _y = v2.z * v1.x - v2.x * v1.z;
        const float _z = v2.x * v1.y - v2.y * v1.x;

        return glm::vec3(_x, _y, _z);
    }
    const glm::mat4* GetTrans()
    {
        //=========================================
        glm::mat4 ScaleTrans(
            sinf(m_scale.x), 0.0f, 0.0f, 0.0f,
            0.0f, sinf(m_scale.y), 0.0f, 0.0f,
            0.0f, 0.0f, sinf(m_scale.z), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f); // размер
        //=========================================
        glm::mat4 TranslationTrans(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            m_worldPos.x, m_worldPos.y, m_worldPos.z, 1.0f); // положение
        //=========================================
        const float x = glm::radians(m_rotateInfo.x);
        const float y = glm::radians(m_rotateInfo.y);
        const float z = glm::radians(m_rotateInfo.z);

        glm::mat4x4 transformMatrixZ(
            cosf(z), -sinf(z), 0.0f, 0.0f,
            sinf(z), cosf(z), 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f); //вокруг Z
        glm::mat4x4 transformMatrixY(
            cosf(y), 0.0f, -sinf(x), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            sinf(x), 0.0f, cosf(y), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f); //вокруг Y
        glm::mat4x4 transformMatrixX(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cosf(x), -sinf(x), 0.0f,
            0.0f, sinf(x), cosf(x), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f); // вокруг X
        glm::mat4x4 RotateTrans = transformMatrixZ * transformMatrixY * transformMatrixX;
        //=========================================

        /*const float ar = m_persProj.Width / m_persProj.Height;
        const float zNear = m_persProj.zNear;
        const float zFar = m_persProj.zFar;
        const float zRange = zNear - zFar;
        const float tanHalfFOV = tanf(glm::radians(m_persProj.FOV / 2.0));
        glm::mat4 PersProjTrans(
            1.0f / (tanHalfFOV * ar), cosf(x), sinf(x), 0.0f,
            0.0f, 1.0f / tanHalfFOV, cosf(x), 0.0f,
            0.0f, 0.0f, (-zNear - zFar) / zRange, 1.0f,
            0.0f, 0.0f, 2.0f * zFar * zNear / zRange, 0.0f);
       
        glm::vec3 N = m_camera.Target;
        Normalize(N);
        glm::vec3 U = m_camera.Up;
        Normalize(U);
        U = Cross(m_camera.Target, U);
        glm::vec3 V = Cross(U, N);

        glm::mat4 CameraRotateTrans(
            U.x, V.x, N.x, 0.0f,
            U.y, V.y, N.y, 0.0f,
            U.z, V.z, N.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
        
        glm::mat4 CameraTranslationTrans(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z, 1.0f);*/
        

        m_transformation = TranslationTrans * RotateTrans * ScaleTrans;
        return &m_transformation;
    }
    void SetCamera(const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up)
    {
        m_camera.Pos = Pos;
        m_camera.Target = Target;
        m_camera.Up = Up;
    }
    const glm::mat4x4* GetTransWorld()
    {
        //===================
        GetTrans();
        const float ar = m_persProj.Width / m_persProj.Height;
        const float zNear = m_persProj.zNear;
        const float zFar = m_persProj.zFar;
        const float zRange = zNear - zFar;
        const float tanHalfFOV = tanf(glm::radians(m_persProj.FOV / 2.0));
        glm::mat4x4 ProjectionMatrix(
            1.0f / (tanHalfFOV * ar), 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / tanHalfFOV, 0.0f, 0.0f,
            0.0f, 0.0f, (-zNear - zFar) / zRange, 2.0f * zFar * zNear / zRange,
            0.0f, 0.0f, 1.0f, 0.0f);
        //====================
        glm::vec3 N = m_camera.Target; // m_camera.Target, m_camera.Up
        Normalize(N);
        glm::vec3 U = m_camera.Up;
        Normalize(U);
        U = Cross(m_camera.Target, U);// N вместо таргета
        glm::vec3 V = Cross(U, N);

        glm::mat4 CameraRotateTrans(
            U.x, V.x, N.x, 0.0f,
            U.y, V.y, N.y, 0.0f,
            U.z, V.z, N.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
        //====================
        glm::mat4 CameraTranslationTrans(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z, 1.0f);
        //====================

        worldM = m_transformation * CameraTranslationTrans * CameraRotateTrans * ProjectionMatrix;

        return &worldM;
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
    glm::vec3 m_scale;
    glm::vec3 m_worldPos;
    glm::vec3 m_rotateInfo;
    glm::mat4x4 m_transformation = InitEdm();
    glm::mat4x4 worldM = InitEdm();
    struct {
        float FOV;
        float Width;
        float Height;
        float zNear;
        float zFar;
    } m_persProj;
    struct {
        glm::vec3 Pos;
        glm::vec3 Target;
        glm::vec3 Up;
    } m_camera;
};

#endif	/* PIPELINE_H */
