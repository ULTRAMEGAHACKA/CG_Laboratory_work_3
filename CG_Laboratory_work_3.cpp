#include "PipeLine.h"
#include "camera.h"
#include "Texture.h"
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <Magick++.h>
#include <Magick++/Image.h>


using namespace std;
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
//Texture* pTexture = NULL;
GLuint VBO;
GLuint IBO;
GLuint gWorldLocation;
GLuint gSampler;
GLint success;

static float scale = 0.0;

static const char* pVS = "                                                      \n\
     #version 330                                                                   \n\
    layout (location = 0) in vec3 pos;                                             \n\
    layout (location = 1) in vec2 tex;                                             \n\
    layout (location = 2) in vec3 norm;                                            \n\
    uniform mat4 gWVP;                                                             \n\
    uniform mat4 gWorld;                                                           \n\
	out vec3 pos0;																	\n\
    out vec2 tex0;                                                                 \n\
    out vec3 norm0;                                                                \n\
    void main()                                                                    \n\
    {                                                                              \n\
        gl_Position = gWVP * vec4(pos, 1.0);                                       \n\
        tex0 = tex;                                                                \n\
        norm0 = (gWorld * vec4(norm, 0.0)).xyz;                                    \n\
		pos0 = (gWorld * vec4(pos, 1.0)).xyz;										\n\
    }";

static const char* pFS = "                                                         \n\
    #version 410                                                                    \n\
    in vec2 tex0;                                                                   \n\
	in vec3 norm0;																	\n\
	in vec3 pos0;																	\n\
    struct DirectionalLight                                                         \n\
    {                                                                               \n\
        vec3 Color;                                                                 \n\
        float AmbientIntensity;                                                     \n\
		vec3 Direction;                                                             \n\
        float DiffuseIntensity;                                                     \n\
    };                                                                              \n\
    uniform sampler2D gSampler;                                                     \n\
    uniform DirectionalLight gDirectionalLight;                                     \n\
	uniform vec3 gEyeWorldPos;                                                      \n\
    uniform float gMatSpecularIntensity;                                            \n\
    uniform float gSpecularPower;                                                   \n\
    out vec4 fragcolor;                                                             \n\
    void main()                                                                     \n\
    {                                                                               \n\
        vec4 AmbientColor = vec4(gDirectionalLight.Color, 1.0f) *                   \n\
                        gDirectionalLight.AmbientIntensity;                         \n\
                                                                                    \n\
                                                                                    \n\
                                                                                    \n\
		vec3 lightdir = -gDirectionalLight.Direction;                               \n\
        vec3 normal = normalize(norm0);                                             \n\
                                                                                    \n\
        float DiffuseFactor = dot(normal,lightdir);                                 \n\
                                                                                    \n\
        vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                      \n\
        vec4 SpecularColor = vec4(0, 0, 0, 0);                                      \n\
        if (DiffuseFactor > 0){                                                     \n\
            DiffuseColor = vec4(gDirectionalLight.Color, 1.0f) *                    \n\
			 gDirectionalLight.DiffuseIntensity *									\n\
                       DiffuseFactor;												\n\
        vec3 VertexToEye = normalize(gEyeWorldPos - pos0);                          \n\
        vec3 LightReflect = normalize(reflect(gDirectionalLight.Direction, normal));\n\
        float SpecularFactor = dot(VertexToEye, LightReflect);                      \n\
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                       \n\
                                                                                    \n\
        if (SpecularFactor > 0){                                                    \n\
            SpecularColor = vec4(gDirectionalLight.Color, 1.0f) *                   \n\
                            gMatSpecularIntensity *                                 \n\
                            SpecularFactor;                                         \n\
            }                                                                       \n\
        }                                                                           \n\
        fragcolor = texture2D(gSampler, tex0.xy) *                                  \n\
                    (AmbientColor + DiffuseColor + SpecularColor);                  \n\
    }";

struct vertex {
	glm::vec3 m_pos;
	glm::vec2 m_tex;
	glm::vec3 m_norm;
	vertex(glm::vec3 inp1, glm::vec2 inp2) {
		m_pos = inp1;
		m_tex = inp2;
		m_norm = glm::vec3(0.0f, 0.0f, 0.0f);
	}
};
struct DirectionLight
{
	glm::vec3 Color;
	float AmbientIntensity;
	glm::vec3 Direction;
	float DiffuseIntensity;
};
Camera GameCamera;

