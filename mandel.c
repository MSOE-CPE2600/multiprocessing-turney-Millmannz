/** 
 * mandel.c
 * Based on example code found here:
 * https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
 * Converted to use jpg instead of BMP and other minor changes
 * 
 * Modified by: Zac Millmann
 * Assignment: Lab11 Multiprocessing
 * Course: CPE 2600 112
 * 
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <ctype.h>
#include <math.h>
#include "jpegrw.h"

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max );
static void show_help();
void create_image(int imageNum, const double k, double scale_start, double scale_end, double yscale, double xcenter, double ycenter, char *prefix, char *outfile, int max, int image_height, int image_width);
void extract_prefix(const char *filename, char *prefix);


int main( int argc, char *argv[] )
{
	char c;

	//Variables for filenames and their setup
	char prefix[256] = "";
	char outfile[256] = "";
	strncpy(outfile, "mandel.jpg", sizeof(outfile));
	outfile[sizeof(outfile)-1] = '\0';

	// Custom start point for good visual
	double xcenter = -1.41870966;

	// Default values
	double ycenter = 0;
	double yscale = 0; // calc later
	double xscale;
	int image_width = 1000;
	int image_height = 1000;
	int max = 1000;

	//num images can be changed here
	int numImg = 50;

	int numeric_value = 0;

	// Used to create a custom zoom function
	const double scale_start = 2.0;
	const double scale_end   = 0.0;
	const double epsilon     = 0.000001;   
	const double total_iters = (double)numImg;
	const double k = -log(epsilon) / total_iters;

	// For each command line argument given,
	// override the appropriate configuration value.
	// c added to allow user to specify # of processes to use

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:h:c:"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				strncpy(outfile, optarg, sizeof(outfile));
				outfile[sizeof(outfile)-1] = '\0';
				break;
			case 'h':
				show_help();
				exit(1);
				break;
			case 'c':
			 	numeric_value = strtol(optarg, NULL, 10);
                printf("Option -c received with numeric value: %d\n", numeric_value);
				break;
		}
	}

	// # of processes must be betweeen 1-20 otherwise images will not be created
	if(numeric_value > 0 && numeric_value < 21){

		int num_processes = numeric_value;
		int divSize = numImg/num_processes;
		int remain = numImg % num_processes;

		extract_prefix(outfile, prefix);

		// Used to debug, tells user information about how each process will behave
		printf("Num of processes: %d\n", num_processes);
		printf("Division size: %d\n", divSize);
		printf("Remainder: %d\n", remain);
		printf("Prefix: %s\n", prefix);

		// Create N number of processes
		for (int i = 0; i < num_processes; i++) {
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork failed");
				exit(1);
			} 
			else if (pid == 0) {
		
				//For each process create images for its assigned values
				//Ex. If there are 10 processes each makes 5 images
				//Process 2 would have images 6-10
				// for(int j = divSize*i+1; j < divSize*i + divSize+1; j++){
				// 	create_image(j, k, scale_start, scale_end, yscale, xcenter, ycenter, prefix, outfile);
				// }

				/** Tested implementation that also works but is slower
				 * More zoomed in images take longer to process so each process takes equal work load
				 * In this case process 2 would have images 2, 12, 22, 32, 42
				 * This didn't work because it is slower to write randomly than in order
				 * But could use pipe in future to fix problem and improve performance
				 */
				for(int j = i+1; j < numImg+1; j+=num_processes){
					create_image(j, k, scale_start, scale_end, yscale, xcenter, ycenter, prefix, outfile, max, image_height, image_width);
				}

				//Handles remainders by telling some process that they must make one more image
				//Ex. if there is 6 remainders processes 0-5 would each take an extra image
				int remain_num = numImg - (remain-i) + 1;
				if(i<remain){
					create_image(remain_num, k, scale_start, scale_end, yscale, xcenter, ycenter, prefix, outfile, max, image_height, image_width);
				}

				exit(0); 
			}
		}
		//Wait for all children processes to complete
		for (int i = 0; i < num_processes; i++) {
			wait(NULL);
		}
		}
		//If # of processes isn't specified run as normal
		else{
			
			extract_prefix(outfile, prefix);
			create_image(1, k, scale_start, scale_end, yscale, xcenter, ycenter, prefix, outfile, max, image_height, image_width);
		}

	return 0;
}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max )
{
	int i,j;

	int width = img->width;
	int height = img->height;

	// For every pixel in the image...

	for(j=0;j<height;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}


void extract_prefix(const char *filename, char *prefix) {
    int len = strlen(filename);
    int end = len - 1;

    while (end >= 0 && filename[end] != '.')
		//Find the . in the file name and save the location
        end--;

    if (end < 0) { 
		//if there is no . then assume that the name given doesn't have extension and return it
        end = len;
    }

    int i = end - 1;
	//ignore any # at the end of the filename
    while (i >= 0 && isdigit((unsigned char)filename[i])) {
        i--;
    }

	//copy the string to prefix to be used later
    strncpy(prefix, filename, i + 1);
    prefix[i + 1] = '\0';
}

//Copied and pasted previous code to not repeat code
//xscale uses custom zoom fucntion to create better video
void create_image(int imageNum, const double k, double scale_start, double scale_end, double yscale, double xcenter, double ycenter, char *prefix, char *outfile, int max, int image_height, int image_width){
	double xscale = scale_end + (scale_start - scale_end) * exp(-k * imageNum);

	//Stores all new images in a folder images/ and gives images custom # and
	// name based on user input and # of images
	sprintf(outfile, "images/%s%d.jpg", prefix, imageNum);

	yscale = xscale / image_width * image_height;

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(1000,1000);

	// Fill it with a black
	setImageCOLOR(img,0);

	// Compute the Mandelbrot image
	compute_image(img,xcenter-xscale/2,xcenter+xscale/2,ycenter-yscale/2,ycenter+yscale/2,max);

	// Save the image in the stated file.
	storeJpegImageFile(img,outfile);

	// free the mallocs
	freeRawImage(img);
}