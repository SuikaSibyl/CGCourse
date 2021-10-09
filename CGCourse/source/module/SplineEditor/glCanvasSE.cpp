// Included files for OpenGL Rendering
#include <Windows.h>
#include <GL/gl.h>
#include "../OpenGL/ThirdParty/glut.h"

#include "glCanvasSE.h"
#include "ArgParser.h"
#include "Spline.h"
#include "SplineParser.h"

// ========================================================

// STATIC VARIABLES
ArgParser* GLCanvasCurveEditor::args = NULL;
SplineParser* GLCanvasCurveEditor::splines = NULL;
int GLCanvasCurveEditor::width = 300;
int GLCanvasCurveEditor::height = 300;
float GLCanvasCurveEditor::size = 10;
Spline* GLCanvasCurveEditor::selected_spline;
int GLCanvasCurveEditor::selected_control_point;

// ========================================================
// Initialize all appropriate OpenGL variables, set
// callback functions, and start the main event loop.
// This function will not return but can be terminated
// by calling 'exit(0)'
// ========================================================

void GLCanvasCurveEditor::initialize(ArgParser* _args, SplineParser* _splines) {
    args = _args;
    splines = _splines;

    // Set window parameters
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Curve Editor");

    // Initialize callback functions
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    // Enter the main rendering loop
    glutMainLoop();
}

// ========================================================
// Callback for the OpenGL display loop.  To request that
// this function get called, use glutPostRedisplay()
// ========================================================

void GLCanvasCurveEditor::display(void) {

    // Clear the display buffer
    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the camera parameters
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 10,
        0, 0, 0,
        0, 1, 0);

    // Draw the splines
    for (int i = 0; i < splines->getNumSplines(); i++) {
        splines->getSpline(i)->Paint(args);
    }

    glutSwapBuffers();
}

// ========================================================
// Callback function for window resize
// ========================================================

void GLCanvasCurveEditor::reshape(int w, int h) {

    // Set the OpenGL viewport to fill the entire window
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // Set the camera parameters to reflect the changes
    width = w;
    height = h;

    // orthographic camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float horiz = size / 2.0;
    float vert = horiz * height / float(width);
    glOrtho(-horiz, horiz, -vert, vert, 0.1, 1000.0);
}

// ========================================================
// Callback function for mouse click or release
// ========================================================

// must click within 10 pixels of point
#define PIXEL_EPSILON 10

void GLCanvasCurveEditor::mouseToScreen(int i, int j, float& x, float& y, float& epsilon) {
    // maps the screen coordinate of the mouse to world space for picking
    x = ((i / float(width)) - 0.5) * size;
    y = -((j / float(height)) - 0.5) * size * height / float(width);
    epsilon = PIXEL_EPSILON * size / float(width);
}

void GLCanvasCurveEditor::mouse(int button, int state, int i, int j) {
    // mouse release
    if (state == 1) { selected_spline = NULL; return; }

    float x, y, epsilon;
    mouseToScreen(i, j, x, y, epsilon);

    // move control point
    if (button == GLUT_LEFT_BUTTON) {
        Spline* s;
        int pt;
        // find the closest control point
        splines->Pick(x, y, epsilon, s, pt);
        if (s == NULL) return;
        s->moveControlPoint(pt, x, y);
        selected_spline = s;
        selected_control_point = pt;
    }

    // add control point
    if (button == GLUT_MIDDLE_BUTTON) {
        Spline* s;
        int pt;
        // find the closest edge (& control point)
        splines->PickEdge(x, y, epsilon, s, pt);
        if (s == NULL) return;
        s->addControlPoint(pt, x, y);
        selected_spline = s;
        selected_control_point = pt;
    }

    // delete control point
    if (button == GLUT_RIGHT_BUTTON) {
        Spline* s;
        int pt;
        // find the closest control point
        splines->Pick(x, y, epsilon, s, pt);
        if (s == NULL) return;
        s->deleteControlPoint(pt);
    }

    // Redraw the scene 
    glutPostRedisplay();
}

// ========================================================
// Callback function for mouse drag
// ========================================================

void GLCanvasCurveEditor::motion(int i, int j) {
    // move control point
    if (selected_spline == NULL) return;
    float x, y, epsilon;
    mouseToScreen(i, j, x, y, epsilon);
    selected_spline->moveControlPoint(selected_control_point, x, y);
    // Redraw the scene
    glutPostRedisplay();
}

// ========================================================
// Callback function for keyboard events
// ========================================================

void GLCanvasCurveEditor::keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 's':  case 'S':
        printf("Saving... "); fflush(stdout);
        splines->SaveBezier(args);
        splines->SaveBSpline(args);
        splines->SaveTriangles(args);
        printf("done\n");
        break;
    case 'q':  case 'Q':
        exit(0);
        break;
    default:
        printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
    }
}

// ========================================================
// ========================================================
