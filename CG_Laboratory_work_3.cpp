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

static float scale = 0.0;

static const char* pVS = "                                                         \n\
    #version 330                                                                   \n\
    layout (location = 0) in vec3 pos;                                             \n\
    layout (location = 1) in vec2 tex;                                             \n\
    uniform mat4 gWorld;                                                           \n\
    out vec2 tex0;                                                                 \n\
    void main()                                                                    \n\
    {                                                                              \n\
        gl_Position = gWorld * vec4(pos, 1.0);                                     \n\
        tex0 = tex;                                                                \n\
    }";

static const char* pFS = "                                                          \n\
    #version 330                                                                    \n\
    in vec2 tex0;                                                                   \n\
    struct DirectionalLight                                                         \n\
    {                                                                               \n\
        vec3 Color;                                                                 \n\
        float AmbientIntensity;                                                     \n\
    };                                                                              \n\
    uniform sampler2D gSampler;                                                     \n\
    uniform DirectionalLight gDirectionalLight;                                     \n\
    out vec4 fragcolor;                                                             \n\
    void main()                                                                     \n\
    {                                                                               \n\
        fragcolor = texture2D(gSampler, tex0.xy)*                                   \n\
        vec4(gDirectionalLight.Color, 1.0f) *                                       \n\
        gDirectionalLight.AmbientIntensity;											\n\
    }";

struct vertex {
	glm::vec3 fst;
	glm::vec2 snd;

	vertex(glm::vec3 inp1, glm::vec2 inp2) {
		fst = inp1;
		snd = inp2;
	}
};
struct DirectionLight
{
	glm::vec3 Color;
	float AmbientIntensity;
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

		GLint success;

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

		gWorldLocation = GetUniformLocation("gWorld");
		samplerLocation = GetUniformLocation("gSampler");
		dirLightColorLocation = GetUniformLocation("gDirectionalLight.Color");
		dirLightAmbientIntensityLocation = GetUniformLocation("gDirectionalLight.AmbientIntensity");

		if (dirLightAmbientIntensityLocation == 0xFFFFFFFF || gWorldLocation == 0xFFFFFFFF || samplerLocation == 0xFFFFFFFF || dirLightColorLocation == 0xFFFFFFFF)  return false;

		return true;
	};

	void SetgWorld(const glm::mat4* gWorld) {
		glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)gWorld);
	};

	void SetTextureUnit(unsigned int unit) {
		glUniform1i(samplerLocation, unit);
	};

	void SetDirectionalLight(const DirectionLight& Light) {
		glUniform3f(dirLightColorLocation, Light.Color.x, Light.Color.y, Light.Color.z);
		glUniform1f(dirLightAmbientIntensityLocation, Light.AmbientIntensity);
	};
private:
	GLuint gWorldLocation;
	GLuint samplerLocation;
	GLuint dirLightColorLocation;
	GLuint dirLightAmbientIntensityLocation;
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
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

		unsigned int Indices[] = { 0, 3, 1,
							   1, 3, 2,
							   2, 3, 0,
							   0, 2, 1 };

		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
	}
public:
	Main()
	{
		pTexture = NULL;
		light = NULL;
		dirLight.Color = glm::vec3(1.0f, 1.0f, 1.0f);
		dirLight.AmbientIntensity = 0.5f;
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

		light->SetgWorld(p.GetTrans());
		light->SetDirectionalLight(dirLight);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid*)12);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		pTexture->Bind(GL_TEXTURE0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		glutSwapBuffers();
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