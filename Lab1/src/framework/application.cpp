#include "application.h"
#include "utils.h"
#include "image.h"
#include <math.h>
#include <windows.h>
#include <time.h>


using namespace std;

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);
	memcpy((void*)&(this->current_keystate), this->keystate, SDL_NUM_SCANCODES);

	framebuffer.resize(w, h);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;

	// New images
	waifu = new Image();
	amanda = new Image();
	toolbar = new Image();

	// Load images
	amanda->loadTGA("../res/loadings/amanda.tga");
	waifu->loadTGA("../res/loadings/Waifu1.tga");
	toolbar->loadTGA("../res/loadings/toolbar.tga");
	
	//MENU
	printf("\nFRAMEWORK JOB\n\n");
	printf("Take screenshot in savings folder (S)\n\n");
	printf("Task 1:\n\n");
	printf("Line (Keep right mouse button pressed)\n");
	printf("Rectangle (R)\n");
	printf("Filled rectangle (E)\n");
	printf("Circle (C)\n");
	printf("Filled circle (V)\n");
	printf("\nTask 2:\n\n");
	printf("Gradient (G)\n");
	printf("Notch gradient (N)\n");
	printf("Checked Frame (K)\n");
	printf("Bilinear interpolation (2)\n");
	printf("Sinusoid gradient (J)\n");
	printf("Chess board (H)\n");
	printf("\nTask 3:\n\n");
	printf("Load image (L)\n");
	printf("Grayscale (W)\n");
	printf("Invert (I)\n");
	printf("Channel manipulation (M)\n");
	printf("Threshold (T)\n");
	printf("Blur (B)\n");
	printf("Fade (F)\n");
	printf("\nTask 4: \n\n");
	printf("Right rotation (->) \n");
	printf("Left rotation (<-) \n");
	printf("Zoom image (Z) \n");
	printf("\nTask 5: \n \n");
	printf("Start starfield animation (P)\n");
	printf("Stop starfield animation (BACKSPACE)\n");
	printf("\nTask 6: \n\n");
	printf("Load canvas (D)\n");
	printf("\nSpecial commands:\n\n");
	printf("Reset (0)\n\n");

	//Angle beta initialization
	beta = 0;

	//Particle keyword initialization
	particle_keyword = 0;

	//Canvas state initilization
	canvas_state = 0;

	//Left mouse state initilization
	mouse_left_state = 0;

	//Left mouse state initilization
	mouse_right_state = 0;

	for (int i = 0; i < 400; i++) {
		//Inicializamos la posición de todas las partículas en el centro de la pantalla
		particles[i].x = (double)window_width / 2;
		particles[i].y = (double)window_height / 2;
		particles[i].size = 2;

		//Escogemos un valor aleatorio para cada partícula para que se mueve hacia ella
		particles[i].aux_x = randomValue() * (double)window_width;
		particles[i].aux_y = randomValue() * (double)window_height;

		//Calculamos el vector entre la posición inicial y la posición a la que irá
		particles[i].v_x = (particles[i].aux_x - particles[i].x);
		particles[i].v_y = (particles[i].aux_y - particles[i].y);

		//Le sumamos a cada posición inicial la mitad del vector para que no empiece exactamente desde el centro
		particles[i].x += particles[i].v_x / 2;
		particles[i].y += particles[i].v_y / 2;

		//Elección de color según la posición
		if (i % 3 == 0) {
			particles[i].color = Color::CYAN;
		}
		else if (i % 3 == 1) {
			particles[i].color = Color::BLUE;
		}
		else if (i % 3 == 2) {
			particles[i].color = Color::PURPLE;
		}
	}
}

