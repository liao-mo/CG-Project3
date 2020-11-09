#pragma once
#include <iostream>
#include <glad/glad.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#include "Utilities/3DUtils.H"
#include "Utilities/Pnt3f.H"

#include "Utilities/Matrices.h"
#include "Utilities/objloader.hpp"
#include <vector>
#include "Utilities/ArcBallCam.H"

using namespace std;
using namespace glm;


//my funtions



//find the qt with different spline method and given points, parameter t
Pnt3f find_qt(int spline_type, vector<Pnt3f> points, float t) {
	Pnt3f qt;

	//geometric constraints matrix
	Matrix4 G(
		points[0].x, points[0].y, points[0].z, 1,
		points[1].x, points[1].y, points[1].z, 1,
		points[2].x, points[2].y, points[2].z, 1,
		points[3].x, points[3].y, points[3].z, 1);


	//Cardinal Cubic matrix
	Matrix4 M1(-1, 3, -3, 1, 2, -5, 4, -1, -1, 0, 1, 0, 0, 2, 0, 0);
	M1 = (1.0 / 2.0) * M1;

	//Cubic B spline matrix
	Matrix4 M2(-1, 3, -3, 1, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0);
	M2 = (1.0 / 6.0) * M2;

	//parameter matrix
	Vector4 T(t * t * t, t * t, t, 1);

	Vector4 temp_qt;

	if (spline_type == 1) {
		//linear
		qt = (1 - t) * points[1] + t * points[2];
	}
	else if (spline_type == 2) {
		//Cardinal Cubic
		temp_qt = G * M1 * T;
		qt.x = temp_qt.x;
		qt.y = temp_qt.y;
		qt.z = temp_qt.z;
	}
	else if (spline_type == 3) {
		//Cubic B spline
		temp_qt = G * M2 * T;
		qt.x = temp_qt.x;
		qt.y = temp_qt.y;
		qt.z = temp_qt.z;
	}
	return qt;
}

//find two qt for some reason
vector<Pnt3f> find_two_qt(int spline_type, vector<Pnt3f> points, float t) {
	float t0 = t;
	float t1 = t0 + 0.01;
	
	Pnt3f qt0, qt1;


	//geometric constraints matrix
	Matrix4 G(
		points[0].x, points[0].y, points[0].z, 1,
		points[1].x, points[1].y, points[1].z, 1,
		points[2].x, points[2].y, points[2].z, 1,
		points[3].x, points[3].y, points[3].z, 1);

	//Cardinal Cubic matrix
	Matrix4 M1(-1, 3, -3, 1, 2, -5, 4, -1, -1, 0, 1, 0, 0, 2, 0, 0);
	M1 = (1.0 / 2.0) * M1;

	//Cubic B spline matrix
	Matrix4 M2(-1, 3, -3, 1, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0);
	M2 = (1.0 / 6.0) * M2;

	//parameter matrix
	Vector4 T0(t0 * t0 * t0, t0 * t0, t0, 1);
	Vector4 T1(t1 * t1 * t1, t1 * t1, t1, 1);

	Vector4 temp_qt0;
	Vector4 temp_qt1;

	if (spline_type == 1) {
		//linear
		qt0 = (1 - t0) * points[1] + t0 * points[2];
		qt1 = (1 - t1) * points[1] + t1 * points[2];
	}
	else if (spline_type == 2) {
		//Cardinal Cubic
		temp_qt0 = G * M1 * T0;
		qt0.x = temp_qt0.x;
		qt0.y = temp_qt0.y;
		qt0.z = temp_qt0.z;

		temp_qt1 = G * M1 * T1;
		qt1.x = temp_qt1.x;
		qt1.y = temp_qt1.y;
		qt1.z = temp_qt1.z;
	}
	else if (spline_type == 3) {
		//Cubic B spline
		temp_qt0 = G * M2 * T0;
		qt0.x = temp_qt0.x;
		qt0.y = temp_qt0.y;
		qt0.z = temp_qt0.z;

		temp_qt1 = G * M2 * T1;
		qt1.x = temp_qt1.x;
		qt1.y = temp_qt1.y;
		qt1.z = temp_qt1.z;
	}
	
	vector<Pnt3f> output = { qt0 ,qt1 };
	return output;

}

