#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "GL/glu.h"
#include "Utilities/objloader.hpp"
#include "string"


class TrainModel
{
public:
	TrainModel() {
		bool res = loadOBJ("none", vertices, uvs, normals);
	}
	TrainModel(std::string file_name) :f_name(file_name) {
		bool res = loadOBJ(f_name.c_str(), vertices, uvs, normals);
	}
	

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals; // Won't be used at the moment.
	std::string f_name;

};
