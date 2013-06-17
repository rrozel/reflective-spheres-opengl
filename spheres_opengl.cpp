// Ryan Rozelle - Programming Assignment 5, CS645; due Nov. 28, 2012
//  Associated "my_setup.h" file is required (2 lines commented out in reshape callback)
//
//  New Extras: Manual light movement on curtains (uhjk)
//  OLD EXTRAS: camera movement (wasd), raytrace light movement (uhjk), and shadows (f).
//
// Program Architecture:
//  Key Program activities:
//   raysphere() / rayplane() - Finds intersection, if exist, between a ray and sphere or plane.
//   traceRay() - Determines illumination (color) at a point on the virtual screen by
//      tracing the ray through it from the camera. Also used, when given a reflected ray,
//      to add in additional illumination from reflection at an intersection.
//   display_func() - Registered as OpenGL display function callback.
//      Performs transformations, sets perspective, draws curtains and raytrace scene.
//   keypress() - Registered as OpenGL keyboard key press function callback.
//      Captures key press events, allows on-the-fly scene movement.
//  Architecture:
//   An array of 'Sphere' structs stores the material info, reflectiveness, position,
//		etc. of spheres (and floor) in the scene.
//   Several global variables are used to store the various states, such as whether or not 
//     the light position is changed or camera or light location.
//
// Key presses:
// G: Draws scene initially.
// L: Toggles light position between original and x+=20.
//      Extras:
// r: Resets the following to initial state:
//   u,h,j,k: Move light position up, left, down, or right.
//   w,a,s,d: Move camera position up, left, down, or right.
//   1,2: Decreases or increases specular exponent.
//   f: Toggles shadow feeling on/off.
// X: Quit the program.
  
#include <GL/glut.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "my_setup.h"

#define canvas_Width 550
#define canvas_Height 300
#define canvas_Name "Ryan Rozelle Prog5"
#define MW 1.7  // multiply width

#define MAX_DEPTH 2
#define RES 300
#define MESH_RES 20


struct Point
{
	float x;
	float y;
	float z;
};

struct Color
{
	float r;
	float g;
	float b;
};

struct Sphere
{
	Point c;	// center
	float rgb[3];	// color
	float radius;
	float reflect; // [0.0,1.0]
};
	
Sphere s[10];  // #10 represents the mirror floor
Point light_pos = {0,0,10}; // light location
Point cam = {0,0,0}; // camera location centroid
float vsz = 0;  // virtual screen z location
bool LIGHT_TOGGLE = false; // 'L' option
float spec_exp = 7.0; // specular exponent
bool DISPLAY_ON = false; // has 'G' been pressed yet
bool SHOW_SHADOWS = false;

void printKey()
{
	printf ("Key presses:\n");
	printf ("G: Draws scene initially.\n");
	printf ("L: Toggles light position between original and x+=20.\n");
	printf ("      Old Extras:\n");
	printf ("r: Resets the following to initial state:\n");
	printf ("  u,h,j,k: Move light position up, left, down, or right.\n");
	printf ("  w,a,s,d: Move camera position up, left, down, or right.\n");
	printf ("  1,2: Decreases or increases specular exponent.\n");
	printf ("  f: Toggle shadow feeling on/off.\n");
	printf ("\n");
	printf ("X: Quit the program.\n");
	printf ("\n");
}

float raysphere(Point c, Point d, Point st, float r)  // center, unit direction, start, radius
{	
	Point v = {st.x-c.x, st.y-c.y, st.z-c.z};
	float v_d = v.x*d.x + v.y*d.y + v.z*d.z;
	float disc = v_d*v_d - ((v.x*v.x + v.y*v.y + v.z*v.z) - (r*r));
	if (disc < 0) return 0;
	float t1 = -1*v_d + sqrt(disc);
	float t2 = -1*v_d - sqrt(disc);
	if ((t1>0&&t2<0) || (t1>0&&t2>0&&t1<t2)) return t1;
	else if ((t2>0&&t1<0) || (t2>0&&t1>0&&t2<t1)) return t2;
	else return 0;
}

float rayplane(Point d, Point st, float planeY)
{
	float denom = d.y-st.y;
	if (denom==0) return 0;
	float t = (planeY - st.y) / denom;
	if (t>0) return t;
	else return 0;
}

