//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// OpenGL Sprite class v 1.1
//
// by Christopher Rasmussen
//
// CISC 440/660, September, 2008
// updated February, 2012
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#include <GL/glut.h>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void load_sprites(int, char **);
unsigned char *read_PPM(char *, int *, int *);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// display a single static image

class Sprite 
{
public:

  float s, t, sdelta, tdelta;    // bounding rect in texture coordinates
  float w, h;                    // width and height of scaled quad
  float r, g, b;                 // base color
  int texture_number;            // which texture

  // width, height; background r, g, b; texture number

  Sprite(float iw, float ih, float ir, float ig, float ib, int itn)
  { w = iw, h = ih; r = ir; g = ig; b = ib; texture_number = itn; 
    s = t = 0.0; sdelta = tdelta = 1.0; }

  void draw(float, float, float, float, float);
  void draw(float, float, float);

};

//----------------------------------------------------------------------------

// sequentially display subimages from a single texture image

class AnimatedSprite : public Sprite
{
public:

  int cols, rows;         // arrangement of frame subimages within larger image
  int frame, max_frame;   // current, last frame numbers of animation sequence
  int delay, max_delay;   // inter-frame wait during animation (controls speed)
  float sd, td;           // width, height of each frame subimage in texture coordinates

  // sprite texture is array of cols x rows frames; delay is how long to wait before showing next image;
  // width, height, r, g, b, and texture number are as above

  AnimatedSprite(int icols, int irows, int imax_delay, float iw, float ih, float ir, float ig, float ib, int itn) :
    Sprite(iw, ih, ir, ig, ib, itn)
  { cols = icols; rows = irows; max_delay = imax_delay; frame = delay = 0; max_frame = cols * rows - 1; 
    sd = 1.0 / (float) cols; td = 1.0 / (float) rows; }

  void draw(float, float, float, float, float);
  void draw(float, float, float);

};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
