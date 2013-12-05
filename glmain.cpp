
 /**************************************************************************
File: glmain.c
Does: basic lighting and modeling for cs1566 hw4 Modeler
Author: Steven Lauck, based on some hwa
Date: 01/08/09
**************************************************************************/

#include "glmain.h"
#define my_assert(X,Y) ((X)?(void) 0:(printf("error:%s in %s at %d", Y, __FILE__,__LINE__), myabort()))

#define min(a,b) ((a) < (b)? a:b)
#define FALSE 0
#define TRUE  1
#define MAX_LIGHTS  8
#define NUM_OBJECTS 8
#define BOARD_SIZE 4
#define TILE_SIZE 2

typedef struct _Object {
  int sid;

  // example object storage arrays for vertex and normals
  GLfloat vertices_cube_smart[8][4];
  GLfloat normals_cube_smart[8][3];

  GLfloat shine;
  GLfloat emi[4];
  GLfloat amb[4];
  GLfloat diff[4];
  GLfloat spec[4];

  GLfloat translate[4];
  GLfloat scale[4];
  GLfloat rotate[4];

}OBJECT;

typedef struct _CAM{
  GLfloat pos[4];
  GLfloat at[4];
  GLfloat up[4];

  GLfloat dir[4];
}CAM;

typedef struct _LITE{
  GLfloat amb[4];
  GLfloat diff[4];
  GLfloat spec[4];
  GLfloat pos[4];
  GLfloat dir[3];
  GLfloat angle;
}LITE;

GLfloat colors [][3] = {
  {0.0, 0.0, 0.0},  /* black   */
  {1.0, 0.0, 0.0},  /* red     */
  {1.0, 1.0, 0.0},  /* yellow  */
  {1.0, 0.0, 1.0},  /* magenta */
  {0.0, 1.0, 0.0},  /* green   */
  {0.0, 1.0, 1.0},  /* cyan    */
  {0.0, 0.0, 1.0},  /* blue    */
  {0.5, 0.5, 0.5},  /* 50%grey */
  {1.0, 1.0, 1.0},   /* white   */
  {.588, .294, 0.0}   /* brown   */
};

GLfloat vertices_axes[][4] = {
        {0.0, 0.0, 0.0, 1.0},  /* origin */
        {5.0, 0.0, 0.0, 1.0},  /* maxx */
        {0.0, 5.0, 0.0, 1.0}, /* maxy */
        {0.0, 0.0, 5.0, 1.0}  /* maxz */

};

void make_cube_smart(OBJECT *po, double size );

void real_translation(OBJECT *po, GLfloat x, GLfloat y, GLfloat z);
void real_scaling(OBJECT *po, GLfloat sx, GLfloat sy, GLfloat sz);
void real_rotation(OBJECT *po, GLfloat deg, GLfloat x, GLfloat y, GLfloat z);

void real_translation(Shape3d* sh, GLfloat x, GLfloat y, GLfloat z);
void real_scaling(Shape3d* sh, GLfloat sx, GLfloat sy, GLfloat sz);
void real_rotation(Shape3d* sh, GLfloat deg, GLfloat ax, GLfloat ay, GLfloat az,
GLfloat cx, GLfloat cy, GLfloat cz );

OBJECT my_objects[NUM_OBJECTS];
LITE my_lights[MAX_LIGHTS];
int num_objects;
int  num_lights;

Shape3d* shapelist[NUM_OBJECTS];
int nextindex;

Vertex3d* intersectionlist[NUM_OBJECTS];
int numintersections;

int currTile=0;
Tile* theTiles[BOARD_SIZE][BOARD_SIZE];
Shape3d* tileShapes[BOARD_SIZE][BOARD_SIZE];

double sph_rad ;
double cyl_h ;
double cyl_rad;
double con_h;
double con_rad;
double tor_w;
double tor_rad;

bool ray_enabled;
bool norms_enabled;
bool jump_enabled;
int jumpcount;

int theta_x, theta_y, theta_z;
int camera_mode = NONE_MODE;

// camera variables
CAM my_cam;
GLfloat camx, camy, camz;
GLfloat atx, aty, atz;
GLfloat upx, upy, upz;

//TRUE or FALSE
int firstPersonView;

int crt_render_mode;
int crt_shape, crt_rs, crt_vs;
int crt_transform;

void myabort(void) {
  abort();
  exit(1); /* exit so g++ knows we don't return. */
}

int main(int argc, char** argv)
{
  setbuf(stdout, NULL);   /* for writing to stdout asap */
  glutInit(&argc, argv);

  my_setup(argc, argv);
  glut_setup();
  gl_setup();

  printf("Command List:\n");
  printf("Camera movement modes (hold):\n");
  printf("\tt: Translation\n");
  printf("\tr: Rotation\n");
  printf("Camera movement (uses current Camera Mode):\n");
  printf("\tX,x: Positive x, negative x\n");
  printf("\tY,y: Positive y, negative y\n");
  printf("\tZ,z: Positive z, negative z\n\n");

  glutMainLoop();
  return(0);
}


void glut_setup (){

  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);

  glutInitWindowSize(700,700);
  glutInitWindowPosition(20,20);
  glutCreateWindow("CS1566 Project 4");

  /* set up callback functions */
  glutDisplayFunc(my_display);
  glutReshapeFunc(my_reshape);
  glutMouseFunc(my_mouse);
  glutMotionFunc(my_mouse_drag);
  glutKeyboardFunc(my_keyboard);
  glutIdleFunc( my_idle );
  glutTimerFunc( 1000, my_TimeOut, 0);

  return;
}

void gl_setup(void) {

  // enable depth handling (z-buffer)
  glEnable(GL_DEPTH_TEST);

  // enable auto normalize
  glEnable(GL_NORMALIZE);

  // define the background color
  glClearColor(0,0,0,1);

  glMatrixMode(GL_PROJECTION) ;
  glLoadIdentity() ;
  gluPerspective( 40, 1.0, 1, 200.0);
  glMatrixMode(GL_MODELVIEW) ;
  glLoadIdentity() ;  // init modelview to identity

  // toggle to smooth shading (instead of flat)
  glShadeModel(GL_SMOOTH);
  lighting_setup();


  return ;
}

void my_setup(int argc, char **argv){

  firstPersonView = 1;
  num_objects = num_lights = 0;

  // initialize global shape defaults and mode for drawing
  crt_render_mode = GL_POLYGON;
  crt_shape = 0;

  crt_rs = 40;
  crt_vs = 40;

  theta_x = 0;
  theta_y = 0;
  theta_z = 0;
  camera_mode = NONE_MODE;

  nextindex = 0;
  numintersections = 0;

        sph_rad = 1;
        cyl_h = 1;
        cyl_rad = 1;
        con_h = 1;
        con_rad = .5;
        tor_w = 1;
        tor_rad = .5;

        ray_enabled = false;
        norms_enabled = false;
        jump_enabled = false;
        jumpcount = 0;

  //If you want to allow the user to type in the spec file
  //then modify the following code.
  //Otherwise, the program will attempt to load the file as specified
  //on the command line:
  //EX: ./glmain spec3
  my_assert(argc >1, "need to supply a spec file");
  read_spec(argv[1]);
  return;
}



void parse_floats(char *buffer, GLfloat nums[]) {
  int i;
  char *ps;

  ps = strtok(buffer, " ");
  for (i=0; ps; i++) {
    nums[i] = atof(ps);
    ps = strtok(NULL, " ");
    //printf("read %f ",nums[i]);
  }

}