//render one frame
void Application::render( Image& framebuffer )
{
	// Framebuffer constants
	const int width = 100;
	const int height = 150;
	const int radius = 100;

	// Clean framebuffer
	if (wasKeyPressed(SDL_SCANCODE_0)) 
	{
		framebuffer.fill(Color::BLACK);
	}
	
	// Primitives
	else if (mouse_right_state) // Line
	{
		Vector2 mouse_displacement;
		mouse_displacement.x = mouse_position.x - mouse_starting_position.x;
		mouse_displacement.y = mouse_position.y - mouse_starting_position.y;
		framebuffer.drawLine(mouse_starting_position.x, mouse_starting_position.y, mouse_displacement, Color(randomValue() * 255, randomValue() * 255, randomValue() * 255));
		mouse_right_state = 0;

	}
	else if (isKeyPressed(SDL_SCANCODE_R)) // Rectangle
	{ 
		framebuffer.drawRectangle(mouse_position.x, mouse_position.y, width, height, Color(randomValue() * 255, randomValue() * 255, randomValue() * 255), false);
	}
	else if (isKeyPressed(SDL_SCANCODE_E)) // Filled rectangle
	{ 
		framebuffer.drawRectangle(mouse_position.x, mouse_position.y, width, height, Color(randomValue() * 255, randomValue() * 255, randomValue() * 255), true);
	}
	else if (isKeyPressed(SDL_SCANCODE_C)) // Circle
	{ 
		framebuffer.drawCircle(mouse_position.x, mouse_position.y, radius, Color(randomValue() * 255, randomValue() * 255, randomValue() * 255), false);
	}
	else if (isKeyPressed(SDL_SCANCODE_V)) // Filled circle
	{ 
		framebuffer.drawCircle(mouse_position.x, mouse_position.y, radius, Color(randomValue() * 255, randomValue() * 255, randomValue() * 255), true);
	}

	// Patterns
	else if (wasKeyPressed(SDL_SCANCODE_G)) // Gradient
	{ 
		framebuffer.drawGradient(window_width, window_height);
	}
	else if (wasKeyPressed(SDL_SCANCODE_N)) // Notch gradient
	{ 
		framebuffer.drawNotchGradient(window_width, window_height);
	}
	else if (wasKeyPressed(SDL_SCANCODE_K)) // Checked frame
	{ 
		framebuffer.drawCheckedFrame();
	}
	else if (wasKeyPressed(SDL_SCANCODE_2)) // Bilinear interpolation
	{
		framebuffer.drawBilinearInterpolation(window_height * 3 / 4, window_height * 3 / 4, window_width, window_height);
	}
	else if (wasKeyPressed(SDL_SCANCODE_J)) // Sinusoid gradient
	{
		framebuffer.drawSinusoidGradient(window_height * 3 / 4, window_height * 3 / 4, window_width, window_height);
	}
	else if (wasKeyPressed(SDL_SCANCODE_H)) // Chess board
	{
		framebuffer.drawChessBoard(window_width, window_height, window_width, window_height);
	}

	// Image filters
	else if (wasKeyPressed(SDL_SCANCODE_L)) // Load image
	{
		// Get image to work with
		const Image* img = waifu;

		// Compute offset
		const int x_offset = (framebuffer.width - img->width) / 2;
		const int y_offset = (int(framebuffer.height) - int(img->height)) / 2;

		// Iterate image and fill framebuffer
		for (unsigned int i = 0; i < img->width; i++) 
		{
			for (unsigned int j = 0; j < img->height; j++) 
			{
				const Color c = img->getPixelSafe(i, j);
				framebuffer.setPixelSafe(x_offset + i, y_offset + j, c);
			}
		}
	}
	else if (wasKeyPressed(SDL_SCANCODE_W)) // Gray scale filter
	{ 
		framebuffer.grayscale();
	}
	else if (wasKeyPressed(SDL_SCANCODE_I)) // Invert color filter
	{ 
		framebuffer.invert();
	}
	else if (wasKeyPressed(SDL_SCANCODE_M)) // Swap channels filter
	{ 
		framebuffer.channelManipulation();
	}
	else if (wasKeyPressed(SDL_SCANCODE_T)) // Threshold filter
	{ 
		framebuffer.threshold();
	}
	else if (wasKeyPressed(SDL_SCANCODE_B)) // Blur filter
	{ 
		framebuffer.blur();
	}
	else if (wasKeyPressed(SDL_SCANCODE_F)) // Fade filter
	{ 
		framebuffer.fade();
	}
	else if (isKeyPressed(SDL_SCANCODE_Z)) // Zoom filter
	{ 
		framebuffer.zoom(waifu, 0.4, mouse_position.x, mouse_position.y);
	}

	// Canvas toolbox
	else if (wasKeyPressed(SDL_SCANCODE_D)) { 
		framebuffer.fill(Color::WHITE);
		framebuffer.loadToolbar(toolbar, toolbar_size);
	}

	// Particle animation
	if(particle_keyword){
		framebuffer.fill(Color::BLACK);
		for (int i = 0; i < 400; i++) {
			for (int a = -particles[i].size; a < particles[i].size; a++) {
				for (int b = -particles[i].size; b < particles[i].size; b++) {
					framebuffer.setPixelSafe(particles[i].x + a, particles[i].y + b, particles[i].color);
				}
			}
		}
	}

	// Canvas 
	if (canvas_state) {
		if (mouse_state && mouse_left_state) {
			
			//Definition of variables for mouse position
			const float x = mouse_position.x;
			const float y = mouse_position.y;

			//Auxiliar variable
			const float h = window_height;

			//canvas height
			const int ch = window_height - toolbar_size; 
			
			if (11.0 <= x && x <= 34.0 && h - 40.0 <= y && y <= h - 8.0) {
				framebuffer.fill(Color::WHITE);
				framebuffer.loadToolbar(toolbar, 50);
			}
			else if (60.0 <= x && x <= 91.0 && h - 40.0 <= y && y <= h - 9.0) {
				framebuffer.screenshot(window_width, ch, "");
				Sleep(150);
			}
			else if (112.0 <= x && x <= 138.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::BLACK;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::BLACK);
			}
			else if (162.0 <= x && x <= 188.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::RED;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::RED);
			}
			else if (212.0 <= x && x <= 238.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::GREEN;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::GREEN);
			}
			else if (262.0 <= x && x <= 288.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::BLUE;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::BLUE);
			}
			else if (312.0 <= x && x <= 338.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::YELLOW;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::YELLOW);
			}
			else if (362.0 <= x && x <= 388.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::PURPLE;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::PURPLE);
			}
			else if (412.0 <= x && x <= 438.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::CYAN;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::CYAN);
			}
			else if (462.0 <= x && x <= 488.0 && h - 38.0 <= y && y <= h - 12.0) {
				drawing_color = Color::WHITE;
				framebuffer.chosenColor(toolbar, 50, window_height, Color::WHITE);
			}
			framebuffer.drawCanvas(mouse_position.x, mouse_position.y, mouse_delta, ch, drawing_color);
		}
	}

	// Additional
	if (wasKeyPressed(SDL_SCANCODE_9)) // Store all possible swaps in local storage
	{
		amanda->tryAllSwaps();
	}

	if (wasKeyPressed(SDL_SCANCODE_S))
	{
		framebuffer.screenshot(framebuffer.width, framebuffer.height, "");
	}
}