void output(float x, float y, float r, float g, float b, void *font, char *string)
{
	glPushMatrix();
	glColor3f(r,g,b);
	glRasterPos2f(x,y);
	int len, i;
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);	// Output text in OpenGL, one character at a time
	}
	glPopMatrix();
}

void initializeSpheres()
{
	s[0].c = {0,0,-20};
	s[0].rgb[0] = 0.753;
	s[0].rgb[1] = 0.753;
	s[0].rgb[2] = 0.753; // polished silver
	s[0].radius = 6;
	s[0].reflect = 0.7;
	
	s[1].c = {15,15,-20};
	s[1].rgb[0] = 1;
	s[1].rgb[1] = 1;
	s[1].rgb[2] = 1;	// white
	s[1].radius = 7;
	s[1].reflect = 0.5;
	
	s[2].c = {78,52,-70};
	s[2].rgb[0] = 0;
	s[2].rgb[1] = 1;
	s[2].rgb[2] = 0; // green
	s[2].radius = 10;
	s[2].reflect = 0.5;
	
	s[3].c = {48,51,-68};
	s[3].rgb[0] = 1;
	s[3].rgb[1] = 0;
	s[3].rgb[2] = 0; // red
	s[3].radius = 10;
	s[3].reflect = 0.5;
	
	s[4].c = {50,50,-40};
	s[4].rgb[0] = 1;
	s[4].rgb[1] = 1;
	s[4].rgb[2] = 0; // yellow
	s[4].radius = 4;
	s[4].reflect = 0.5;
	
	s[5].c = {-9,11,-11};
	s[5].rgb[0] = 1;
	s[5].rgb[1] = 0.647;
	s[5].rgb[2] = 0;	// orange
	s[5].radius = 10;
	s[5].reflect = 0.5;
	
	s[6].c = {3,11,-11};
	s[6].rgb[0] = 1;
	s[6].rgb[1] = 0;
	s[6].rgb[2] = 0; // red
	s[6].radius = 2;
	s[6].reflect = 0.5;
	
	s[7].c = {-50,0,-100};
	s[7].rgb[0] = 1;
	s[7].rgb[1] = 1;
	s[7].rgb[2] = 0; // yellow
	s[7].radius = 25;
	s[7].reflect = 0.5;
	
	s[8].c = {45,5,20};
	s[8].rgb[0] = 0;
	s[8].rgb[1] = 0;
	s[8].rgb[2] = 1; // blue
	s[8].radius = 18;
	s[8].reflect = 0.5;
	
	s[9].c = {0,0,0};
	s[9].rgb[0] = 0;
	s[9].rgb[1] = 0;
	s[9].rgb[2] = 0; // add no color
	s[9].reflect = 1; // full reflect (mirror)
}

void initial()
{
	initializeSpheres();
	
	glClearColor (0.9, 0.9, 0.9, 0.0);
	glShadeModel (GL_SMOOTH); // may be pointless in this program
}

void keypress(unsigned char key, int xmouse, int ymouse)
{
	switch (key)
	{
		case 'G': 
			if(!DISPLAY_ON) DISPLAY_ON=true;
		break;
		
		case 'L':
			if (!LIGHT_TOGGLE) light_pos.x = 20;
			else light_pos.x = 0;
			LIGHT_TOGGLE=!LIGHT_TOGGLE;
		break;
		
		case 'r':	// reset vars to initial
			LIGHT_TOGGLE = false;
			light_pos.x = 0;
			light_pos.y = 0;
			cam.x = 0;
			cam.y = 0;
			spec_exp = 5.0;
			vsz = 0;
			SHOW_SHADOWS = false;
		break;
		
		case 'f':
			SHOW_SHADOWS=!SHOW_SHADOWS;
		break;
		
		case 'k':
			light_pos.x++;
		break;
		
		case 'h':
			light_pos.x--;
		break;
		
		case 'j':
			light_pos.y--;
		break;
		
		case 'u':
			light_pos.y++;
		break;
		
		case 'w':
			cam.y++;
		break;
		
		case 's':
			cam.y--;
		break;
		
		case 'a':
			cam.x--;
		break;
		
		case 'd':
			cam.x++;
		break;
		
		case '2':
			spec_exp+=2;
		break;
		
		case '1':
			spec_exp-=2;
		break;

		case 'x': 
			exit(0);
		break;
		
		case 'X': 
			exit(0);
		break;

		default:
		 break;
	} // end switch
	
	glutPostRedisplay(); // Request display
}