void parse_obj(char *buffer){
  OBJECT *po;
  char *pshape, *pshine, *pemi, *pamb, *pdiff, *pspec, *ptranslate, *pscale, *protate;



  my_assert ((num_objects < NUM_OBJECTS), "too many objects");
  po = &my_objects[num_objects++];

  pshape  = strtok(buffer, " ");
  //printf("pshape is %s\n",pshape);

  ptranslate    = strtok(NULL, "()");  strtok(NULL, "()");
  pscale        = strtok(NULL, "()");  strtok(NULL, "()");
  protate       = strtok(NULL, "()");  strtok(NULL, "()");

  pshine  = strtok(NULL, "()");strtok(NULL, "()");
  //printf("pshine is %s\n",pshine);

  pemi    = strtok(NULL, "()");  strtok(NULL, "()");
  pamb    = strtok(NULL, "()");  strtok(NULL, "()");
  pdiff   = strtok(NULL, "()");  strtok(NULL, "()");
  pspec   = strtok(NULL, "()");  strtok(NULL, "()");


  po->sid  = atoi(pshape);

  po->shine = atof(pshine);

  //parse_floats(ptranslate, po->translate);
  //parse_floats(pscale, po->scale);
  //parse_floats(protate, po->rotate);

  //printf("po scale vals: %f, %f, %f\n", po->scale[0], po->scale[1], po->scale[2]);

  //parse_floats(pemi, po->emi);
  //parse_floats(pamb, po->amb);
  //parse_floats(pdiff, po->diff);
  //parse_floats(pspec, po->spec);

  shapelist[nextindex] = new Shape3d(po->sid);
  Shape3d* sh = shapelist[nextindex];
  nextindex++;

  sh->shine = po->shine;

  parse_floats(ptranslate, sh->translate);
  parse_floats(pscale, sh->scale);
  parse_floats(protate, sh->rotate);

 // printf("scale vals: %f, %f, %f\n", sh->scale[0], sh->scale[1], sh->scale[2]);

  parse_floats(pemi, sh->emi);
  parse_floats(pamb, sh->amb);
  parse_floats(pdiff, sh->diff);
  parse_floats(pspec, sh->spec);

  for(int a=0; a<3; a++) // make the rotation in rads
  {
          sh->rotate[a] = (sh->rotate[a]/180) * PI;
  }

  // use switch to create your objects, cube given as example
  switch (po->sid)
  {
  case 1: //cube
         // make_cube_smart(po, 1);
          sh->make_cube(.5);
          //printf("old cube coords:\n");

          /*printf("my cube coords:\n");
          for(int b=0; b<2; b++)
          {
                  for(int c=0; c<4; c++)
                  {
                          sh->verts[b][c]->printvertex();
                  }
          }*/
    break;
        case 2: //house
                sh->make_house(.5);
        break;
        case 3: //sphere
                sh->make_sphere(sph_rad, crt_rs, crt_vs);
        break;
        case 4: //cylinder
                sh->make_cylinder(cyl_h, cyl_rad, crt_rs, crt_vs);
        break;
        case 5: //cone
                sh->make_cone(con_h, con_rad, crt_rs, crt_vs);
        break;
        case 6: //torus
                sh->make_torus(tor_w, tor_rad, crt_rs, crt_vs);
        break;
  }

  // scale, rotate, translate using your real tranformations from assignment 3 depending on input from spec file

  /*real_scaling(po, po->scale[0], po->scale[1], po->scale[2]);
  real_rotation(po, po->rotate[0], 1, 0, 0);
  real_rotation(po, po->rotate[1], 0, 1, 0);
  real_rotation(po, po->rotate[2], 0, 0, 1);
  real_translation(po, po->translate[0], po->translate[1], po->translate[2]);*/

        //sh->printcoords();

        //printf("scale vals: %f, %f, %f\n", sh->scale[0], sh->scale[1], sh->scale[2]);

        real_scaling(sh, sh->scale[0], sh->scale[1], sh->scale[2]);
        real_rotation(sh, sh->rotate[0], 1, 0, 0, 0, 0, 0); //top then bot
        real_rotation(sh, sh->rotate[1], 0, 1, 0, 0, 0, 0);
        real_rotation(sh, sh->rotate[2], 0, 0, 1, 0, 0, 0);
        real_translation(sh, sh->translate[0], sh->translate[1], sh->translate[2]);

  printf("read object\n");
  //sh->printcoords();
}

void parse_camera(char *buffer){
  CAM *pc;
  char *ppos, *plook, *pup;

  pc = &my_cam;

  strtok(buffer, "()");
  ppos  = strtok(NULL, "()");  strtok(NULL, "()");
  plook  = strtok(NULL, "()");  strtok(NULL, "()");
  pup  = strtok(NULL, "()");  strtok(NULL, "()");

  parse_floats(ppos, pc->pos);
  parse_floats(plook, pc->at);
  parse_floats(pup, pc->up);

  pc->at[0] += pc->pos[0];
  pc->at[1] += pc->pos[1];
  pc->at[2] += pc->pos[2];

  pc->dir[0] = pc->at[0] - pc->pos[0];
  pc->dir[1] = pc->at[1] - pc->pos[1];
  pc->dir[2] = pc->at[2] - pc->pos[2];
  normalize(pc->dir);
  printf("read camera\n");
}

void parse_light(char *buffer){
  LITE *pl;
  char *pamb, *pdiff, *pspec, *ppos, *pdir, *pang;
  my_assert ((num_lights < MAX_LIGHTS), "too many lights");
  pl = &my_lights[++num_lights];

  strtok(buffer, "()");
  pamb  = strtok(NULL, "()");  strtok(NULL, "()");
  pdiff = strtok(NULL, "()");  strtok(NULL, "()");
  pspec = strtok(NULL, "()");  strtok(NULL, "()");
  ppos  = strtok(NULL, "()");  strtok(NULL, "()");
  pdir  = strtok(NULL, "()");  strtok(NULL, "()");
  pang  = strtok(NULL, "()");

  parse_floats(pamb, pl->amb);
  parse_floats(pdiff, pl->diff);
  parse_floats(pspec, pl->spec);
  parse_floats(ppos, pl->pos);
  if (pdir) {
    parse_floats(pdir, pl->dir);
    pl->angle = atof(pang);
    //printf("angle %f\n", pl->angle);
  }
  else
    pl->dir[0]= pl->dir[1]= pl->dir[2] =0;
  printf("read light\n");

}

void parse_tile(char *buffer)
{
  char *postion, *state;

  strtok(buffer, "()");
  postion  = strtok(NULL, "()");  strtok(NULL, "()");
  state    = strtok(NULL, "()");

  int row = atoi(strtok(postion, " "));
  int col = atoi(strtok(NULL, " "));

  int stateI = atoi(strtok(state, " "));


  int i = currTile % BOARD_SIZE;
  int j = int(currTile / BOARD_SIZE);

  //printf("ceate tile (%d, %d) at position (%d, %d) with state %d\n", i, j, row, col, stateI);
  //creates the tile object
  theTiles[i][j] = new Tile(row,col,stateI);
  currTile++;

  //creates the tile shape (cube)
  tileShapes[i][j] = new Shape3d(CUBE);
  Shape3d* sh = tileShapes[i][j];

  sh->make_cube(TILE_SIZE);
  real_scaling(sh, 1.0, .25, 1.0);
  real_translation(sh, (float(TILE_SIZE) + float(i*2*TILE_SIZE)), -float(TILE_SIZE/4.0), (float(TILE_SIZE) + float(j*2*TILE_SIZE)));
}

/* assuming the spec is going to be properly written
   not error-checking here */
