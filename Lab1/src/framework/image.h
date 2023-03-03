/*** image.h  Javi Agenjo (javi.agenjo@gmail.com) 2013
	This file defines the class Image that allows to manipulate images.
	It defines all the need operators for Color and Image.
	It has a TGA loader and saver.
***/

#ifndef IMAGE_H
#define IMAGE_H

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include "framework.h"

//remove unsafe warnings
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

//Class Image: to store a matrix of pixels
class Image
{
	//a general struct to store all the information about a TGA file
	typedef struct sTGAInfo 
	{
		unsigned int width;
		unsigned int height;
		unsigned int bpp; //bits per pixel
		unsigned char* data; //bytes with the pixel information
	} TGAInfo;

public:
	unsigned int width;
	unsigned int height;
	Color* pixels;

	// CONSTRUCTORS 
	Image();
	Image(unsigned int width, unsigned int height);
	Image(const Image& c);
	Image& operator = (const Image& c); //assign operator

	// Destructor
	~Image();

	// Get the pixel at position x,y
	Color getPixel(unsigned int x, unsigned int y) const { return pixels[ y * width + x ]; }
	Color& getPixelRef(unsigned int x, unsigned int y)	{ return pixels[ y * width + x ]; }
	Color getPixelSafe(unsigned int x, unsigned int y) const {	
		x = clamp((unsigned int)x, 0, width-1); 
		y = clamp((unsigned int)y, 0, height-1); 
		return pixels[ y * width + x ]; 
	}

	// Set the pixel at position x,y with value C
	inline void setPixel(unsigned int x, unsigned int y, const Color& c) { pixels[ y * width + x ] = c; }
	inline void setPixelSafe(unsigned int x, unsigned int y, const Color& c) const { x = clamp(x, 0, width-1); y = clamp(y, 0, height-1); pixels[ y * width + x ] = c; }

	void resize(unsigned int width, unsigned int height);
	void scale(unsigned int width, unsigned int height);
	
	void flipY(); //flip the image top-down
	void flipX(); //flip the image left-right

	// Fill the image with the color C
	void fill(const Color& c) { for(unsigned int pos = 0; pos < width*height; ++pos) pixels[pos] = c; }

	// Returns a new image with the area from (startx,starty) of size width,height
	Image getArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height);

	// Save or load images from the hard drive
	bool loadTGA(const char* filename);
	bool saveTGA(const char* filename);

	// Methods for taking a screenshot
	char** getCurrentTime(char* current_time);
	void screenshot(const int width, const int height, const std::string str);

	// Primitive shapes
	void drawLine(float x0, float y0, Vector2 v, Color c);
	void drawRectangle(int startx, int starty, int w, int h, Color c, bool fill);
	void drawCircle(int a, int b, int r, Color c, bool fill);
	
	// Frame patterns
	void drawGradient(int w, int h);
	void drawNotchGradient(int w, int h);
	void drawCheckedFrame();
	void drawBilinearInterpolation(const int width, const int height, const int window_width, const int window_height);
	void drawSinusoidGradient(const int width, const int height, const int window_width, const int window_height);
	void drawChessBoard(const int width, const int height, const int window_width, const int window_heigth);

	// Image filters
	void grayscale();
	void invert();
	void channelManipulation();
	void tryAllSwaps();
	void threshold();
	void blur();
	void fade();

	//image edit
	void rotate(Image* img, double beta);
	void zoom(Image* img, double zoom, float mouse_x, float mouse_y);

	//Canvas
	void loadToolbar(Image* toolbar, int toolbar_size);
	void chosenColor(Image* toolbar, int toolbar_size, int h, Color color);
	void drawCanvas(float x, float y, Vector2 v, int canvas_height, Color color);

	// Used to easy code
	#ifndef IGNORE_LAMBDAS

	// Applies an algorithm to every pixel in an image
	// You can use lambda sintax:   img.forEachPixel( [](Color c) { return c*2; });
	// Or callback sintax:   img.forEachPixel( mycallback ); //the callback has to be Color mycallback(Color c) { ... }
	template <typename F>
	Image& forEachPixel( F callback )
	{
		for(unsigned int pos = 0; pos < width*height; ++pos)
			pixels[pos] = callback(pixels[pos]);
		return *this;
	}

	#endif


};

#endif