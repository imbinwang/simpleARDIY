/*
*  simpleARDIY.cpp
*
*  DIY code to demonstrate use of ARToolKit
*    in Shandong University Visual Computing Summer School.
*
*  Author(s): Interdisciplinary Research Center, School of Computer Science and Technology, Shandong University
*  Homepage:  http://irc.cs.sdu.edu.cn/
*
*/

#include"DIY.h"


ARTApp app;

// You need to change these pathes to your own data
const float pattWidth = 40.0f;					// size of marker, in mm unit, change it for your marker
const char *cparamName = "Data/camera_para.dat"; // 相机标定参数文件
const char *pattName = "Data/patt.irc";			// 标记模板文件
const char *objModelFile = "Data/bunny.obj";	// .obj三维模型文件

// ============================================================================
//	Function prototypes.
// ============================================================================
static void Keyboard(unsigned char key, int x, int y);
static void mainLoop(void);
static void Reshape(int w, int h);

//display function would not be called
void Display()
{
}


int main(int argc, char** argv)
{
	if (!app.init(cparamName, pattName, objModelFile, pattWidth, 1.0))
		return -1;

	glutInit(&argc, argv);

	// Set up GL context(s) for OpenGL to draw into.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow(argv[0]);

	// Setup ARgsub_lite library for current OpenGL context.
	ARGL_CONTEXT_SETTINGS_REF arglSettings = arglSetupForCurrentContext(&(app.cParam->param), arVideoGetPixelFormat());
	if (!arglSettings) 
	{
		ARLOGe("arglSetupForCurrentContext() returned error.\n");
		return -1;
	}
	arglSetupDebugMode(arglSettings, app.arHandle);
	arUtilTimerReset();
	app.arglSettings = arglSettings;

	// Register GLUT event-handling callbacks.
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutDisplayFunc(Display);
	glutIdleFunc(mainLoop);

	glutMainLoop();

	arglCleanup(arglSettings);

	return (0);
}


static void mainLoop(void)
{
	static int ms_prev;

	// Find out how long since mainLoop() last ran.
	int ms = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = ms - ms_prev;
	if (elapsed < 10) 
		return; // Don't update more often than 100 Hz.
	ms_prev = ms;

	ARUint8 *image = arVideoGetImage();
	if (image) 
	{
		app.process(image);
	}
}

//	This function is called when the GLUT window is resized.
static void Reshape(int w, int h)
{
	app.setWindowSize(w, h);
}

static void Keyboard(unsigned char key, int x, int y)
{
	switch (key) 
	{
	case 'Q':
	case 'q':
		exit(0);
		break;
	}
}