void read_spec(char *fname) {
  char buffer[300];
  FILE *fp;


  fp = fopen(fname, "r");
  my_assert(fp, "can't open spec");
  while(!feof(fp)){
    fgets(buffer, 300, fp);
    //printf("read line: %s\n", buffer);
    switch (buffer[0]) {
    case '#':
      break;
    case '1': //shapes
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
             //read in the shape
          parse_obj(buffer);
           break;
        //etc

    case 'l':
      parse_light(buffer);
      break;

    case 'c':
      parse_camera(buffer);
      break;

	case 't':
		parse_tile(buffer);
		break;

    default:
      break;
    }
  }
}

void lighting_setup () {
  int i;
  GLfloat globalAmb[]     = {.1, .1, .1, .1};

  // create flashlight
  GLfloat amb[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat dif[] = {0.8, 0.8, 0.8, 1.0};
  GLfloat spec[] = {5.0, 5.0, 5.0, 1.0};

  //enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);

  // reflective propoerites -- global ambiant light
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

  // this was for the flashlights

 // glLightfv(GL_LIGHT0, GL_POSITION, my_cam.at);
 // glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, my_cam.dir);

 // glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
 // glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
 // glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
 // glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);

 // glEnable(GL_LIGHT0);

  // setup properties of lighting
  for (i=1; i<=num_lights; i++) {
    glEnable(GL_LIGHT0+i);
    glLightfv(GL_LIGHT0+i, GL_AMBIENT, my_lights[i].amb);
    glLightfv(GL_LIGHT0+i, GL_DIFFUSE, my_lights[i].diff);
    glLightfv(GL_LIGHT0+i, GL_SPECULAR, my_lights[i].spec);
    glLightfv(GL_LIGHT0+i, GL_POSITION, my_lights[i].pos);
    if ((my_lights[i].dir[0] > 0) ||  (my_lights[i].dir[1] > 0) || 
(my_lights[i].dir[2] > 0)) {
      glLightf(GL_LIGHT0+i, GL_SPOT_CUTOFF, my_lights[i].angle);
      glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, my_lights[i].dir);
    }
  }

}

void my_reshape(int w, int h) {
  // ensure a square view port
  glViewport(0,0,min(w,h),min(w,h)) ;
  return ;
}

//For the following 3 functions, do with them as you like.
//You will have to somehow rotate / translate / scale\
//the object as specified in the spec file.

void conetwist(GLfloat deg)
{
        Shape3d* p = shapelist[0];
        real_rotation(p, deg, p->ttop->coords[0], p->ttop->coords[1], p->ttop->coords[2], p->tbot->coords[0], p->tbot->coords[1], p->tbot->coords[2]);
}

void conewalk(float dist) 
{
        Shape3d* sh = shapelist[0];
		//sh->printcoords();
        float xa = sh->tfront->coords[0] - sh->tbot->coords[0];
        float ya = sh->tfront->coords[1] - sh->tbot->coords[1];
        float za = sh->tfront->coords[2] - sh->tbot->coords[2];

        float sum = (xa*xa) + (ya*ya) + (za*za);
        sum = sqrt(sum);

        xa = xa/sum;
        ya = ya/sum;
        za = za/sum;

        xa = xa * dist;
        ya = ya * dist;
        za = za * dist;

        real_translation(sh, xa, ya, za);
}

/*void my_scale(GLfloat x, GLfloat y, GLfloat z) {

}

// reuse code from assignment 3 here to do transformations when reading spec
void real_translation(OBJECT *po, GLfloat x, GLfloat y, GLfloat z) {

}

void real_scaling(OBJECT *po, GLfloat sx, GLfloat sy, GLfloat sz) {

}

void real_rotation(OBJECT *po, GLfloat deg, GLfloat x, GLfloat y, GLfloat z) {

}*/

void move_camera(float dx, float dy, float dz)
{
	my_cam.pos[0] += dx;
	my_cam.pos[1] += dy;
	my_cam.pos[2] += dz;

	my_cam.at[0] += dx;
	my_cam.at[1] += dy;
	my_cam.at[2] += dz;
}

void rot_camera(float d, int x, int y, int z)
{
	if(x == 1)
	{
		if(d>0)
			theta_x = (theta_x+2) %360;
		else
			theta_x = (theta_x-2) %360;
	}
	if(y == 1)
	{
		if(d>0)
			theta_y = (theta_y+2) %360;
		else
			theta_y = (theta_y-2) %360;
	}
	if(z == 1)
	{
		if(d>0)
			theta_z = (theta_z+2) %360;
		else
			theta_z = (theta_z-2) %360;
	}
}

void real_translation(Shape3d* sh, GLfloat x, GLfloat y, GLfloat z)
{
        Matrix* m = new Matrix(); // make translation matrix
        m->grid[0][3] = x;
        m->grid[1][3] = y;
        m->grid[2][3] = z;

        sh->ctm = sh->ctm->multiply(m); // multiply the new translation matrix into cumulative translation matrix

        switch(sh->type)
        {
        case CUBE:
                {
                sh->transform(2,4);
                sh->cubeface();
                };
                break;
        case TORUS:
                {
                        sh->transform(crt_vs, crt_rs);
                        sh->torface(crt_vs, crt_rs);
                };
                break;
        default:
                {
                sh->transform(crt_vs, crt_rs);
                sh->cylface(crt_vs, crt_rs);
                };
                break;
        }
}

void real_scaling(Shape3d* sh, GLfloat sx, GLfloat sy, GLfloat sz)
{
        Matrix* m = new Matrix(); // make scaling matrix

        m->grid[0][0] = sx;
        m->grid[1][1] = sy;
        m->grid[2][2] = sz;

        //printf("scale mat:\n");
        //m->printmatrix();

        sh->ctm = sh->ctm->multiply(m); // multiply the new translation matrix into cumulative translation matrix

        switch(sh->type)
        {
        case CUBE:
                {
                sh->transform(2,4);
                sh->cubeface();
                };
                break;
        case TORUS:
                {
                        sh->transform(crt_vs, crt_rs);
                        sh->torface(crt_vs, crt_rs);
                };
                break;
        default:
                {
                sh->transform(crt_vs, crt_rs);
                sh->cylface(crt_vs, crt_rs);
                };
                break;
        }
}

void real_rotation(Shape3d* sh, GLfloat deg, GLfloat ax, GLfloat ay, GLfloat az, GLfloat cx, GLfloat cy, GLfloat cz )
{
        //c is bot, a is top

        Matrix* m = new Matrix(); // first translation matrix

        m->grid[0][3] = -cx;  // translate the shape vertices down
        m->grid[1][3] = -cy;
        m->grid[2][3] = -cz;

        sh->ctm = sh->ctm->multiply(m);

        float ux = ax - cx; // move the point of the vector down
        float uy = ay - cy;
        float uz = az - cz;

        float sum = ux * ux; // normalize the vector
        sum += uy * uy;
        sum += uz * uz; 
        
        float length = sqrt(sum);

        ux = ux/length;
        uy = uy/length;
        uz = uz/length;
        
        m = new Matrix(); // second matrix: rotation

        m->grid[0][0] = (ux * ux) + cos(deg) * (1-(ux * ux));
        m->grid[0][1] = (ux * uy) * (1 - cos(deg)) - uz * sin(deg);
        m->grid[0][2] = (uz * ux) * (1 - cos(deg)) + uy * sin(deg);

        m->grid[1][0] = (ux * uy) * (1 - cos(deg)) + uz * sin(deg);
        m->grid[1][1] = (uy * uy) + cos(deg) * (1-(uy * uy));
        m->grid[1][2] = (uy * uz) * (1 - cos(deg)) - ux * sin(deg);

        m->grid[2][0] = (uz * ux) * (1 - cos(deg)) - uy * sin(deg);
        m->grid[2][1] = (uy * uz) * (1 - cos(deg)) + ux * sin(deg);
        m->grid[2][2] = (uz * uz) + cos(deg) * (1-(uz * uz));

        sh->ctm = sh->ctm->multiply(m);  // multiply rotation matrix in

        m = new Matrix(); // third matrix, translate back

        m->grid[0][3] = cx;  // translate the shape vertices up
        m->grid[1][3] = cy;
        m->grid[2][3] = cz;

        sh->ctm = sh->ctm->multiply(m);  // multiply translation matrix in

        switch(sh->type)
        {
        case CUBE:
                {
                sh->transform(2,4);
                sh->cubeface();
                };
                break;
        case TORUS:
                {
                        sh->transform(crt_vs, crt_rs);
                        sh->torface(crt_vs, crt_rs);
                };
                break;
        default:
                {
                sh->transform(crt_vs, crt_rs);
                sh->cylface(crt_vs, crt_rs);
                };
                break;
        }
}


