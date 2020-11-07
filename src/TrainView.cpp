/************************************************************************
	 File:        TrainView.cpp

	 Author:
				  Michael Gleicher, gleicher@cs.wisc.edu

	 Modifier
				  Yu-Chi Lai, yu-chi@cs.wisc.edu

	 Comment:
						The TrainView is the window that actually shows the
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within
						a TrainWindow
						that is the outer window with all the widgets.
						The TrainView needs
						to be aware of the window - since it might need to
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know
						about it (beware circular references)

	 Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#include "GL/glu.h"

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"


#include "Utilities/Matrices.h"
#include "Utilities/objloader.hpp"
#include "My_functions.H"
#include <vector>


using namespace std;

#ifdef EXAMPLE_SOLUTION
#	include "TrainExample/TrainExample.H"
#endif

#define DIVIDE_LINE 100
#define DEBUG


//find 4 control point around current point
std::vector<Pnt3f> TrainView::find_Cpoints(int currentPoint) {
	Pnt3f cp_pos_p0;
	if (currentPoint == 0) {
		cp_pos_p0 = m_pTrack->points[(m_pTrack->points.size() - 1)].pos;
	}
	else {
		cp_pos_p0 = m_pTrack->points[(currentPoint - 1)].pos;
	}
	Pnt3f cp_pos_p1 = m_pTrack->points[currentPoint].pos;
	Pnt3f cp_pos_p2 = m_pTrack->points[(currentPoint + 1) % m_pTrack->points.size()].pos;
	Pnt3f cp_pos_p3 = m_pTrack->points[(currentPoint + 2) % m_pTrack->points.size()].pos;

	std::vector<Pnt3f> output = { cp_pos_p0, cp_pos_p1, cp_pos_p2, cp_pos_p3 };
	return output;

}



//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l)
	: Fl_Gl_Window(x, y, w, h, l), my_train("./TrackFiles/cube.obj")
	//========================================================================
{
	mode(FL_RGB | FL_ALPHA | FL_DOUBLE | FL_STENCIL);

	resetArcball();


}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event))
			return 1;

	// remember what button was used
	static int last_push;

	switch (event) {
		// Mouse button being pushed event
	case FL_PUSH:
		last_push = Fl::event_button();
		// if the left button be pushed is left mouse button
		if (last_push == FL_LEFT_MOUSE) {
			doPick();
			damage(1);
			return 1;
		};
		break;

		// Mouse button release event
	case FL_RELEASE: // button release
		damage(1);
		last_push = 0;
		return 1;

		// Mouse button drag event
	case FL_DRAG:

		// Compute the new control point position
		if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
			ControlPoint* cp = &m_pTrack->points[selectedCube];

			double r1x, r1y, r1z, r2x, r2y, r2z;
			getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

			double rx, ry, rz;
			mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
				static_cast<double>(cp->pos.x),
				static_cast<double>(cp->pos.y),
				static_cast<double>(cp->pos.z),
				rx, ry, rz,
				(Fl::event_state() & FL_CTRL) != 0);

			cp->pos.x = (float)rx;
			cp->pos.y = (float)ry;
			cp->pos.z = (float)rz;
			damage(1);
		}
		break;

		// in order to get keyboard events, we need to accept focus
	case FL_FOCUS:
		return 1;

		// every time the mouse enters this window, aggressively take focus
	case FL_ENTER:
		focus(this);
		break;

	case FL_KEYBOARD:
		int k = Fl::event_key();
		int ks = Fl::event_state();
		if (k == 'p') {
			// Print out the selected control point information
			if (selectedCube >= 0)
				printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
					selectedCube,
					m_pTrack->points[selectedCube].pos.x,
					m_pTrack->points[selectedCube].pos.y,
					m_pTrack->points[selectedCube].pos.z,
					m_pTrack->points[selectedCube].orient.x,
					m_pTrack->points[selectedCube].orient.y,
					m_pTrack->points[selectedCube].orient.z);
			else
				printf("Nothing Selected\n");

			return 1;
		};
		break;
	}

	return Fl_Gl_Window::handle(event);
}

//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		const char* vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
			"}\0";

		const char* fragmentShaderSource = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor = vec4(0.317, 0.941, 0.490, 1.0);\n"
			"}\0";

		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		int  success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		unsigned int fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::LINK_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);


		float vertices[] = {
		0.5f,  0.5f, 0.0f,  // top right
		0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
		};

		unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
		};

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


	}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	// Set up the view port
	glViewport(0, 0, w(), h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[] = { 0,1,1,0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { 1, 0, 0, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .1f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);



	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	setupFloor();
	glDisable(GL_LIGHTING);
	drawFloor(200, 10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();

	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		}
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
	}
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	
	else {
		
		int Cp = m_pTrack->trainU / 1;

		vector<Pnt3f> points = find_Cpoints(Cp);

		float t0 = m_pTrack->trainU - Cp;
		float t1 = t0 + 0.01;
		if (t1 >= 1) {
			t1 = 1;
		}

		vector<Pnt3f> qts = find_two_qt(tw->splineBrowser->value(), points, t0);


		glm::vec3 qt0_v(qts[0].x, qts[0].y, qts[0].z);
		glm::vec3 qt1_v(qts[1].x, qts[1].y, qts[1].z);
		glm::vec3 forward = qt1_v - qt0_v;
		forward = glm::normalize(forward);


		Pnt3f cp_orient_p0 = m_pTrack->points[Cp].orient;
		Pnt3f cp_orient_p1 = m_pTrack->points[(Cp + 1) % m_pTrack->points.size()].orient;

		Pnt3f orient_t0 = find_orient(cp_orient_p0, cp_orient_p1, t0);
		Pnt3f orient_t1 = find_orient(cp_orient_p0, cp_orient_p1, t1);

		glm::vec3 orient_t0_v(orient_t0.x, orient_t0.y, orient_t0.z);
		orient_t0_v = glm::normalize(orient_t0_v);
		glm::vec3 orient_t1_v(orient_t1.x, orient_t1.y, orient_t1.z);
		orient_t1_v = glm::normalize(orient_t1_v);
		
#ifdef DEBUG
		cout << "----------------------------------" << endl;
		cout << "t0: " << t0 << " t1: " << t1 << endl;
		cout << "qts[0]: " << qts[0].x << " " << qts[0].y << " " << qts[0].z << endl;
		cout << "qts[1]: " << qts[1].x << " " << qts[1].y << " " << qts[1].z << endl;
		cout << "orient1: " << orient_t0_v.x << " " << orient_t0_v.y << " " << orient_t0_v.z << endl;
		cout << "orient2: " << orient_t1_v.x << " " << orient_t1_v.y << " " << orient_t1_v.z << endl;
		cout << "forward: " << forward.x << " " << forward.y << " " << forward.z << endl;
		
#endif // DEBUG
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(120, 1, 1, 200); 
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(
			qt0_v.x + orient_t0_v.x * 7,
			qt0_v.y + orient_t0_v.y * 7,
			qt0_v.z + orient_t0_v.z * 7,
			qt0_v.x + forward.x + orient_t1_v.x * 7,
			qt0_v.y + forward.y + orient_t1_v.y * 7,
			qt0_v.z + forward.z + orient_t1_v.z * 7,
			orient_t1.x, 
			orient_t1.y, 
			orient_t1.z
		);



#ifdef EXAMPLE_SOLUTION
		trainCamView(this, aspect);
#endif
	}
}


//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################
	draw_track(doingShadows);
	


#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
	draw_train(doingShadows);

	// draw our first triangle
	//glUseProgram(shaderProgram);
	//glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	


#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();

	// where is the mouse?
	int mx = Fl::event_x();
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((double)mx, (double)(viewport[3] - my),
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
		glLoadName((GLuint)(i + 1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n", selectedCube);
}

void TrainView::draw_train(bool doingShadows) {
	if (!tw->trainCam->value()) {
		int Cp = m_pTrack->trainU / 1;

		vector<Pnt3f> points = find_Cpoints(Cp);

		float t0 = m_pTrack->trainU - Cp;
		float t1 = t0 + 0.01;
		if (t1 >= 1) {
			t1 = 1;
		}

		vector<Pnt3f> qts = find_two_qt(tw->splineBrowser->value(), points, t0);


		glm::vec3 qt0_v(qts[0].x, qts[0].y, qts[0].z);
		glm::vec3 qt1_v(qts[1].x, qts[1].y, qts[1].z);
		glm::vec3 forward = qt1_v - qt0_v;
		forward = glm::normalize(forward);


		Pnt3f cp_orient_p0 = m_pTrack->points[Cp].orient;
		Pnt3f cp_orient_p1 = m_pTrack->points[(Cp + 1) % m_pTrack->points.size()].orient;

		Pnt3f orient_t0 = find_orient(cp_orient_p0, cp_orient_p1, t0);
		Pnt3f orient_t1 = find_orient(cp_orient_p0, cp_orient_p1, t1);

		glm::vec3 orient_t0_v(orient_t0.x, orient_t0.y, orient_t0.z);
		orient_t0_v = glm::normalize(orient_t0_v);
		glm::vec3 orient_t1_v(orient_t1.x, orient_t1.y, orient_t1.z);
		orient_t1_v = glm::normalize(orient_t1_v);



		unsigned r = 100;
		unsigned g = 120;
		unsigned b = 140;
		if (!doingShadows) {
			glColor3ub(r, g, b);
		}

		glm::mat4 scale = glm::mat4(1.0f);
		scale = glm::scale(scale, glm::vec3(5, 5, 5));

		quat MyQuaternion = my_LookAt(forward, orient_t0_v);

		mat4 RotationMatrix = glm::toMat4(MyQuaternion);

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(trans, qt0_v);
		trans = glm::translate(trans, 10.0f* orient_t0_v);

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < my_train.vertices.size(); ++i) {
			
			//glColor3f(0.5, 1, 0.8);
			glm::vec4 vec(my_train.vertices[i].x, my_train.vertices[i].y, my_train.vertices[i].z, 1.0f);

			
			

			

			vec = trans * RotationMatrix *scale * vec;

			glVertex3f(vec.x, vec.y, vec.z);
			//cout << vec.x << " " << vec.y << " " << vec.z << endl;

		}
		glEnd();


		//glBegin(GL_QUADS);
		//glColor3f(1.0, 1.0, 0.0);
		//glTexCoord2f(0.0f, 0.0f);
		//glVertex3f(qt.x - 5, qt.y - 5, qt.z - 5);
		//glTexCoord2f(1.0f, 0.0f);
		//glVertex3f(qt.x + 5, qt.y - 5, qt.z - 5);
		//glTexCoord2f(1.0f, 1.0f);
		//glVertex3f(qt.x + 5, qt.y + 5, qt.z - 5);
		//glTexCoord2f(0.0f, 1.0f);
		//glVertex3f(qt.x - 5, qt.y + 5, qt.z - 5);
		//glEnd();
		//cout << qt.x << " " << qt.y << " " << qt.z << endl;
	}
}

void TrainView::draw_track(bool doingShadows) {
	for (int i = 0; i < m_pTrack->points.size(); ++i) {

		Pnt3f cp_pos_p0;
		if (i == 0) {
			cp_pos_p0 = m_pTrack->points[(m_pTrack->points.size() - 1)].pos;
		}
		else {
			cp_pos_p0 = m_pTrack->points[(i - 1)].pos;
		}
		Pnt3f cp_pos_p1 = m_pTrack->points[i].pos;
		Pnt3f cp_pos_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].pos;
		Pnt3f cp_pos_p3 = m_pTrack->points[(i + 2) % m_pTrack->points.size()].pos;

		Pnt3f cp_orient_p1 = m_pTrack->points[i].orient;
		Pnt3f cp_orient_p2 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;

		vector<Pnt3f> points = { cp_pos_p0, cp_pos_p1, cp_pos_p2, cp_pos_p3 };

		float percent = 1.0f / DIVIDE_LINE;
		float t = 0;


		int spline_type = tw->splineBrowser->value();
		Pnt3f qt = find_qt(spline_type, points, t);

		Pnt3f qt0;
		Pnt3f qt1;
		Pnt3f orient_t;



		for (size_t j = 0; j < DIVIDE_LINE; j++) {
			qt0 = find_qt(spline_type, points, t - 0.005);;

			//linear interpolation orient
			orient_t = (1 - t) * cp_orient_p1 + t * cp_orient_p2;

			t += percent;

			qt = find_qt(spline_type, points, t);
			qt1 = qt;

			glm::vec3 qt0_vec(qt0.x, qt0.y, qt0.z);
			glm::vec3 qt1_vec(qt1.x, qt1.y, qt1.z);
			glm::vec3 forward_vec(qt1.x - qt0.x, qt1.y - qt0.y, qt1.z - qt0.z);
			glm::vec3 orient_vec(orient_t.x, orient_t.y, orient_t.z);
			glm::vec3 offset_vec1 = glm::cross(forward_vec, orient_vec);
			offset_vec1 = glm::normalize(offset_vec1);
			offset_vec1 *= 1.5 * DIVIDE_LINE / 100;
			glm::vec3 offset_vec2 = 2.0f * offset_vec1;


			glm::vec3 left_track0 = qt0_vec + offset_vec1;
			glm::vec3 left_track1 = qt1_vec + offset_vec1;
			glm::vec3 left_track2 = qt1_vec + offset_vec2;
			glm::vec3 left_track3 = qt0_vec + offset_vec2;
			glm::vec3 right_track0 = qt0_vec - offset_vec1;
			glm::vec3 right_track1 = qt1_vec - offset_vec1;
			glm::vec3 right_track2 = qt1_vec - offset_vec2;
			glm::vec3 right_track3 = qt0_vec - offset_vec2;

			glLineWidth(5);

			//single track
			//glBegin(GL_LINES);
			//if (!doingShadows) {
			//	glColor3ub(32, 32, 64);
			//}
			//glVertex3f(qt0.x, qt0.y, qt0.z);
			//glVertex3f(qt1.x, qt1.y, qt1.z);
			//glEnd();

			//left track
			glBegin(GL_POLYGON);
			unsigned r = 123;
			unsigned g = 200;
			unsigned b = 30;
			if (!doingShadows) {
				glColor3ub(r,g,b);
			}
			glVertex3f(left_track0.x, left_track0.y, left_track0.z);
			glVertex3f(left_track1.x, left_track1.y, left_track1.z);
			glVertex3f(left_track2.x, left_track2.y, left_track2.z);
			glVertex3f(left_track3.x, left_track3.y, left_track3.z);
			glEnd();

			//right track
			glBegin(GL_POLYGON);
			if (!doingShadows) {
				glColor3ub(r, g, b);
			}
			glVertex3f(right_track0.x, right_track0.y, right_track0.z);
			glVertex3f(right_track1.x, right_track1.y, right_track1.z);
			glVertex3f(right_track2.x, right_track2.y, right_track2.z);
			glVertex3f(right_track3.x, right_track3.y, right_track3.z);
			glEnd();



			glLineWidth(1);
		}
	}
}