
/***********************************************************


   This file has initialization function calls and set-ups
for CS 445/545 Open GL programs that use the GLUT.  (The initializations
will work for Mesa as well as Open GL.)  The initializations involve
defining a callback handler (my_reshape_function) that sets viewing 
parameters for 2D display. The x and y coordinates of the drawing canvas
upper corner are determined by the STRT_X_POS and STRT_Y_POS variables, resp.

   TSN 01/2010

 ************************************************************/


/* reshape callback handler - defines viewing parameters */

void my_reshape_handler(int width, int height)
    {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    //  setting viewing parameters to simple 2D viewing
    //glLoadIdentity();
    //gluOrtho2D (0.0, (GLdouble) width, 0.0, (GLdouble) height);
    glMatrixMode(GL_MODELVIEW);
    }

#define STRT_X_POS 25
#define STRT_Y_POS 25

void my_setup(int width, int height, char *window_name_str)
    {
    // initialization
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);  
    glutInitWindowSize(width, height);
    glutInitWindowPosition(STRT_X_POS,STRT_Y_POS);

    glutCreateWindow(window_name_str);

    // one callback handler - viewing parameters set upon resize event
    glutReshapeFunc(my_reshape_handler);
    }

