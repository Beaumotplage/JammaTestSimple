// A little freebie.
// Reads in a bitmap and outputs it as an array for you to paste into your code
// So make this happen, paste a 24-bit BMP file into the .exe folder, of the correct dimensions, 
// change the name below and go to help\about when running the program
// It might crash, but it should save to cout.txt


#include <iostream>
#include <fstream> // for file I/O
#include "framework.h"
#include <stdio.h>
#include "..\..\App\app_global_utils.h"


//24-bit
constexpr char BITMAPBYTES = 3;


// Our custom 15-bit format
#define PACK16 

// Big buffer, 'cos I'm too lazy to make it dynamically sized on the file read in
#define IMAGEBUFFER (320*240*100)

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
}pixel;


// The file to open
char filepointer[20] = "skyline.bmp";
char outputname[20] = "skyline";

unsigned long g_imagelength = 0;
unsigned long g_imagewidth = 0;
unsigned long g_imageheight = 0;


pixel g_test_array[IMAGEBUFFER] = { 0 };
unsigned char g_image332[IMAGEBUFFER];
int g_image565[IMAGEBUFFER];




static unsigned char* readBMP(char* filename, int* width, int* height);
//void open_file(unsigned char* data, int x, int y);
static void output_to_file(unsigned char* data, int x, int y);


// Routines for importing a bitmap (for creating fonts)
void import_bitmap(void)
{

    int x, y;
    unsigned char* bitmapdata = readBMP(filepointer, &x, &y);

    g_imagelength = x * y;
    g_imagewidth = x;
    g_imageheight = y;

    if (g_imagelength < IMAGEBUFFER)
    {

        // Stick it in one of my pixel arrays for fun
        int x_out = 0;
        int y_out = y - 1;

        for (int z = 0; z < g_imagelength; z++)
        {
            g_test_array[z].blue = bitmapdata[z * BITMAPBYTES];
            g_test_array[z].green = bitmapdata[z * BITMAPBYTES + 1];
            g_test_array[z].red = bitmapdata[z * BITMAPBYTES + 2];
            // g_test_array[z].alpha = bitmapdata[z * BITMAPBYTES + 3];

            uint16_t colourcode16 = 0;
            uint8_t colourcode8;
            uint8_t red = g_test_array[z].red;
            uint8_t green = g_test_array[z].green;
            uint8_t blue = g_test_array[z].blue;

#ifdef PACK16        
            app_global_utils::packRGB332_extend16(red, green, blue, &colourcode16);
            if (colourcode16 == 0)
            {
                colourcode16 = BLACK;
            }
            g_image565[x_out + (y_out * x)] = colourcode16;

  
#else
            packRGB332(red, green, blue, &colourcode8);
            image332[x_out + (y_out * x)] = colourcode8;
#endif

            // increment through bitmap (y axis is bottom to top, so decrement here)
            x_out++;
            if (x_out >= x)
            {
                x_out = 0;
                y_out--;
            }

        }
        output_to_file(bitmapdata, x, y);
    }
    else
    {
        // File too big for buffer
        // Maybe output an error to the file

    }
}



// Read in a 24-bit bmp file 
unsigned char* readBMP(char* filename, int* width, int* height)
{

    FILE* f;
    errno_t err = fopen_s(&f, filename, "r b");

    //FILE* f = fopen_s(filename, "rb");
    unsigned char info[54];

    // read the 54-byte header
    fread_s(info, 54, sizeof(unsigned char), 54 / sizeof(unsigned char), f);


    // extract image height and width from header
    *width = *(int*)&info[18];
    *height = *(int*)&info[22];

    // allocate 3 bytes per pixel
    int size = BITMAPBYTES * *width * *height;
    unsigned char* data = new unsigned char[size];

    // read the rest of the data at once
    int numread = fread_s(data, size, sizeof(unsigned char), size / sizeof(unsigned char), f);
    fclose(f);

    return data;
}




/* Little utility to output the bitmap to a 'cout' file as a text string, 
    so you can then paste it back into your code as a sprite or something
    draws sixteen per line to avoid hours of scrolling
*/
void output_to_file(unsigned char* data, int x, int y)
{
    std::ofstream file;
    file.open("cout.txt");
    std::streambuf* sbuf = std::cout.rdbuf();
    std::cout.rdbuf(file.rdbuf());
    //cout is now pointing to a file

    std::cout << "unsigned short " <<outputname <<"_width = " <<(int)g_imagewidth << "\; \n";
    std::cout << "unsigned short " << outputname << "_height = " << (int)g_imageheight << "\; \n";
    std::cout << "unsigned short " << outputname <<"["<<g_imagelength<<"] = { \n";

    int line = 0;
    for (int z = 0; z < g_imagelength; z++)
    {

#ifdef PACK16        
        std::cout << (int)g_image565[z] << ",";
#else
        std::cout << (int)image332[z] << ",";
#endif
        line++;
        if (line == 16)
        {
            line = 0;
            std::cout <<  "\n";  
        }

    }

    std::cout << "}; \n ";

    file.close();
}


