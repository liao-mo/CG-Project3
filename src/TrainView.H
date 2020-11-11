/************************************************************************
     File:        TrainView.H

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

#pragma once

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"
#include "Utilities/Pnt3f.H"
#include "TrainModel.h"
#include <vector>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#define DIVIDE_LINE 100

class TrainView : public Fl_Gl_Window
{
	public:
		// note that we keep the "standard widget" constructor arguments
		TrainView(int x, int y, int w, int h, const char* l = 0);

		// overrides of important window things
		virtual int handle(int);
		virtual void draw();

		// all of the actual drawing happens in this routine
		// it has to be encapsulated, since we draw differently if
		// we're drawing shadows (no colors, for example)
		void drawStuff(bool doingShadows=false);

		//my function that draws the train
		void draw_train(bool doingShadows);

		//my function that draws the track
		void draw_track(bool doingShadows);

		//my function that draws the sleeper
		void draw_sleeper(bool doingShadows);
		

		// setup the projection - assuming that the projection stack has been
		// cleared for you
		void setProjection();

		// Reset the Arc ball control
		void resetArcball();

		// pick a point (for when the mouse goes down)
		void doPick();

		//setupt train's head light
		void set_train_light();

		//update track data 
		void update_arcLengh();

		//use current length to find parameter t
		float length_to_t(float length);

		//use current length to find current index
		size_t length_to_index(float length);

		//use trainU t to find current index
		size_t trainU_to_index(float trainU);

		//match current length and trainU
		void match_length();


	public:
		ArcBallCam		arcball;			// keep an ArcBall for the UI
		int				selectedCube;  // simple - just remember which cube is selected

		TrainWindow*	tw;				// The parent of this display window
		CTrack*			m_pTrack;		// The track of the entire scene
		TrainModel my_train;			//my train model
		TrainModel my_sleeper;			//my train model
		TrainModel my_track;			//my train model

		unsigned int VAO;
		unsigned int VBO;
		unsigned int EBO;
		unsigned int shaderProgram;

		//find 4 control point around current point
		std::vector<Pnt3f> find_Cpoints(int currentPoint);
		//find 4 orient vector around current point
		std::vector<Pnt3f> find_orient_vectors(int currentPoint);

		std::vector<float> t_param;
		std::vector<float> arc_length;
		std::vector<float> accumulate_length;
		std::vector<glm::vec3> all_qt;
		std::vector<glm::vec3> all_orient;
		std::vector<glm::vec3> all_forward;
};

