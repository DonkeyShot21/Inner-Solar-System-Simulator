#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <ctime>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "InputState.h"
#include "Viewer.h"
#include "Shader.hpp"
#include "Sphere.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define VALS_PER_VERT 3
#define VALS_PER_NORMAL 3
#define VALS_PER_TEX 2
#define LIGHT_SPEED 0.2f
#define NUM_OBJECTS 5
#define NUM_PLANET_TEXTURES 8
#define NUM_SUN_TEXTURES 2
#define TEXTURES_PER_PLANET 2

//std::string soundMode = "ON";
std::string soundMode = "OFF";



// MERCURY - index = 0 - mass = 0.0000001652 MS - distance from Sun = 0.39 AU - speed = 47.87 km/s - normalized speed = 1
// VENUS - index = 1 - mass = 0.000002447 MS - distance from Sun = 0.723 AU - speed = 35.02 km/s - normalized speed = 0.7315646542719866
// EARTH - index = 2 - mass = 0.000003003 MS - distance from Sun = 1 AU - speed = 29.78 km/s - normalized speed = 0.6074785878420723
// MARS - index = 3 - mass = 0.0000003213 MS - distance from Sun = 1.524 AU - speed = 24.077 km/s - normalized speed = 0.5029663672446208
// SUN - index = 4 - mass = 1 MS - distance from Sun = 0 AU - speed = 0 km/s - normalized speed = 0

using namespace std::chrono;

int winX = 800;
int winY = 800;

long present_time;
long last_time = 0.0;
float timeSpeed =0.00000000001f;
float rotationSpeed = 500.0f;

float d2[NUM_OBJECTS][NUM_OBJECTS];   /* array for interparticle distances */

Sphere *obj[NUM_OBJECTS];

GLuint objectVaoHandle[NUM_OBJECTS];

GLuint skyboxVaoHandle;
unsigned int skyboxTexID;

WorldObjectViewer *WorldCam;
ObjectViewer *ObjCam;
FollowViewer *FollowCam;
Viewer *Camera;

int planet_to_follow = 2;
float heightOnOrbitPlane = 7.0f;
float zoom = 0.0f;

glm::vec3 cameraPos(0.0f, 5.0f, -12.0f);

InputState Input;

GLuint planetTexID[NUM_PLANET_TEXTURES];
GLuint sunTexID[NUM_SUN_TEXTURES];

unsigned int programID;
unsigned int skyboxProgramID;
unsigned int proceduralProgramID;
unsigned int sunProgramID;

float mtlambientSun[3] = { 0.5f, 0.5f, 0.5f };	// ambient material
float mtlambientPlanets[3] = { 0.05f, 0.05f, 0.05f };	// ambient material


//
// Callbacks
//
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch(key)
            {
            case GLFW_KEY_ESCAPE: // escape key pressed
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case '1':
                Camera = ObjCam;
                break;
            case '2':
                Camera = WorldCam;
                break;
            case '3':
                Camera = FollowCam;
                break;
            case GLFW_KEY_M:
                planet_to_follow = 0;
                zoom = 0.0f;
                break;
            case GLFW_KEY_V:
                planet_to_follow = 1;
                zoom = 0.0f;
                break;
            case GLFW_KEY_E:
                planet_to_follow = 2;
                zoom = 0.0f;
                break;
            case GLFW_KEY_N:
                planet_to_follow = 3;
                zoom = 0.0f;
                break;
            case GLFW_KEY_UP:
                if (heightOnOrbitPlane < 7)
                  heightOnOrbitPlane += 1.0;
                break;
            case GLFW_KEY_DOWN:
                if (heightOnOrbitPlane > -7)
                  heightOnOrbitPlane -= 1.0;
                break;
            case GLFW_KEY_Z:
                if (zoom < planet_to_follow+2)
                  zoom += 1.0;
                else
                  zoom = 0.0;
                break;
            case '0':
              rotationSpeed += 100.0f;
              break;
            case '9':
              rotationSpeed -= 100.0f;
            case GLFW_KEY_S:
              if (sunProgramID == proceduralProgramID)
                sunProgramID = programID;
              else
                sunProgramID = proceduralProgramID;
              break;
              break;
            default:
                break;
            }
    }
}

