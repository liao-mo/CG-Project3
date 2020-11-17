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


#define DEBUG


//find 4 control point around current point
std::vector<Pnt3f> TrainView::find_Cpoints(int currentPoint) {
#ifdef DEBUG
	
#endif // DEBUG

	Pnt3f cp_pos_p0;
	if (currentPoint == 0) {
		cp_pos_p0 = m_pTrack->points[(m_pTrack->points.size() - 1)].pos;
		//cout << "cp0: " << m_pTrack->points.size() - 1;
	}
	else {
		cp_pos_p0 = m_pTrack->points[(currentPoint - 1)].pos;
		//cout << "cp0: " << currentPoint - 1;
	}
	Pnt3f cp_pos_p1 = m_pTrack->points[currentPoint].pos;
	//cout << " cp1: " << currentPoint;
	Pnt3f cp_pos_p2 = m_pTrack->points[(currentPoint + 1) % m_pTrack->points.size()].pos;
	//cout << " cp2: " << (currentPoint + 1) % m_pTrack->points.size();
	Pnt3f cp_pos_p3 = m_pTrack->points[(currentPoint + 2) % m_pTrack->points.size()].pos;
	//cout << " cp3: " << (currentPoint + 2) % m_pTrack->points.size();
	//cout << endl;

	

	std::vector<Pnt3f> output = { cp_pos_p0, cp_pos_p1, cp_pos_p2, cp_pos_p3 };
	return output;

}

//find 4 orient vector around current point
std::vector<Pnt3f> TrainView::find_orient_vectors(int currentPoint) {
#ifdef DEBUG

#endif // DEBUG

	Pnt3f orient0;
	if (currentPoint == 0) {
		orient0 = m_pTrack->points[(m_pTrack->points.size() - 1)].orient;
	}
	else {
		orient0 = m_pTrack->points[(currentPoint - 1)].orient;
	}
	Pnt3f orient1 = m_pTrack->points[currentPoint].orient;
	Pnt3f orient2 = m_pTrack->points[(currentPoint + 1) % m_pTrack->points.size()].orient;
	Pnt3f orient3 = m_pTrack->points[(currentPoint + 2) % m_pTrack->points.size()].orient;

	std::vector<Pnt3f> output = { orient0, orient1, orient2, orient3 };
	return output;

}

// * Constructor to set up the GL window
TrainView::
TrainView(int x, int y, int w, int h, const char* l)
	: Fl_Gl_Window(x, y, w, h, l),
	my_train("./TrackFiles/train2.obj"),
	my_sleeper("./TrackFiles/sleeper.obj"),
	my_track("./TrackFiles/cilinder.obj"),
	my_car("./TrackFiles/cube.obj"),
	my_ground("./TrackFiles/ground.obj"),
	num_of_car(0)
	//========================================================================
{
	mode(FL_RGB | FL_ALPHA | FL_DOUBLE | FL_STENCIL);
	resetArcball();
	TrainModel airplane("./TrackFiles/airplane.obj");
	env_models.push_back(airplane);

	TrainModel tunnel("./TrackFiles/tunnel.obj");
	env_models.push_back(tunnel);
}


// * Reset the camera to look at the world
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}


// * FlTk Event handler for the window
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

	//update arc length data
	update_arcLengh();
	return Fl_Gl_Window::handle(event);
}

// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
void TrainView::draw()
{
	// * Set up basic opengl informaiton
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		
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
	update_arcLengh(); //initial arc length here

	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	if (tw->envLight->value() == 1) 	glEnable(GL_LIGHT0);
	else 	glDisable(GL_LIGHT0);



	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		if (tw->envLight->value() == 1) {
			glEnable(GL_LIGHT1);
			glEnable(GL_LIGHT2);
			glEnable(GL_LIGHT6);
		}
		else {
			glDisable(GL_LIGHT1);
			glDisable(GL_LIGHT2);
			glDisable(GL_LIGHT6);
		}
	}


	// * set the light parameters
	GLfloat lightPosition1[] = { 0, 5000, 5000, 0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { 10000, 5000, 0, 0 };
	GLfloat lightPosition3[] = { 5000, 10000, 5000, 0 };
	GLfloat lightPosition6[] = { -300, 300, -300, 1 };
	GLfloat yellowLight[] = { 0.3f, 0.3f, .05f, 1.0 };
	GLfloat whiteLight[] = { 0.8f, 0.8f, 0.8f, 1.0 }; 
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat whiteLight2[] = { .8f,.8f,.7f,1.0 };
	
	GLfloat grayLight[] = { .1f, .1f, .1f, 1.0 };
	float direction[] = { 1,-1,1 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);


	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	//spot light
	glLightfv(GL_LIGHT6, GL_AMBIENT, grayLight);
	glLightfv(GL_LIGHT6, GL_DIFFUSE, whiteLight2);
	glLightfv(GL_LIGHT6, GL_POSITION, lightPosition6);

	//spot direction
	glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, direction);
	//angle of the clone
	glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, 30.0);
	//concentration of the light
	glLightf(GL_LIGHT6, GL_SPOT_EXPONENT, 25.0);

	//light attenuation
	glLightf(GL_LIGHT6, GL_CONSTANT_ATTENUATION, 0.2f);
	glLightf(GL_LIGHT6, GL_LINEAR_ATTENUATION, 0.0f);
	glLightf(GL_LIGHT6, GL_QUADRATIC_ATTENUATION, 0.0f);

	set_train_light();

	// now draw the ground plane
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	glEnable(GL_LIGHTING);
	setupFloor();
	//glDisable(GL_LIGHTING);
	//drawFloor(1600, 50);

	draw_ground();

	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	glEnable(GL_LIGHTING);
	setupObjects();
	

	drawStuff();


	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}

	current_time = std::chrono::system_clock::now();
	elapsed_seconds = current_time - start_time;
}

// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
void TrainView::
setProjection()
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
	// put code for train view projection here!	
	else {
		size_t i;
		if (tw->arcLength->value() == 0) {
			i = trainU_index();
		}
		else {
			i = C_length_index();
		}
		float t0 = t_param[i];

		glm::vec3 qt0_v = all_qt[i];
		glm::vec3 qt1_v;
		if (i == t_param.size() - 1) qt1_v = all_qt[0];
		else qt1_v = all_qt[i + 1];

		glm::vec3 orient_t0_v = all_orient[i];
		glm::vec3 forward = all_forward[i];


		float FPV_up_value = 10.0f;
		float TPV_up_value = 20.0f;
		float TPV_backward_value = 30.0f;

		glm::vec4 eye(qt0_v.x, qt0_v.y, qt0_v.z, 1);
		glm::vec4 center(qt0_v.x + forward.x, qt0_v.y + forward.y, qt0_v.z + forward.z, 1);
		glm::vec3 offset0(orient_t0_v.x, orient_t0_v.y, orient_t0_v.z);
		glm::mat4 trans = glm::mat4(1.0f);

		
#ifdef DEBUG

		
#endif // DEBUG
		if (tw->FPV->value() == 1) {
			trans = glm::translate(trans, FPV_up_value * offset0);
			eye = trans * eye;

			center = trans * center;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(120, 1, 1, 1000);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(
				eye.x,
				eye.y,
				eye.z,
				center.x,
				center.y,
				center.z,
				orient_t0_v.x,
				orient_t0_v.y,
				orient_t0_v.z
			);
		}
		else if (tw->TPV->value() == 1) {
			trans = glm::translate(trans, TPV_up_value * offset0);
			trans = glm::translate(trans, TPV_backward_value * -forward);
			eye = trans * eye;

			center = trans * center;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(120, 1, 1, 1000);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(
				eye.x,
				eye.y,
				eye.z,
				center.x,
				center.y,
				center.z,
				orient_t0_v.x,
				orient_t0_v.y,
				orient_t0_v.z
			);
		}
		

	}
}