static void SpecialKeyboardCB(int Key, int x, int y)
{
	GameCamera.OnKeyboard(Key);
}


class Technique
{
public:

	Technique() { m_shaderProg = 0; };

	~Technique()
	{
		for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++) {
			glDeleteShader(*it);
		}
		if (m_shaderProg != 0) {
			glDeleteProgram(m_shaderProg);
			m_shaderProg = 0;
		}
	}
	virtual bool Init()
	{
		m_shaderProg = glCreateProgram();

		if (m_shaderProg == 0) {
			GLchar InfoLog[1024];
			cerr << "Error creating shader program: " << InfoLog << endl;
			return false;
		}
		return true;
	}
	void Enable() { glUseProgram(m_shaderProg); };

protected:

	bool AddShader(GLenum ShaderType, const char* pShaderText)
	{
		GLuint ShaderObj = glCreateShader(ShaderType);

		if (ShaderObj == 0) {
			fprintf(stderr, "Error creating shader type %d\n", ShaderType);
			exit(0);
		}

		const GLchar* p[1];
		p[0] = pShaderText;

		glShaderSource(ShaderObj, 1, p, NULL);
		glCompileShader(ShaderObj);


		glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar InfoLog[1024];
			glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
			fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
			exit(1);
		}

		glAttachShader(m_shaderProg, ShaderObj);
		return true;
	}


	bool Finalize()
	{
		GLint success;

		glLinkProgram(m_shaderProg);

		glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &success);
		if (success == 0) {
			GLchar ErrorLog[1024];
			glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
			fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
			return false;
		}
		for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++) {
			glDeleteShader(*it);
		}
		m_shaderObjList.clear();

		return true;
	}

	GLint GetUniformLocation(const char* pUniformName)
	{
		GLint Location = glGetUniformLocation(m_shaderProg, pUniformName);

		if (Location == 0xFFFFFFFF) {
			cerr << "Warning! Unable to get the location of uniform " << pUniformName << endl;
		}
		return Location;
	};

private:

	GLuint m_shaderProg;
	typedef list<GLuint> ShaderObjList;
	ShaderObjList m_shaderObjList;
};

class LightingTechnique : public Technique {
public:
	LightingTechnique() {};

	virtual bool Init() {
		if (!Technique::Init()) return false;
		if (!AddShader(GL_VERTEX_SHADER, pVS)) return false;
		if (!AddShader(GL_FRAGMENT_SHADER, pFS)) return false;
		if (!Finalize())  return false;

		gWVPLocation = GetUniformLocation("gWVP");
		gWorldLocation = GetUniformLocation("gWorld");
		samplerLocation = GetUniformLocation("gSampler");
		LightColor = GetUniformLocation("gDirectionalLight.Color");
		LightAmbientIntensity = GetUniformLocation("gDirectionalLight.AmbientIntensity");
		LightDirection = GetUniformLocation("gDirectionalLight.Direction");
		LightDiffuseIntensity = GetUniformLocation("gDirectionalLight.DiffuseIntensity");
		eyeWorldPosition = GetUniformLocation("gEyeWorldPos");
		matSpecularIntensityLocation = GetUniformLocation("gMatSpecularIntensity");
		matSpecularPowerLocation = GetUniformLocation("gSpecularPower");
		if (LightAmbientIntensity == 0xFFFFFFFF ||
			gWorldLocation == 0xFFFFFFFF ||
			samplerLocation == 0xFFFFFFFF ||
			LightColor == 0xFFFFFFFF ||
			LightDirection == 0xFFFFFFFF ||
			LightDiffuseIntensity == 0xFFFFFFFF ||
			eyeWorldPosition == 0xFFFFFFFF ||
			matSpecularIntensityLocation == 0xFFFFFFFF ||
			matSpecularPowerLocation == 0xFFFFFFFF)  return false;

		return true;
	};

