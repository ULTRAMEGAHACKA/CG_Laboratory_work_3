#include "camera.h"

const static float StepScale = 0.1f;
const static int MARGIN = 10;

Camera::Camera()
{
    m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_target = glm::vec3(0.0f, 0.0f, 1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
}


void Normalize(glm::vec3 vector)
{
    const float Length = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    vector.x /= Length;
    vector.y /= Length;
    vector.z /= Length;
}

glm::vec3 Cross(const glm::vec3& v1, glm::vec3 v2)
{
    const float _x = v2.y * v1.z - v2.z * v1.y;
    const float _y = v2.z * v1.x - v2.x * v1.z;
    const float _z = v2.x * v1.y - v2.y * v1.x;

    return glm::vec3(_x, _y, _z);
}

Camera::Camera(int WindowWidth, int WindowHeight)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_target = glm::vec3(0.0f, 0.0f, 1.0f);
    Normalize(m_target);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    Init();
}

Camera::Camera(int WindowWidth, int WindowHeight, const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;

    m_target = Target;
    Normalize(m_target);

    m_up = Up;
    Normalize(m_target);

    Init();
}

void Camera::Init()
{
    glm::vec3 HTarget(m_target.x, 0.0, m_target.z);
    Normalize(HTarget);

    if (HTarget.z >= 0.0f)
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = 360.0f - glm::degrees(asin(HTarget.z));
        }
        else
        {
            m_AngleH = 180.0f + glm::degrees(asin(HTarget.z));
        }
    }
    else
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = glm::degrees(asin(-HTarget.z));
        }
        else
        {
            m_AngleH = 90.0f + glm::degrees(asin(-HTarget.z));
        }
    }

    m_AngleV = -glm::degrees(asin(m_target.y));

    m_OnUpperEdge = false;
    m_OnLowerEdge = false;
    m_OnLeftEdge = false;
    m_OnRightEdge = false;
    m_mousePos.x = m_windowWidth / 2;
    m_mousePos.y = m_windowHeight / 2;

    glutWarpPointer(m_mousePos.x, m_mousePos.y);
}


bool Camera::OnKeyboard(int Key)
{
    bool Ret = false;

    switch (Key) {

    case GLUT_KEY_UP:
    {
        m_pos += (m_target * StepScale);
        Ret = true;
    }
    break;

    case GLUT_KEY_DOWN:
    {
        m_pos -= (m_target * StepScale);
        Ret = true;
    }
    break;

    case GLUT_KEY_LEFT:
    {
        glm::vec3 Left = Cross(m_up, m_target);
        Normalize(Left);
        Left *= StepScale;
        m_pos += Left;
        Ret = true;
    }
    break;

    case GLUT_KEY_RIGHT:
    {
        glm::vec3 Right = Cross(m_target, m_up);
        Normalize(Right);
        Right *= StepScale;
        m_pos += Right;
        Ret = true;
    }
    break;
    }

    return Ret;
}
void Camera::OnMouse(int x, int y)
{
    const int DeltaX = x - m_mousePos.x;
    const int DeltaY = y - m_mousePos.y;

    m_mousePos.x = x;
    m_mousePos.y = y;

    m_AngleH += (float)DeltaX / 20.0f;
    m_AngleV += (float)DeltaY / 20.0f;

    if (DeltaX == 0) {
        if (x <= MARGIN) {
            //    m_AngleH -= 1.0f;
            m_OnLeftEdge = true;
        }
        else if (x >= (m_windowWidth - MARGIN)) {
            //    m_AngleH += 1.0f;
            m_OnRightEdge = true;
        }
    }
    else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= MARGIN) {
            m_OnUpperEdge = true;
        }
        else if (y >= (m_windowHeight - MARGIN)) {
            m_OnLowerEdge = true;
        }
    }
    else {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    //Update();
}
//void Camera::Update()
//{
//    const glm::vec3 Vaxis(0.0f, 1.0f, 0.0f);
//
//    // Rotate the view vector by the horizontal angle around the vertical axis
//    glm::vec3 View(1.0f, 0.0f, 0.0f);
//
//    Rotate(m_AngleH, Vaxis, View);
//    Normalize(View);
//
//    // Rotate the view vector by the vertical angle around the horizontal axis
//    glm::vec3 Haxis = Cross(View, Vaxis);
//    Normalize(Haxis);
//    Rotate(m_AngleV, Haxis, View);
//
//    m_target = View;
//    Normalize(m_target);
//
//    m_up = Cross(Haxis,m_target);
//    Normalize(m_up);
//}
//glm::vec3 Rotate(float Angle, const glm::vec3& Axe, glm::vec3& some_vec)
//{
//    const float SinHalfAngle = sinf(glm::radians(Angle / 2));
//    const float CosHalfAngle = cosf(glm::radians(Angle / 2));
//
//    const float Rx = Axe.x * SinHalfAngle;
//    const float Ry = Axe.y * SinHalfAngle;
//    const float Rz = Axe.z * SinHalfAngle;
//    const float Rw = CosHalfAngle;
//    Quaternion RotationQ(Rx, Ry, Rz, Rw);
//
//    Quaternion ConjugateQ = RotationQ.Conjugate();
//    //  ConjugateQ.Normalize();
//    Quaternion W = RotationQ * some_vec * ConjugateQ;
//
//    some_vec.x = W.x;
//    some_vec.y = W.y;
//    some_vec.z = W.z;
//}