// * this draws all of the stuff in the world
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows

// TODO: 
// if you have other objects in the world, make sure to draw them
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
	draw_track(doingShadows);
	
	draw_sleeper(doingShadows);

	// draw the train
	draw_train(doingShadows);

	//draw env models
	draw_env_model(doingShadows);

	float first_offset = 20.0f;
	for (int i = 0; i < num_of_car; ++i) {
		draw_car(doingShadows, first_offset + 15.0 * i);
	}
}

// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
void TrainView::
doPick()
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
#ifdef DEBUG
	//cout << "====================" << endl;
#endif // DEBUG
	if (!(tw->trainCam->value() == 1 && tw->FPV->value() == 1)) {
		size_t i;
		if (tw->arcLength->value() == 0) {
			i = int(m_pTrack->trainU / 1) * DIVIDE_LINE;
			i = i + (m_pTrack->trainU - int(m_pTrack->trainU / 1)) * DIVIDE_LINE - 0.1;
			if (i < 0) {
				i = (float)m_pTrack->points.size() - i;
			}
		}
		else {
			i = C_length_index();
		}
		float t0 = t_param[i]; 

		glm::vec3 qt0_v = all_qt[i];
		glm::vec3 qt1_v;
		if (i == t_param.size() - 1) qt1_v = all_qt[0];
		else qt1_v = all_qt[i + 1];

		glm::vec3 orient_t0_v = all_orient[i];
		glm::vec3 forward = all_forward[i];


		float up_offset = 0.0f;
		float scale_value = 0.3f;

		//glm::mat4 scale = glm::mat4(1.0f);
		//scale = glm::scale(scale, glm::vec3(scale_value, scale_value, scale_value));

		mat4 rotate = glm::inverse(glm::lookAt(qt0_v, qt1_v, orient_t0_v));
		float rotationArray[16] = { 0.0 };
		const float* pSource2 = (const float*)glm::value_ptr(rotate);
		for (int i = 0; i < 16; ++i)
			rotationArray[i] = pSource2[i];

		//glm::mat4 trans = glm::mat4(1.0f);
		//trans = glm::translate(trans, qt0_v);
		//trans = glm::translate(trans, 10.0f* orient_t0_v);

		glPushMatrix();

		//glTranslated(up_offset * orient_t0_v.x, up_offset * orient_t0_v.y, up_offset * orient_t0_v.z);
		glMultMatrixf(rotationArray);
		glRotated(180, 0, 1, 0);
		glScalef(scale_value, scale_value, scale_value);

		unsigned int r = 200;
		unsigned int g = 200;
		unsigned int b = 200;
		if (!doingShadows) {
			glColor3ub(r, g, b);
		}

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < my_train.vertices.size(); ++i) {
			glm::vec4 vec(my_train.vertices[i].x, my_train.vertices[i].y, my_train.vertices[i].z, 1.0f);
			glNormal3d(my_train.normals[i].x, my_train.normals[i].y,  my_train.normals[i].z);
			glVertex3f(vec.x, vec.y, vec.z);
		}
		glEnd();
		glPopMatrix();
	}
}

