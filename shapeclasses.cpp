#include "shapeclasses.h"
//include <iostream>
//#include <math.h> 

Vertex3d::Vertex3d()
{
        coords[0] = 0;
        coords[1] = 0;
        coords[2] = 0;
        coords[3] = 0;
}

Vertex3d::Vertex3d(float x, float y, float z)
{
        coords[0] = x;
        coords[1] = y;
        coords[2] = z;
        coords[3] = 1;
}

Vertex3d Vertex3d::getvector(Vertex3d* v)
{
        Vertex3d result;
        
        result.coords[0] = coords[0] - v->coords[0];
        result.coords[1] = coords[1] - v->coords[1];
        result.coords[2] = coords[2] - v->coords[2];
        result.coords[3] = 1;

        return result;
}

Vertex3d Vertex3d::crossmult(Vertex3d v)
{
        Vertex3d result;
        
        result.coords[0] = (coords[1] * v.coords[2]) - (coords[2] * v.coords[1]);
        result.coords[1] = ((coords[0] * v.coords[2]) - (coords[2] * v.coords[0])) * -1;
        result.coords[2] = (coords[0] * v.coords[1]) - (coords[1] * v.coords[0]);
        result.coords[3] = 1;

        return result;
}


void Vertex3d::addnorm(Vertex3d v)
{
        vnorm[0] += v.coords[0];
        vnorm[1] += v.coords[1];
        vnorm[2] += v.coords[2];
        vnorm[3] = 1;

        touchcount++;
}


Vertex3d Vertex3d::normalize()
{
        float xs = coords[0] * coords[0];
        float ys = coords[1] * coords[1];
        float zs = coords[2] * coords[2];
        float sum = xs+ys+zs;
        float len = sqrt(sum);
        
        if(sum == 0)
        {
                return *this;
        }

        float nx = coords[0]/len;
        float ny = coords[1]/len;
        float nz = coords[2]/len;

        return *(new Vertex3d(nx, ny, nz));
}


void Vertex3d::normalizevnorm()
{
        float sum = vnorm[0] * vnorm[0]; 
        sum += vnorm[1] * vnorm[1];
        sum += vnorm[2] * vnorm[2]; 
        
        float length = sqrt(sum);

        vnorm[0] = vnorm[0]/length;
        vnorm[1] = vnorm[1]/length;
        vnorm[2] = vnorm[2]/length;
        vnorm[3] = 1;
}


void Vertex3d::printvertex()
{
        printf("%f, %f, %f, %f\n", coords[0], coords[1], coords[2], coords[3]);
}


Vertex3d* Vertex3d::translate(double xv, double yv, double zv)
{
        Matrix* m = new Matrix();

        m->grid[0][3] = xv;
        m->grid[1][3] = yv;
        m->grid[2][3] = zv;

        Vertex3d* result = m->multiply(this);

        return result;
}


/***********************************
  FUNCTION: rotate_z
  ARGS: starting vertex, angle to rotate by
  RETURN: a new vertex
  DOES: transformation function; 
        rotates a vertex about the z axis
************************************/
Vertex3d* Vertex3d::rotate_z(double theta)
{
        double co = cos(theta);
        double si = sin(theta);

        Matrix* m = new Matrix();

        m->grid[0][0] = co;
        m->grid[0][1] = -1*si;
        m->grid[1][0] = si;
        m->grid[1][1] = co;

        Vertex3d* result = m->multiply(this);

        return result;
}


/***********************************
  FUNCTION: rotate_y
  ARGS: starting vertex, angle to rotate by
  RETURN: a new vertex
  DOES: transformation function; 
        rotates a vertex about the y axis
************************************/
Vertex3d* Vertex3d::rotate_y(double theta)
{
        double co = cos(theta);
        double si = sin(theta);

        Matrix* m = new Matrix();

        m->grid[0][0] = co;
        m->grid[0][2] = si;
        m->grid[2][0] = -1*si;
        m->grid[2][2] = co;

        Vertex3d* result = m->multiply(this);

        return result;
}