	void SetgWVP(const glm::mat4* gWorld) {
		glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)gWorld);
	};
	void SetWorld(const glm::mat4* World)
	{
		glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)World);
	}
	void SetTextureUnit(unsigned int unit) {
		glUniform1i(samplerLocation, unit);
	};
	void SetMatSpecularIntensity(float Intensity)
	{
		glUniform1f(matSpecularIntensityLocation, Intensity);
	}
	void SetMatSpecularPower(float Power)
	{
		glUniform1f(matSpecularPowerLocation, Power);
	}
	void SetEyeWorldPos(const glm::vec3& EyeWorldPos)
	{
		glUniform3f(eyeWorldPosition, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
	}
	void SetDirectionalLight(const DirectionLight& Light) {
		glUniform3f(LightColor, Light.Color.x, Light.Color.y, Light.Color.z);
		glUniform1f(LightAmbientIntensity, Light.AmbientIntensity);
		glm::vec3 Direction = Light.Direction;
		normalize(Direction);
		glUniform3f(LightDirection, Direction.x, Direction.y, Direction.z);
		glUniform1f(LightDiffuseIntensity, Light.DiffuseIntensity);
	};
private:
	GLuint gWVPLocation;
	GLuint gWorldLocation;
	GLuint samplerLocation;
	GLuint LightColor;
	GLuint LightAmbientIntensity;
	GLuint LightDirection;
	GLuint LightDiffuseIntensity;
	GLuint eyeWorldPosition;
	GLuint matSpecularIntensityLocation;
	GLuint matSpecularPowerLocation;
};

class ICallbacks
{
public:
	virtual void KeyboardCB(unsigned char Key, int x, int y) = 0;
	virtual void RenderSceneCB() = 0;
	virtual void IdleCB() = 0;
};

void GLUTBackendInit(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	Magick::InitializeMagick(*argv);
};

bool GLUTBackendCreateWindow(unsigned int Width, unsigned int Height, const char* name) {
	glutInitWindowSize(Width, Height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(name);
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		cerr << "Error: " << glewGetErrorString(res) << endl;
		return false;
	}
	return true;
};
ICallbacks* ICall = NULL;
void RenderScene() {
	ICall->RenderSceneCB();
}
void Idle() {
	ICall->IdleCB();
}
void Keyboard(unsigned char Key, int x, int y) {
	ICall->KeyboardCB(Key, x, y);
}
void CB() {
	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(Keyboard);
}
void GLUTBackendRun(ICallbacks* p) {
	if (!p) {
		fprintf(stderr, "%s : callbacks not specified!\n", __FUNCTION__);
		return;
	}
	ICall = p;
	CB();
	glutMainLoop();
};


class Main : public ICallbacks
{
private:
	GLuint VBO;
	GLuint IBO;
	LightingTechnique* light;