//find the orient vector
Pnt3f find_orient(Pnt3f O1, Pnt3f O2, float t) {
	Pnt3f output;
	output = (1 - t) * O1 + t * O2;
	return output;
}

//find the orient vecotr with spline method and given orients, t
Pnt3f spline_orient(vector<Pnt3f> orients, float t) {
	Pnt3f output_orient;

	//geometric constraints matrix
	Matrix4 G(
		orients[0].x, orients[0].y, orients[0].z, 1,
		orients[1].x, orients[1].y, orients[1].z, 1,
		orients[2].x, orients[2].y, orients[2].z, 1,
		orients[3].x, orients[3].y, orients[3].z, 1);


	//Cardinal Cubic matrix
	Matrix4 M1(-1, 3, -3, 1, 2, -5, 4, -1, -1, 0, 1, 0, 0, 2, 0, 0);
	M1 = (1.0 / 2.0) * M1;

	//Cubic B spline matrix
	//Matrix4 M2(-1, 3, -3, 1, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0);
	//M2 = (1.0 / 6.0) * M2;

	//parameter matrix
	Vector4 T(t * t * t, t * t, t, 1);

	Vector4 temp_qt;

		temp_qt = G * M1 * T;
		output_orient.x = temp_qt.x;
		output_orient.y = temp_qt.y;
		output_orient.z = temp_qt.z;

	return output_orient;
}

// Returns a quaternion such that q*start = dest
quat RotationBetweenVectors(vec3 start, vec3 dest) {
	start = normalize(start);
	dest = normalize(dest);

	float cosTheta = dot(start, dest);
	vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f) {
		// special case when vectors in opposite directions :
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		// This implementation favors a rotation around the Up axis,
		// since it's often what you want to do.
		rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
		if (length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
			rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return angleAxis(glm::radians(180.0f), rotationAxis);
	}

	// Implementation from Stan Melax's Game Programming Gems 1 article
	rotationAxis = cross(start, dest);

	float s = sqrt((1 + cosTheta) * 2);
	float invs = 1 / s;

	return quat(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);


}

// Returns a quaternion that will make your object looking towards 'direction'.
// Similar to RotationBetweenVectors, but also controls the vertical orientation.
// This assumes that at rest, the object faces +Z.
// Beware, the first parameter is a direction, not the target point !
quat my_LookAt(vec3 direction, vec3 desiredUp) {

	if (length2(direction) < 0.0001f)
		return quat();

	// Recompute desiredUp so that it's perpendicular to the direction
	// You can skip that part if you really want to force desiredUp
	vec3 right = cross(direction, desiredUp);
	desiredUp = cross(right, direction);

	// Find the rotation between the front of the object (that we assume towards +Z,
	// but this depends on your model) and the desired direction
	quat rot1 = RotationBetweenVectors(vec3(0.0f, 0.0f, 1.0f), direction);
	// Because of the 1rst rotation, the up is probably completely screwed up. 
	// Find the rotation between the "up" of the rotated object, and the desired up
	vec3 newUp = rot1 * vec3(0.0f, 1.0f, 0.0f);
	quat rot2 = RotationBetweenVectors(newUp, desiredUp);

	// Apply them
	return rot2 * rot1; // remember, in reverse order.
}

// Like SLERP, but forbids rotation greater than maxAngle (in radians)
// In conjunction to LookAt, can make your characters 
quat RotateTowards(quat q1, quat q2, float maxAngle) {

	if (maxAngle < 0.001f) {
		// No rotation allowed. Prevent dividing by 0 later.
		return q1;
	}

	float cosTheta = dot(q1, q2);

	// q1 and q2 are already equal.
	// Force q2 just to be sure
	if (cosTheta > 0.9999f) {
		return q2;
	}

	// Avoid taking the long path around the sphere
	if (cosTheta < 0) {
		q1 = q1 * -1.0f;
		cosTheta *= -1.0f;
	}

	float angle = acos(cosTheta);

	// If there is only a 2?difference, and we are allowed 5?
	// then we arrived.
	if (angle < maxAngle) {
		return q2;
	}

	// This is just like slerp(), but with a custom t
	float t = maxAngle / angle;
	angle = maxAngle;

	quat res = (sin((1.0f - t) * angle) * q1 + sin(t * angle) * q2) / sin(angle);
	res = normalize(res);
	return res;

}