Matrix::Matrix()
{
        for(int i=0; i<4; i++)
        {
                for(int j=0; j<4; j++)
                {
                        if(i == j)
                        {
                                grid[i][j] = 1;
                        }
                        else
                        {
                                grid[i][j] = 0;
                        }
                }
        }
}

Vertex3d* Matrix::multiply(Vertex3d* v)
{
        Vertex3d* result = new Vertex3d();

        for(int mr = 0; mr < 4; mr++)
        {
                for(int mc = 0; mc<4; mc++)
                {
                        result->coords[mr] += grid[mr][mc] * v->coords[mc];
                }
        }

        return result;
}


Matrix* Matrix::multiply(Matrix* left)
{
        // I called it left as a reminder that it is the one on the left of the multiplication

        Matrix* result = new Matrix();

        double sum;

        for(int a=0; a<4; a++)
        {
                for(int b=0; b<4; b++)
                {
                        sum = 0;

                        for(int c=0; c<4; c++)
                        {        
                                double mk = left->grid[a][c] * grid[c][b];
                                sum = sum + (mk);
                        }

                        result->grid[a][b] = sum;
                }
        }

        return result;
}


void Matrix::printmatrix()
{
        for(int a=0; a<4; a++)
        {
                printf("[ %f %f %f %f ]\n", grid[a][0], grid[a][1], grid[a][2], grid[a][3]);
        }
}


Face::Face()
{
}

Face::Face(Vertex3d* fir, Vertex3d* sec, Vertex3d* thr)
{
        corners[0] = fir;
        corners[1] = sec;
        corners[2] = thr;

        vertnum = 3;
}

Face::Face(Vertex3d* fir, Vertex3d* sec, Vertex3d* thr, Vertex3d* frt)
{
        corners[0] = fir;
        corners[1] = sec;
        corners[2] = thr;
        corners[3] = frt;

        vertnum = 4;
}

Shape3d::Shape3d(int t)
{
        type = t;
        ctm = new Matrix();
        rendermode = GL_POLYGON;
}


void Shape3d::make_cube(double size)
{
        verts[0][0] = new Vertex3d(-1 * size, size, -1 * size);
        verts[0][1] = new Vertex3d(-1 * size, size, size);
        verts[0][2] = new Vertex3d(size, size, size);
        verts[0][3] = new Vertex3d(size, size, -1 * size);
        verts[1][0] = new Vertex3d(-1 * size, -1 * size, -1 * size);
        verts[1][1] = new Vertex3d(-1 * size, -1 * size, size);
        verts[1][2] = new Vertex3d(size, -1 * size, size);
        verts[1][3] = new Vertex3d(size, -1 * size, -1 * size);

        bot = new Vertex3d(0, -1 * size, 0);
        top = new Vertex3d(0, size, 0);

        transform(2, 4);

        makeface(1,1);
}

void Shape3d::make_house(float size)
{
        verts[0][0] = new Vertex3d(-1 * size, size, -1 * size);
        verts[0][1] = new Vertex3d(-1 * size, size, size);
        verts[0][2] = new Vertex3d(size, size, size);
        verts[0][3] = new Vertex3d(size, size, -1 * size);
        verts[1][0] = new Vertex3d(-1 * size, -1 * size, -1 * size);
        verts[1][1] = new Vertex3d(-1 * size, -1 * size, size);
        verts[1][2] = new Vertex3d(size, -1 * size, size);
        verts[1][3] = new Vertex3d(size, -1 * size, -1 * size);

        verts[2][0] = new Vertex3d(0, size *1.5, 0); //top

        bot = new Vertex3d(0, -1 * size, 0);
        top = new Vertex3d(0, size *1.5, 0);

        transform(2, 4);

        makeface(1,1);
}