// Set the projection matrix. Takes into account window aspect ratio, so called
// when the window is resized.
void setProjection(unsigned int pID)
{
  glm::mat4 projection;

  projection = glm::perspective( (float)M_PI/3.0f, (float) winX / winY, 1.0f, 40.0f );

	// Load it to the shader program
	int projHandle = glGetUniformLocation(pID, "projection");
	if (projHandle == -1) {
		std::cout << "Uniform: projection_matrix is not an active uniform label\n";
	}
	glUniformMatrix4fv( projHandle, 1, false, glm::value_ptr(projection) );
}

// Called when the window is resized.
void reshape_callback( GLFWwindow *window, int x, int y )
{
    winX = x;
    winY = y;
    setProjection(programID);
    setProjection(skyboxProgramID);
    glViewport( 0, 0, x, y );
}

void mouse_pos_callback(GLFWwindow* window, double x, double y)
{
    // Use a global data structure to store mouse info
    // We can then use it however we want
    Input.update((float)x, (float)y);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        Input.rMousePressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        Input.rMousePressed = false;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        Input.lMousePressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        Input.lMousePressed = false;
    }
}

float forces( int i, int j )
{
    int k;
    float force = 0.0;
    for ( k = NUM_OBJECTS-1; k < NUM_OBJECTS; k++ ) {
      if ( k != i )
        force += - 3 * ( obj[i]->position[j] - obj[k]->position[j] ) / ( 0.001 + d2[i][k] );
    }
    return ( force );
}

void computePosVel(float dt)
{
    int i, j, k;

    for ( i = 0; i < NUM_OBJECTS - 1; i++ ) {
        for ( j = 0; j < 3; j++ ) {
            obj[i]->position[j] += dt * obj[i]->velocity[j]; // update position
            obj[i]->velocity[j] += dt * forces( i, j ) / obj[i]->mass; // update velocity
        }
    }
    for ( i = 0; i < NUM_OBJECTS-1; i++ )
      for ( k = 0; k < i; k++ ) {
        d2[i][k] = 0.0;
        for ( j = 0; j < 3; j++ )
          d2[i][k] += ( obj[i]->position[j] - obj[k]->position[j] ) * ( obj[i]->position[j] - obj[k]->position[j] );
        d2[k][i] = d2[i][k];
      }
    last_time = present_time;
}

void loadRGBTexture(const char *path)
{
    int x, y, n;

    // Load from file. Force RGB image pixel format
    unsigned char *data = stbi_load(path, &x, &y, &n, 3);

    // Copy to GPU as data for the currently active texture.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    glGenerateMipmap(GL_TEXTURE_2D);
}



int createSunTextures()
{
  glGenTextures( 1, &sunTexID[0] );

  glActiveTexture(GL_TEXTURE0);
  glBindTexture( GL_TEXTURE_2D, sunTexID[0] );
  loadRGBTexture( "sunmap.jpg" );

  glActiveTexture(GL_TEXTURE0);
  glBindTexture( GL_TEXTURE_2D, sunTexID[1] );
  loadRGBTexture( "2k_sun.jpg" );
  return 0;
}

int createPlanetTextures(std::string prefix, int index)
{
  for (int i = 0; i<TEXTURES_PER_PLANET; i++)
  glGenTextures( 1, &planetTexID[index + i] );

  glActiveTexture(2*index);
  glBindTexture( GL_TEXTURE_2D, planetTexID[index] );
  loadRGBTexture( (prefix + "_diffuse.jpg").c_str() );

  glActiveTexture(2*index + 1);
  glBindTexture( GL_TEXTURE_2D, planetTexID[index + 1] );
  loadRGBTexture( (prefix + "_specular.jpg").c_str() );

  return 0;
}


