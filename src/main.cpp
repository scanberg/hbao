#include "Types.h"
#include "Log.h"
#include "Geometry.h"
#include "ObjLoader.h"
#include "Shader.h"
#include "Camera.h"
#include "Surface.h"
#include "Model.h"
#include "MtlLoader.h"
#include "Framebuffer2D.h"
#include "MeshToGeometryAdapter.h"
#include "MaterialToSurfaceAdapter.h"

#define WIDTH 1280
#define HEIGHT 720

#define RES_RATIO 2
#define AO_WIDTH (WIDTH/RES_RATIO)
#define AO_HEIGHT (HEIGHT/RES_RATIO)
#define AO_RADIUS 0.3
#define AO_DIRS 6
#define AO_SAMPLES 3
#define AO_STRENGTH 2.5;

#define NOISE_RES 4

#define MOVE_SPEED 3.5
#define MOUSE_SPEED 9.5

bool setupGL();
void calcFPS(float &dt);
void GLFWCALL keyCallback(int key, int action);

void init();
void cleanUp();

void modifyCamera(float dt);
void generateNoiseTexture(int width, int height);

bool running = true;
bool blur = false;
bool fullres = true;

Model * mdl;
Geometry * model;
Geometry * fsquad;
Camera * cam;

Shader * geometryShader;
Shader * geometryBackShader;
Shader * compositShader;
Shader * hbaoShader;
Shader * hbaoFullShader;
//Shader * ssaoShader;
Shader * blurXShader;
Shader * blurYShader;
Shader * downsampleShader;
Shader * upsampleShader;

Framebuffer2D * fboFullRes;
Framebuffer2D * fboHalfRes;

double timeStamps[7];

Surface * surface0;

std::map<std::string, Surface*> surfaces;
std::vector<Geometry> models;

GLuint noiseTexture;