	Texture* texture;
	DirectionLight dirLight;
	void GenBuff() {
		vertex Vertices[4] = {
		vertex(glm::vec3(-0.2, -0.2, 0),glm::vec2(0,0)),
		vertex(glm::vec3(0.3, -0.2, 0.5),glm::vec2(0.5,0)),
		vertex(glm::vec3(0.3, -0.2, -0.5),glm::vec2(1,0)),
		vertex(glm::vec3(0, 0.4, 0),glm::vec2(0.5,1)),
		};


		unsigned int Indices[] = { 0, 3, 1,
							   1, 3, 2,
							   2, 3, 0,
							   0, 2, 1 };
		CalcNorm(Indices, 12, Vertices, 4);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
	}
	void CalcNorm(const unsigned int* indices, unsigned int indcount, vertex* vertices, unsigned int vertcount) {
		for (unsigned int i = 0; i < indcount; i += 3) {
			unsigned int Index0 = indices[i];
			unsigned int Index1 = indices[i + 1];
			unsigned int Index2 = indices[i + 2];
			glm::vec3 v1 = vertices[Index1].m_pos - vertices[Index0].m_pos;
			glm::vec3 v2 = vertices[Index2].m_pos - vertices[Index0].m_pos;
			glm::vec3 norm = cross(v1, v2);
			normalize(norm);

			vertices[Index0].m_norm += norm;
			vertices[Index1].m_norm += norm;
			vertices[Index2].m_norm += norm;
		}
		for (unsigned int i = 0; i < vertcount; i++) {
			normalize(vertices[i].m_norm);
		}
	}
public:
	Main()
	{
		pTexture = NULL;
		light = NULL;
		dirLight.Color = glm::vec3(1.0f, 1.0f, 1.0f);
		dirLight.AmbientIntensity = 0.5f;
		dirLight.DiffuseIntensity = 0.75f;
		dirLight.Direction = glm::vec3(1.0f, 0.0, 0.0);
	}
	~Main() {
		delete light;
		delete pTexture;
	};
	bool Init()
	{
		GenBuff();
		light = new LightingTechnique();
		if (!light->Init())
		{
			return false;
		}
		light->Enable();
		light->SetTextureUnit(0);

		pTexture = new Texture(GL_TEXTURE_2D, "aa.png");

		if (!pTexture->Load()) {
			return false;
		}

		return true;
	}
	void Run()
	{
		GLUTBackendRun(this);
	}
	virtual void RenderSceneCB()
	{
		glClear(GL_COLOR_BUFFER_BIT);

		scale += 0.01f;

		Pipeline p;

		//p.Scale(sinf(scale * 0.1f), sinf(scale * 0.1f), sinf(scale * 0.1f));//sinf(Scale * 0.1f), sinf(Scale * 0.1f), sinf(Scale * 0.1f)
		p.WorldPos(0.0f, 0.0f, -10.0f);//sinf(Scale)
		p.Rotate(scale, scale, scale);//sinf(Scale) * 90.0f, sinf(Scale) * 90.0f, sinf(Scale) * 90.0f
		p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f);

		glm::vec3 CameraPos(0.0f, 0.0f, -3.0f);
		glm::vec3 CameraTarget(0.0f, 0.0f, 2.0f);
		glm::vec3 CameraUp(0.0f, 1.0f, 0.0f);

		p.SetCamera(CameraPos, CameraTarget, CameraUp);
		//p.SetCamera(GameCamera.GetPos(), GameCamera.GetTarget(), GameCamera.GetUp());

		//glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

		light->SetgWVP(p.GetTrans());
		light->SetWorld(p.GetTransWorld());
		light->SetDirectionalLight(dirLight);
		light->SetEyeWorldPos(CameraPos);
		light->SetMatSpecularIntensity(1.0f);
		light->SetMatSpecularPower(32);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid*)12);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid*)20);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		pTexture->Bind(GL_TEXTURE0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glutSwapBuffers(); // меняем фоновый буфер и буфер кадра местами
	}
	virtual void IdleCB()
	{
		RenderSceneCB();
	}
	virtual void KeyboardCB(unsigned char Key, int x, int y)
	{
		switch (Key) {
		case 'q':
			glutLeaveMainLoop();
			break;

		case 'a':
			dirLight.AmbientIntensity += 0.05f;
			break;

		case 's':
			dirLight.AmbientIntensity -= 0.05f;
			break;

		case 'z':
			dirLight.DiffuseIntensity += 0.05f;
			break;

		case 'x':
			dirLight.DiffuseIntensity -= 0.05f;
			break;
		}
	}

};





static void CreateVertexBuffer()
{
	glm::vec3 Vertices[4];

	Vertices[0] = glm::vec3(-1.0f, -1.0f, 0.5773f);
	Vertices[1] = glm::vec3(0.0f, -1.0f, -1.15475);
	Vertices[2] = glm::vec3(1.0f, -1.0f, 0.5773f);
	Vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	unsigned int Indices[] = { 0, 3, 1,
							   1, 3, 2,
							   2, 3, 0,
							   0, 2, 1 };

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{
	GLuint ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}


	AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);


	GLint success = 0;
	glLinkProgram(ShaderProgram);

	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (success == 0) {
		GLchar ErrorLog[1024];
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);

	glUseProgram(ShaderProgram);
	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
	GLUTBackendInit(argc, argv);

	if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Window")) {
		return 1;
	}

	Main* pApp = new Main();

	if (!pApp->Init()) return 1;

	pApp->Run();

	delete pApp;

	return 0;
}