int createSphereVAO(Sphere *sphere, unsigned int index)
{
	// Using the sphere class to generate vertices and element indices
	glGenVertexArrays(1, &objectVaoHandle[index]);
	glBindVertexArray(objectVaoHandle[index]);

	unsigned int buffer[4];
	glGenBuffers(4, buffer);

	// Set vertex position attribute
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * sphere->vertCount,
                 sphere->vertices,
                 GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

	// Normal attributes
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * sphere->normCount,
                 sphere->normals,
                 GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, VALS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0, 0);

  // Tex coords
	glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * sphere->texCount,
                 sphere->texcoords,
                 GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, VALS_PER_TEX, GL_FLOAT, GL_FALSE, 0, 0);

	// Vertex indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(unsigned int) * sphere->indCount,
                 sphere->indices,
                 GL_STATIC_DRAW);

	return 0; // success
}

unsigned int loadCubemap()
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load("stars.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load: " << i << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void setupSkybox(){
  // Using the sphere class to generate vertices and element indices
  glGenVertexArrays(1, &skyboxVaoHandle);
  glBindVertexArray(skyboxVaoHandle);

  unsigned int buffer;
  glGenBuffers(1, &buffer);

  float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
  };

  // Set vertex position attribute
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER,
                 sizeof(skyboxVertices) * sizeof(float),
                 skyboxVertices,
                 GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, VALS_PER_VERT, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
}

void setupSolarSystem(){

    glm::vec3 position;
    glm::vec3 velocity;

    // MERCURY
    position = glm::vec3(0.39f,0.0f,0.0f);
    velocity = glm::vec3(0.0f,0.0f,-0.39f);
    obj[0] = new Sphere(0.12f, 1.0f, position, velocity, 50, 50);
    createSphereVAO(obj[0], 0);
    createPlanetTextures("mercury", 0 * 2);

    // VENUS
    position = glm::vec3(0.0f,0.0f,0.723f);
    velocity = glm::vec3(0.723f,0.0f,0.0f);
    obj[1] = new Sphere(0.28f, 1.0f, position, velocity, 50, 50);
    createSphereVAO(obj[1], 1);
    createPlanetTextures("venus", 1 * 2);

    // EARTH
    position = glm::vec3(-1.0f,0.0f,0.0f);
    velocity = glm::vec3(0.0f,0.0f,0.723f);
    obj[2] = new Sphere(0.3f, 1.0f, position, velocity, 50, 50);
    createSphereVAO(obj[2], 2);
    createPlanetTextures("earth", 2 * 2);

    // MARS
    position = glm::vec3(0.0f,0.0f,-1.524f);
    velocity = glm::vec3(-1.524f,0.0f,0.0f);
    obj[3] = new Sphere(0.16f, 1.0f, position, velocity, 50, 50);
    createSphereVAO(obj[3],3);
    createPlanetTextures("mars", 3 * 2);

    // SUN
    position = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);
    obj[4] = new Sphere(1.0f, 1.0f, position, velocity, 50, 50);
    createSphereVAO(obj[4], 4);
    createSunTextures();


}

int setupMviewShader(unsigned int id)
{
	glUseProgram(id);

  // Uniform lighting variables
  int lightambientHandle = glGetUniformLocation(id, "light_ambient");
	int lightdiffuseHandle = glGetUniformLocation(id, "light_diffuse");
	int lightspecularHandle = glGetUniformLocation(id, "light_specular");
	if ( lightambientHandle == -1 || lightdiffuseHandle == -1 || lightspecularHandle == -1) {
    fprintf(stderr, "Error: can't find light uniform variables\n");
		return 1;
  }

	float lightambient[3] = { 0.2f, 0.2f, 0.2f };	// ambient light components
  float lightdiffuse[3] = { 0.8f, 0.8f, 0.8f };	// diffuse light components
  float lightspecular[3] = { 1.0f, 1.0f, 1.0f };	// specular light components

	glUniform3fv(lightambientHandle, 1, lightambient);
	glUniform3fv(lightdiffuseHandle, 1, lightdiffuse);
	glUniform3fv(lightspecularHandle, 1, lightspecular);

	return 0;	// return success
}


