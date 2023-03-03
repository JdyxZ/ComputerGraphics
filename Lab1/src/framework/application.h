/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the application, is in charge of creating the data, getting the user input, process the update and render.
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "includes.h"
#include "framework.h"
#include "image.h"


//Particle Struct
struct Particle {
	double x;
	double y;
	double aux_x;
	double aux_y;
	double v_x;
	double v_y;
	double size;
	Color color;
};

class Application
{
public:
	
	//window
	SDL_Window* window;
	float window_width;
	float window_height;

	//Images
	Image framebuffer;
	Image* amanda;
	Image* waifu;
	Image* toolbar;

	float app_time;

	//toolbar size
	int const toolbar_size = 50;

	//canvas state
	int canvas_state;

	//Drawing color
	Color drawing_color = Color::BLACK;

	//keyboard state
	const Uint8* keystate;
	Uint8 current_keystate[SDL_NUM_SCANCODES];
	Uint8 prev_keystate[SDL_NUM_SCANCODES]; //previous before

	//Mouse state
	int mouse_state; //tells which buttons are pressed
	Vector2 mouse_position; //last mouse position
	Vector2 mouse_starting_position; //start mouse position
	Vector2 mouse_delta; //mouse movement in the last frame

	//Mouse left state
	int mouse_left_state;

	//Mouse right state
	int mouse_right_state;

	//Rotation angle
	double beta;

	//Particles array
	Particle particles[400];

	//Particles animation keyword
	int particle_keyword;

	//constructor
	Application(const char* caption, int width, int height);

	//main methods
	void init( void );
	void render( Image& framebuffer );
	void update( double dt );

	// methods for keyboard
	inline bool isKeyPressed(const int key_code) { return this->keystate[key_code] != 0; };
	inline bool wasKeyPressed(const int key_code) { return this->keystate[key_code] != 0 && this->prev_keystate[key_code] == 0; };

	//methods for events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp( SDL_MouseButtonEvent event );

	//other methods to control the app
	void setWindowSize(int width, int height) {
		glViewport( 0,0, width, height );
		this->window_width = width;
		this->window_height = height;
		framebuffer.resize(width,height);
	}

	Vector2 getWindowSize()
	{
		int w,h;
		SDL_GetWindowSize(window,&w,&h);
		return Vector2(w,h);
	}

	void start();
};


#endif 