Color traceRay(Point ray, Point st, int depth)
{
	Color illum = {0,0,0};
	if (depth > MAX_DEPTH) return illum;
	
	// find closest ray object/intersection;
	// for each object in scene, get distance or 0 if no intersect
	int closestIndex = -1;
	float closestDist = 9999;
	float res = 0;
	for (int i=0; i<9; i++) // for each sphere
	{
		res = raysphere(s[i].c,ray,st,s[i].radius); // get t from raysphere()
		if(res!=0 && res < closestDist)
		{
			closestDist = res;
			closestIndex = i;
		}
	}
	
	if (closestIndex == -1) // no intersection with sphere, check versus floor
	{
		res = rayplane(ray,st,-50.0); // get t from rayplane()
		if (res!=0)
		{
			closestDist = res;
			closestIndex = 9;  // mirror floor
		}
	}
	
	if (closestIndex != -1) // if intersection exists
	{ 
		Point inter = {(st.x+(closestDist*ray.x)),(st.y+(closestDist*ray.y)),(st.z+(closestDist*ray.z))};
		
		// if toggled, compute shadows
		Point shad_ray = {light_pos.x-inter.x, light_pos.y-inter.y, light_pos.z-inter.z};
		float mag = sqrt(shad_ray.x*shad_ray.x + shad_ray.y*shad_ray.y + shad_ray.z*shad_ray.z);
		shad_ray.x /= mag;
		shad_ray.y /= mag;
		shad_ray.z /= mag;
		int shadIndex = -1; // default no shadow
		float shadDist = 9999;
		if (SHOW_SHADOWS)
		{
			for (int j=0; j<9; j++) // for each sphere,
			{
				res = raysphere(s[j].c,shad_ray,inter,s[j].radius);
				if(res!=0 && res < shadDist) // check if point is occluded by shadow
				{
					shadDist = res;
					shadIndex = j;
				}
			}
		}
		
		Point norm;
		if (closestIndex==9) // calculate normal for floor
		{
			float nmag = sqrt(inter.x*inter.x + (-49)*(-49) + inter.z*inter.z);
			norm.x = inter.x/nmag;
			norm.y = -49/nmag;
			norm.z = inter.z/nmag;
		}
		else // calculate normal for sphere
		{
			float nmag = sqrt( (inter.x-s[closestIndex].c.x)*(inter.x-s[closestIndex].c.x) + (inter.y-s[closestIndex].c.y)*(inter.y-s[closestIndex].c.y) + (inter.z-s[closestIndex].c.z)*(inter.z-s[closestIndex].c.z) );
			norm.x = ((inter.x-s[closestIndex].c.x)/nmag);
			norm.y = ((inter.y-s[closestIndex].c.y)/nmag);
			norm.z = ((inter.z-s[closestIndex].c.z)/nmag);
		}

		if (shadIndex==-1) // if no obstruction to light
		{
			float L_N = shad_ray.x*norm.x + shad_ray.y*norm.y + shad_ray.z*norm.z;
			Point H = {(shad_ray.x+ray.x)/2,(shad_ray.y+ray.y)/2,(shad_ray.z+ray.z)/2};
			float H_N = H.x*norm.x + H.y*norm.y + H.z*norm.z; // halfway vector *dot* normal
			
			illum.r += s[closestIndex].rgb[0] * L_N + pow(H_N,spec_exp); // Phong illum.:
			illum.g += s[closestIndex].rgb[1] * L_N + pow(H_N,spec_exp); // diffuse + specular
			illum.b += s[closestIndex].rgb[2] * L_N + pow(H_N,spec_exp);
		}

		// if surface is reflective
		if (s[closestIndex].reflect > 0)
		{
			float n_d = 2.0*(norm.x*ray.x + norm.y*ray.y + norm.z*ray.z);
			Point refl_ray = {ray.x-(n_d*norm.x),ray.y-(n_d*norm.y),ray.z-(n_d*norm.z)};
			float mag = sqrt(refl_ray.x*refl_ray.x + refl_ray.y*refl_ray.y + refl_ray.z*refl_ray.z);
			refl_ray.x /= mag;
			refl_ray.y /= mag;
			refl_ray.z /= mag;
			Color refl = traceRay(refl_ray, inter, depth+1); // recursive here
			illum.r += refl.r * s[closestIndex].reflect;
			illum.g += refl.g * s[closestIndex].reflect;
			illum.b += refl.b * s[closestIndex].reflect;
		}
	}
	return illum;
}