void my_keyboard( unsigned char key, int x, int y ) {

  switch( key ) {
	//commands to control the camera
	case 'T':
	case 't':
		camera_mode = TRANSLATION_MODE;
		break;
	case 'R':
	case 'r':
		camera_mode = ROTATION_MODE;
		break;
	case 'x': {
		switch( camera_mode) {
			case ROTATION_MODE: rot_camera(-.5,1,0,0); break;
			case TRANSLATION_MODE: move_camera(-.5,0,0); break;
		}
		glutPostRedisplay(); 
	}; break;
	case 'X': {
		switch( camera_mode) {
			case ROTATION_MODE: rot_camera(.5,1,0,0); break;
			case TRANSLATION_MODE: move_camera(.5,0,0); break;
		}
		glutPostRedisplay(); 
	}; break;
	case 'y': {
		switch( camera_mode) {
			case ROTATION_MODE: rot_camera(-.5,0,1,0); break;
			case TRANSLATION_MODE: move_camera(0,-.5,0); break;
		}
		glutPostRedisplay(); 
	}; break;
	case 'Y': {
		switch( camera_mode) {
			case ROTATION_MODE: rot_camera(.5,0,1,0); break;
			case TRANSLATION_MODE: move_camera(0,.5,0); break;
		}
		glutPostRedisplay(); 
	}; break;
	case 'z': {
		switch( camera_mode) {
			case ROTATION_MODE: rot_camera(-.5,0,0,1); break;
			case TRANSLATION_MODE: move_camera(0,0,-.5); break;
		}
		glutPostRedisplay(); 
	}; break;
	case 'Z': {
		switch( camera_mode) {
			case ROTATION_MODE: rot_camera(.5,0,0,1); break;
			case TRANSLATION_MODE: move_camera(0,0,.5); break;
		}
		glutPostRedisplay(); 
	}; break;

  case ' ':
    //code to switch between first person view and back as appropriate
    glutPostRedisplay();
    break;
  case 'd':
    conetwist(PI/-30);
    glutPostRedisplay() ;
    break;
  case 'a':
    conetwist(PI/30);
    glutPostRedisplay() ;
    break;
  case 'w':
    conewalk(-1);
    glutPostRedisplay();
    break;
  case 's':
    conewalk(1);
    glutPostRedisplay();
    break;
  case 'b':
          if(norms_enabled)
          {
                  norms_enabled = false;
          }
          else
          {
                  norms_enabled = true;
          }
    glutPostRedisplay();
    break;
  case 'j':
    jump_enabled = true;
    break;
  case 'q':
  case 'Q':
    exit(0) ;
    break ;
  default: break;
  }

  return ;
}

/*******************************************************
FUNCTION: my_keyboard_up
ARGS: key id, x, y
RETURN:
DOES: handles keyboard up events
********************************************************/
void my_keyboard_up( unsigned char key, int x, int y )
{
	switch( key ) {
	case 'S':
	case 's': 
	case 'T':
	case 't':
	case 'R':
	case 'r': {
		camera_mode = NONE_MODE;
		break;
		}
	}
}

void my_mouse_drag(int x, int y) {
}

int my_raytrace_cube(OBJECT *s, float *rayStart, float *rayDirection, float result[3])
{
        // If there is an intersection, this should set result to the closest one to the ray start.

        // should return non-zero there is an intersection

        return 0;
}


Matrix* rotmat(GLfloat deg, int x, int y, int z)
{
        double si = sin(deg);
        double co = cos(deg);

        Matrix* m = new Matrix(); // make translation matrix

        if(x == 1)
        {
                m->grid[1][1] = co;
                m->grid[1][2] = si * -1;
                m->grid[2][1] = si;
                m->grid[2][2] = co;
        }

        if(y == 1)
        {
                m->grid[0][0] = co;
                m->grid[0][2] = si;
                m->grid[2][0] = si * -1;
                m->grid[2][2] = co;
        }

        if(z == 1)
        {
                m->grid[0][0] = co;
                m->grid[0][1] = si * -1;
                m->grid[1][0] = si;
                m->grid[1][1] = co;
        }

        return m;
}


float gett(float a, float b)
{
        bool agood = false;
        bool bgood = false;
        
        if(a >= 0)
        {
                agood = true;
        }
        if (b >= 0)
        {
                bgood = true;
        }

        if(agood && bgood)
        {
                return min(a, b);
        }
        else 
        {
                if(agood)
                {
                        return a;
                }
                if(bgood)
                {
                        return b;
                }
        }

        return -1;
}


float circlecheck(Vertex3d* p, Vertex3d* d, float y)
{
        //return -1 if no intersect

        if(d->coords[1] == 0)
        {
                return -1;
        }

        float tval = (y - p->coords[1])/d->coords[1];

        float xval = p->coords[0] + (d->coords[0] * tval);
        float zval = p->coords[2] + (d->coords[2] * tval);

        if(xval < -1 || xval > 1)
        {
                return -1;
        }

        if(zval < -1 || zval > 1)
        {
                return -1;
        }

        return tval;
}


float squarecheck(Vertex3d* p, Vertex3d* d, float dist, int x, int y, int z)
{
        //get the axis the square is perpendicular to
        int axis, other1, other2;
        if(x == 1)
        {
                axis = 0;
                other1 = 1;
                other2 = 2;
        }
        if(y == 1)
        {
                axis = 1;
                other1 = 0;
                other2 = 2;
        }
        if(z == 1)
        {
                axis = 2;
                other1 = 0;
                other2 = 1;
        }

        if(d->coords[axis] == 0) // if no intersect with the plane
        {
                return -1;
        }

        float tval = (dist - p->coords[axis])/d->coords[axis];

        float val1 = p->coords[other1] + (d->coords[other1] * tval);
        float val2 = p->coords[other2] + (d->coords[other2] * tval);

        val1 = abs(val1);
        val2 = abs(val2);

        float side;

        if(val1 > val2)
        {
                side = val1;
        }
        else
        {
                side = val2;
        }

        if(side > .5) // outside square
        {
                return -1;
        }

        return tval;
}


