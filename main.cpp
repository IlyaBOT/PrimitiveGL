#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <ApplicationServices/ApplicationServices.h>
#include <math.h>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

const float MOVE_SPEED = 2.0f;
const float TURN_SPEED = 90.0f;
const double PI = 3.14159265358979323846;

float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 4.0f;
float cameraYaw = 0.0f;

int lastTime = 0;

// Mac virtual key codes.
const CGKeyCode KEY_A = 0x00;
const CGKeyCode KEY_S = 0x01;
const CGKeyCode KEY_D = 0x02;
const CGKeyCode KEY_Q = 0x0C;
const CGKeyCode KEY_W = 0x0D;
const CGKeyCode KEY_R = 0x0F;
const CGKeyCode KEY_SPACE = 0x31;
const CGKeyCode KEY_SHIFT = 0x38;
const CGKeyCode KEY_RIGHT_SHIFT = 0x3C;

bool keyDown(CGKeyCode key)
{
    return CGEventSourceKeyState(
        kCGEventSourceStateCombinedSessionState,
        key
    );
}

void drawBackground()
{
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);

        // Bottom: dark gray
        glColor3f(0.20f, 0.20f, 0.20f);
        glVertex2f(0.0f, 0.0f);

        glColor3f(0.20f, 0.20f, 0.20f);
        glVertex2f(1.0f, 0.0f);

        // Top: light gray
        glColor3f(0.80f, 0.80f, 0.80f);
        glVertex2f(1.0f, 1.0f);

        glColor3f(0.80f, 0.80f, 0.80f);
        glVertex2f(0.0f, 1.0f);

    glEnd();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}

void cubeVertex(float x, float y, float z)
{
    const float size = 0.75f;

    // Map XYZ position to RGB color.
    glColor3f(
        (x + size) / (size * 2.0f),
        (y + size) / (size * 2.0f),
        (z + size) / (size * 2.0f)
    );

    glVertex3f(x, y, z);
}

void drawCube()
{
    const float s = 0.75f;

    glPushMatrix();

    // Keep the cube slightly rotated so its 3D shape is visible immediately.
    glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(35.0f, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);

        // Front
        cubeVertex(-s, -s,  s);
        cubeVertex( s, -s,  s);
        cubeVertex( s,  s,  s);
        cubeVertex(-s,  s,  s);

        // Back
        cubeVertex( s, -s, -s);
        cubeVertex(-s, -s, -s);
        cubeVertex(-s,  s, -s);
        cubeVertex( s,  s, -s);

        // Left
        cubeVertex(-s, -s, -s);
        cubeVertex(-s, -s,  s);
        cubeVertex(-s,  s,  s);
        cubeVertex(-s,  s, -s);

        // Right
        cubeVertex( s, -s,  s);
        cubeVertex( s, -s, -s);
        cubeVertex( s,  s, -s);
        cubeVertex( s,  s,  s);

        // Top
        cubeVertex(-s,  s,  s);
        cubeVertex( s,  s,  s);
        cubeVertex( s,  s, -s);
        cubeVertex(-s,  s, -s);

        // Bottom
        cubeVertex(-s, -s, -s);
        cubeVertex( s, -s, -s);
        cubeVertex( s, -s,  s);
        cubeVertex(-s, -s,  s);

    glEnd();

    glPopMatrix();
}

void applyCamera()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRotatef(-cameraYaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-cameraX, -cameraY, -cameraZ);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground();

    applyCamera();
    drawCube();

    glutSwapBuffers();
}

void update()
{
    int now = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (now - lastTime) / 1000.0f;
    lastTime = now;

    // Avoid a huge camera jump after a pause or debugger stop.
    if (deltaTime > 0.05f)
        deltaTime = 0.05f;

    if (keyDown(KEY_Q))
        cameraYaw -= TURN_SPEED * deltaTime;

    if (keyDown(KEY_R))
        cameraYaw += TURN_SPEED * deltaTime;

    double angle = cameraYaw * PI / 180.0;

    float forwardX = (float)sin(angle);
    float forwardZ = (float)-cos(angle);

    float rightX = (float)cos(angle);
    float rightZ = (float)sin(angle);

    float move = MOVE_SPEED * deltaTime;

    if (keyDown(KEY_W)) {
        cameraX += forwardX * move;
        cameraZ += forwardZ * move;
    }

    if (keyDown(KEY_S)) {
        cameraX -= forwardX * move;
        cameraZ -= forwardZ * move;
    }

    if (keyDown(KEY_A)) {
        cameraX -= rightX * move;
        cameraZ -= rightZ * move;
    }

    if (keyDown(KEY_D)) {
        cameraX += rightX * move;
        cameraZ += rightZ * move;
    }

    if (keyDown(KEY_SPACE))
        cameraY += move;

    if (keyDown(KEY_SHIFT) || keyDown(KEY_RIGHT_SHIFT))
        cameraY -= move;

    glutPostRedisplay();
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

    // Smooth interpolation is used by both the background gradient
    // and the RGB cube.
    glShadeModel(GL_SMOOTH);
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

    lastTime = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(render);
    glutReshapeFunc(resize);
    glutIdleFunc(update);

    glutMainLoop();

    return 0;
}