void TrainView::draw_car(bool doingShadows, float backward_offset) {
	if (!(tw->trainCam->value() == 1 && tw->FPV->value() == 1)) {
		size_t i;

		
		i = length_to_index(m_pTrack->C_length - backward_offset);
		float t0 = t_param[i];

		glm::vec3 qt0_v = all_qt[i];
		glm::vec3 qt1_v;
		if (i == t_param.size() - 1) qt1_v = all_qt[0];
		else qt1_v = all_qt[i + 1];

		glm::vec3 orient_t0_v = all_orient[i];
		glm::vec3 forward = all_forward[i];

		float scale_value = 5.0;
		float up_offset = 5.0f;

		//glm::mat4 scale = glm::mat4(1.0f);
		//scale = glm::scale(scale, glm::vec3(scale_value, scale_value, scale_value));


		mat4 rotate = glm::inverse(glm::lookAt(qt0_v, qt1_v, orient_t0_v));
		float rotationArray[16] = { 0.0 };
		const float* pSource2 = (const float*)glm::value_ptr(rotate);
		for (int i = 0; i < 16; ++i)
			rotationArray[i] = pSource2[i];

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(trans, qt0_v);
		trans = glm::translate(trans, 10.0f * orient_t0_v);

		glPushMatrix();
		glTranslated(up_offset * orient_t0_v.x, up_offset * orient_t0_v.y, up_offset * orient_t0_v.z);
		glMultMatrixf(rotationArray);
		glScalef(scale_value, scale_value, scale_value);

		unsigned int r = 150 + sin(elapsed_seconds.count()) * 100;
		unsigned int g = 150 + cos(elapsed_seconds.count()) * 100;
		unsigned int b = 150 + cos(elapsed_seconds.count()) * 100;
		if (!doingShadows) {
			glColor3ub(r, g, b);
		}

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < my_car.vertices.size(); ++i) {
			glm::vec4 vec(my_car.vertices[i].x, my_car.vertices[i].y, my_car.vertices[i].z, 1.0f);

			glNormal3d(my_car.normals[i].x, my_car.normals[i].y, my_car.normals[i].z);
			glVertex3f(vec.x, vec.y, vec.z);

		}
		glEnd();
		glPopMatrix();
	}
}

