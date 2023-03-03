#include "image.h"

using namespace std;

Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

//copy constructor
Image::Image(const Image& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
}

//assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height*sizeof(Color)];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete pixels;
}



//change image size (the old one will remain in the top-left corner)
void Image::resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = getPixel(x,y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

//change image size and scale the content
void Image::scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = getPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::getArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.setPixel( x, y, getPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::flipX()
{
	for(unsigned int x = 0; x < width * 0.5; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			Color temp = getPixel(width - x - 1, y);
			setPixel( width - x - 1, y, getPixel(x,y));
			setPixel( x, y, temp );
		}
}

void Image::flipY()
{
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height * 0.5; ++y)
		{
			Color temp = getPixel(x, height - y - 1);
			setPixel( x, height - y - 1, getPixel(x,y) );
			setPixel( x, y, temp );
		}
}


//Loads an image from a TGA file
bool Image::loadTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int bytesPerPixel;
	unsigned int imageSize;

	FILE * file = fopen(filename, "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << filename << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;
    
	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];
    
	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		std::cerr << "TGA file seems to have errors or it is compressed, only uncompressed TGAs supported" << std::endl;
		fclose(file);
		delete tgainfo;
		return NULL;
	}
    
	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;
    
	tgainfo->data = new unsigned char[imageSize];
    
	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;
            
		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	//save info in image
	if(pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	//convert to float all pixels
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			this->setPixel(x , height - y - 1, Color( tgainfo->data[pos+2], tgainfo->data[pos+1], tgainfo->data[pos]) );
		}

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::saveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE *file = fopen(filename, "wb");
	if ( file == NULL )
	{
		fclose(file);
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	//tgainfo->width = header[1] * 256 + header[0];
	//tgainfo->height = header[3] * 256 + header[2];

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	//convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[(height-y-1)*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);
	return true;
}

// Get current time
char** Image::getCurrentTime(char* current_time) {

	// Split time malloc
	char** split_time = (char**)malloc(sizeof(char*) * 5);

	// Declare some vars
	int rows = 0;
	int columns = 0;
	int itr = 0;

	// Start loop
	while(current_time[itr] != '\n') {

		// Temporal byte array
		char* time_format = (char*)malloc(sizeof(char) * 9);
		
		// Store words
		while (current_time[itr] != ' ' && current_time[itr] != '\n') {
			time_format[columns] = current_time[itr];
			itr++;
			columns++;
		}

		// Initialize array into split time
		split_time[rows] = (char*)malloc(sizeof(char) * (columns + 1));

		// Fill array
		for (int i = 0; i < columns; i++) {
			split_time[rows][i] = time_format[i];
		}

		// End the array with the special char
		split_time[rows][columns] = '\0';

		// Update iterators
		while (current_time[itr] == ' ') itr++; // Skip word spaces for next loop iteration
		rows++;
		columns = 0;

		// Free time format
		free(time_format);
	}

	// Change ':' by '_' for compatibility issues
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < strlen(split_time[i]); j++)
		{
			if (split_time[i][j] == ':') 
				split_time[i][j] = '_';
		}
	}

	// Output
	return split_time;
}

// Take a screenshot of the image and save it in local storage
void Image::screenshot(const int width, const int height, const string str)
{
	// Get current time and split it
	time_t timer = time(NULL);
	char** split_time = getCurrentTime(ctime(&timer)); //Wed Feb 13 16:06:10 2013

	// Build image path
	string image_path;
	if (!str.empty()) image_path = "../res/savings/Image " + string(split_time[2]) + " " + string(split_time[1]) + " " + string(split_time[4]) + " " + string(split_time[3]) + " " + str + ".tga";
	else image_path = "../res/savings/Image " + string(split_time[2]) + " " + string(split_time[1]) + " " + string(split_time[4]) + " " + string(split_time[3]) + ".tga";
	
	// Create new image and fill it with the desired part of the called image
	Image* img = new Image(width, height);
	for (int x = 0; x <= width; x++)
	{
		for (int y = 0; y <= height; y++)
		{
			img->setPixelSafe(x, y, this->getPixelSafe(x, y));
		}
	}

	// Flip image and save it
	img->flipY();
	img->saveTGA(image_path.c_str());

	// Notify success
	cout << "Image successfully saved" << endl;
}


			///////////////////          \\\\\\\\\\\\\\\\\\\\
			///////////////////  TASK 1  \\\\\\\\\\\\\\\\\\\\
			///////////////////			 \\\\\\\\\\\\\\\\\\\\

