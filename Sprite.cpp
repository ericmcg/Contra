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

#include "Sprite.hh"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// display the sprite centered at (x, y) and rotated theta degrees
// sw, sh = scaled width, height.  if none provided, default sprite size is used

void Sprite::draw(float x, float y, float theta)
{
  draw(x, y, theta, w, h);
}

void Sprite::draw(float x, float y, float theta, float sw, float sh)
{
  glColor3f(r, g, b);

  glBindTexture(GL_TEXTURE_2D, texture_number);  

  glPushMatrix();

  glTranslatef(x, y, 0);
  glRotatef(theta, 0, 0, 1);

  glBegin(GL_QUADS);
    glTexCoord2f(s, t);                   glVertex3f(-0.5*sw, -0.5*sh, 0.0);
    glTexCoord2f(s, t + tdelta);          glVertex3f(-0.5*sw,  0.5*sh, 0.0);
    glTexCoord2f(s + sdelta, t + tdelta); glVertex3f( 0.5*sw,  0.5*sh, 0.0);
    glTexCoord2f(s + sdelta, t);          glVertex3f( 0.5*sw, -0.5*sh, 0.0);
  glEnd();

  glPopMatrix();

  glColor3f(1,1,1);
}

//----------------------------------------------------------------------------

void AnimatedSprite::draw(float x, float y, float theta)
{
  draw(x, y, theta, w, h);
}

void AnimatedSprite::draw(float x, float y, float theta, float sw, float sh)
{
  s = sd * (float) (frame % cols);
  t = td * (float) (frame / cols);
  sdelta = sd;
  tdelta = td;
  ((Sprite *) this)->draw(x, y, theta, sw, sh);

  if (++delay > max_delay) {
    delay = 0; 
    if (frame <= max_frame) 
      frame++;
  }
  
  s = t = 0.0; sdelta = tdelta = 1.0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void load_sprites(int numTextures, char **texfilename)
{
  int i;
  unsigned char **texim;
  int *texim_w, *texim_h;

  texim = (unsigned char **) calloc(numTextures, sizeof(unsigned char *));
  texim_w = (int *) calloc(numTextures, sizeof(int));
  texim_h = (int *) calloc(numTextures, sizeof(int));

  for (i = 0; i < numTextures; i++) {

    texim[i] = read_PPM(texfilename[i], &texim_w[i], &texim_h[i]);

    printf("loading %s: w = %i, h = %i\n", texfilename[i], texim_w[i], texim_h[i]);

    glBindTexture(GL_TEXTURE_2D, i+1);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, texim_w[i], texim_h[i], 0, GL_RGBA, 
		 GL_UNSIGNED_BYTE, texim[i]);
  }
}

//----------------------------------------------------------------------------

// sets black pixels to transparent

unsigned char *read_PPM(char *filename, int *width, int *height)
{
  int i, j, w, h;
  FILE *fp;
  int r, g, b, t;
  bool iscomment;
  unsigned char ur, ug, ub;
  char imtype[3];
  char str[80];
  char c;
  unsigned char *im;
  int bpp;

  // get size info

  fp = fopen(filename, "rb");
  if (!fp) {
    printf("read_PPM(): no such file");
    exit(1);
  }

  fscanf(fp, "%s\n", &imtype);

  // attempt to eat comments

  do {
    iscomment = false;
    c = fgetc(fp);
    ungetc(c, fp);
    if (c == '#') {
      iscomment = true;
      fgets (str, 79, fp);
    }
  } while (iscomment);

  // read image dimensions

  fscanf(fp, "%i %i\n255\n", &w, &h);    
  *width = w;
  *height = h;

  // allocate image

  bpp = 4;

  im = (unsigned char *) calloc(w * h * bpp, sizeof(unsigned char));

  // actually read it in

  // ascii

  if (!strcmp(imtype, "P3")) {

    for (j = 0, t = 0; j < h; j++) {
      for (i = 0; i < w; i++, t += bpp) {
	fscanf(fp, "%i %i %i ", &r, &g, &b);
	im[t] = (unsigned char) r;
	im[t+1] = (unsigned char) g;
	im[t+2] = (unsigned char) b;
	if (r == 0 && g == 0 && b == 0)
	  im[t+3] = (unsigned char) 0;
	else
	  im[t+3] = (unsigned char) 255;
      }
    }
    fscanf(fp, "\n");
  }

  // binary

  else if (!strcmp(imtype, "P6")) {
    
    for (j = 0, t = 0; j < h; j++) {
      for (i = 0; i < w; i++, t += bpp) {
	fscanf(fp, "%c%c%c", &ur, &ug, &ub);
	im[t] = (unsigned char) ur;
	im[t+1] = (unsigned char) ug;
	im[t+2] = (unsigned char) ub;
	if (ur == 0 && ug == 0 && ub == 0)
	  im[t+3] = (unsigned char) 0;
	else
	  im[t+3] = (unsigned char) 255;
      }
    }
  }

  // unknown

  else {
    printf("unrecognized ppm file type");
    exit(1);
  }

  // finish up

  fclose(fp);

  return im;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