void TrainView::draw_track(bool doingShadows) {
	//draw track with respect to my t_param and qt vectors data
	for (int i = 0; i < t_param.size(); ++i) {
		glm::vec3 qt0_v = all_qt[i];
		glm::vec3 qt1_v;
		if (i == t_param.size() - 1) qt1_v = all_qt[0];
		else qt1_v = all_qt[i + 1];

		glm::vec3 orient_t0_v = all_orient[i];
		glm::vec3 forward = all_forward[i];

		glm::vec3 offset_vec1 = glm::cross(forward, orient_t0_v);
		offset_vec1 = glm::normalize(offset_vec1);
		offset_vec1 *= 1.5 * DIVIDE_LINE / 100;
		glm::vec3 offset_vec2 = 1.5f * offset_vec1;


		glm::vec3 left_track0 = qt0_v + offset_vec1;
		glm::vec3 left_track1 = qt1_v + offset_vec1;
		glm::vec3 left_track2 = qt1_v + offset_vec2;
		glm::vec3 left_track3 = qt0_v + offset_vec2;
		glm::vec3 right_track0 = qt0_v - offset_vec1;
		glm::vec3 right_track1 = qt1_v - offset_vec1;
		glm::vec3 right_track2 = qt1_v - offset_vec2;
		glm::vec3 right_track3 = qt0_v - offset_vec2;

		glLineWidth(5);

		//draw selected track type
		if (tw->trackBrowser->value() == 1) {
			//single track
			glBegin(GL_LINES);
			if (!doingShadows) {
				glColor3ub(32, 32, 64);
			}
			glVertex3f(qt0_v.x, qt0_v.y, qt0_v.z);
			glVertex3f(qt1_v.x, qt1_v.y, qt1_v.z);
			glEnd();
		}
		else if (tw->trackBrowser->value() == 2) {
			//left track
			glBegin(GL_POLYGON);
			unsigned r = 50;
			unsigned g = 50;
			unsigned b = 50;
			if (!doingShadows) {
				glColor3ub(r, g, b);
			}
			glNormal3d(orient_t0_v.x, orient_t0_v.y, orient_t0_v.z);
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
			glNormal3d(orient_t0_v.x, orient_t0_v.y, orient_t0_v.z);
			glVertex3f(right_track0.x, right_track0.y, right_track0.z);
			glVertex3f(right_track1.x, right_track1.y, right_track1.z);
			glVertex3f(right_track2.x, right_track2.y, right_track2.z);
			glVertex3f(right_track3.x, right_track3.y, right_track3.z);
			glEnd();
		}
		else if (tw->trackBrowser->value() == 3) {
			float scale_value = 0.5f;
			glm::mat4 scale = glm::mat4(1.0f);
			scale = glm::scale(scale, glm::vec3(scale_value, scale_value, scale_value));

			quat MyQuaternion = my_LookAt(forward, orient_t0_v);

			mat4 RotationMatrix = glm::toMat4(MyQuaternion);
			float rotateArray[16] = { 0.0 };
			const float* pSource = (const float*)glm::value_ptr(RotationMatrix);
			for (int i = 0; i < 16; ++i)
				rotateArray[i] = pSource[i];

			mat4 rotate = glm::inverse(glm::lookAt(
				qt0_v,
				qt1_v,
				orient_t0_v));
			float rotateArray2[16] = { 0.0 };
			const float* pSource2 = (const float*)glm::value_ptr(rotate);
			for (int i = 0; i < 16; ++i)
				rotateArray2[i] = pSource2[i];

			float up_offset = -0.3f;
			glm::vec3 side_offset_v = glm::cross(forward, orient_t0_v);
			side_offset_v = glm::normalize(side_offset_v);
			side_offset_v *= 1.5f;

			//glm::mat4 trans = glm::mat4(1.0f);
			//trans = glm::translate(trans, qt0_v);
			//trans = glm::translate(trans, up_offset * orient_t0_v);
			//trans = glm::translate(trans, side_offset_v*1.0f);


			glPushMatrix();
			//glTranslated(qt0_v.x, qt0_v.y, qt0_v.z);
			glTranslated(up_offset * orient_t0_v.x, up_offset * orient_t0_v.y, up_offset * orient_t0_v.z);
			//glTranslated(side_offset_v.x, side_offset_v.y,side_offset_v.z);
			glMultMatrixf(rotateArray2);
			glRotatef(90.0f, 0, 1, 0);
			glScalef(scale_value, scale_value, scale_value);

			unsigned int r = 89;
			unsigned int g = 58;
			unsigned int b = 5;
			if (!doingShadows) {
				glColor3ub(r, g, b);
			}

			glBegin(GL_TRIANGLES);
			for (int i = 0; i < my_track.vertices.size(); ++i) {

				glm::vec4 vec(my_track.vertices[i].x, my_track.vertices[i].y, my_track.vertices[i].z, 1.0f);

				glNormal3d(my_track.normals[i].x, my_track.normals[i].y, my_track.normals[i].z);
				glVertex3f(vec.x, vec.y, vec.z);

			}






			glEnd();
			glPopMatrix();
		}

		glLineWidth(1);
	}
}

