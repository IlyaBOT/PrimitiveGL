#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

const char *ENGINE_VERSION = "0.2.0";

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

const float MOVE_SPEED = 2.0f;
const float TURN_SPEED = 90.0f;
const double PI = 3.14159265358979323846;

float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 4.0f;
float cameraYaw = 0.0f;

int windowWidth = WINDOW_WIDTH;
int windowHeight = WINDOW_HEIGHT;

bool debugMode = false;

int lastTime = 0;
int fpsTime = 0;
int frameCount = 0;
int currentFPS = 0;

const char *openGLVersion = "Unknown";
const char *openGLRenderer = "Unknown";
const char *openGLVendor = "Unknown";

double gpuUtilization = -1.0;
long long gpuVramUsed = -1;
long long gpuVramTotal = -1;

int sceneObjects = 1;
int renderedObjects = 1;
int texturesUsed = 0;
long long textureMemoryUsed = 0;

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

bool getNumber(CFDictionaryRef dictionary, CFStringRef key, long long &value)
{
    CFTypeRef object = CFDictionaryGetValue(dictionary, key);

    if (object == NULL || CFGetTypeID(object) != CFNumberGetTypeID())
        return false;

    SInt64 number = 0;

    if (!CFNumberGetValue((CFNumberRef)object, kCFNumberSInt64Type, &number))
        return false;

    value = (long long)number;
    return true;
}

long long detectVideoMemory()
{
    CGLContextObj context = CGLGetCurrentContext();

    if (context == NULL)
        return -1;

    GLint currentRenderer = 0;

    if (CGLGetParameter(
        context,
        kCGLCPCurrentRendererID,
        &currentRenderer
    ) != kCGLNoError)
        return -1;

    CGLRendererInfoObj rendererInfo = NULL;
    GLint rendererCount = 0;

    if (CGLQueryRendererInfo(
        0xFFFFFFFF,
        &rendererInfo,
        &rendererCount
    ) != kCGLNoError)
        return -1;

    long long videoMemory = -1;

    for (GLint i = 0; i < rendererCount; i++) {
        GLint rendererID = 0;

        CGLDescribeRenderer(
            rendererInfo,
            i,
            kCGLRPRendererID,
            &rendererID
        );

        if (rendererID == currentRenderer) {
            GLint bytes = 0;

            if (CGLDescribeRenderer(
                rendererInfo,
                i,
                kCGLRPVideoMemory,
                &bytes
            ) == kCGLNoError && bytes > 0) {
                videoMemory = (long long)bytes;
            }

            break;
        }
    }

    CGLDestroyRendererInfo(rendererInfo);

    return videoMemory;
}

void updateGPUStats()
{
    gpuUtilization = -1.0;
    gpuVramUsed = -1;

    CFMutableDictionaryRef matching = IOServiceMatching("IOAccelerator");

    if (matching == NULL)
        return;

    io_iterator_t iterator = 0;

    if (IOServiceGetMatchingServices(
        kIOMasterPortDefault,
        matching,
        &iterator
    ) != KERN_SUCCESS)
        return;

    io_registry_entry_t service;

    while ((service = IOIteratorNext(iterator)) != 0) {
        CFMutableDictionaryRef properties = NULL;

        if (IORegistryEntryCreateCFProperties(
            service,
            &properties,
            kCFAllocatorDefault,
            kNilOptions
        ) == KERN_SUCCESS && properties != NULL) {

            CFTypeRef statsObject = CFDictionaryGetValue(
                properties,
                CFSTR("PerformanceStatistics")
            );

            if (statsObject != NULL &&
                CFGetTypeID(statsObject) == CFDictionaryGetTypeID()) {

                CFDictionaryRef stats = (CFDictionaryRef)statsObject;
                long long value = 0;

                // Some drivers expose an ordinary percentage.
                if (getNumber(stats, CFSTR("Device Utilization %"), value)) {
                    gpuUtilization = (double)value;
                }
                // Older Intel/ATI drivers often expose this fixed-point value.
                else if (getNumber(stats, CFSTR("GPU Core Utilization"), value)) {
                    if (value > 100)
                        gpuUtilization = (double)value / 10000000.0;
                    else
                        gpuUtilization = (double)value;
                }

                if (gpuUtilization > 100.0)
                    gpuUtilization = 100.0;

                if (gpuUtilization < 0.0)
                    gpuUtilization = -1.0;

                if (!getNumber(stats, CFSTR("vramUsedBytes"), gpuVramUsed))
                    getNumber(stats, CFSTR("In use system memory"), gpuVramUsed);

                // If CGL could not report total VRAM, try driver used + free.
                if (gpuVramTotal < 0 && gpuVramUsed >= 0) {
                    long long freeBytes = 0;

                    if (getNumber(stats, CFSTR("vramFreeBytes"), freeBytes))
                        gpuVramTotal = gpuVramUsed + freeBytes;
                }
            }

            CFRelease(properties);
        }

        IOObjectRelease(service);

        if (gpuUtilization >= 0.0 || gpuVramUsed >= 0)
            break;
    }

    IOObjectRelease(iterator);
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

        glColor3f(0.20f, 0.20f, 0.20f);
        glVertex2f(0.0f, 0.0f);

        glColor3f(0.20f, 0.20f, 0.20f);
        glVertex2f(1.0f, 0.0f);

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

void drawText(int x, int y, const char *text)
{
    glRasterPos2i(x, y);

    while (*text != '\0') {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *text);
        text++;
    }
}