void loadView(unsigned int pID){
  if (Camera == FollowCam)
    Camera->follow(obj[planet_to_follow]->position, heightOnOrbitPlane, zoom);
  else
    Camera->update(Input);

  // First load the viewing matrix from the current camera
  glm::mat4 viewMatrix;
  viewMatrix = Camera->getViewMtx();

  // Load it to the shader program
  int viewHandle = glGetUniformLocation(pID, "view");
  if (viewHandle == -1) {
    std::cout << "Uniform: view is not an active uniform label\n";
  }
  glUniformMatrix4fv( viewHandle, 1, false, glm::value_ptr(viewMatrix) );
}

void renderSkybox(){
  glUseProgram(skyboxProgramID);
  loadView(skyboxProgramID);

  glDepthMask(GL_FALSE);
  glBindVertexArray(skyboxVaoHandle);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthMask(GL_TRUE);
}

void render(float dt)
{
  // Update the camera, and draw the scene.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  renderSkybox();

  glUseProgram(programID);
  loadView(programID);

  int modelHandle = glGetUniformLocation(programID, "model_matrix");
  int normHandle = glGetUniformLocation(programID, "normal_matrix");
  int lightposHandle = glGetUniformLocation(programID, "light_pos");
	if (modelHandle == -1 || normHandle==-1 || lightposHandle == -1) {
    fprintf(stderr, "Error: can't find matrix uniforms\n");
		exit(1);
  }

  // Update the light position
  float lightPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  glUniform4fv(lightposHandle, 1, lightPos);

  // Texture unit 0 is diffuse, texture 1 is specular
  int diffHandle = glGetUniformLocation(programID, "diffMap");
  int specHandle = glGetUniformLocation(programID, "specMap");
  int mtlambientHandle = glGetUniformLocation(programID, "mtl_ambient");
  if (diffHandle == -1 || specHandle == -1 || mtlambientHandle == -1) {
    fprintf(stderr, "Error: can't find texture maps\n");
    exit(1);
  }
  glUniform1i(diffHandle, 0);
  glUniform1i(specHandle, 1);
  glUniform3fv(mtlambientHandle, 1, mtlambientSun);

  // Call particle system routine
  computePosVel(dt);

  glm::mat4 modelMatrix;
  glm::mat3 normMatrix;

  glUseProgram(sunProgramID);
  loadView(sunProgramID);

  int proceduralModelHandle = glGetUniformLocation(sunProgramID, "model_matrix");
  if (proceduralModelHandle == -1) {
    fprintf(stderr, "Error: can't find procedural model handle maps\n");
    exit(1);
  }

  // draw the Sun first
  glBindVertexArray(objectVaoHandle[NUM_OBJECTS-1]);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture( GL_TEXTURE_2D, sunTexID[0] );
  glActiveTexture(GL_TEXTURE1);
  glBindTexture( GL_TEXTURE_2D, sunTexID[1] );
  modelMatrix = glm::translate(modelMatrix, glm::vec3(obj[NUM_OBJECTS-1]->position.x, obj[NUM_OBJECTS-1]->position.y, obj[NUM_OBJECTS-1]->position.z));
  modelMatrix = glm::rotate(modelMatrix, timeSpeed * rotationSpeed * present_time, glm::vec3(0.0f, 1.0f, 0.0f));
  glUniformMatrix4fv(proceduralModelHandle, 1, false, glm::value_ptr(modelMatrix));
  glUniformMatrix4fv(modelHandle, 1, false, glm::value_ptr(modelMatrix));
  normMatrix = glm::mat3(modelMatrix);
  glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));
  glDrawElements(GL_TRIANGLES, obj[NUM_OBJECTS-1]->indCount, GL_UNSIGNED_INT, 0);

  glUseProgram(programID);
  glUniform3fv(mtlambientHandle, 1, mtlambientPlanets);


  // then draw planets
  for (int i=0; i < NUM_OBJECTS-1; i++){
    glBindVertexArray(objectVaoHandle[i]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, planetTexID[i*2] );
    glActiveTexture(GL_TEXTURE1);
    glBindTexture( GL_TEXTURE_2D, planetTexID[i*2+1] );

    glm::mat4 modelMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(obj[i]->position.x, obj[i]->position.y, obj[i]->position.z));
    modelMatrix = glm::rotate(modelMatrix, timeSpeed * rotationSpeed * present_time, glm::vec3(0.0f, 1.0f, 0.0f));
    normMatrix = glm::mat3(modelMatrix);
    glUniformMatrix4fv(modelHandle, 1, false, glm::value_ptr(modelMatrix));
    glUniformMatrix3fv(normHandle, 1, false, glm::value_ptr(normMatrix));

    glDrawElements(GL_TRIANGLES, obj[i]->indCount, GL_UNSIGNED_INT, 0);
  }

  glBindVertexArray(0);

}

