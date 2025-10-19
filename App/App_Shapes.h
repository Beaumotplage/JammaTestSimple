#pragma once
#include <stdlib.h>

#define int16_t short

//#define ROBOCOP
//#define CUBE
//#define SONIC
#define LANDSCAPE

#ifdef CUBE
#define POLYGONS_ROM 12
#define VERTEXES_ROM 8
#define POLYGONS 12
#define VERTICES 8
#endif
#ifdef SONIC
#define POLYGONS_ROM 	286
#define VERTEXES_ROM 	285
#define POLYGONS 	286
#define VERTICES 	285
#endif

#ifdef ROBOCOP
#define POLYGONS_ROM 1641
#define VERTEXES_ROM 1641
#define POLYGONS 1641
#define VERTICES 1641
#endif

#ifdef LANDSCAPE
#define POLYGONS_ROM 2176
#define VERTEXES_ROM 1155
#define POLYGONS 1800
#define VERTICES 1155

#endif


extern const int16_t indices_rom[POLYGONS_ROM][3];
extern const int16_t vertices_rom[VERTEXES_ROM][3];
extern const unsigned char colours_rom[POLYGONS_ROM];