void Shape3d::make_cylinder(double height, double ray, int rs, int vs ) 
{
        Vertex3d* ringbase;
        double thisy, theta;
        double yspace = height/vs; // distance between each ring along y axis
        double rspace = TWOPI/rs;
        double starth = height/2; // y value of top of cylinder

        //h for height, r for ring

        for(int h=0; h<=vs; h++) // for each ring (at each height)
        {
                thisy = starth - (h * yspace);
                ringbase = new Vertex3d(ray, thisy, 0); // this seems faster than doing a full translate multiplication, but that would be another option
                verts[h][0] = ringbase;

                for(int r=1; r<rs; r++)
                {
                        theta = r * rspace;
                        verts[h][r] = ringbase->rotate_y(theta);
                }
        }

        verts[vs+1][0] = new Vertex3d(0, starth, 0);
        verts[vs+1][1] = new Vertex3d(0, starth * -1, 0);

        bot = new Vertex3d(0, starth * -1, 0);
        top = new Vertex3d(0, starth, 0);

        transform(vs, rs);
        cylface(vs, rs);
}

void Shape3d::make_sphere( double ray, int rs, int vs ) 
{
        Vertex3d* ringbase;
        double ztheta, ytheta;

        Vertex3d* base = new Vertex3d(0, ray, 0);
        double vspace = PI/vs;
        double rspace = TWOPI/rs;

        //h for height, r for ring

        for(int h=0; h<=vs; h++) // for each ring (at each height)
        {
                ztheta = 0 - (vspace * h);
                ringbase = base->rotate_z(ztheta);
                
                for(int r=0; r<rs; r++)
                {
                        ytheta = r * rspace;
                        verts[h][r] = ringbase->rotate_y(ytheta);
                }
        }

        bot = new Vertex3d(0, ray * -1, 0);
        top = new Vertex3d(0, ray, 0);
        
        transform(vs, rs);
        cylface((vs), rs);
}


/***********************************
  FUNCTION: make_cone 
  ARGS: height (along the Y axis), ray, and tesselation parameters (how many side
faces) 
  RETURN: none
  DOES: helper build function; 
        defines a cone centered at the origin, 
        of height 'height' along the Y axis,
        and ray 'ray' in the XOZ plane
************************************/

void Shape3d::make_cone(double height, double ray, int rs, int vs)
{
        Vertex3d* ringbase;
        double rise, currad, ytheta;

        Vertex3d* base = new Vertex3d(0, (height/-2), 0);
        double yspace = height/vs;
        double rspace = TWOPI/rs;

        //h for height, r for ring

        for(int h=vs; h>=0; h--) // for each ring (at each height) from top to bottom
        {
                rise = yspace * h;
                currad = rise * (ray/height);
                ringbase = base->translate(currad, rise, 0);
                
                for(int r=0; r<rs; r++)
                {
                        ytheta = r * rspace;
                        verts[h][r] = ringbase->rotate_y(ytheta);
                }
        }

        bot = new Vertex3d(0, 0, 0);
        top = new Vertex3d(0, 1, 0);
        front = new Vertex3d(0, 0, 1);
        aim = new Vertex3d(1, 0, 0);

        transform(vs, rs);
        cylface(vs, rs);
}


/***********************************
  FUNCTION: make_torus() 
  ARGS: distance to center of tube, radius of tube, sides of tube, rings of tube
  RETURN: none
  DOES: defines a torus centered at the origin
************************************/
void Shape3d::make_torus(double width, double ray, int rs, int vs)
{
        Vertex3d* hold;

        Vertex3d* base = new Vertex3d(ray, 0, 0);
        double ringturn = TWOPI/vs;
        double sliceturn = TWOPI/rs;

        for(int slice = 0; slice < rs; slice++)
        {
                hold = base->rotate_z((sliceturn*slice));
                hold = hold->translate( width, 0, 0);

                for(int ring=0; ring < vs; ring++)
                {
                        verts[ring][slice] = hold->rotate_y((ringturn*ring));
                }
        }

        bot = new Vertex3d(0, ray * -1, 0);
        top = new Vertex3d(0, ray, 0);

        transform(vs, rs);
        torface(vs, rs);
}


void Shape3d::makeface(int vs, int rs)
{
        switch(type)
        {
        case CUBE:
                {
                        cubeface();
                };
                break;
        case HOUSE:
                {
                        houseface();
                };
                break;
        case TORUS:
                {
                        torface(vs, rs);
                };
                break;
        default:
                {
                        cylface(vs, rs);
                };
                break;
        }
}