void Image::drawLine(float x0, float y0, Vector2 v, Color c) {
	// General equation: Ax + By + C = 0
	// B = -v.x
	// A = v.y
	// C = k

	if (v.x == 0 && v.y == 0) {
		setPixelSafe(x0, y0, c);
	}
	else {
		float xf = x0 + v.x;
		float yf = y0 + v.y;

		//v.normalize();
		float k = v.x * y0 - v.y * x0;

		if (v.x == 0) {
			if (v.y > 0) {
				for (double y = y0; y < yf; y += 0.0001) {
					double x = (v.x * y - k) / v.y; // x = (-By - C)/A
					setPixelSafe(x, y, c);
				}
			}
			else {
				for (double y = yf; y < y0; y += 0.0001) {
					double x = (v.x * y - k) / v.y; // x = (-By - C)/A
					setPixelSafe(x, y, c);
				}
			}
		}
		else if (v.x > 0) {
			for (double x = x0; x < xf; x += 0.0001) {
				double y = (v.y * x + k) / v.x; // y = (-Ax - C)/B
				setPixelSafe(x, y, c);
			}
		}
		else if (v.x < 0) {
			for (double x = xf; x < x0; x += 0.0001) {
				double y = (v.y * x + k) / v.x; // y = (-Ax - C)/B
				setPixelSafe(x, y, c);
			}
		}
	}
}


void Image::drawRectangle(int startx, int starty, int w, int h, Color c, bool fill) {		

	int centerx = startx - w/2;
	int centery = starty - h/2;

	if (fill == true) { //fill the rectangle
		for (int x  = centerx; x < centerx + w; ++x) {
			for (int y = centery; y < centery + h; ++y) {
				setPixelSafe(x, y, c); 
			}
		}
	}
	else { //draw only the border

		for (int x = centerx; x < (centerx + w); ++x) // Horizontal edges
		{
			setPixelSafe(x, centery, c);
			setPixelSafe(x, centery + h - 1, c);
		}
		for (int y = centery; y < centery + h; ++y) // Vertical edges
		{
			setPixelSafe(centerx, y, c);
			setPixelSafe(centerx + w, y, c);
		}
	}
}

void Image::drawCircle(int a, int b, int r, Color c, bool fill) {
	for (double x = -r; x <= r; x++) {
		for (double y = -r; y <= r; y++) {
			double theta = atan2(y, x);
			if (fill == true) {
				if (abs(x) <= abs(r * cos(theta)) && abs(y) <= abs(r * sin(theta))) {
					setPixelSafe(x + a,y + b,c);
				}
			}
			else {
				if (x == round(r * cos(theta)) && y == round(r * sin(theta))) {
					setPixelSafe(x + a, y + b, c);
				}
			}
		}
	}

}

			///////////////////          \\\\\\\\\\\\\\\\\\\\
			///////////////////  TASK 2  \\\\\\\\\\\\\\\\\\\\
			///////////////////			 \\\\\\\\\\\\\\\\\\\\

void Image::drawGradient(int w, int h) {
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			float f = x / (float)w;
			f = f * 255;
			setPixelSafe(x, y, Color(f, 0, 255 - f));
		}
	}
}

void Image::drawNotchGradient(int w, int h) {
	double diagonal = sqrt(pow((w / 2), 2) + pow(h / 2, 2));
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int x_diff = abs((int)width / 2 - x);
			int y_diff = abs((int)height / 2 - y);
			double radius = sqrt(pow(x_diff, 2) + pow(y_diff, 2));
			double color_degree = 255 * radius / diagonal;
			setPixelSafe(x, y, Color(color_degree, color_degree, color_degree));
		}
	}
}