int main()
{
	init();

  GLenum buffer[1];
  float dt;

  cam->translate(vec3(0,0,2));

	while(running)
	{
    calcFPS(dt);

    timeStamps[0] = glfwGetTime();

		modifyCamera(dt);
    cam->setup();

    // RENDER GEOMETRY PASS
    glEnable(GL_DEPTH_TEST);

    fboFullRes->bind();
    glClearColor(0.2, 0.3, 0.5, 1.0);
    glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    geometryShader->bind();

    glUniformMatrix4fv(geometryShader->getViewMatrixLocation(), 1, false, glm::value_ptr(cam->getViewMatrix()));
    glUniformMatrix4fv(geometryShader->getProjMatrixLocation(), 1, false, glm::value_ptr(cam->getProjMatrix()));

    mdl->draw();

    timeStamps[1] = glfwGetTime();

    glDisable(GL_DEPTH_TEST);

    //glColorMask(1, 0, 0, 0);
    //glDepthFunc(GL_ALWAYS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));

    if(!fullres)
    {

      // Downsample depth
      fboHalfRes->bind();

      buffer[0] = GL_COLOR_ATTACHMENT0;
      glDrawBuffers(1, buffer);

      downsampleShader->bind();
      fsquad->draw();

    }

    timeStamps[2] = glfwGetTime();

    // AO pass

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    if(!fullres)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, fboHalfRes->getBufferHandle(FBO_AUX0));

      buffer[0] = GL_COLOR_ATTACHMENT1;
      glDrawBuffers(1, buffer);

      hbaoShader->bind();
      //ssaoShader->bind();

      fsquad->draw();
    }
    else
    {
      buffer[0] = GL_COLOR_ATTACHMENT1;
      glDrawBuffers(1, buffer);

      hbaoFullShader->bind();
      fsquad->draw();
    }

    timeStamps[3] = glfwGetTime();

    // Upsample

    if(!fullres)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_DEPTH));

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, fboHalfRes->getBufferHandle(FBO_AUX1));

      fboFullRes->bind();

      buffer[0] = GL_COLOR_ATTACHMENT1;
      glDrawBuffers(1, buffer);

      upsampleShader->bind();

      fsquad->draw();
    }

    timeStamps[4] = glfwGetTime();

    if(blur)
    {
      // BLUR 

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));

      // X
      buffer[0] = GL_COLOR_ATTACHMENT2;
      glDrawBuffers(1, buffer);

      blurXShader->bind();

      fsquad->draw();

      // Y
      buffer[0] = GL_COLOR_ATTACHMENT1;
      glDrawBuffers(1, buffer);

      blurYShader->bind();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX2));

      fsquad->draw();

    }

    timeStamps[5] = glfwGetTime();

    fboFullRes->unbind();

    // RENDER TO SCREEN PASS

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX0));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fboFullRes->getBufferHandle(FBO_AUX1));

    //glClearColor(1.0, 1.0, 1.0, 0.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, WIDTH, HEIGHT);

    compositShader->bind();

    fsquad->draw();

    timeStamps[6] = glfwGetTime();
		
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
  Surface::init();

	cam = new Camera();
  mdl = new Model();

  Mesh mesh = loadMeshFromObj("resources/meshes/dragon.obj", 0.1);
  Geometry dragon = createGeometryFromMesh(mesh);

  Surface *surface = new Surface();
  //surface->loadDiffuseTexture("resources/meshes/textures/sponza_floor_a_spec.tga");
  surfaces.insert(std::pair<std::string, Surface*> (std::string("default"), surface) );

  mdl->addGeometryAndSurface(&dragon, surface);

  // Geometry floor;

  // Geometry::sVertex v;
  // v.position = vec3(-1, 0,-1); v.texCoord = vec2(0,0); floor.addVertex(v);
  // v.position = vec3( 1, 0,-1); v.texCoord = vec2(1,0); floor.addVertex(v);
  // v.position = vec3( 1, 0, 1); v.texCoord = vec2(1,1); floor.addVertex(v);
  // v.position = vec3(-1, 0, 1); v.texCoord = vec2(0,1); floor.addVertex(v);

  // floor.addTriangle(uvec3(0,2,1));
  // floor.addTriangle(uvec3(0,3,2));

  // mdl->addGeometryAndSurface(&floor, surface);

  //model = new Geometry();

  //mesh = loadMeshFromObj("resources/meshes/sponza.obj", 0.01f);
  //*model = createGeometryFromMesh(mesh);
  //model->createStaticBuffers();

  std::vector<Mesh> meshes = loadMeshesFromObj("resources/meshes/sponza.obj", 0.01f);
  std::vector<Geometry> geometries = createGeometryFromMesh(meshes);

  std::vector<Material> materials = loadMaterialsFromMtl("resources/meshes/sponza.mtl");
  surfaces = createSurfaceFromMaterial(materials, "resources/meshes/");

  for(unsigned int i=0; i<geometries.size(); ++i)
  {
    mdl->addGeometryAndSurface(&geometries[i], surfaces[geometries[i].material]);
  }

  mdl->prepare();

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

  hbaoFullShader = new Shader("resources/shaders/fullscreen_vert.glsl",
              "resources/shaders/hbao_full_frag.glsl");

  // ssaoShader = new Shader("resources/shaders/fullscreen_vert.glsl",
  //             "resources/shaders/ssao_frag.glsl");

	compositShader = new Shader("resources/shaders/fullscreen_vert.glsl",
								"resources/shaders/composit_frag.glsl");

  blurXShader = new Shader("resources/shaders/fullscreen_vert.glsl",
                "resources/shaders/blur_x_frag.glsl");

  blurYShader = new Shader("resources/shaders/fullscreen_vert.glsl",
                "resources/shaders/blur_y_frag.glsl");

  downsampleShader = new Shader("resources/shaders/fullscreen_vert.glsl",
                    "resources/shaders/downsample_depth_frag.glsl");

  upsampleShader = new Shader("resources/shaders/fullscreen_vert.glsl",
                    "resources/shaders/upsample_aoz_frag.glsl");

  // Full res deferred base
  fboFullRes = new Framebuffer2D(WIDTH, HEIGHT);
  fboFullRes->attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR, GL_LINEAR);
  fboFullRes->attachBuffer(FBO_AUX0, GL_RGBA8, GL_RGBA, GL_FLOAT);
  fboFullRes->attachBuffer(FBO_AUX1, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
  fboFullRes->attachBuffer(FBO_AUX2, GL_RG16F, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
  //fboFullRes->attachBuffer(FBO_AUX3, GL_R8, GL_RED, GL_FLOAT);

  // Half res buffer for AO
  fboHalfRes = new Framebuffer2D(AO_WIDTH, AO_HEIGHT);
  //fboHalfRes->attachBuffer(FBO_DEPTH, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
  fboHalfRes->attachBuffer(FBO_AUX0, GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);
  fboHalfRes->attachBuffer(FBO_AUX1, GL_R8, GL_RED, GL_FLOAT, GL_LINEAR, GL_LINEAR);


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
  glUniform2f(pos, (float)AO_WIDTH, (float)AO_HEIGHT);
  pos = hbaoShader->getUniformLocation("InvAORes");
  glUniform2f(pos, 1.0f/(float)AO_WIDTH, 1.0f/(float)AO_HEIGHT);

  pos = hbaoShader->getUniformLocation("R");
  glUniform1f(pos, AO_RADIUS);
  pos = hbaoShader->getUniformLocation("R2");
  glUniform1f(pos, AO_RADIUS*AO_RADIUS);
  pos = hbaoShader->getUniformLocation("NegInvR2");
  glUniform1f(pos, -1.0f / (AO_RADIUS*AO_RADIUS));

  pos = hbaoShader->getUniformLocation("NoiseScale");
  glUniform2f(pos, (float)AO_WIDTH/(float)NOISE_RES, (float)AO_HEIGHT/(float)NOISE_RES);
  pos = hbaoShader->getUniformLocation("NumDirections");
  glUniform1i(pos, AO_DIRS);
  pos = hbaoShader->getUniformLocation("NumSamples");
  glUniform1i(pos, AO_SAMPLES);

  hbaoFullShader->bind();
  pos = hbaoFullShader->getUniformLocation("FocalLen");
  glUniform2f(pos, FocalLen[0], FocalLen[1]);
  pos = hbaoFullShader->getUniformLocation("UVToViewA");
  glUniform2f(pos, UVToViewA[0], UVToViewA[1]);
  pos = hbaoFullShader->getUniformLocation("UVToViewB");
  glUniform2f(pos, UVToViewB[0], UVToViewB[1]);
  pos = hbaoFullShader->getUniformLocation("LinMAD");
  glUniform2f(pos, LinMAD[0], LinMAD[1]);

  pos = hbaoFullShader->getUniformLocation("AORes");
  glUniform2f(pos, (float)WIDTH, (float)HEIGHT);
  pos = hbaoFullShader->getUniformLocation("InvAORes");
  glUniform2f(pos, 1.0f/(float)WIDTH, 1.0f/(float)HEIGHT);

  pos = hbaoFullShader->getUniformLocation("R");
  glUniform1f(pos, AO_RADIUS);
  pos = hbaoFullShader->getUniformLocation("R2");
  glUniform1f(pos, AO_RADIUS*AO_RADIUS);
  pos = hbaoFullShader->getUniformLocation("NegInvR2");
  glUniform1f(pos, -1.0f / (AO_RADIUS*AO_RADIUS));

  pos = hbaoFullShader->getUniformLocation("NoiseScale");
  glUniform2f(pos, (float)AO_WIDTH/(float)NOISE_RES, (float)AO_HEIGHT/(float)NOISE_RES);
  pos = hbaoFullShader->getUniformLocation("NumDirections");
  glUniform1i(pos, AO_DIRS);
  pos = hbaoFullShader->getUniformLocation("NumSamples");
  glUniform1i(pos, AO_SAMPLES);

  blurXShader->bind();
  pos = blurXShader->getUniformLocation("AORes");
  glUniform2f(pos, AO_WIDTH, AO_HEIGHT);
  pos = blurXShader->getUniformLocation("InvAORes");
  glUniform2f(pos, 1.0f/AO_WIDTH, 1.0f/AO_HEIGHT);
  pos = blurXShader->getUniformLocation("FullRes");
  glUniform2f(pos, WIDTH, HEIGHT);
  pos = blurXShader->getUniformLocation("InvFullRes");
  glUniform2f(pos, 1.0f/WIDTH, 1.0f/HEIGHT);
  pos = blurXShader->getUniformLocation("LinMAD");
  glUniform2f(pos, LinMAD[0], LinMAD[1]);

  blurYShader->bind();
  pos = blurYShader->getUniformLocation("AORes");
  glUniform2f(pos, AO_WIDTH, AO_HEIGHT);
  pos = blurYShader->getUniformLocation("InvAORes");
  glUniform2f(pos, 1.0f/AO_WIDTH, 1.0f/AO_HEIGHT);
  pos = blurYShader->getUniformLocation("FullRes");
  glUniform2f(pos, WIDTH, HEIGHT);
  pos = blurYShader->getUniformLocation("InvFullRes");
  glUniform2f(pos, 1.0f/WIDTH, 1.0f/HEIGHT);
  pos = blurYShader->getUniformLocation("LinMAD");
  glUniform2f(pos, LinMAD[0], LinMAD[1]);

  downsampleShader->bind();
  pos = downsampleShader->getUniformLocation("LinMAD");
  glUniform2f(pos, LinMAD[0], LinMAD[1]);
  pos = downsampleShader->getUniformLocation("ResRatio");
  glUniform1i(pos, RES_RATIO);

  upsampleShader->bind();
  pos = upsampleShader->getUniformLocation("LinMAD");
  glUniform2f(pos, LinMAD[0], LinMAD[1]);

  glGenTextures(1, &noiseTexture);
  glBindTexture(GL_TEXTURE_2D, noiseTexture);
  generateNoiseTexture(NOISE_RES, NOISE_RES);

}

void cleanUp()
{
	delete model;
  delete fsquad;

  delete surface0;

	delete cam;

	delete geometryShader;
	delete hbaoShader;
  delete hbaoFullShader;
  delete compositShader;
  delete blurXShader;
  delete blurYShader;
  delete downsampleShader;

  delete fboFullRes;
  delete fboHalfRes;

  for(std::map<std::string, Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); ++it)
    delete it->second;

  glDeleteTextures(1, &noiseTexture);

  Surface::cleanUp();
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
  glfwDisable(GLFW_MOUSE_CURSOR);

	glfwSetKeyCallback(keyCallback);

	return true;
}