void TrainView::draw_sleeper(bool doingShadows) {
	float current_length = 0;
	while (current_length < accumulate_length.back()) {
		size_t i = length_to_index(current_length);

		glm::vec3 qt0_v = all_qt[i];
		glm::vec3 qt1_v;
		if (i == t_param.size() - 1) qt1_v = all_qt[0];
		else qt1_v = all_qt[i + 1];

		glm::vec3 orient_t0_v = all_orient[i];
		glm::vec3 forward = all_forward[i];

		glm::vec3 offset_vec1 = glm::cross(forward, orient_t0_v);
		offset_vec1 = glm::normalize(offset_vec1);
		offset_vec1 *= 1.5 * DIVIDE_LINE / 100;
		glm::vec3 offset_vec2 = 1.5f * offset_vec1;
		

		//draw selected track type
		if (tw->trackBrowser->value() == 1) {
			//no sleepers
		}
		else if ((tw->trackBrowser->value() == 2 || tw->trackBrowser->value() == 3)) {
			float scale_value = 3.5f;
			glm::mat4 scale = glm::mat4(1.0f);
			scale = glm::scale(scale, glm::vec3(scale_value, scale_value, scale_value));

			quat MyQuaternion = my_LookAt(forward, orient_t0_v);

			mat4 RotationMatrix = glm::toMat4(MyQuaternion);
			float rotateArray[16] = { 0.0 };
			const float* pSource = (const float*)glm::value_ptr(RotationMatrix);
			for (int i = 0; i < 16; ++i)
				rotateArray[i] = pSource[i];

			mat4 rotate = glm::inverse(glm::lookAt(
				qt0_v,
				qt1_v,
				orient_t0_v));
			float rotateArray2[16] = { 0.0 };
			const float* pSource2 = (const float*)glm::value_ptr(rotate);
			for (int i = 0; i < 16; ++i)
				rotateArray2[i] = pSource2[i];

			glm::mat4 trans = glm::mat4(1.0f);
			trans = glm::translate(trans, qt0_v);
			trans = glm::translate(trans, 1.0f * -orient_t0_v);

			glPushMatrix();
			//glTranslated(qt0_v.x, qt0_v.y, qt0_v.z);
			float up_offset = -1.5f;
			glTranslated(up_offset * orient_t0_v.x, up_offset * orient_t0_v.y, up_offset * orient_t0_v.z);
			//glMultMatrixf(rotateArray);
			glMultMatrixf(rotateArray2);
			glRotatef(90.0f, 0, 1, 0);
			glScalef(scale_value, scale_value, scale_value);

			unsigned int r = 230;
			unsigned int g = 180;
			unsigned int b = 50;
			if (!doingShadows) {
				glColor3ub(r, g, b);
			}
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < my_sleeper.vertices.size(); ++i) {
				//glColor3f(0.5, 1, 0.8);
				glm::vec4 vec(my_sleeper.vertices[i].x, my_sleeper.vertices[i].y, my_sleeper.vertices[i].z, 1.0f);

				//vec = trans * RotationMatrix *scale * vec;
				glNormal3d(my_sleeper.normals[i].x, my_sleeper.normals[i].y, my_sleeper.normals[i].z);
				glVertex3f(vec.x, vec.y, vec.z);
				//cout << vec.x << " " << vec.y << " " << vec.z << endl;
			}
			glEnd();
			glPopMatrix();
		}
		current_length += 5;
	}

}

void TrainView::draw_ground() {
	float scale_value = 5.0;
	unsigned int r = 50;
	unsigned int g = 255;
	unsigned int b = 150;
	glPushMatrix();
	glTranslated(0, -0, 0);
	glScalef(scale_value, scale_value, scale_value);
	glColor3ub(r, g, b);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < my_ground.vertices.size(); ++i) {

		glm::vec4 vec(my_ground.vertices[i].x, my_ground.vertices[i].y, my_ground.vertices[i].z, 1.0f);
		glNormal3d(my_ground.normals[i].x, my_ground.normals[i].y, my_ground.normals[i].z);
		glVertex3f(vec.x, vec.y, vec.z);

	}
	glEnd();
	glPopMatrix();
}

void TrainView::draw_env_model(bool doingShadows) {
	float scale_value =5.0f;
	unsigned int r = 150;
	unsigned int g = 50;
	unsigned int b = 130;

	glPushMatrix();
	glTranslated(0, 70.0, 0);
	glScalef(scale_value, scale_value, scale_value);
	if (!doingShadows) {
		glColor3ub(r, g, b);
	}
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < env_models[0].vertices.size(); ++i) {

		glm::vec4 vec(env_models[0].vertices[i].x, env_models[0].vertices[i].y, env_models[0].vertices[i].z, 1.0f);
		glNormal3d(env_models[0].normals[i].x, env_models[0].normals[i].y, env_models[0].normals[i].z);
		glVertex3f(vec.x, vec.y, vec.z);

	}
	glEnd();
	glPopMatrix();

	scale_value = 5.0f;
	glPushMatrix();
	glTranslated(300, 0.0, 0);
	glScalef(scale_value, scale_value, scale_value);
	r = 58;
	g = 85;
	b = 169;
	if (!doingShadows) {
		glColor3ub(r, g, b);
	}
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < env_models[1].vertices.size(); ++i) {

		glm::vec4 vec(env_models[1].vertices[i].x, env_models[1].vertices[i].y, env_models[1].vertices[i].z, 1.0f);
		glNormal3d(env_models[1].normals[i].x, env_models[1].normals[i].y, env_models[1].normals[i].z);
		glVertex3f(vec.x, vec.y, vec.z);

	}
	glEnd();
	glPopMatrix();
}