void Image::drawCheckedFrame() {
	fill(Color::BLACK);

	// Lines constants
	int const line_thickness = 10;
	int const line_space = 20;

	// Horizontal blue lines
	for (int aux = 0; aux <= height; aux += (line_space + 10)) {
		for (int y = aux; y < aux + line_thickness; y++) {
			for (int x = 0; x < width; x++) {
				setPixelSafe(x, y, Color(0, 0, 255));
			}
		}
	}

	// Vertical red lines
	for (int aux = 0; aux <= width; aux += (line_space + 10)) {
		for (int x = aux; x < aux + line_thickness; x++) {
			for (int y = 0; y < height; y++) {
				setPixelSafe(x, y, Color(255, 0, 0));
			}
		}
	}

	// Pink squares
	for (int aux = 0; aux <= height; aux += (line_space + 10)) {
		for (int aux2 = 0; aux2 <= width; aux2 += (line_space + 10)) {
			for(int y = aux; y < aux + line_thickness; y++){
				for (int x = aux2; x < aux2 + line_thickness; x++) {
					setPixelSafe(x, y, Color(255, 0, 255));
				}
			}
		}
	}
}

void Image::drawBilinearInterpolation(const int width, const int height, const int window_width, const int window_height)
{
	// Compute offset
	const int x_offset = (window_width - width) / 2;
	const int y_offset = (window_height - height) / 2;
	
	// Define grid parameters
	const int num_steps = 10; // Number of steps
	const Vector2 step_size(width / num_steps, height / num_steps); // Step size

	// Define vertex colors
	Vector3 vertex_colors[4] =
	{
		Vector3(0, 255, 0), // top left corner -> GREEN
		Vector3(0, 0, 0), // bottom left corner -> BLACK
		Vector3(255, 0, 0), // bottom right corner -> RED
		Vector3(255, 255, 0) // top right corner -> YELLOW
	};

	// Iterate over the pixels of the image
	for (int x = 0; x <= width; x++) {
		for (int y = 0; y <= height; y++) {

			// Find the current step in the grid and map it to the range [0, 1]
			Vector2 current_step
			(
				clamp(floor(x / step_size.x) / (num_steps - 1), 0, 1), 
				clamp(floor(y / step_size.y) / (num_steps - 1), 0, 1)
			);

			// Bilinear interpolation
			const Vector3 x_bottom_color = vertex_colors[1] * (1 - current_step.x) + vertex_colors[2] * current_step.x;
			const Vector3 x_top_color = vertex_colors[0] * (1 - current_step.x) + vertex_colors[3] * current_step.x;
			const Vector3 bilinear_color = x_bottom_color * (1 - current_step.y) + x_top_color * current_step.y;

			// Assing pixel color
			setPixelSafe(x + x_offset, y + y_offset, Color(bilinear_color.x, bilinear_color.y, bilinear_color.z));
		}
	}	
}

void Image::drawSinusoidGradient(const int width, const int height, const int window_width, const int window_height)
{
	// Compute offset
	const int x_offset = (window_width - width) / 2;
	const int y_offset = (window_height - height) / 2;

	// Iterate over the pixels of the image
	for (int x = 0; x <= width; x++) {
		
		// Normalize x coordinate to work in normal space
		float normal_x = x / (float)width;

		// Sinus attributes
		const float amplitude = 0.15;
		const float offset = 0.5;

		// Compute sinus value
		const float sin_val = amplitude * sin(2 * PI * normal_x) + offset;

		for (int y = 0; y <= height; y++) {

			// Normalize y coordinate to work in normal space
			float normal_y = y / (float)height;

			// Y-axis linear interpolation
			float range_size = 230; // In order to resemble better the suggested photo and visualize better the borders we don't use the full range.
			float f = range_size * normal_y; // Interpolation factor
			
			// Magic
			normal_y > sin_val ? setPixelSafe(x + x_offset, y + y_offset, Color(0, 1 - f, 0)) : setPixelSafe(x + x_offset, y + y_offset, Color(0, f, 0));
		}
	}

}

void Image::drawChessBoard(const int width, const int height, const int window_width, const int window_height)
{
	// Compute offset
	const int x_offset = (window_width - width) / 2;
	const int y_offset = (window_height - height) / 2;

	// Define grid parameters
	const int square_size = 30; // pixels

	// Iterate over the pixels of the image
	for (int x = 0; x <= width; x++) {
		for (int y = 0; y <= height; y++) {

			// Find the current step in the grid
			const int current_square = floor(x / square_size) + floor(y / square_size);

			// Magic
			current_square % 2 == 0 ? setPixelSafe(x + x_offset, y + y_offset, Color::WHITE) : setPixelSafe(x + x_offset, y + y_offset, Color::BLACK);
		}
	}

}

			///////////////////          \\\\\\\\\\\\\\\\\\\\
			///////////////////  TASK 3  \\\\\\\\\\\\\\\\\\\\
			///////////////////			 \\\\\\\\\\\\\\\\\\\\

