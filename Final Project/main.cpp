#pragma once
// Math constants
#define _USE_MATH_DEFINES
#include <cmath>  
#include <random>

// Std. Includes
#include <string>
#include <time.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include "glm/ext.hpp"
#include "glm/ext.hpp"

// Other Libs
#include "SOIL2/SOIL2.h"

// project includes
#include "Application.h"
#include "Shader.h"
#include "Mesh.h"
#include "Body.h"
#include "Particle.h"
#include "RigidBody.h"
#include "Sphere.h"


// time
GLfloat t = 0.0f;
const GLfloat deltaTime = 1.0f/60.0f;
GLfloat currentTime = (GLfloat)glfwGetTime();
GLfloat accumulator = 0.0f;

// Gravity constant
glm::vec3 g = glm::vec3(0.0f, -9.8f, 0.0f);
Gravity* fgravity = new Gravity(g);


// main function
int main()
{
	// create application
	Application app = Application::Application();
	app.initRender();
	Application::camera.setCameraPosition(glm::vec3(0.0f, 5.0f, 20.0f));

	// Create table
	Mesh table = Mesh::Mesh(Mesh::QUAD);
	// Table size is 30x30
	table.scale(glm::vec3(30.0f, 0.0f, 30.0f));
	Shader lambert = Shader("resources/shaders/physics.vert", "resources/shaders/physics.frag");
	table.setShader(lambert);

	// Array of spheres
	Sphere spheres[5];
	int spheresNumber = 5;
	for (int i = 0; i < spheresNumber; i++)
	{
		Sphere s = Sphere::Sphere();
		Shader sShader = Shader("resources/shaders/physics.vert ", "resources/shaders/physics.frag ");
		s.getMesh().setShader(sShader);
		s.setMass(1.0f);
		// Set random initial linear velocity
		int randomX = rand() % 4000 - 2000;
		int randomZ = rand() % 4000 - 2000;
		float vX = (float)randomX / 100.0f;
		float vZ = (float)randomZ / 100.0f;
		s.setVel(glm::vec3(vX, 0.0f, vZ));
		// Set random initial position
		randomX = rand() % 3000 - 1500;
		randomZ = rand() % 3000 - 1500;
		float rX = (float)randomX / 100.0f;
		float rZ = (float)randomZ / 100.0f;
		s.setPos(glm::vec3(rX, 1.0f, rZ));
		// Check that the sphere does not overlap with any of the other spheres
		for (Sphere sp : spheres)
		{
			float centreDistances = glm::sqrt(glm::pow(sp.getPos().x - s.getPos().x,2)+glm::pow(sp.getPos().z - s.getPos().z,2));
			float radiusDistances = s.getRadius() + sp.getRadius();
			// If the distance between the spheres is less or equal than the sum of the radius there is overlap
			if (centreDistances <= radiusDistances)
			{
				int randomX = rand() % 3000 - 1500;
				int randomZ = rand() % 3000 - 1500;
				float rX = (float)randomX / 100.0f;
				float rZ = (float)randomZ / 100.0f;
				s.setPos(glm::vec3(rX, 1.0f, rZ));
			}
		}
		// Add sphere to array
		spheres[i] = s;
	}

	// impulse elements
	float e = 0.6f;
	bool collision = false;
	int vertexCount = 0;
	glm::vec3 verticesInCollision[4];
	float frictionCoefficient = 0.25f;
	
	// Game loop
	while (!glfwWindowShouldClose(app.getWindow()))
	{
		// Set frame time
		GLfloat newTime = (GLfloat)glfwGetTime();
		GLfloat frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;

		/*
		**	INTERACTION
		*/
		// Manage interaction
		app.doMovement(deltaTime);


		/*
		**	SIMULATION
		*/
		while (accumulator >= deltaTime)
		{
			// integration (translation)
			for (Sphere& s : spheres)
			{
				s.setAcc(s.applyForces(s.getPos(), s.getVel(), t, deltaTime));
				s.setVel(s.getVel() + deltaTime * s.getAcc());
				s.translate(s.getVel() * deltaTime);
				// Collision with table
				if ((s.getPos().x + s.getRadius()) >= table.getPos().x || (s.getPos().x - s.getRadius()) <= table.getPos().x)
				{
					// r is the vector of the centre of mass and the point of collision
					glm::vec3 r = (s.getPos() - s.getPos();
					// n is the normal of the plane of collision, in this case the normal of the plane
					glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
					// vr is the relative velocity
					glm::vec3 vr = rb.getVel() + glm::cross(rb.getAngVel(), r);
					// vt is the direction of the tangencial impulse
					glm::vec3 vt = vr - (glm::dot(vr, n)*n);
					// calculate the normal impulse
					float jn = (-(1.0f + e) * glm::dot(vr, n)) / ((1 / rb.getMass()) + glm::dot(n, (glm::cross(rb.getInvInertia()* glm::cross(r, n), r))));
				}

			}

			// integration ( rotation )
			/*rb.setAngVel(rb.getAngVel() + deltaTime * rb.getAngAcc());
			// create skew symmetric matrix for w
			glm::mat3 angVelSkew = glm::matrixCross3(rb.getAngVel());
			// create 3x3 rotation matrix from rb rotation matrix
			glm::mat3 R = glm::mat3(rb.getRotate());
			// update rotation matrix
			R += deltaTime * angVelSkew *R;
			R = glm::orthonormalize(R);
			rb.setRotate(glm::mat4(R));

			// Check how many vertex are in collision with the plane 
			float lowestCoordinate = plane.getPos().y;
			for (auto vertex : rb.getMesh().getVertices())
			{
				glm::vec3 coordinates = rb.getMesh().getModel() * glm::vec4(vertex.getCoord(), 1.0f);
				if (coordinates.y <= plane.getPos().y)
				{
					verticesInCollision[vertexCount] = coordinates;
					vertexCount++;
					collision = true;
					// Set which coordinate is the lowest
					if (coordinates.y < lowestCoordinate)
					{
						lowestCoordinate = coordinates.y;
					}
				}
			}
			// Apply the collision
			if (collision)
			{
				//std::cout << "Colliding Vertices : " << '\n';
				glm::vec3 collisionPoint;
				// Vertex
				if (vertexCount == 1)
				{
					collisionPoint = verticesInCollision[0];
				}
				// Edge
				else if (vertexCount == 2)
				{
					collisionPoint = (verticesInCollision[0] + verticesInCollision[1]) / 2;
				}
				// Face
				else if (vertexCount == 4)
				{
					collisionPoint = (verticesInCollision[0] + verticesInCollision[1] + verticesInCollision[2] + verticesInCollision[3]) / 4;
				}
				// reset count and boolean
				vertexCount = 0;
				collision = false;
				// Translate the lowest vertex a bit so it does not overlap the plane
				rb.translate(glm::vec3(0.0f, glm::abs(plane.getPos().y - lowestCoordinate), 0.0f));
				// r is the vector of the centre of mass and the point of collision
				glm::vec3 r = collisionPoint - rb.getPos();
				// n is the normal of the plane of collision, in this case the normal of the plane
				glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
				// vr is the relative velocity
				glm::vec3 vr = rb.getVel() + glm::cross(rb.getAngVel(), r);
				// vt is the direction of the tangencial impulse
				glm::vec3 vt = vr - (glm::dot(vr, n)*n);
				// calculate the normal impulse
				float jn = (-(1.0f + e) * glm::dot(vr, n)) / ((1 / rb.getMass()) + glm::dot(n, (glm::cross(rb.getInvInertia()* glm::cross(r, n), r))));
				// calculate tangencial impulse
				glm::vec3 jt;
				if (vt == glm::vec3(0.0f, 0.0f, 0.0f))
				{
					jt = glm::vec3(0.0f, 0.0f, 0.0f);
				}
				else
				{
					jt = -frictionCoefficient * glm::abs(jn) * glm::normalize(vt);
				}
				// total impulse
				float j = glm::length(jt + (jn * n));
				// set the new velocities
				rb.setVel(rb.getVel() + (j / rb.getMass())*n);
				rb.setAngVel(rb.getAngVel() + j * rb.getInvInertia() * glm::cross(r, n));
				// dicrease the velocities to make the rigid body stop
				if (glm::length(rb.getVel()) < initialVelocity && glm::length(rb.getVel()) > 0.2f)
				{
					rb.setVel(rb.getVel() / 1.05f);
				}
				else if (glm::length(rb.getVel()) < 0.2f)
				{
					rb.setVel(glm::vec3(0.0f));
					rb.setAngVel(rb.getAngVel()/1.05f);
				}
				if (glm::length(rb.getAngVel()) < 0.08f)
				{
					rb.setAngVel(glm::vec3(0.0f));
				}

				// Update rotational matrix
				angVelSkew = glm::matrixCross3(rb.getAngVel());
				R = glm::mat3(rb.getRotate());
				R += deltaTime * angVelSkew *R;
				R = glm::orthonormalize(R);
				rb.setRotate(glm::mat4(R));
			}*/
			accumulator -= deltaTime;
			t += deltaTime;
		}

		/*
		**	RENDER
		*/
		// clear buffer
		app.clear();
		// draw groud plane
		app.draw(table);
		//draw the spheres
		for (Sphere s : spheres)
		{
			app.draw(s.getMesh());
		}

		app.display();
	}

	app.terminate();

	return EXIT_SUCCESS;
}