void TrainView::set_train_light() {
	size_t i = C_length_index();
	float t0 = t_param[i];

	glm::vec3 qt0_v3 = all_qt[i];
	glm::vec3 qt1_v3;
	if (i == t_param.size() - 1) qt1_v3 = all_qt[0];
	else qt1_v3 = all_qt[i + 1];

	glm::vec3 orient_t0_v = all_orient[i];
	glm::vec3 forward = all_forward[i];

	glm::vec4 qt0_v4(qt0_v3.x, qt0_v3.y, qt0_v3.z, 1);
	glm::vec4 qt1_v4(qt1_v3.x, qt1_v3.y, qt1_v3.z, 1);

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, 5.0f * orient_t0_v);
	trans = glm::translate(trans, 5.0f * forward);
	qt0_v4 = trans * qt0_v4;

	float ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diffuse[] = { 0.9f, 0.9f, 0.8f, 1.0f };
	float position[] = { qt0_v4.x, qt0_v4.y, qt0_v4.z, 1.0f };

	//float position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	float direction[] = { forward.x, forward.y, forward.z };
	//float direction[] = { 0, 1.0f, 0 };

#ifdef DEBUG
  //cout << "pos: " << position[0] << " " << position[1] << " " << position[2] << endl;
  //cout << "forward: " << forward.x << " " << forward.x << " " << forward.x << " " << endl;
#endif // DEBUG

	//directional light
	position[3] = 0.0f;
	glLightfv(GL_LIGHT3, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT3, GL_POSITION, position);

	position[3] = 1.0f;
	//point light
	glLightfv(GL_LIGHT4, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT4, GL_POSITION, position);

	//spot light
	glLightfv(GL_LIGHT5, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT5, GL_POSITION, position);

	//spot direction
	glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, direction);
	//angle of the clone
	glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 45.0f);
	//concentration of the light
	glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 25.0);

	//light attenuation
	glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.0f);
	glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.0f);

	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);
	glDisable(GL_LIGHT5);
	if (tw->lightBrowser->value() == 1) {
		glEnable(GL_LIGHT3);
	}
	else if (tw->lightBrowser->value() == 2) {
		glEnable(GL_LIGHT4);
	}
	else if (tw->lightBrowser->value() == 3) {
		glEnable(GL_LIGHT5);
	}
	
}