void Shape3d::cubeface()
{
        facecount = 12;

        facelist = new Face*[12];

        int ap;
        int scount = 0;
        Vertex3d p, q, r;

        for(int a=0; a<4; a++)
        {
                ap = a+1;

                if(ap == 4)
                {
                        ap = 0;
                }

                facelist[scount] = new Face(transverts[0][a], transverts[1][a], transverts[0][ap]);
        
                p = transverts[0][a]->getvector(transverts[1][a]);
                q = transverts[0][a]->getvector(transverts[0][ap]);

                r = p.crossmult(q);

                transverts[0][a]->addnorm(r);
                transverts[1][a]->addnorm(r);  
                transverts[0][ap]->addnorm(r);

                scount++;

                facelist[scount] = new Face(transverts[1][a], transverts[1][ap], transverts[0][ap]);
                
                p = transverts[1][ap]->getvector(transverts[0][ap]);
                q = transverts[1][ap]->getvector(transverts[1][a]);

                r = p.crossmult(q);

                transverts[1][a]->addnorm(r);
                transverts[1][ap]->addnorm(r);  
                transverts[0][ap]->addnorm(r);

                scount++;
        }

        facelist[8] = new Face(transverts[0][0], transverts[0][1], transverts[0][3]);
        //printf("made face 8\n");
        facelist[9] = new Face(transverts[0][1], transverts[0][2], transverts[0][3]);
        //printf("made face 9\n");
        facelist[10] = new Face(transverts[1][1], transverts[1][0], transverts[1][2]);
        //printf("made face 10\n");
        facelist[11] = new Face(transverts[1][0], transverts[1][3], transverts[1][2]);
        //printf("made face 11\n");

        r = *(new Vertex3d(0, 1, 0));

        transverts[0][0]->addnorm(r);
        transverts[0][1]->addnorm(r);
        transverts[0][2]->addnorm(r);
        transverts[0][3]->addnorm(r);

        r = *(new Vertex3d(0, -1, 0));

        transverts[1][0]->addnorm(r);
        transverts[1][1]->addnorm(r);
        transverts[1][2]->addnorm(r);
        transverts[1][3]->addnorm(r);

        for(int m=0; m<2; m++)
        {
                for (int n=0; n<4; n++)
                {
                        transverts[m][n]->vnorm[0] = transverts[m][n]->vnorm[0] / (transverts[m][n]->touchcount);
                        transverts[m][n]->vnorm[1] = transverts[m][n]->vnorm[1] / (transverts[m][n]->touchcount); // this is cylface
                        transverts[m][n]->vnorm[2] = transverts[m][n]->vnorm[2] / (transverts[m][n]->touchcount);

                        transverts[m][n]->normalizevnorm();
                }
        }
}


void Shape3d::houseface()
{
        facecount = 16;

        facelist = new Face*[16];

        int ap;
        int scount = 0;

        for(int a=0; a<4; a++)
        {
                ap = a+1;

                if(ap == 4)
                {
                        ap = 0;
                }

                facelist[scount] = new Face(transverts[0][a], transverts[1][a], transverts[0][ap]);
                        //printf("made face %d\n", scount);
                scount++;
                facelist[scount] = new Face(transverts[1][a], transverts[1][ap], transverts[0][ap]);
                        //printf("made face %d\n", scount);
                scount++;
        }

        facelist[8] = new Face(transverts[0][0], transverts[0][1], transverts[0][3]);
        facelist[9] = new Face(transverts[0][1], transverts[0][2], transverts[0][3]);
        facelist[10] = new Face(transverts[1][1], transverts[1][0], transverts[1][2]);
        facelist[11] = new Face(transverts[1][0], transverts[1][3], transverts[1][2]);

        facelist[12] = new Face(transverts[2][0], transverts[0][0], transverts[0][1]);
        facelist[13] = new Face(transverts[2][0], transverts[0][1], transverts[0][2]);
        facelist[14] = new Face(transverts[2][0], transverts[0][2], transverts[0][3]);
        facelist[15] = new Face(transverts[2][0], transverts[0][3], transverts[0][0]);
}