/**
 * Error callback for GLFW. Simply prints error message to stderr.
 */
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

int main (int argc, char **argv)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
      exit(1);
  }

  // Specify that we want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create the window and OpenGL context
  window = glfwCreateWindow(winX, winY, "Modelling and viewing", NULL, NULL);
  if (!window)
  {
      glfwTerminate();
      exit(1);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(1);
	}

	// Set up the shaders we are to use. 0 indicates error.
  std::cout << "Loading mview shaders..." << std::endl;
	programID = LoadShaders("mview.vert", "mview.frag");
	if (programID == 0) {
		exit(1);
  }

  // Load uniform variables for Mview shader
  setupMviewShader(programID);

  // Set up skybox shaders
  std::cout << "Loading skybox shaders..." << std::endl;
  skyboxProgramID = LoadShaders("skybox.vert", "skybox.frag");
  if (skyboxProgramID == 0) {
    exit(1);
  }

  // Set up skybox shaders
  std::cout << "Loading procedural shaders..." << std::endl;
  proceduralProgramID = LoadShaders("procedural.vert", "procedural.frag");
  if (proceduralProgramID == 0) {
    exit(1);
  }

  // Set OpenGL state we need for this application.
  glClearColor(0.5F, 0.5F, 0.5F, 0.0F);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(programID);

  // Set up projection
  setProjection(programID);

  glUseProgram(skyboxProgramID);
  // Set up projection
  setProjection(skyboxProgramID);
  // Load texture for skybox
  skyboxTexID = loadCubemap();
  setupSkybox();

  // Set up projection
  glUseProgram(proceduralProgramID);
  setProjection(proceduralProgramID);

  // Set up solar system data
  setupSolarSystem();

  WorldCam = new WorldObjectViewer( cameraPos );
  ObjCam = new ObjectViewer( cameraPos );
  FollowCam = new FollowViewer(glm::vec3(0.0f, heightOnOrbitPlane, 0.0f));
  Camera = ObjCam;

  // Define callback functions and start main loop
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_pos_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetFramebufferSizeCallback(window, reshape_callback);

  sunProgramID = proceduralProgramID;

  if (strcmp(soundMode.c_str(), "ON") == 0){
    system("afplay halo3soundtrack.mp3 &");
  }

  last_time = static_cast<long int>(std::chrono::duration_cast<std::chrono::nanoseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count());

  while (!glfwWindowShouldClose(window))
  {
      present_time = static_cast<long int>(std::chrono::duration_cast<std::chrono::nanoseconds>
                    (std::chrono::high_resolution_clock::now().time_since_epoch()).count());
      float dt = timeSpeed * float(present_time - last_time);
      render(dt);

      glfwSwapBuffers(window);
      glfwPollEvents();
      last_time = present_time;
  }

  if (strcmp(soundMode.c_str(), "ON") == 0){
    system("killall afplay");
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(0);

  return 0;
}