Vertex3d* unit_sphere_intersect(Vertex3d* p, Vertex3d* d)
{
        Vertex3d* result = new Vertex3d();
        float tfirst;

        //d should be normalized
        float a = 0;
        float b = 0;
        float c = 0;
        
        for(int e=0; e<3; e++) //get values
        {
                a += d->coords[e] * d->coords[e];
                b += p->coords[e] * d->coords[e];
                c += p->coords[e] * p->coords[e];
        }

        b = b*2;
        c -= 1;

        float root = b*b-4*a*c;

        if(root < 0)
        {
                // no intersection
                result->coords[3] = 0;
                return result;
        }

        float negb = b*-1;
        float denom = 2*a;

        if(root == 0)
        {
                //only one intersection
                tfirst = negb/denom;
        }
        else
        {
                root = sqrt(root);
                float t1 = (negb + root) / denom;
                float t2 = (negb - root) / denom;

                tfirst = gett(t1, t2);

                if(tfirst < 0)
                {
                        // no intersection
                        result->coords[3] = 0;
                        return result;
                }
        }

        float tx = d->coords[0] * tfirst;
        float ty = d->coords[1] * tfirst;
        float tz = d->coords[2] * tfirst;

        tx += p->coords[0];
        ty += p->coords[1];
        tz += p->coords[2];

        result->coords[0] = tx;
        result->coords[1] = ty;
        result->coords[2] = tz;
        result->coords[3] = 1;

        return result;
}


Vertex3d* unit_cylinder_intersect(Vertex3d* p, Vertex3d* d)
{
        Vertex3d* result = new Vertex3d();
        float topts[4];
        float ty;

        //d should be normalized
        // do sides first
        float a = d->coords[0] * d->coords[0];
        a += d->coords[2] * d->coords[2];

        float b = d->coords[0] * p->coords[0];
        b += d->coords[2] * p->coords[2];
        b *= 2;

        float c = p->coords[0] * p->coords[0];
        c += p->coords[2] * p->coords[2];
        c -= 1;

        float root = b*b-4*a*c;
        float negb = b*-1;
        float denom = 2*a;

        if(root < 0)
        {
                // no intersection with sides
                topts[0] = -1;
                topts[1] = -1;
        }
        else if(root == 0)
        {
                //only one intersection
                topts[0] = negb/denom;
                topts[1] = -1;
        }
        else
        {
                root = sqrt(root);
                topts[0] = (negb + root) / denom;
                topts[1] = (negb - root) / denom;
        }

        for(int e=0; e<2; e++) // make sure they are within the actual height
        {
                ty = d->coords[1] * topts[e] + p->coords[1];
                if(ty > .5 || ty < -.5)
                {
                        topts[e] = -1;  // invalid if intersect too high or low
                }
        }

        // now check caps

        //float holdt = (1 - p->coords[1])/d->coords[1];
        topts[2] = circlecheck(p, d, 1); // check top
        topts[3] = circlecheck(p, d, -1); // check bottom

        // now we find the biggest result

        float max = topts[0];
        for(int f=1; f<4; f++)
        {
                if(topts[f] > max)
                {
                        max = topts[f];
                }
        }
        
        max += 3;

        for(int g=0; g<4; g++)  // make the negatives the biggest things in the array
        {
                if(topts[g] < 0)
                {
                        topts[g] = max;
                }
        }
        
        //now the 't's that were negative are the biggest things in the array
        // now we can just find the smallest option
        
        float min = topts[0];
        for(int h=1; h<4; h++)
        {
                if(topts[h] < min)
                {
                        min = topts[h];
                }
        }

        if(min == max)  // everything was negative
        {
                // no positive t intersect
                result->coords[3] = 0;
                return result;
        }

        float tx = d->coords[0] * min;
        ty = d->coords[1] * min;
        float tz = d->coords[2] * min;

        tx += p->coords[0];
        ty += p->coords[1];
        tz += p->coords[2];

        result->coords[0] = tx;
        result->coords[1] = ty;
        result->coords[2] = tz;
        result->coords[3] = 1;

        return result;
}


Vertex3d* unit_cube_intersect(Vertex3d* p, Vertex3d* d)
{
        Vertex3d* result = new Vertex3d();
        float topts[6]; // 6 sides it could hit
        //float ty;

        //d should be normalized
        // check all the sides

        topts[0] = squarecheck(p, d, .5, 1, 0, 0);
        topts[1] = squarecheck(p, d, -.5, 1, 0, 0);
        topts[2] = squarecheck(p, d, .5, 0, 1, 0);
        topts[3] = squarecheck(p, d, -.5, 0, 1, 0);
        topts[4] = squarecheck(p, d, .5, 0, 0, 1);
        topts[5] = squarecheck(p, d, -.5, 0, 0, 1);

        // now we find the biggest result

        float max = topts[0];
        for(int f=1; f<6; f++)
        {
                if(topts[f] > max)
                {
                        max = topts[f];
                }
        }
        
        max += 3;

        for(int g=0; g<6; g++)  // make the negatives the biggest things in the array
        {
                if(topts[g] < 0)
                {
                        topts[g] = max;
                }
        }
        
        //now the 't's that were negative are the biggest things in the array
        // now we can just find the smallest option
        
        float min = topts[0];
        for(int h=1; h<6; h++)
        {
                if(topts[h] < min)
                {
                        min = topts[h];
                }
        }

        if(min == max)  // everything was negative
        {
                // no positive t intersect
                result->coords[3] = 0;
                return result;
        }

        float tx = d->coords[0] * min;
        float ty = d->coords[1] * min;
        float tz = d->coords[2] * min;

        tx += p->coords[0];
        ty += p->coords[1];
        tz += p->coords[2];

        result->coords[0] = tx;
        result->coords[1] = ty;
        result->coords[2] = tz;
        result->coords[3] = 1;

        return result;
}


Vertex3d* pick_shape(Shape3d* sh, Vertex3d* p, Vertex3d* d)
{
        Vertex3d* result;
        switch(sh->type)
        {
        case CUBE:
                result = unit_cube_intersect(p, d);
                break;
        case SPHERE:
                result = unit_sphere_intersect(p, d);
                break;
        case CYLINDER:
                result = unit_cylinder_intersect(p, d);
                break;
        default: break;

                return result;
        }
}


Vertex3d* shape_intersect(Shape3d* sh)
{
        Shape3d* cone = shapelist[0];

        Vertex3d* start = new Vertex3d(cone->tbot->coords[0], cone->tbot->coords[1],cone->tbot->coords[2]);
        Vertex3d* direction = new Vertex3d(cone->taim->coords[0], cone->taim->coords[1],cone->taim->coords[2]);

        direction->coords[0] -= start->coords[0];
        direction->coords[1] -= start->coords[1];
        direction->coords[2] -= start->coords[2];

        direction = &(direction->normalize());

        direction->coords[3] = 0; //maybe?

        //have p and d
        //un translate, then unrotate, then unscale
        //
        // opposite of sh transforms

        Matrix* m = new Matrix();

        m->grid[0][3] = -1 * sh->translate[0];
        m->grid[1][3] = -1 * sh->translate[1];
        m->grid[2][3] = -1 * sh->translate[2];

        start = m->multiply(start);       // anti translate
        direction = m->multiply(direction);

        m = rotmat((sh->rotate[2] * -1), 0, 0, 1); // anti z rotate
        start = m->multiply(start);
        direction = m->multiply(direction);

        m = rotmat((sh->rotate[1] * -1), 0, 1, 0); // anti y rotate
        start = m->multiply(start);
        direction = m->multiply(direction);

        m = rotmat((sh->rotate[0] * -1), 1, 0, 0); // anti x rotate
        start = m->multiply(start);
        direction = m->multiply(direction);

        m = new Matrix();

        m->grid[0][0] = 1/sh->scale[0];
        m->grid[1][1] = 1/sh->scale[1];
        m->grid[2][2] = 1/sh->scale[2];

        start = m->multiply(start);        // anti scale
        direction = m->multiply(direction);

        direction = &(direction->normalize());

        
        Vertex3d* result = pick_shape(sh, start, direction); // this is where we get the intersect point

        if(result->coords[3] == 0) // if there was no intersect
        {
                return result;
        }

        m = new Matrix();

        m->grid[0][0] = sh->scale[0];  // scale
        m->grid[1][1] = sh->scale[1];
        m->grid[2][2] = sh->scale[2];

        result = m->multiply(result);

        m = rotmat((sh->rotate[0]), 1, 0, 0); // x rotate
        result = m->multiply(result);

        m = rotmat((sh->rotate[1]), 0, 1, 0); // y rotate
        result = m->multiply(result);

        m = rotmat((sh->rotate[2]), 0, 0, 1); // z rotate
        result = m->multiply(result);

        m = new Matrix();

        m->grid[0][3] = sh->translate[0]; // translate
        m->grid[1][3] = sh->translate[1];
        m->grid[2][3] = sh->translate[2];

        result = m->multiply(result);

        return result;
}