void Image::grayscale() 
{
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Color c = getPixel(x, y);
			double color_degree = ((double)c.r / 255 + (double)c.g / 255 + (double)c.b / 255)/3.0 * 255.0;
			c.set(color_degree, color_degree, color_degree);
			setPixel(x, y, c);
		}
	}
}

void Image::threshold() 
{
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Color c = getPixel(x, y);
			double color_degree = ((double)c.r / 255 + (double)c.g / 255 + (double)c.b / 255) / 3.0 * 255.0;
			if (color_degree > 127) {color_degree = 255;}
			else {color_degree = 0;}
			c.set(color_degree, color_degree, color_degree);
			setPixel(x, y, c);
		}
	}
}

void Image::invert() 
{
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Color c = getPixel(x, y);
			c.set(255 - c.r, 255 - c.g, 255 - c.b);
			setPixel(x, y, c);
		}
	}
}

void Image::channelManipulation() 
{
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Color c = getPixel(x, y);
			// c.set(c.b,c.g,c.r);
			c.set(c.r * 2, c.g / 2, c.b / 2);
			setPixelSafe(x, y, c);
		}
	}
}


void Image::tryAllSwaps()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				// Create a copy of the image to manipulate
				Image* img = new Image(*this);

				// Set the pixel colors of the copy as desired
				for (int x = 0; x < img->width; x++)
				{
					for (int y = 0; y < img->height; y++)
					{
						// Get image pixel color
						Color c = img->getPixelSafe(x, y);
						
						// Declare all possibilities
						float p[4] = { 0, c.r, c.b, c.g };

						// Set color upon possibilities
						c.set(p[i], p[j], p[k]);

						// Set color to copy
						img->setPixelSafe(x, y, c);
					}
				}

				// Get combination string
				ostringstream ss;
				ss << convertToCharComponents(i) << "_" << convertToCharComponents(j) << "_" << convertToCharComponents(k);
				string combination(ss.str());

				// Save copy in local storage
				img->screenshot(img->width, img->height, combination);

			}
		}
	}	
}

void Image::blur() {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			Color c1 = getPixel(x, y);
			Color c2 = getPixel(x + 1, y);
			Color c3 = getPixel(x + 2, y);
			Color c4 = getPixel(x + 3, y);
			Color c5 = getPixel(x + 4, y);
			Color c6 = getPixel(x + 5, y);
			Color c7 = getPixel(x + 6, y);
			Color c = Color(
				(c1.r + c2.r + c3.r + c4.r + c5.r + c6.r + c7.r) / 7, // RED
				(c1.g + c2.g + c3.g + c4.g + c5.g + c6.g + c7.g) / 7, // GREEN
				(c1.b + c2.b + c3.b + c4.b + c5.b + c6.b + c7.b) / 7 // BLUE
			);
			setPixel(x, y, c);
		}
	}
}

void Image::fade() {
	double const diagonal = sqrt(pow((width / 2), 2) + pow(height / 2, 2));
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int x_diff = abs((int)width/2 - x);
			int y_diff = abs((int)height / 2 - y);
			double radius = sqrt(pow(x_diff,2) + pow(y_diff,2));
			double color_degree = 4*radius / diagonal + 0.25;
			Color c = getPixel(x, y);
			c.set(c.r/color_degree,c.g/color_degree,c.b/color_degree);
			setPixel(x, y, c);
		}
	}
}


			///////////////////          \\\\\\\\\\\\\\\\\\\\
			///////////////////  TASK 4  \\\\\\\\\\\\\\\\\\\\
			///////////////////			 \\\\\\\\\\\\\\\\\\\\