GLfloat mesh1[4][4][3] = {
   {{-91.66, -50.00, -2}, {-86.458, -50.00, -1}, {-81.25, -50.00, 0}, {-76.04, -50.00, -1}}, 
   {{-91.66, -16.66, -2}, {-86.458, -16.66, -1}, {-81.25, -16.66, 0}, {-76.04, -16.66, -1}}, 
   {{-91.66, 16.66, -2}, {-86.458, 16.66, -1}, {-81.25, 16.66, 0}, {-76.04, 16.66, -1}}, 
   {{-91.66, 50.00, -2}, {-86.458, 50.00, -1}, {-81.25, 50.00, 0}, {-76.04, 50.00, -1}}
};
GLfloat mesh2[4][4][3] = {
   {{-76.04, -50.00, -1}, {-70.83, -50.00, -2}, {-65.62, -50.00, -1}, {-60.41, -50.00, 0}}, 
   {{-76.04, -16.66, -1}, {-70.83, -16.66, -2}, {-65.62, -16.66, -1}, {-60.41, -16.66, 0}}, 
   {{-76.04, 16.66, -1}, {-70.83, 16.66, -2}, {-65.62, 16.66, -1}, {-60.41, 16.66, 0}}, 
   {{-76.04, 50.00, -1}, {-70.83, 50.00, -2}, {-65.62, 50.00, -1}, {-60.41, 50.00, 0}}
};
GLfloat mesh3[4][4][3] = {
   {{-60.41, -50.00, 0}, {-56.94, -50.00, -0.5}, {-53.47, -50.00, -1.5}, {-50, -50.00, -2}}, 
   {{-60.41, -16.66, 0}, {-56.94, -16.66, -0.5}, {-53.47, -16.66, -1.5}, {-50, -16.66, -2}}, 
   {{-60.41, 16.66, 0}, {-56.94, 16.66, -0.5}, {-53.47, 16.66, -1.5}, {-50, 16.66, -2}}, 
   {{-60.41, 50.00, 0}, {-56.94, 50.00, -0.5}, {-53.47, 50.00, -1.5}, {-50, 50.00, -2}}
};
GLfloat mesh4[4][4][3] = {
   {{91.66, -50.00, -2}, {86.458, -50.00, -1}, {81.25, -50.00, 0}, {76.04, -50.00, -1}}, 
   {{91.66, -16.66, -2}, {86.458, -16.66, -1}, {81.25, -16.66, 0}, {76.04, -16.66, -1}}, 
   {{91.66, 16.66, -2}, {86.458, 16.66, -1}, {81.25, 16.66, 0}, {76.04, 16.66, -1}}, 
   {{91.66, 50.00, -2}, {86.458, 50.00, -1}, {81.25, 50.00, 0}, {76.04, 50.00, -1}}
};
GLfloat mesh5[4][4][3] = {
   {{76.04, -50.00, -1}, {70.83, -50.00, -2}, {65.62, -50.00, -1}, {60.41, -50.00, 0}}, 
   {{76.04, -16.66, -1}, {70.83, -16.66, -2}, {65.62, -16.66, -1}, {60.41, -16.66, 0}}, 
   {{76.04, 16.66, -1}, {70.83, 16.66, -2}, {65.62, 16.66, -1}, {60.41, 16.66, 0}}, 
   {{76.04, 50.00, -1}, {70.83, 50.00, -2}, {65.62, 50.00, -1}, {60.41, 50.00, 0}}
};
GLfloat mesh6[4][4][3] = {
   {{60.41, -50.00, 0}, {56.94, -50.00, -0.5}, {53.47, -50.00, -1.5}, {50, -50.00, -2}}, 
   {{60.41, -16.66, 0}, {56.94, -16.66, -0.5}, {53.47, -16.66, -1.5}, {50, -16.66, -2}}, 
   {{60.41, 16.66, 0}, {56.94, 16.66, -0.5}, {53.47, 16.66, -1.5}, {50, 16.66, -2}}, 
   {{60.41, 50.00, 0}, {56.94, 50.00, -0.5}, {53.47, 50.00, -1.5}, {50, 50.00, -2}}
};

