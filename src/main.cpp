#include "Types.h"
#include "Log.h"
#include "Geometry.h"
#include "ObjLoader.h"
#include "Shader.h"
#include "Camera.h"
#include "Framebuffer2D.h"

#define WIDTH 1024
#define HEIGHT 768

#define AO_WIDTH (WIDTH/2)
#define AO_HEIGHT (HEIGHT/2)

#define ROTATION_SPEED 0.0

bool setupGL();
void calcFPS();
void GLFWCALL keyCallback(int key, int action);

void init();
void cleanUp();

void modifyModel(mat4 &m);
void modifyCamera(Camera *cam);

void generateNoiseTexture(int width, int height);

static bool running = true;
static Geometry * mesh;
static Geometry * fsquad;
static Camera * cam;

static Shader * geometryShader;
static Shader * compositShader;
static Shader * hbaoShader;

static Framebuffer2D * fboFullRes;
static Framebuffer2D * fboHalfRes;

static GLuint noiseTexture;

int main()
{
	init();

	mat4 modelMatrix;
  vec3 lookAt = vec3(0.0,0.0,0.0);

  cam->translate(vec3(0.0,0.0,2.0));
  cam->lookAt(&lookAt);

	while(running)
	{
    calcFPS();

		modifyCamera(cam);
		modifyModel(modelMatrix);

    cam->setup();

    // RENDER BUNNY PASS
    glEnable(GL_DEPTH_TEST);

    fboFullRes->bind();
    glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		geometryShader->bind();

    glUniformMatrix4fv(geometryShader->getModelMatrixLocation(), 1, false, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(geometryShader->getViewMatrixLocation(), 1, false, glm::value_ptr(cam->getViewMatrix()));
    glUniformMatrix4fv(geometryShader->getProjMatrixLocation(), 1, false, glm::value_ptr(cam->getProjMatrix()));

		mesh->draw();

    // AO pass

    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX2));

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    fboHalfRes->bind();

    hbaoShader->bind();

    fsquad->draw();

    fboHalfRes->unbind();

    // RENDER TO SCREEN PASS

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX0));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fboHalfRes->getBufferHandle(FBO_AUX0));

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));

    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, WIDTH, HEIGHT);

    compositShader->bind();

    fsquad->draw();
		
		glfwSwapBuffers();

		if(!glfwGetWindowParam(GLFW_OPENED))
			running = false;
	}

	cleanUp();

	return 0;
}

void init()
{
	setupGL();

	cam = new Camera();

	mesh = new Geometry();
	loadObj(*mesh, "resources/meshes/bunny.obj", 0.1f);
	mesh->createStaticBuffers();

  fsquad = new Geometry();

  Geometry::sVertex v;
  v.position = vec3(-1,-1, 0); v.texCoord = vec2(0,0); fsquad->addVertex(v);
  v.position = vec3( 1,-1, 0); v.texCoord = vec2(1,0); fsquad->addVertex(v);
  v.position = vec3( 1, 1, 0); v.texCoord = vec2(1,1); fsquad->addVertex(v);
  v.position = vec3(-1, 1, 0); v.texCoord = vec2(0,1); fsquad->addVertex(v);

  fsquad->addTriangle(uvec3(0,1,2));
  fsquad->addTriangle(uvec3(0,2,3));
  fsquad->createStaticBuffers();

	geometryShader = new Shader("resources/shaders/geometry_vert.glsl",
							"resources/shaders/geometry_frag.glsl");

  hbaoShader = new Shader("resources/shaders/fullscreen_vert.glsl",
              "resources/shaders/hbao_frag.glsl");

	compositShader = new Shader("resources/shaders/fullscreen_vert.glsl",
								"resources/shaders/composit_frag.glsl");

  // Full res deferred base
  fboFullRes = new Framebuffer2D(WIDTH, HEIGHT);
  fboFullRes->attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
  fboFullRes->attachBuffer(FBO_AUX0, GL_RGBA8, GL_RGBA, GL_FLOAT);
  fboFullRes->attachBuffer(FBO_AUX1, GL_RG16F, GL_RG, GL_FLOAT);
  fboFullRes->attachBuffer(FBO_AUX2, GL_RGB32F, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);

  // Half res buffer for AO
  fboHalfRes = new Framebuffer2D(AO_WIDTH, AO_HEIGHT);
  fboHalfRes->attachBuffer(FBO_AUX0, GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);

  float fovRad = cam->getFov() * 3.14159265f / 180.0f;

  vec2 FocalLen, InvFocalLen, UVToViewA, UVToViewB;

  FocalLen[0]      = 1.0f / tanf(fovRad * 0.5f) * ((float)AO_HEIGHT / (float)AO_WIDTH);
  FocalLen[1]      = 1.0f / tanf(fovRad * 0.5f);
  InvFocalLen[0]   = 1.0f / FocalLen[0];
  InvFocalLen[1]   = 1.0f / FocalLen[1];

  UVToViewA[0] = -2.0f * InvFocalLen[0];
  UVToViewA[1] = -2.0f * InvFocalLen[1];
  UVToViewB[0] =  1.0f * InvFocalLen[0];
  UVToViewB[1] =  1.0f * InvFocalLen[1];

  hbaoShader->bind();
  int pos;
  pos = hbaoShader->getUniformLocation("FocalLen");
  glUniform2f(pos, FocalLen[0], FocalLen[1]);
  pos = hbaoShader->getUniformLocation("UVToViewA");
  glUniform2f(pos, UVToViewA[0], UVToViewA[1]);
  pos = hbaoShader->getUniformLocation("UVToViewB");
  glUniform2f(pos, UVToViewB[0], UVToViewB[1]);

  pos = hbaoShader->getUniformLocation("AORes");
  glUniform2f(pos, AO_WIDTH, AO_HEIGHT);
  pos = hbaoShader->getUniformLocation("InvAORes");
  glUniform2f(pos, 1.0f/AO_WIDTH, 1.0f/AO_HEIGHT);

  glGenTextures(1, &noiseTexture);
  glBindTexture(GL_TEXTURE_2D, noiseTexture);
  generateNoiseTexture(4, 4);

}