void Shape3d::cylface(int vs, int rs)
{
        int stacks = vs-1;
        facecount = (stacks * rs) * 2;

        if(type == CYLINDER)
        {
                facecount += rs * 2;
        }

        facelist = new Face*[facecount];

        int ap, bp, cp;
        int scount = 0;
        Vertex3d p, q, r;

        for(int a=0; a<stacks; a++)
        {
                for(int b=0; b<rs; b++)
                {
                        ap = a+1;
                        bp = b+1; // this is cylface

                        if(bp == rs)
                        {
                                bp = 0;
                        }

                        /*facelist[scount] = new Face(transverts[a][b], transverts[ap][b],
transverts[a][bp]);
                        scount++;

                        facelist[scount] = new Face(transverts[ap][b], transverts[ap][bp],
transverts[a][bp]);
                        scount++;*/

                        facelist[scount] = new Face(transverts[a][b], transverts[ap][b], transverts[a][bp]);

                        p = transverts[a][b]->getvector(transverts[ap][b]);
                        q = transverts[a][b]->getvector(transverts[a][bp]);

                        r = p.crossmult(q);

                        transverts[a][b]->addnorm(r);
                        transverts[a][bp]->addnorm(r);  // this is cylface
                        transverts[ap][b]->addnorm(r);

                        scount++;

                        facelist[scount] = new Face(transverts[ap][b], transverts[ap][bp], transverts[a][bp]);

                        p = transverts[ap][bp]->getvector(transverts[a][bp]);
                        q = transverts[ap][bp]->getvector(transverts[ap][b]);

                        r = p.crossmult(q);

                        transverts[ap][b]->addnorm(r);
                        transverts[ap][bp]->addnorm(r);  // this is cylface
                        transverts[a][bp]->addnorm(r);

                        scount++;
                }
        }

        for(int m=0; m<stacks; m++)
        {
                for (int n=0; n<rs; n++)
                {
                        transverts[m][n]->vnorm[0] = transverts[m][n]->vnorm[0] / (transverts[m][n]->touchcount);
                        transverts[m][n]->vnorm[1] = transverts[m][n]->vnorm[1] / (transverts[m][n]->touchcount); // this is cylface
                        transverts[m][n]->vnorm[2] = transverts[m][n]->vnorm[2] / (transverts[m][n]->touchcount);

                        transverts[m][n]->normalizevnorm();
                }
        }
        //int tcount = 0;

        if(type == CYLINDER) // top and bottom
        {
                for(int c=0; c<rs; c++)
                {
                        cp = c+1; // this is cylface

                        if(cp == rs)
                        {
                                cp = 0;
                        }

                        facelist[scount] = new Face(transverts[vs+1][0], transverts[0][c], transverts[0][cp]);
                        scount++;
                        //tcount++;

                        facelist[scount] = new Face(transverts[vs+1][1], transverts[stacks][c], transverts[stacks][cp]);
                        scount++;
                        //tcount++;
                }
        }

        //printf("tcount total %d, rs is %d\n", tcount, rs);
        //printf("facecount is %d, scount is %d\n", facecount, scount);
} // end cyl face



void Shape3d::torface(int rings, int rs)
{
        facecount = (rings * rs) * 2;

        facelist = new Face*[facecount];

        int ap, bp;
        int scount = 0;
        Vertex3d p, q, r;

        for(int a=0; a<rings; a++)
        {
                for(int b=0; b<rs; b++) // this is torface
                {
                        ap = a+1;
                        bp = b+1;

                        if(ap == rings)
                        {
                                ap = 0;
                        }

                        if(bp == rs)
                        {
                                bp = 0;
                        }

                        facelist[scount] = new Face(transverts[a][b], transverts[a][bp], transverts[ap][bp]); //this is torface
                        scount++;

                        facelist[scount] = new Face(transverts[ap][bp], transverts[ap][b], transverts[a][b]); //this is torface
                        scount++;
                }
        }
}