void GLFWCALL keyCallback(int key, int action)
{
  if(key == GLFW_KEY_ESC)
    running = false;
  else if(key == 'B' && action == GLFW_PRESS)
    blur = !blur;
  else if(key == 'F' && action == GLFW_PRESS)
    fullres = !fullres;
}

void calcFPS(float &dt)
{
    static float t0=0.0;
    static float t1=0.0;
    static char title[256];
    static int frames=0;

    float t = (float)glfwGetTime();

    dt = t - t0;
    t0 = t;

    t1 += dt;

    if(t1 > 0.25)
    {
      float fps = (float)frames / t1;
      double geom = timeStamps[1] - timeStamps[0];
      double down = timeStamps[2] - timeStamps[1];
      double ao   = timeStamps[3] - timeStamps[2];
      double up   = timeStamps[4] - timeStamps[3];
      double blur = timeStamps[5] - timeStamps[4];
      double comp = timeStamps[6] - timeStamps[5];
      double tot  = timeStamps[6] - timeStamps[0];


      sprintf(title, "Fullres: %i, FPS: %2.1f, time: geom %1.5f, down %1.5f, ao %1.5f, up %1.5f, blur %1.5f, comp %1.5f tot %1.5f",
        (int)fullres,
        fps,
        geom,
        down,
        ao,
        up,
        blur,
        comp,
        tot);
      glfwSetWindowTitle(title);
      t1 = 0.0;
      frames = 0;
    }

    ++frames;
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

  GLint internalFormat = GL_RGB32F;
  GLint format = GL_RGB;
  GLint type = GL_FLOAT;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, noise);
}
