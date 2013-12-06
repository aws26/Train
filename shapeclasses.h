#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h> 
#include <GL/glut.h>

#define PI 3.14159
#define TWOPI 6.28319
#define HALFPI 1.57080
#define RAD_FRACTION 6

#define CUBE         1
#define HOUSE         2
#define SPHERE         3
#define CYLINDER 4
#define CONE         5
#define TORUS         6

class Vertex3d
{
       public:        
               float coords[4];
               float vnorm[4];
               int touchcount;

               Vertex3d();
               Vertex3d(float,float,float);
               Vertex3d getvector(Vertex3d*);
               Vertex3d crossmult(Vertex3d);
               void addnorm(Vertex3d);
               Vertex3d normalize(void);
               void normalizevnorm(void);
               void printvertex(void);

               Vertex3d* translate(double xv, double yv, double zv);
               Vertex3d* rotate_z(double theta);
               Vertex3d* rotate_y(double theta);

};

class Matrix
{
       public:
               double grid[4][4];

               Matrix();
               Vertex3d* multiply(Vertex3d*);
               Matrix* multiply(Matrix*);
               void printmatrix(void);
};

class Face
{
       public:
               int vertnum;
               Vertex3d* corners[4];

               Face();
               Face(Vertex3d*,Vertex3d*,Vertex3d*);
               Face(Vertex3d*,Vertex3d*,Vertex3d*,Vertex3d*);

               //could put in a color val, maybe, like stitcher.h
};

class Shape3d
{
       public:
               int type;
               int rendermode;
               Vertex3d* bot;
               Vertex3d* top;
               Vertex3d* front;
               Vertex3d* aim;
               Vertex3d* tbot; 
               Vertex3d* ttop;
               Vertex3d* tfront;
               Vertex3d* taim;

               int shapevs;
               int shapers;
               Vertex3d* verts[50][50];
               Vertex3d* transverts[50][50];
               int facecount;
               Face ** facelist;
               Matrix *ctm;

               GLfloat shine;
               GLfloat emi[4];
               GLfloat amb[4];
               GLfloat diff[4];
               GLfloat spec[4];

               GLfloat translate[4];
               GLfloat scale[4];
               GLfloat rotate[4];

               Shape3d(int);

               void make_cube(double size);
               void make_house(float size);
               void make_cylinder( double height, double ray, int rs, int vs );
               void make_sphere( double ray, int rs, int vs ) ;
               void make_cone(double height, double ray, int rs, int vs);
               void make_torus(double width, double ray, int rs, int vs);

               void makeface(int,int);
               void cubeface();
               void houseface();
               void cylface(int,int);
               void cylfacenew(int,int);
               void oldcylface(int,int);
               void torface(int,int);
               void tetface();
               void octface();

               void transform(int,int);
               void do_transform(int,int);

               void printcoords();
};


class Tile
{
public:
       Matrix* movement;  //set this manually for the moment
       int row;
       int col;
       int state;
	   Vertex3d corners[4]; // 0->3 is bottom left, top left, top right, bottom right

       Tile();
       Tile(int,int,int,float);
       void setstate(int, float);
	   void setspeed(float);
       void getnext(int*, int*);
	   Matrix get_rotate(Vertex3d, float,float);
};

class Train
{
public:
       Tile* position; //current tile
       Shape3d* body; //shape array
       bool move;  //to move or not, for more use later
       int movecount; //how many moves so far
       int maxmoves; //moves to get across tile
       Tile* map[5][5];

       Train(Tile*);
       //void maketiles();
       void followtrack();
};