//update track data 
void TrainView::update_arcLengh() {
	size_t size = DIVIDE_LINE * m_pTrack->points.size();

	glm::vec3 default_vec3(0, 0, 0);
	t_param.assign(size, 0);
	arc_length.assign(size, 0);
	accumulate_length.assign(size, 0);
	speeds.assign(size, 0);
	all_qt.assign(size, default_vec3);
	all_orient.assign(size, default_vec3);
	all_forward.assign(size, default_vec3);

	//initialize t_param
	for (int i = 0; i < size; ++i) {
		int fraction = i % 100;
		t_param[i] = float(fraction) / float(DIVIDE_LINE);
	}

	//setup arcLength and all qt, orient and forward
	for (int i = 0; i < m_pTrack->points.size(); ++i) {
		Pnt3f cp_orient_p0 = m_pTrack->points[i].orient;
		Pnt3f cp_orient_p1 = m_pTrack->points[(i + 1) % m_pTrack->points.size()].orient;

		vector<Pnt3f> points = find_Cpoints(i);

		for (size_t j = 0; j < DIVIDE_LINE; j++) {
			size_t current_index = i * DIVIDE_LINE  + j;
			float t0 = t_param[current_index];

			vector<Pnt3f> qts = find_two_qt(tw->splineBrowser->value(), points, t0);

			glm::vec3 qt0_v(qts[0].x, qts[0].y, qts[0].z);
			glm::vec3 qt1_v(qts[1].x, qts[1].y, qts[1].z);
			glm::vec3 forward = qt1_v - qt0_v;

			//forward 可以改一下，遇到邊界無法跨度兩個線段
			forward = glm::normalize(forward);

			Pnt3f orient_t0 = find_orient(cp_orient_p0, cp_orient_p1, t0);
			glm::vec3 orient_t0_v(orient_t0.x, orient_t0.y, orient_t0.z);
			orient_t0_v = glm::normalize(orient_t0_v);

			float length = glm::distance(qt0_v, qt1_v);
			arc_length[current_index] = length;
			if (current_index == 0) accumulate_length[current_index] = length;
			else  accumulate_length[current_index] = accumulate_length[current_index - 1] + length;
			all_qt[current_index] = qt0_v;
			all_orient[current_index] = orient_t0_v;
			all_forward[current_index] = forward;
		}
	}

	//setup speeds
	float total_energy = 2000.0f;
	float kinetic_energy = 2000.0f;
	float potential_energy = 0.0f;
	float h_coefficient = 20.0f;
	float current_h = all_qt[0].y;
	speeds[0] = 0.05 * sqrt(kinetic_energy);
	for (int i = 1; i < size; ++i) {
		float h_diff = all_qt[i].y - all_qt[i - 1].y;
		potential_energy = potential_energy + h_coefficient * h_diff;
		kinetic_energy = total_energy - potential_energy;
		if (kinetic_energy <= 50) {
			total_energy += 50.0f;
			kinetic_energy = 50.0f;
		} 
		//cout << "p: " << potential_energy << endl;
		float current_speed = 0.05* sqrt(kinetic_energy);
		speeds[i] = current_speed;
	}

}



//use current length to find parameter t
float TrainView::length_to_t(float length) {
	for (size_t i = 0; i < accumulate_length.size() - 1; ++i) {
		if (length < accumulate_length[i]) {
			return t_param[i];
		}
	}
	return t_param.back();
}

//use given length to find current index
size_t TrainView::length_to_index(float length) {

	if (length < 0) {
		length = accumulate_length.back() + length;
	}
	for (size_t i = 0; i < accumulate_length.size() - 1; ++i) {
		if (length < accumulate_length[i]) {
			return i;
		}
	}
	return accumulate_length.size() - 1;
}

//find current parameter t by C_length
size_t TrainView::C_length_index() {
	int index = -1;
	for (size_t i = 0; i < accumulate_length.size() - 1; ++i) {
		if (m_pTrack->C_length < accumulate_length[i]) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		index = accumulate_length.size() - 1;
	}
	return index;
}

//find current index by trainU
size_t TrainView::trainU_index() {
	int index = int(m_pTrack->trainU / 1) * DIVIDE_LINE;
	index = index + (m_pTrack->trainU - int(m_pTrack->trainU / 1)) * DIVIDE_LINE;
	if (index == t_param.size()) --index;
	return index;
}

//match current length and trainU
void TrainView::match_length() {
	int index = C_length_index();

	m_pTrack->trainU = (index / DIVIDE_LINE) + length_to_t(m_pTrack->C_length);
	//cout << "index: " << index << endl;
	//cout << "length_to_t(m_pTrack->C_length): " << length_to_t(m_pTrack->C_length) << endl;
	//cout << "C_length: " << m_pTrack->C_length << endl;
	//cout << " trainU: " << m_pTrack->trainU << endl;

}

//match current trainU to length 
void TrainView::match_trainU() {
	int index = trainU_index();
	m_pTrack->C_length = accumulate_length[index];
}