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

GLuint VBO;
GLuint IBO;
GLuint gWorldLocation;
GLuint gSampler;

Texture* pTexture = NULL;

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
    out vec4 fragcolor;																\n\
	uniform sampler2D gSampler;                                                     \n\
    void main()                                                                     \n\
    {                                                                               \n\
        fragcolor = texture2D(gSampler, tex0.xy);                                   \n\
    }";

struct vertex {
	glm::vec3 fst;
	glm::vec2 snd;

	vertex(glm::vec3 inp1, glm::vec2 inp2) {
		fst = inp1;
		snd = inp2;
	}
};

Camera GameCamera;

static void SpecialKeyboardCB(int Key, int x, int y)
{
	GameCamera.OnKeyboard(Key);
}

void RenderSceneCB()
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

	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

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
	glutIdleFunc(RenderSceneCB);

}

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

	glutInit(&argc, argv); // инициализируем GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //устанавливает начальный режим отображения 

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(200, 50);
	glutCreateWindow("Window");

	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	Magick::InitializeMagick(*argv);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();

	/*GLuint ShaderProgram = glCreateProgram();
	AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);*/

	CompileShaders();

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glUniform1i(gSampler, 0);

	pTexture = new Texture(GL_TEXTURE_2D, "aa.png");

	if (!pTexture->Load()) {
		return 1;
	}

	glutDisplayFunc(RenderSceneCB);
	glutMainLoop();

	return 0;
}