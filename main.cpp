#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

void drawBackground()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);

        glColor3f(0.25f, 0.25f, 0.25f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);

        glColor3f(0.75f, 0.75f, 0.75f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);

    glEnd();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawCube()
{
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -4.0f);
    glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(35.0f, 0.0f, 1.0f, 0.0f);

    GLfloat color[] = {
        1.0f, 1.0f, 1.0f, 1.0f
    };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    glutSolidCube(1.5);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground();
    drawCube();

    glutSwapBuffers();
}

void resize(int width, int height)
{
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        60.0,
        (double)width / (double)height,
        0.1,
        100.0
    );

    glMatrixMode(GL_MODELVIEW);
}

void initRenderer()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat ambient[] = {
        0.3f, 0.3f, 0.3f, 1.0f
    };

    GLfloat diffuse[] = {
        0.8f, 0.8f, 0.8f, 1.0f
    };

    GLfloat position[] = {
        -2.0f, 3.0f, 4.0f, 1.0f
    };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glShadeModel(GL_FLAT);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGB |
        GLUT_DEPTH
    );

    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("PrimitiveGL");

    initRenderer();

    glutDisplayFunc(render);
    glutReshapeFunc(resize);

    glutMainLoop();

    return 0;
}