void Shape3d::tetface()
{
        facecount = 4;

        facelist = new Face*[4];

        facelist[0] = new Face(transverts[0][0], transverts[0][2], transverts[0][1]);
        facelist[1] = new Face(transverts[0][0], transverts[0][3], transverts[0][1]);
        facelist[2] = new Face(transverts[0][0], transverts[0][3], transverts[0][2]);
        facelist[3] = new Face(transverts[0][1], transverts[0][2], transverts[0][3]);
}


void Shape3d::octface()
{
        facecount = 8;

        facelist = new Face*[8];

        int count = 0;

        for(int a=1; a<5; a++)
        {
                int ap = a+1;

                if (ap == 5)
                {
                        ap = 1;
                }

                facelist[count] = new Face(transverts[0][0], transverts[0][a], transverts[0][ap]);
                count++;
        }


        for(int b=1; b<5; b++)
        {
                int bp = b+1;

                if (bp == 5)
                {
                        bp = 1;
                }

                facelist[count] = new Face(transverts[0][5], transverts[0][bp], transverts[0][b]);
                count++;
        }
}


void Shape3d::transform(int vs, int rs)
{
        switch(type)
        {
        case HOUSE:
        case CUBE:
                {
                        do_transform(2,4);
                };
                break;
        default:
                {
                        do_transform(vs, rs);
                };
                break;
        }
}

void Shape3d::do_transform(int vs, int rs)
{
        shapevs = vs;
        shapers = rs;

        for (int a=0; a<vs; a++)
        {
                for(int b=0; b<rs; b++)
                {
                        transverts[a][b] = ctm->multiply(verts[a][b]);
                }
        }

        if(type == HOUSE)
        {
                transverts[2][0] = ctm->multiply(verts[2][0]);
        }
        if(type == CYLINDER)
        {
                transverts[vs+1][0] = ctm->multiply(verts[vs+1][0]);
                transverts[vs+1][1] = ctm->multiply(verts[vs+1][1]);
        }

        tbot = ctm->multiply(bot);
        ttop = ctm->multiply(top);

        if(type == CONE)
        {
                tfront = ctm->multiply(front);
                taim = ctm->multiply(aim);
        }
}

void Shape3d::printcoords()
{
        printf("Base verts:\n");
        for(int a=0; a<shapevs; a++)
        {
                for(int b=0; b<shapers; b++)
                {
                        verts[a][b]->printvertex();
                }
        }

        printf("Trans verts:\n");
        for(int c=0; c<shapevs; c++)
        {
                for(int d=0; d<shapers; d++)
                {
                        transverts[c][d]->printvertex();
                }
        }
}


Tile::Tile()
{
}


Tile::Tile(int r, int c, int s)
{
        row = r;
        col = c;
        setstate(s);
}


void Tile::setstate(int s) //incomplete
{
        state = s;
}

void Tile::getnext(int* a, int*b)
{
        int r = row;
        int c = col;

        switch(state)
        {
        case(1): //to the right
        case(10):
        case(11):
                a = &r;
                c++;
                b = &c;
                break;
        case(3): //down
        case(5):
        case(12):
                r++;
                a = &r;
                b = &c;
                break;
        case(2): //to the left
        case(6):
        case(7):
                c--;
                a = &r;
                b = &c;
                break;
        case(4): //up
        case(8):
        case(9):
                r--;
                a = &r;
                b = &c;
                break;
        default: break;
        }
}


Train::Train(Tile* p)
{
        move = true;
        maxmoves = 8;  // this is the default moves to get across the tile, maybe change later
        body = new Shape3d(CUBE);
        body->make_cube(1);
        position = p;  //starting tile

        maketiles();
}


void Train::maketiles()//this makes tiles for the map
{
}


void Train::followtrack()
{
        int* row;
		int* col;

        while(move)
        {
                movecount = 0;

                while(movecount < maxmoves)
                {
                        body->ctm = body->ctm->multiply(position->movement);
                        movecount++;
                }

                position->getnext(row, col);
                position = map[*row][*col];
        }
}