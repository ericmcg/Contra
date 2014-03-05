//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// "Drop!" v. 1.4
//
// by Christopher Rasmussen
//
// CISC 440/640, September, 2008
// updated February, 2012
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "Sprite.hh"

#include <math.h>
#include <string.h>
#include <sys/time.h>

#include <GL/glut.h>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// numTextures = 4

void initialize_random();
double uniform_random(double, double);
int uniform_random_int(int, int);
void draw_string(float, float, float, char *);

bool checkCollision(float, float);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define MAX2(A, B)          (((A) > (B)) ? (A) : (B))

#define STATE_NORMAL    1
#define STATE_EXPLODING 2
#define STATE_GONE      3

// these values depend on the order in load_sprites()

#define TEXTURE_TIEFIGHTER  1
#define TEXTURE_SAUCER      2
#define TEXTURE_SHOT        3
#define TEXTURE_EXPLOSION   4

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class Dropper
{
public:

  float x, y, theta;                   // position, angle
  int state;                           // alive, exploding, or dead
  bool gunner_flag;                    // does it go for the spaceship directly?
  float dy, dtheta;                    // speed of falling, rotation
  Sprite *sprite;                      // live appearance
  AnimatedSprite *explosion_sprite;    // explosion animation to play

  Dropper(float, float, float, bool, int);

  void draw();
  void update();
  bool collision(float, float);

};

//----------------------------------------------------------------------------

class Ship 
{
public:

  int state;                             // alive or exploding (regenerates after death)
  float x, y;                            // position
  float dx;                              // lateral speed
  float w, h;                            // dimensions
  bool moving_left, moving_right;        // state of user key input 

  float shot_x, shot_y;                  // shot location
  float shot_dy;                         // shot vertical speed
  float shot_h;                          // shot height (size)

  Sprite *shot_sprite;                   // shot appearance
  AnimatedSprite *explosion_sprite;      // explosion animation

  Ship();

  void update();
  void draw();
  void draw_shot();

};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int win_w = 400;
int win_h = 500;

Dropper **dropper;    // array of falling objects
Ship *ship;           // user-controllable ship

char *score_string;   // level, lives, score go here
int level = 1;
int num_destroyed = 0;

bool textureFlag = true;

int numObjects = 8;   
int numGunners = 5;

int msDelay = 2;
float x_wiggle_scale = 1.0;   // lateral wiggle
float x_attack_scale = 1.0;   // gunner aggressiveness
float y_speed_scale = 1.0;    // downward drop

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

Ship::Ship()
{
  state = STATE_NORMAL; 
  x = win_w/2; 
  y = 20; 
  w = 20; 
  h = 30; 
  dx = 1.0;  // 0.25 

  moving_left = moving_right = false;

  shot_x = win_h + 1; 
  shot_dy = 4;   // 2
  shot_h = 15; 

  shot_sprite = new Sprite(32, 32, 1, 1, 1, TEXTURE_SHOT);
  // row, cols of animation frames; how many times to show each frame; width, height, color info, texture id
  explosion_sprite = new AnimatedSprite(4, 4, 10, 96, 96, .5, 1, .5, TEXTURE_EXPLOSION);  
}

//----------------------------------------------------------------------------

void Ship::update()
{
  // user input

  if (moving_left && x > 1)
    x -= dx * msDelay;
  if (moving_right && x < win_w - 1) 
    x += dx * msDelay;

  // rising shot

  if (shot_y <= win_h) {
    shot_y += shot_dy * msDelay;
    if (checkCollision(shot_x, shot_y)) 
      shot_y = win_h + 1;
  }

  // ship collision

  if (state == STATE_NORMAL && checkCollision(x, y + h/2)) {
    state = STATE_EXPLODING;
    num_destroyed++;
  }

  // end of explosion

  if (state == STATE_EXPLODING && explosion_sprite->frame >= explosion_sprite->max_frame) {
    state = STATE_NORMAL;
    explosion_sprite->delay = explosion_sprite->frame = 0;
  }
}

//----------------------------------------------------------------------------

// shot and ship exploding--this is texture stuff