void drawDebugOverlay()
{
    if (!debugMode)
        return;

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Dark translucent background for readable text.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);

    glBegin(GL_QUADS);
        glVertex2i(8, 8);
        glVertex2i(windowWidth - 8, 8);
        glVertex2i(windowWidth - 8, 158);
        glVertex2i(8, 158);
    glEnd();

    glDisable(GL_BLEND);

    glColor3f(1.0f, 1.0f, 1.0f);

    char line[512];
    int y = 25;
    const int lineHeight = 14;

    sprintf(
        line,
        "PrimitiveGL 3D Engine ver. %s, build %s. By IlyaBOT",
        ENGINE_VERSION,
        __DATE__
    );
    drawText(16, y, line);
    y += lineHeight;

    sprintf(line, "FPS: %d", currentFPS);
    drawText(16, y, line);
    y += lineHeight;

    sprintf(line, "Renderer: OpenGL %s", openGLVersion);
    drawText(16, y, line);
    y += lineHeight;

    sprintf(line, "GPU: %s", openGLRenderer);
    drawText(16, y, line);
    y += lineHeight;

    if (gpuUtilization >= 0.0)
        sprintf(line, "GPU Util: %.1f%%", gpuUtilization);
    else
        sprintf(line, "GPU Util: N/A");

    drawText(16, y, line);
    y += lineHeight;

    if (gpuVramUsed >= 0 && gpuVramTotal > 0) {
        sprintf(
            line,
            "GPU VRAM Used: %.1f MB / %.1f MB",
            (double)gpuVramUsed / (1024.0 * 1024.0),
            (double)gpuVramTotal / (1024.0 * 1024.0)
        );
    }
    else {
        sprintf(line, "GPU VRAM Used: N/A");
    }

    drawText(16, y, line);
    y += lineHeight;

    sprintf(
        line,
        "Objects: %d | Rendered: %d",
        sceneObjects,
        renderedObjects
    );
    drawText(16, y, line);
    y += lineHeight;

    sprintf(
        line,
        "Textures used: %d | %.1f MB",
        texturesUsed,
        (double)textureMemoryUsed / (1024.0 * 1024.0)
    );
    drawText(16, y, line);
    y += lineHeight;

    sprintf(
        line,
        "Viewport: %dx%d | OpenGL Vendor: %s",
        windowWidth,
        windowHeight,
        openGLVendor
    );
    drawText(16, y, line);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}

void applyCamera()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRotatef(-cameraYaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-cameraX, -cameraY, -cameraZ);
}

void updateWindowTitle()
{
    char title[128];

    sprintf(
        title,
        "PrimitiveGL 3D Engine | FPS: %d FPS",
        currentFPS
    );

    glutSetWindowTitle(title);
}

void updateFPS()
{
    int now = glutGet(GLUT_ELAPSED_TIME);

    if (now - fpsTime >= 1000) {
        currentFPS = (int)(
            frameCount * 1000.0 / (double)(now - fpsTime) + 0.5
        );

        frameCount = 0;
        fpsTime = now;

        updateWindowTitle();

        if (debugMode)
            updateGPUStats();
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground();

    applyCamera();

    renderedObjects = 1;
    drawCube();

    drawDebugOverlay();

    glutSwapBuffers();

    frameCount++;
    updateFPS();
}

void update()
{
    int now = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (now - lastTime) / 1000.0f;
    lastTime = now;

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

    windowWidth = width;
    windowHeight = height;

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
    glShadeModel(GL_SMOOTH);

    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *vendor = glGetString(GL_VENDOR);

    if (version != NULL)
        openGLVersion = (const char *)version;

    if (renderer != NULL)
        openGLRenderer = (const char *)renderer;

    if (vendor != NULL)
        openGLVendor = (const char *)vendor;

    gpuVramTotal = detectVideoMemory();

    if (debugMode)
        updateGPUStats();
}

void parseArguments(int &argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debugMode = true;

            for (int j = i; j < argc - 1; j++)
                argv[j] = argv[j + 1];

            argc--;
            i--;
        }
    }
}

int main(int argc, char **argv)
{
    parseArguments(argc, argv);

    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGB |
        GLUT_DEPTH
    );

    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("PrimitiveGL 3D Engine | FPS: 0 FPS");

    initRenderer();

    lastTime = glutGet(GLUT_ELAPSED_TIME);
    fpsTime = lastTime;

    glutDisplayFunc(render);
    glutReshapeFunc(resize);
    glutIdleFunc(update);

    glutMainLoop();

    return 0;
}