//called after render
void Application::update(double seconds_elapsed)
{

	if (keystate[SDL_SCANCODE_RIGHT]){
		beta -= 0.01;
		framebuffer.rotate(waifu, beta);

	}
	else if (keystate[SDL_SCANCODE_LEFT]) 
	{
		beta += 0.01;
		framebuffer.rotate(waifu, beta);
	}
	else if (keystate[SDL_SCANCODE_D]) {
		canvas_state = 1;
	}

	if (particle_keyword) {
		for (int i = 0; i < 400; i++) {
			//Si la posición sale de la pantalla, vuelve a la posición original
			if (particles[i].x >= window_width || particles[i].y >= window_height) {
				particles[i].x = ((double)window_width / 2) + particles[i].v_x / 2;
				particles[i].y = ((double)window_height / 2) + particles[i].v_y / 2;
			}
			else if (particles[i].x <= 0 || particles[i].y <= 0) {
				particles[i].x = ((double)window_width / 2) + particles[i].v_x / 2;
				particles[i].y = ((double)window_height / 2) + particles[i].v_y / 2;
			}

			//El color de cada partícula va cambiando en cada nuevo paso
			if (particles[i].color.r == Color::CYAN.r && particles[i].color.g == Color::CYAN.g && particles[i].color.b == Color::CYAN.b) {
				particles[i].color = Color::BLUE;
			}
			else if (particles[i].color.r == Color::BLUE.r && particles[i].color.g == Color::BLUE.g && particles[i].color.b == Color::BLUE.b) {
				particles[i].color = Color::PURPLE;
			}
			else if (particles[i].color.r == Color::PURPLE.r && particles[i].color.g == Color::PURPLE.g && particles[i].color.b == Color::PURPLE.b) {
				particles[i].color = Color::CYAN;
			}

			//Movimiento de las partículas
			particles[i].x = particles[i].x + particles[i].v_x * seconds_elapsed * 2;
			particles[i].y = particles[i].y + particles[i].v_y * seconds_elapsed * 2;
		}
	}

}

//keyboard press event 
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode

	switch(event.keysym.scancode)
	{
		case SDL_SCANCODE_ESCAPE:
			exit(0); 
			break; //ESC key, kill the app
		case SDL_SCANCODE_P:
			particle_keyword = 1;
			break;
		case SDL_SCANCODE_BACKSPACE:
			particle_keyword = 0;
			break;
		default:
			canvas_state = 0;
			particle_keyword = 0;
			break;
	}
}

//keyboard key up event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	//...
}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	//if you read mouse position from the event, careful, Y is reversed, use mouse_position instead
	
	if (event.button == SDL_BUTTON_LEFT) { // left mouse pressed
		mouse_left_state = 1;
	}
	else if (event.button == SDL_BUTTON_RIGHT) {//right mouse pressed
		mouse_left_state = 0;
		mouse_starting_position.x = mouse_position.x;
		mouse_starting_position.y = mouse_position.y;
	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_RIGHT) {
		mouse_right_state = 1;
	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
