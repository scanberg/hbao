#include "Types.h"
#include "Log.h"
#include "Geometry.h"
#include "ObjLoader.h"
#include "Shader.h"
#include "Camera.h"
#include "Framebuffer2D.h"
#include "MeshToGeometryAdapter.h"

#define WIDTH 1024
#define HEIGHT 768

#define AO_WIDTH (WIDTH)
#define AO_HEIGHT (HEIGHT)

#define ROTATION_SPEED 0.0
#define MOVE_SPEED 3.5
#define MOUSE_SPEED 9.5

bool setupGL();
void calcFPS(float &dt);
void GLFWCALL keyCallback(int key, int action);

void init();
void cleanUp();

void modifyCamera(float dt);

void generateNoiseTexture(int width, int height);

static bool running = true;
static Geometry * model;
static Geometry * fsquad;
static Camera * cam;

static Shader * geometryShader;
static Shader * geometryBackShader;
static Shader * compositShader;
static Shader * hbaoShader;
static Shader * blurXShader;
static Shader * blurYShader;

static Framebuffer2D * fboFullRes;
static Framebuffer2D * fboHalfRes;

static GLuint noiseTexture;

int main()
{
	init();

  GLenum buffer[1];
  float dt;

  cam->translate(vec3(0,0,2));

	while(running)
	{
    calcFPS(dt);

		modifyCamera(dt);

    cam->setup();

    // RENDER BUNNY PASS
    glEnable(GL_DEPTH_TEST);

    fboFullRes->bind();
    glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    geometryShader->bind();

    //buffer[0] = GL_COLOR_ATTACHMENT1;
    //glDrawBuffers(1, buffer);
    glCullFace(GL_BACK);

    glUniformMatrix4fv(geometryBackShader->getViewMatrixLocation(), 1, false, glm::value_ptr(cam->getViewMatrix()));
    glUniformMatrix4fv(geometryBackShader->getProjMatrixLocation(), 1, false, glm::value_ptr(cam->getProjMatrix()));

    model->draw();

		//geometryShader->bind();

    //buffer[0] = GL_COLOR_ATTACHMENT0;
    //glDrawBuffers(1, buffer);

    //glCullFace(GL_BACK);

    //glUniformMatrix4fv(geometryShader->getViewMatrixLocation(), 1, false, glm::value_ptr(cam->getViewMatrix()));
    //glUniformMatrix4fv(geometryShader->getProjMatrixLocation(), 1, false, glm::value_ptr(cam->getProjMatrix()));

		//model->draw();

    // AO pass

    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));

    glActiveTexture(GL_TEXTURE2);
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

	model = new Geometry();
  Mesh *mesh = new Mesh();
  loadMeshFromObj("resources/meshes/sponza.obj", mesh, 0.01f);
  fillGeometryFromMesh(model, mesh);
	//loadObj(*model, "resources/meshes/sponza.obj", 0.2f);
	model->createStaticBuffers();

  delete mesh;

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

  geometryBackShader = new Shader("resources/shaders/geometry_vert.glsl",
              "resources/shaders/geometry_back_frag.glsl");

  hbaoShader = new Shader("resources/shaders/fullscreen_vert.glsl",
              "resources/shaders/hbao_frag.glsl");

	compositShader = new Shader("resources/shaders/fullscreen_vert.glsl",
								"resources/shaders/composit_frag.glsl");

  // Full res deferred base
  fboFullRes = new Framebuffer2D(WIDTH, HEIGHT);
  fboFullRes->attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
  fboFullRes->attachBuffer(FBO_AUX0, GL_RGBA8, GL_RGBA, GL_FLOAT);
  fboFullRes->attachBuffer(FBO_AUX1, GL_R32F, GL_RED, GL_FLOAT);
  //fboFullRes->attachBuffer(FBO_AUX2, GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);

  // Half res buffer for AO
  fboHalfRes = new Framebuffer2D(AO_WIDTH, AO_HEIGHT);
  fboHalfRes->attachBuffer(FBO_AUX0, GL_RGB16F, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);

  float fovRad = cam->getFov() * 3.14159265f / 180.0f;

  vec2 FocalLen, InvFocalLen, UVToViewA, UVToViewB, LinMAD;

  FocalLen[0]      = 1.0f / tanf(fovRad * 0.5f) * ((float)AO_HEIGHT / (float)AO_WIDTH);
  FocalLen[1]      = 1.0f / tanf(fovRad * 0.5f);
  InvFocalLen[0]   = 1.0f / FocalLen[0];
  InvFocalLen[1]   = 1.0f / FocalLen[1];

  UVToViewA[0] = -2.0f * InvFocalLen[0];
  UVToViewA[1] = -2.0f * InvFocalLen[1];
  UVToViewB[0] =  1.0f * InvFocalLen[0];
  UVToViewB[1] =  1.0f * InvFocalLen[1];

  float near = cam->getNear(), far = cam->getFar();
  LinMAD[0] = (near-far)/(2.0f*near*far);
  LinMAD[1] = (near+far)/(2.0f*near*far);

  hbaoShader->bind();
  int pos;
  pos = hbaoShader->getUniformLocation("FocalLen");
  glUniform2f(pos, FocalLen[0], FocalLen[1]);
  pos = hbaoShader->getUniformLocation("UVToViewA");
  glUniform2f(pos, UVToViewA[0], UVToViewA[1]);
  pos = hbaoShader->getUniformLocation("UVToViewB");
  glUniform2f(pos, UVToViewB[0], UVToViewB[1]);
  pos = hbaoShader->getUniformLocation("LinMAD");
  glUniform2f(pos, LinMAD[0], LinMAD[1]);

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
	delete model;
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

void calcFPS(float &dt)
{
    static double t0=0.0;
    static char title[256];

    double t = glfwGetTime();

    dt = (float)(t - t0);
    t0 = t;

    float fps = 1.0f / dt;

    sprintf(title, "FPS: %3.1f", fps);
    glfwSetWindowTitle(title);
}

void modifyCamera(float dt)
{
    int x,y;

    glfwGetMousePos(&x, &y);

    vec3 camrot = cam->getOrientation();

    camrot.x -= (float)(y-HEIGHT/2) * MOUSE_SPEED * dt;
    camrot.y -= (float)(x-WIDTH/2)  * MOUSE_SPEED * dt;
    
    if(camrot.x > 90.0f) camrot.x = 90.0f;
    if(camrot.x < -90.0f) camrot.x = -90.0f;
    if(camrot.y > 360.0f) camrot.y -= 360.0f;
    if(camrot.y < -360.0f) camrot.y += 360.0f;
    
    cam->setOrientation(camrot);
    glfwSetMousePos(WIDTH/2, HEIGHT/2);

    vec3 movevec;

    if(glfwGetKey('W'))
        movevec.z -= MOVE_SPEED;
    if(glfwGetKey('A'))
        movevec.x -= MOVE_SPEED;
    if(glfwGetKey('S'))
        movevec.z += MOVE_SPEED;
    if(glfwGetKey('D'))
        movevec.x += MOVE_SPEED;

    cam->move(movevec * dt);
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
      float z = glm::linearRand(0.0f, 1.0f);

      int offset = 3*(y*width + x);
      noise[offset + 0] = xy[0];
      noise[offset + 1] = xy[1];
      noise[offset + 2] = z;
    }
  }

  GLint internalFormat = GL_RGB16F;
  GLint format = GL_RGB;
  GLint type = GL_FLOAT;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, noise);
}