void Image::rotate(Image* img, double beta) {
	for (int xf = 0; xf < width; xf++) { 
		for (int yf = 0; yf < height; yf++) {

			// Distancia horizontal respecto al centro del framebuffer
			double xf_aux = (double)xf - width / 2; 

			// Distancia vertical respecto al centro del framebuffer
			double yf_aux = (double)yf - height / 2; 

			// Coordenadas horizontales del píxel de la imagen que corresponde a la posición (xf_aux,yf_aux) del framebuffer
			double xi = xf_aux * cos(beta) + yf_aux * sin(beta); 

			//Coordenadas verticales del píxel de la imagen que corresponde a la posición (xf_aux,yf_aux) del framebuffer
			double yi = yf_aux * cos(beta) - xf_aux * sin(beta); 
			
			//Una vez calculada la dirección, cogemos el píxel en función del origen de coordenadas (0,0) de la imagen
			xi += width / 2; // Sumamos para volver a centrar en el origen de coordenadas
			yi += height / 2; // Sumamos para volver a centrar en el origen de coordenadas

			// Color del píxel (xi,yi) de la imagen
			Color c = img->getPixelSafe(xi, yi);

			// Establecemos el color del píxel (xf,yf) con el color que hemos obtenido anteriormente.
			setPixel(xf, yf, c); 
		}
	}
}

void Image::zoom(Image* img, double zoom, float mouse_x, float mouse_y) {
	float zoom_x_size = zoom * width;
	float zoom_y_size = zoom * height;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {

			// Get zoomed pixel color
			Color c = img->getPixelSafe(x * zoom + mouse_x - zoom_x_size / 2, y * zoom + mouse_y - zoom_y_size / 2);

			// Assign pixel color
			setPixelSafe(x, y, c);
		}
	}
}

			///////////////////          \\\\\\\\\\\\\\\\\\\\
			///////////////////  TASK 6  \\\\\\\\\\\\\\\\\\\\
			///////////////////			 \\\\\\\\\\\\\\\\\\\\

void Image::loadToolbar(Image* toolbar, int toolbar_size) {
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < toolbar_size; y++) {
			setPixel(x, height - toolbar_size + y, toolbar->getPixel(x, y));
		}
	}
}

void Image::chosenColor(Image* toolbar, int toolbar_size, int h, Color color) {
	loadToolbar(toolbar, toolbar_size);
	if (color.r == Color::BLACK.r && color.g == Color::BLACK.g && color.b == Color::BLACK.b) {
		drawRectangle(125, h - 25, 28, 29, Color::WHITE, false);
	}
	else if (color.r == Color::RED.r && color.g == Color::RED.g && color.b == Color::RED.b) {
		drawRectangle(175, h - 25, 28, 29, Color::WHITE, false);
	}
	else if (color.r == Color::GREEN.r && color.g == Color::GREEN.g && color.b == Color::GREEN.b) {
		drawRectangle(225, h - 25, 28, 29, Color::WHITE, false);
	}
	else if (color.r == Color::BLUE.r && color.g == Color::BLUE.g && color.b == Color::BLUE.b) {
		drawRectangle(275, h - 25, 28, 29, Color::WHITE, false);
	}
	else if (color.r == Color::YELLOW.r && color.g == Color::YELLOW.g && color.b == Color::YELLOW.b) {
		drawRectangle(325, h - 25, 28, 29, Color::WHITE, false);
	}
	else if (color.r == Color::PURPLE.r && color.g == Color::PURPLE.g && color.b == Color::PURPLE.b) {
		drawRectangle(375, h - 25, 28, 29, Color::WHITE, false);
	}
	else if (color.r == Color::CYAN.r && color.g == Color::CYAN.g && color.b == Color::CYAN.b) {
		drawRectangle(425, h - 25, 28, 29, Color::WHITE, false);
	}
	else if (color.r == Color::WHITE.r && color.g == Color::WHITE.g && color.b == Color::WHITE.b) {
		drawRectangle(475, h - 25, 28, 29, Color::WHITE, false);
	}
}

void Image::drawCanvas(float x, float y, Vector2 v, int canvas_height, Color color) {
	if (y < canvas_height) {
		if (y + v.y >= canvas_height) {
			v.y = canvas_height - y - 1;
			drawLine(x, y, v, color);
		}else {
			drawLine(x, y, v, color);
		}
	}
	else {
		if (y + v.y <= canvas_height) {
			y = canvas_height - 1;
			drawLine(x, y, v, color);
		}
	}
}

#ifndef IGNORE_LAMBDAS

//you can apply and algorithm for two images and store the result in the first one
//forEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void forEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

#endif