void my_raytrace()
{
        //iterate over each object in the list
        //check for intersections according to shape
        //print intersections

        Shape3d* sh;
        Vertex3d* inters[NUM_OBJECTS];

        for(int a=1; a<nextindex; a++)
        {
                sh = shapelist[a];
                inters[a] = shape_intersect(sh);
        }

        numintersections = 0;

        printf("Intersections:\n");
        for(int b=1; b<nextindex; b++)
        {
                //printf("checking shape %d\n", b);

                if(inters[b]->coords[3] != 0)
                {
                        shapelist[b]->rendermode = GL_LINE_LOOP;
                        printf("Shape %d at ", b);
                        inters[b]->printvertex();
                        intersectionlist[numintersections] = inters[b];
                        numintersections++;
                }
        }
}


void my_raytrace(int mousex, int mousey)
{
        double modelViewMatrix[16];
        double projMatrix[16];
        int viewport[4];
        int foundIntersection = 0;
        int hit = 0;
        int i;
        double clickPoint[3];
        float intersectionPoint[3];
        float closestPoint[3];
        float rayStart[3];
        float rayDirection[3];
        OBJECT *cur;

        // first we need to get the modelview matrix, the projection matrix, and the viewport
        glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
        glGetIntegerv(GL_VIEWPORT, viewport);

        // gluUnProject with a Z value of 1 will find the point on the far clipping plane
        // corresponding the the mouse click. This is not the same as the vector
        // representing the click.
        gluUnProject(mousex, mousey, 1.0, modelViewMatrix, projMatrix, viewport,
&clickPoint[0], &clickPoint[1], &clickPoint[2]);

        // Now we need a vector representing the click. It should start at the camera
        // position. We can subtract the click point, we will get the vector

        /* code for finding direction vector, set rayStart and rayDirection */

        // now go through the shapes and see if there is a hit
        for (i=0; i<num_objects; i++)
        {
                cur = my_objects + i;
                hit = 0;

                switch (cur->sid)
                {
                case 1:
                        hit = my_raytrace_cube(cur, rayStart, rayDirection, intersectionPoint);
                        break;
                default:
                        break;
                }

                // found intersection
                if (hit)
                {
                        if (foundIntersection)
                        {
                                // check to see if it is the closer than the closest one we found,
                                // and if it is, store it in closestPoint
                        }

                        else
                        {
                                closestPoint[0] = intersectionPoint[0];
                                closestPoint[1] = intersectionPoint[1];
                                closestPoint[2] = intersectionPoint[2];
                        }

                        foundIntersection = 1;
                }
        }

        if (foundIntersection)
        {
                printf("Intersected with object %s at (%f, %f, %f)\n", "object_name", closestPoint[0], closestPoint[1], closestPoint[2]);
        }
        else
        {
                printf("No intersection\n");
        }
}

void my_mouse(int button, int state, int mousex, int mousey) {

  switch( button ) {

  case GLUT_LEFT_BUTTON:
    if( state == GLUT_DOWN ) {

                //my_raytrace(mousex, mousey);
                my_raytrace();
                ray_enabled = true;
                glutPostRedisplay();
    }

    if( state == GLUT_UP ) {
    }
    break ;

  case GLUT_RIGHT_BUTTON:
    if ( state == GLUT_DOWN ) {
    }

    if( state == GLUT_UP ) {
    }
    break ;
  }



  return ;
}

float dotprod(float v1[], float v2[]) {
  float tot = 0;
  int i;
  for (i=0; i<4; i++)
    tot += v1[i]*v2[i];
  return tot;
}


void normalize(GLfloat *p) {
  double d=0.0;
  int i;
  for(i=0; i<3; i++) d+=p[i]*p[i];
  d=sqrt(d);
  if(d > 0.0) for(i=0; i<3; i++) p[i]/=d;
}


void cross(GLfloat *a, GLfloat *b, GLfloat *c, GLfloat *d) {
  d[0]=(b[1]-a[1])*(c[2]-a[2])-(b[2]-a[2])*(c[1]-a[1]);
  d[1]=(b[2]-a[2])*(c[0]-a[0])-(b[0]-a[0])*(c[2]-a[2]);
  d[2]=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);
  normalize(d);
}

void print_matrix(float my_matrix[])
{
  int i, j;

  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      printf ("%f ", my_matrix[i+j*4]);
    }
    printf ("\n");
  }
  printf ("\n");
}

void make_quad(GLfloat vertices[][3]) {
  glBegin(GL_POLYGON);
  {
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);
  }
  glEnd();
}

/*************************************
 FUNCTION: make_*; reuse your stitcher code here.
*************************************/
void make_cube_smart(OBJECT *po, double size){
  int i;
  printf("old cube verts:\n");
  // compute verts on PI/4 angels for x y z, then -x y z
  for(i = 0; i < 4; i++){
    po->vertices_cube_smart[i][0] = size*cos((M_PI/4));
    po->vertices_cube_smart[i][1] = -size*sin(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i][2] = size*cos(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i][3] = 1;
    // mirror on x axis
    po->vertices_cube_smart[i+4][0] = -size*cos((M_PI/4));
    po->vertices_cube_smart[i+4][1] = -size*sin(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i+4][2] = size*cos(i*(M_PI/2)+(M_PI/4));
    po->vertices_cube_smart[i+4][3] = 1;

                for(int j=0; j<4; j++)
                {
                        printf("%f, ", po->vertices_cube_smart[i][j]);
                }
                printf("\n");
                for(int k=0; k<4; k++)
                {
                        printf("%f, ", po->vertices_cube_smart[i+4][k]);
                }
                printf("\n");
  }

  //compute normals
    cross(po->vertices_cube_smart[0], po->vertices_cube_smart[1], po->vertices_cube_smart[2], po->normals_cube_smart[0]);
    cross(po->vertices_cube_smart[1], po->vertices_cube_smart[5], po->vertices_cube_smart[6], po->normals_cube_smart[1]);
    cross(po->vertices_cube_smart[5], po->vertices_cube_smart[4], po->vertices_cube_smart[7], po->normals_cube_smart[2]);
    cross(po->vertices_cube_smart[4], po->vertices_cube_smart[0], po->vertices_cube_smart[3], po->normals_cube_smart[3]);
}

/***********************************
 FUNCTION: draw_quad
 ARGS: - a vertex array
 - 4 indices into the vertex array defining a quad face
 - an index into the color array.
 RETURN: none
 DOES:  helper drawing function; draws one quad.
 For the normal to work out, follow left-hand-rule (i.e., counter clock
 wise)
*************************************/
void draw_quad(GLfloat vertices[][4], GLfloat *normals, int iv1, int iv2, int iv3,
int iv4, int ic) {
  glBegin(crt_render_mode);
  {
    glColor3fv(colors[ic]);
    glNormal3fv(normals);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
    glVertex4fv(vertices[iv1]);
    glVertex4fv(vertices[iv2]);
    glVertex4fv(vertices[iv3]);
    glVertex4fv(vertices[iv4]);
  }
  glEnd();
}