void Ship::draw_shot()
{
  if (shot_y <= win_h) 
    shot_sprite->draw(shot_x, shot_y, 0);

  if (state == STATE_EXPLODING) 
    explosion_sprite->draw(x, y, 0);
}

//----------------------------------------------------------------------------

// non-texture stuff--only divided up because of 
// glEnable/glDisable(GL_TEXTURE_2D) division in display

void Ship::draw()
{
  if (state == STATE_NORMAL) {
    glBegin(GL_POLYGON);
    glColor3f(1,1,1);
    glVertex2f(x, y + h);
    glVertex2f(x + w/2, y);
    glVertex2f(x - w/2, y);
    glEnd();
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

Dropper::Dropper(float init_x, float init_y, float init_theta, bool gflag, int texture_number)  
{ 
  float r, g, b;

  x = init_x;
  y = init_y;
  theta = init_theta;

  state = STATE_NORMAL;
  gunner_flag = gflag;

  r = g = b = 1.0;
  if (gunner_flag) {
    g = b = 0.5;
  }

  dtheta = uniform_random(-1, 1); 
  dy = uniform_random(-.2, -.4);

  sprite = new Sprite(24, 24, r, g, b, texture_number);
  explosion_sprite = new AnimatedSprite(4, 4, 10, 64, 64, 1, 1, 1, TEXTURE_EXPLOSION);  
}

//----------------------------------------------------------------------------

void Dropper::draw()
{
  if (state == STATE_EXPLODING) 
    explosion_sprite->draw(x, y, 0);
  else if (state == STATE_NORMAL) 
    sprite->draw(x, y, theta);
}

//----------------------------------------------------------------------------

// is something hitting the dropper?

bool Dropper::collision(float ix, float iy)
{
  if (state == STATE_NORMAL && hypot(ix - x, iy - y) < 10) {
    state = STATE_EXPLODING;
    return true;
  }
  else
    return false;
}

//----------------------------------------------------------------------------

// apply physics

void Dropper::update()
{
  float f;
  
  if (state == STATE_NORMAL) {

    // spin

    theta += dtheta * msDelay;
    if (theta <= 0)
      theta += 360;
    else if (theta >= 360)
      theta -= 360;

    // wiggle laterally

    x += uniform_random(-.5 * x_wiggle_scale, .5 * x_wiggle_scale);

    // "gunners" aim for the ship and fall with acceleration

    if (gunner_flag) {

      f = ((float) win_h - y) / ((float) win_h);
      y -= f * (1 + y_speed_scale * dy) * (1 + y_speed_scale * dy);
      x -= f * f * 0.01 * x_attack_scale * (x - ship->x) * msDelay;
    }
  }
  else if (state == STATE_EXPLODING && explosion_sprite->frame >= explosion_sprite->max_frame) {
    state = STATE_GONE;
    explosion_sprite->delay = explosion_sprite->frame = 0;
  }

  // drop if normal or exploding

  if (state != STATE_GONE) {

    // wrap to other side if leaving image laterally

    if (x <= -10)
      x += win_w + 10;
    else if (x > win_w + 10)
      x -= (win_w + 10);

    // start over again at top when bottom reached

    if (y <= 0) {
      y = win_h;

      // gunners start in random columns; others continue in same ones

      if (gunner_flag)
	x = uniform_random(0, win_w);
    }

    // keep falling

    else 
      y += y_speed_scale * dy * msDelay;
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// MONO characters are all same width: 104.76 pixels

void draw_string(float x, float y, float char_width, char *text)
{
  char *p;
   
  glPushMatrix();
  glTranslatef(x, y, 0);
  glScalef(char_width / 104.76, char_width / 104.76, 1);
  for (p = text; *p; p++)
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *p);
  // proportional: looks better, but characters are different widths
  //    glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
  glPopMatrix();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// set seed based on time so no two games are alike

void initialize_random()
{
  struct timeval tp;

  gettimeofday(&tp, NULL);
  srand48(tp.tv_sec);
}

//----------------------------------------------------------------------------

// random float in range [lower, upper]

double uniform_random(double lower, double upper)
{
  double result;
  double range_size;

  range_size = upper - lower;
  result = range_size * drand48();
  result += lower;

  return result;
}

//----------------------------------------------------------------------------

// random int in range [lower, upper]

int uniform_random_int(int lower, int upper)
{
  return (int) round(uniform_random(lower, upper));
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// is (x, y) in the middle of a dropper?

bool checkCollision(float x, float y)
{
  int i;

  for (i = 0; i < numObjects; i++) 
    if (dropper[i]->collision(x, y)) 
      return true;    

  return false;
}

//----------------------------------------------------------------------------

// has a key been released?

void keyboardup(unsigned char key, int x, int y)
{
  if (key == 'a') 
    ship->moving_left = false;
  else if (key == 'd') 
    ship->moving_right = false;
}

//----------------------------------------------------------------------------

// has a key been pressed?

void keyboard(unsigned char key, int x, int y)
{
  if (key == 'q')
    exit(1);

  if (key == 'a') 
    ship->moving_left = true;
  
  if (key == 'd')
    ship->moving_right = true;

  if (key == 'l' && ship->shot_y > win_h && ship->state == STATE_NORMAL) {
    ship->shot_x = ship->x;
    ship->shot_y = ship->y + ship->h;
  }

  if (key == 'p') 
    textureFlag = !textureFlag;

  if (key == '=')
    msDelay++;
  else if (key == '-' && msDelay > 1)
    msDelay--;

  if (key == ']')
    y_speed_scale += 0.1;
  else if (key == '[' && y_speed_scale > 1.0)
    y_speed_scale -= 0.1;
}

//----------------------------------------------------------------------------

// no other events

//void idle(int value)
void idle()
{
  int i;
  int numGone;

  // falling objects

  for (i = 0, numGone = 0; i < numObjects; i++) {
    dropper[i]->update();
    if (dropper[i]->state == STATE_GONE)
      numGone++;
  }

  // go to next level

  if (numGone == numObjects) {
    level++;
    x_attack_scale *= 1.25;
    y_speed_scale *= 1.25;
    for (i = 0; i < numObjects; i++) {
      dropper[i]->state = STATE_NORMAL;
      dropper[i]->x = uniform_random(0, win_w);
      dropper[i]->y = uniform_random(win_h/2, win_h);
    }
  }

  // ship

  ship->update();

  glutPostRedisplay();

  //  glutTimerFunc(msDelay, idle, 0);
}

//----------------------------------------------------------------------------

void display() 
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT);

  if (textureFlag) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
  }

  // falling objects

  for (i = 0; i < numObjects; i++) 
    dropper[i]->draw();

  // shot

  ship->draw_shot();

  if (textureFlag) {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }

  // ship

  ship->draw();

  // sample text
  
  sprintf(score_string, "Level %i   Ships lost %i", level, num_destroyed);
  draw_string(5, 5, 12, score_string);

  // finish up

  glutSwapBuffers();
}

//----------------------------------------------------------------------------

void init() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluOrtho2D(0, win_w, 0, win_h);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowSize(win_w, win_h);
  glutCreateWindow("drop!");
  init(); 			            // set OpenGL states, variables

  // game objects

  initialize_random();

  int numTextures = 4;
  char **texfilename = (char **) malloc(numTextures * sizeof(char *));
  for (int i = 0; i < numTextures; i++) 
    texfilename[i] = (char *) malloc(256 * sizeof(char));

  sprintf(texfilename[0], "tiefighter.ppm");
  sprintf(texfilename[1], "saucer.ppm");
  sprintf(texfilename[2], "fireball.ppm");
  sprintf(texfilename[3], "explosion.ppm");

  load_sprites(numTextures, texfilename);

  ship = new Ship;

  dropper = (Dropper **) malloc(numObjects * sizeof(Dropper *));

  for (int i = 0; i < numObjects; i++) 
    dropper[i] = new Dropper(uniform_random(0, win_w), uniform_random(win_h/2, win_h), 
			     uniform_random(-180, 180), i < numGunners, 
			     uniform_random_int(TEXTURE_TIEFIGHTER, TEXTURE_SAUCER));

  score_string = (char *) malloc(512 * sizeof(char));   

  // register callback routines

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardup);
  glutIdleFunc(idle);
  //  glutTimerFunc(msDelay, idle, 0);

  glutMainLoop(); 		    // enter event-driven loop

  return 1;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