void initSpline()
{
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MAP2_VERTEX_3);
   glMapGrid2f(MESH_RES, 0.0, 1.0, MESH_RES, 0.0, 1.0);
}

void display_func(void)
{ 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!DISPLAY_ON)
	{
		char line1[40]="Press G to begin simulation.";
		char line2[40]="Press X to exit when you are";
		char line3[40]="finished viewing the image.";
		glLoadIdentity();
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		output(-0.5, 0.3,  0,0,0,  GLUT_BITMAP_HELVETICA_18,line1);
		output(-0.5, 0.0,  0,0,0,  GLUT_BITMAP_HELVETICA_18,line2);
		output(-0.5,-0.2,  0,0,0,  GLUT_BITMAP_HELVETICA_18,line3);
		glMatrixMode (GL_MODELVIEW);
	}
	else
	{
		glClearColor (0.2, 0.4, 0.2, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();  // clear the matrix
				
		glMatrixMode (GL_PROJECTION); // Change mode view in projection mode, then switch back
		glLoadIdentity();
		gluOrtho2D(cam.x-(50*MW),cam.x+(50*MW),cam.y-50,cam.y+50);
		glMatrixMode (GL_MODELVIEW);
		
		// Curtains:
		glPushMatrix();
		GLfloat position[] = {light_pos.x, light_pos.y, light_pos.z, 1.0};
		GLfloat ambient[] = {0.1, 1.0, 0.1};
		GLfloat mat_diffuse[] = {0.6, 0.8, 0.6};
		GLfloat mat_specular[] = {1.0, 1.0, 1.0};
		GLfloat mat_shininess[] = {50.0};

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_POSITION, position);

		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		
		// Draw spline patches (curtains):
		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &mesh1[0][0][0]);
		glEvalMesh2(GL_FILL, 0, MESH_RES, 0, MESH_RES);
		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &mesh2[0][0][0]);
		glEvalMesh2(GL_FILL, 0, MESH_RES, 0, MESH_RES);
		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &mesh3[0][0][0]);
		glEvalMesh2(GL_FILL, 0, MESH_RES, 0, MESH_RES);
		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &mesh4[0][0][0]);
		glEvalMesh2(GL_FILL, 0, MESH_RES, 0, MESH_RES);
		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &mesh5[0][0][0]);
		glEvalMesh2(GL_FILL, 0, MESH_RES, 0, MESH_RES);
		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &mesh6[0][0][0]);
		glEvalMesh2(GL_FILL, 0, MESH_RES, 0, MESH_RES);
		
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glPopMatrix();

		// Ray tracing:
		Point start = cam; // ray starting point
		Point ray={0,0,-1}; // direction ray
		Color illum;
		float mag = 0.0;
		glPointSize(2.0);
		
		start.z = vsz;
		float pp = 100.0 / RES;
			
		for (float x=-50; x<50; x+=pp)
		{
			for (float y=-50; y<50; y+=pp)
			{
				// traceRay to get illumination at this point
				start.x = x + cam.x;
				start.y = y + cam.y;
				illum = traceRay(ray,start,0);

				if (illum.r>1) illum.r = 1;
				if (illum.g>1) illum.g = 1;
				if (illum.b>1) illum.b = 1;  // clamp to 1
		
				// pixel color = illumination mapped to virtual screen
				glBegin(GL_POINTS);
				glColor3f(illum.r,illum.g,illum.b);
				glVertex3f(start.x,start.y,start.z);
				glEnd();
			}
		}
	}
	
	glFlush();  // Flush buffer
}

int main(int argc, char** argv)
{
	printKey();	// Command "key" printed to standard out

	glutInit(&argc, argv);

	my_setup(canvas_Width, canvas_Height, canvas_Name);

	initial();  // Set initial values of global variables
	initSpline();

	glutDisplayFunc(display_func); // Register display callback
	glutKeyboardFunc(keypress); // Register key press callback

	glutMainLoop(); // Execute until killed
	return 0;
}