
#include "glm/glm.hpp"

#ifndef SPHERE_H
#define SPHERE_H

class Sphere {
public:

	/**
	 * Generates a new sphere mesh with given radius, vertical sub-divisions and horizontal
	 * sub-divisions. Produces vertex positions, normal vectors, and element
	 * indices available for use in arrays.
	 * More sub-divisions generates a smoother sphere.
	 * It is a good idea to have vertical and horizontal divisions equal.
	 * @param radius, radius of sphere in generic units
	 * @param vertDiv, (min 1), number of vertical divisions (bands)
	 * @param horzDiv, (min 4), number of horizontal divisions (slices)
	 */
	Sphere(float radius = 1.0f, float mass = 1.0f,
				 glm::vec3 distanceFromCenter = glm::vec3(0.0f), glm::vec3 velocity = glm::vec3(0.0f),
	 			 int vertDiv = 16, int horzDiv = 16);

	float mass;
	glm::vec3 position;
	glm::vec3 velocity;
	float *vertices;		// Vertex position (x,y,z)
	float *normals;			// Normal vector (x,y,z)
  float *texcoords;       // Tex coords s, t
	unsigned int *indices;	// Element indices

	// Counts of array elements
	int vertCount;
	int normCount;
  int texCount;
	int indCount;

};

#endif