/***********************************
 FUNCTION: draw_cube_smart()
 ARGS: none
 RETURN: none
 DOES: draws a cube from quads
************************************/
void draw_cube_smart(OBJECT *po)
{
  /* sides */
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[0],0,1,2,3, BLUE);
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[1],1,5,6,2, BLUE);
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[2],5,4,7,6, BLUE);
  draw_quad(po->vertices_cube_smart, po->normals_cube_smart[3],4,0,3,7, BLUE);

  /* top and bottom */
  //draw_quad(po->vertices_cube_smart,3,2,6,7, BLUE);
  //draw_quad(po->vertices_cube_smart,0,1,5,4, BLUE);
}


/***********************************
  FUNCTION: my_draw_triangle
  ARGS: - a Face
        - an index into the color array.
  RETURN: none
  DOES:  helper drawing function; draws one triangle.
   For the normal to work out, follow left-hand-rule (i.e., ccw)
*************************************/
void my_draw_triangle(Face f, int ic, int mode) {
  glBegin(mode);
  {
        /*printf("triangle stuff:\n");
        for(int a=0; a<3; a++)
        {
                f.corners[a]->printvertex();
        }*/

    glColor3fv(colors[ic]);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
    glVertex4fv(f.corners[0]->coords);
        glVertex4fv(f.corners[1]->coords);
        glVertex4fv(f.corners[2]->coords);

  }
  glEnd();
}


/***********************************
  FUNCTION: draw_cube()
  ARGS: none
  RETURN: none
  DOES: draws a cube from squares
************************************/

void draw_cube(Shape3d* thecube)
{
        //printf("i should draw a cube\n");
        for (int i=0; i<thecube->facecount; i++)
        {
                //printf("face number %d\n", i);
                switch (i)
                {
                        case 0:
                        case 1: my_draw_triangle(*(thecube->facelist[i]), MAGENTA, thecube->rendermode); break;
                        case 2:
                        case 3: my_draw_triangle(*(thecube->facelist[i]), BLUE, thecube->rendermode); break;
                        case 4:
                        case 5: my_draw_triangle(*(thecube->facelist[i]), CYAN, thecube->rendermode); break;
                        case 6:
                        case 7: my_draw_triangle(*(thecube->facelist[i]), YELLOW, thecube->rendermode); break;
                        case 8:
                        case 9: my_draw_triangle(*(thecube->facelist[i]), RED, thecube->rendermode); break;
                        case 10:
                        case 11: my_draw_triangle(*(thecube->facelist[i]), WHITE, thecube->rendermode); break;
                }
        }
}

//copy of draw_cube, but allows to color choosing, will use texturing later
void draw_tile(Shape3d* thecube, int color)
{
	//printf("i should draw a cube\n");
	for (int i=0; i<thecube->facecount; i++)
	{
			//printf("face number %d\n", i);
			switch (i)
			{
					case 0:
					case 1: my_draw_triangle(*(thecube->facelist[i]), BROWN, thecube->rendermode); break;//left
					case 2:
					case 3: my_draw_triangle(*(thecube->facelist[i]), BROWN, thecube->rendermode); break;//front
					case 4:
					case 5: my_draw_triangle(*(thecube->facelist[i]), BROWN, thecube->rendermode); break;//right
					case 6:
					case 7: my_draw_triangle(*(thecube->facelist[i]), BROWN, thecube->rendermode); break;//back
					case 8:
					case 9: my_draw_triangle(*(thecube->facelist[i]), color, thecube->rendermode); break;//top
					case 10:
					case 11: my_draw_triangle(*(thecube->facelist[i]), BROWN, thecube->rendermode); break;//bottom
			}
	}
}


void draw_house(Shape3d* thehouse)
{
        //printf("i should draw a house\n");
        for (int i=0; i<thehouse->facecount; i++)
        {
                switch (i)
                {
                        case 0:
                        case 1: my_draw_triangle(*(thehouse->facelist[i]), MAGENTA, thehouse->rendermode); break;
                        case 2:
                        case 3: my_draw_triangle(*(thehouse->facelist[i]), BLUE, thehouse->rendermode); break;
                        case 4:
                        case 5: my_draw_triangle(*(thehouse->facelist[i]), CYAN, thehouse->rendermode); break;
                        case 6:
                        case 7: my_draw_triangle(*(thehouse->facelist[i]), YELLOW, thehouse->rendermode); break; 
                        case 8:
                        case 9: my_draw_triangle(*(thehouse->facelist[i]), RED, thehouse->rendermode); break;
                        case 10:
                        case 11: my_draw_triangle(*(thehouse->facelist[i]), WHITE, thehouse->rendermode); break;

                        case 12: 
                        case 14: my_draw_triangle(*(thehouse->facelist[i]), GREEN, thehouse->rendermode); break;
                        case 13: 
                        case 15: my_draw_triangle(*(thehouse->facelist[i]), GREY, thehouse->rendermode); break;
                }
        }
}


/***********************************
  FUNCTION: draw_cylinder()
  ARGS: none
  RETURN: none
  DOES: draws a cylinder from triangles
************************************/
/*TODO: stitch cylinder vertices together to make faces
don't call gl directly, use my_draw_triangle and my_draw_quad instead*/
void draw_cylinder(Shape3d* thecylinder)
{
        //printf("draw 4\n");
        for (int i=0; i<thecylinder->facecount; i++)
        {
                //printf("drawing face %d\n", i);
                if(i%4 == 0)
                {
                        my_draw_triangle(*(thecylinder->facelist[i]), WHITE, thecylinder->rendermode);
                }
                else
                {
                        my_draw_triangle(*(thecylinder->facelist[i]), RED, thecylinder->rendermode);
                }
        }
}



/***********************************
  FUNCTION: draw_sphere()
  ARGS: none
  RETURN: none
  DOES: draws a sphere from triangles
************************************/
/*TODO: stitch sphere vertices together to make faces
don't call gl directly, use make_triangle and make_quad instead*/
void draw_sphere(Shape3d* thesphere)
{
        //printf("draw 3\n");
        for (int i=0; i<thesphere->facecount; i++)
        {
                if(i%4 == 0)
                {
                        my_draw_triangle(*(thesphere->facelist[i]), WHITE, thesphere->rendermode);
                }
                else
                {
                        my_draw_triangle(*(thesphere->facelist[i]), RED, thesphere->rendermode);
                }
        }
}


void draw_cone(Shape3d* thecone)
{
        //printf("draw 5\n");
        for (int i=0; i<thecone->facecount; i++)
        {
                if(i%4 == 0)
                {
                        my_draw_triangle(*(thecone->facelist[i]), WHITE, thecone->rendermode);
                }
                else
                {
                        my_draw_triangle(*(thecone->facelist[i]), RED, thecone->rendermode);
                }
        }
}



void draw_torus(Shape3d* thetorus)
{
        //printf("draw 6\n");
        for (int i=0; i<thetorus->facecount; i++)
        {
                if(i%4 == 0)
                {
                        my_draw_triangle(*(thetorus->facelist[i]), WHITE, thetorus->rendermode);
                }
                else
                {
                        my_draw_triangle(*(thetorus->facelist[i]), RED, thetorus->rendermode);
                }
        }
}


/***********************************
 FUNCTION: draw_axes
 ARGS: none
 RETURN: none
 DOES: draws main X, Y, Z axes
************************************/
void draw_axes( void ) {
  glLineWidth( 5.0 );

  glDisable(GL_LIGHTING);

  glBegin(GL_LINES);
  {
    glColor3fv(colors[1]);
    glVertex4fv(vertices_axes[0]);
    glVertex4fv(vertices_axes[1]);

    glColor3fv(colors[4]);
    glVertex4fv(vertices_axes[0]);
    glVertex4fv(vertices_axes[2]);

    glColor3fv(colors[6]);
    glVertex4fv(vertices_axes[0]);
    glVertex4fv(vertices_axes[3]);
  }
  glEnd();
  glLineWidth( 1.0 );

  glEnable(GL_LIGHTING);

}