void cleanUp()
{
	delete mesh;
  delete fsquad;

	delete cam;

	delete geometryShader;
	delete hbaoShader;
  delete compositShader;

  delete fboFullRes;
  delete fboHalfRes;

  glDeleteTextures(1, &noiseTexture);
}

bool setupGL()
{
	glfwInit();

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE, GL_TRUE );

	if(!glfwOpenWindow(	WIDTH,
						HEIGHT,
						8, 8, 8, 8,
						32, 0,
						GLFW_WINDOW))
	{
		logError("could not create GLFW-window");
		return false;
	}

	logErrorsGL();

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		logError("GLEW init error");
		return false;
	}

	logErrorsGL();

	logNote("Successfully created OpenGL-window, version %i.%i",
         glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR),
         glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR));

	logNote("GLSL-version: %s",glGetString(GL_SHADING_LANGUAGE_VERSION));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
	glfwSwapInterval(0);
  glActiveTexture(GL_TEXTURE0);
  glClearColor(1.0, 1.0, 1.0, 0.0);

	glfwSetKeyCallback(keyCallback);

	return true;
}

void GLFWCALL keyCallback(int key, int action)
{
  switch(key)
  {
    case GLFW_KEY_ESC:
    	running = false;
    	break;
  }
}

void calcFPS()
{
    static double t0=0.0;
    static int frameCount=0;
    static char title[256];

    double t = glfwGetTime();

    if( (t - t0) > 0.25 )
    {
        double fps = (double)frameCount / (t - t0);
        frameCount = 0;
        t0 = t;

        sprintf(title, "FPS: %3.1f", fps);
        glfwSetWindowTitle(title);
    }
    frameCount++;
}

void modifyCamera(Camera *cam)
{
    static int oldMouseX=320, oldMouseY=240, oldMouseZ=1;
    static vec2 rotAngle = vec2(PI,0.5*PI);
    static float dist = 2.0;
    int mouseX, mouseY, mouseZ;

    glfwGetMousePos(&mouseX, &mouseY);
    mouseZ = glfwGetMouseWheel();

    vec2 mouseMove((float)(mouseX-oldMouseX), (float)(mouseY-oldMouseY));
    dist += (float)(mouseZ-oldMouseZ) * 0.1f;

    oldMouseX = mouseX; oldMouseY = mouseY; oldMouseZ = mouseZ;

    if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
        rotAngle += mouseMove * 0.02f;

    rotAngle.y = glm::clamp(rotAngle.y, 0.02f, PI * 0.98f);
    dist = glm::clamp(dist, 0.6f, 10.0f);

    float theta = -rotAngle.y;
    float phi = -rotAngle.x;

    cam->setPosition(dist*glm::sin(theta)*glm::sin(phi), dist*glm::cos(theta), dist*glm::sin(theta)*glm::cos(phi));
}

void modifyModel( mat4 &m )
{
    static float angle = 0.0f;
    angle += ROTATION_SPEED;
    if(angle > 360.0f) angle -= 360.0f;
    else if(angle < -360.0) angle += 360.0f;

    m = glm::rotate(mat4(), angle, vec3(0.0f,1.0f,0.0f));
}

void generateNoiseTexture(int width, int height)
{
  float *noise = new float[width*height*3];
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      vec2 xy = glm::circularRand(1.0f);
      float z = glm::linearRand(-1.0f, 1.0f);

      int offset = 3*(y*width + x);
      noise[offset + 0] = xy[0];
      noise[offset + 1] = xy[1];
      noise[offset + 2] = z;
    }
  }

  GLint internalFormat = GL_RGB8;
  GLint format = GL_RGB;
  GLint type = GL_FLOAT;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, noise);
}