void draw_objects() {
        int i;
        for(i=0; i<nextindex; i++)
        {
                Shape3d *cur;
                cur = shapelist[i];

                glMaterialfv(GL_FRONT, GL_AMBIENT, cur->amb);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, cur->diff);
                glMaterialfv(GL_FRONT, GL_SPECULAR, cur->spec);
                glMaterialfv(GL_FRONT, GL_SHININESS, &cur->shine);
                //glMaterialfv(GL_FRONT, GL_EMISSION, cur->emi);

                switch(cur->type)
                {
                  case 1: //cube
                        //printf("going to draw cube\n");
                        //draw_cube_smart(&my_objects[i]);
                    draw_cube(cur);
                    break;
                  case 2: //house
                    draw_house(cur);
                    break;
                  case 3: //sphere
                    draw_sphere(cur);
                    break;
                  case 4: //cylinder
                    draw_cylinder(cur);
                    break;
                  case 5: //cone
                        //printf("going to draw cone\n");
                    draw_cone(cur);
                    break;
                  case 6: //torus
                    draw_torus(cur);
                    break;
                  default: break;
                }

                cur->rendermode = GL_POLYGON;
        }
}


void draw_ray()
{
        //printf("drawing ray\n");

        GLfloat target[4];
        //GLfloat start[4];

        /*start[0] = 0;
        start[1] = 0;
        start[2] = 0;
        start[3] = 1;*/

        //printf("start\n");
        //shapelist[0]->tbot->printvertex();
        //printf("aim\n");
        //shapelist[0]->taim->printvertex();

        target[0] = (shapelist[0]->taim->coords[0] - shapelist[0]->tbot->coords[0]) * 1000;
        target[1] = (shapelist[0]->taim->coords[1] - shapelist[0]->tbot->coords[1]) * 1000;
        target[2] = (shapelist[0]->taim->coords[2] - shapelist[0]->tbot->coords[2]) * 1000;
        target[3] = 1;

        /*target[0] = 5;
        target[1] = 5;
        target[2] = 0;
        target[3] = 1;*/

        glLineWidth( 5.0 );

        glDisable(GL_LIGHTING);

        glBegin(GL_LINES);
        {
                glColor3fv(colors[2]);
                glVertex4fv(shapelist[0]->tbot->coords);
                //glVertex4fv(start);
                glVertex4fv(target);
        }

        glEnd();
        glLineWidth( 1.0 );

        glEnable(GL_LIGHTING);

}


void draw_normals()
{
        Shape3d* base;
        float endline[4];

        glLineWidth( 5.0 );

        glDisable(GL_LIGHTING);

        for(int a=0; a<nextindex; a++)
        {
                base = shapelist[a];

                for(int g=0; g<base->shapevs; g++)
                {
                        for(int h=0; h<base->shapers; h++)
                        {
                                /*if(h==0)
                                {
                                        printf("\nPoint: %f, %f %f\n", base->verts[g][h]->coords[0], base->verts[g][h]->coords[1], base->verts[g][h]->coords[2]);
                                        printf("Normal: %f, %f, %f\n\n", base->verts[g][h]->vnorm[0], base->verts[g][h]->vnorm[1], base->verts[g][h]->vnorm[2]);
                                }*/

                                endline[0] = base->transverts[g][h]->coords[0] + base->transverts[g][h]->vnorm[0] * 0.2;
                                endline[1] = base->transverts[g][h]->coords[1] + base->transverts[g][h]->vnorm[1] * 0.2;
                                endline[2] = base->transverts[g][h]->coords[2] + base->transverts[g][h]->vnorm[2] * 0.2;
                                endline[3] = 1;

                                //draw_line(base->transverts[g][h], endline, GREEN);
                                glBegin(GL_LINES);
                                {
                                        glColor3fv(colors[5]);
                                        glVertex4fv(base->transverts[g][h]->coords);
                                        //glVertex4fv(start);
                                        glVertex4fv(endline);
                                }
                        }
                }
        }

        glEnd();
        glLineWidth( 1.0 );

        glEnable(GL_LIGHTING);
}


void draw_markers()
{
        
        for(int a=0; a<numintersections; a++)
        {
                Shape3d* mark = new Shape3d(SPHERE);
                mark->make_sphere(sph_rad, crt_rs, crt_vs);
                real_scaling(mark, .1, .1, .1);
                real_translation(mark, intersectionlist[a]->coords[0], intersectionlist[a]->coords[1], intersectionlist[a]->coords[2]);
                draw_sphere(mark);
        }
}


void draw_tiles()
{
	for(int i=0; i<BOARD_SIZE; i++)
	{
		for(int j=0; j<BOARD_SIZE; j++)
		{
			Shape3d *cur;
            cur = tileShapes[i][j];
			//switch color based on the state of the tile
			switch(theTiles[i][j]->state)
			{
				case 0:
					//draw_tile(cur, BLACK);
					break;
				case 1:
					draw_tile(cur, GREEN);
					break;
				default:
					draw_tile(cur, RED);
					break;
			}
			cur->rendermode = GL_POLYGON;
		}
	}
	//Shape3d *cur;
    //cur = tileShapes[0][0];
	//draw_tile(cur, GREEN);
}


void my_display() {

  // clear all pixels, reset depth
  glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT );

  glLoadIdentity();
  //setup the camera (1st person? 3rd person?)

  gluLookAt(my_cam.pos[0],my_cam.pos[1], my_cam.pos[2],
            my_cam.at[0],my_cam.at[1],my_cam.at[2],
            my_cam.up[0], my_cam.up[1], my_cam.up[2]);

  glRotatef(theta_x,1,0,0);
  glRotatef(theta_y,0,1,0);
  glRotatef(theta_z,0,0,1);

  //update the flashlight to follow the person

  //draw the objects
  draw_axes();

  draw_objects();

  draw_tiles();

  if(norms_enabled)
  {
          draw_normals();
  }

  if(ray_enabled)
  {
          ray_enabled = false;
          draw_ray();
  }

  if(numintersections > 0)
  {
        draw_markers();
  }

  // this buffer is ready
  glutSwapBuffers();
}

void my_idle(void) {
  //EC idea: Make the flashlight flicker a bit (random flicker strength) when the user is idle.
  return ;
}


void update_jump()
{
        switch(jumpcount)
        {
                case 0:
                        real_translation(shapelist[0], 0, 1, 0);
                        jumpcount++;
                        break;
                case 1:
                        real_translation(shapelist[0], 0, .5, 0);
                        jumpcount++;
                        break;
                case 2:
                        real_translation(shapelist[0], 0, .25, 0);
                        jumpcount++;
                        break;
                case 3:
                        real_translation(shapelist[0], 0, -.25, 0);
                        jumpcount++;
                        break;
                case 4:
                        real_translation(shapelist[0], 0, -.5, 0);
                        jumpcount++;
                        break;
                case 5:
                        real_translation(shapelist[0], 0, -1, 0);
                        jump_enabled = false;
                        jumpcount = 0;
                        break;
                default: break;
        }
}

void my_TimeOut(int id) {
        /* right now, does nothing*/
        /* schedule next timer event, 2 secs from now */ 
        
        if(jump_enabled)
        {
                printf("doing jump\n");
                update_jump();
                glutPostRedisplay();
        }

        glutTimerFunc(1000, my_TimeOut, 0);

        return ;
}