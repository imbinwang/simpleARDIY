
#include"DIY.h"


bool ARTApp::setupCamera(const char *cparamName, char *vconf, ARParamLT **cparamLT_p, ARHandle **arhandle, AR3DHandle **ar3dhandle)
{
	ARParam			cparam;
	int				xsize, ysize;
	AR_PIXEL_FORMAT pixFormat;

	// Open the video path.
	if (arVideoOpen(vconf) < 0) {
		ARLOGe("setupCamera(): Unable to open connection to camera.\n");
		return false;
	}

	// Find the size of the window.
	if (arVideoGetSize(&xsize, &ysize) < 0) {
		ARLOGe("setupCamera(): Unable to determine camera frame size.\n");
		arVideoClose();
		return false;
	}
	ARLOGi("Camera image size (x,y) = (%d,%d)\n", xsize, ysize);

	// Get the format in which the camera is returning pixels.
	pixFormat = arVideoGetPixelFormat();
	if (pixFormat == AR_PIXEL_FORMAT_INVALID) {
		ARLOGe("setupCamera(): Camera is using unsupported pixel format.\n");
		arVideoClose();
		return false;
	}

	// Load the camera parameters, resize for the window and init.
	if (arParamLoad(cparamName, 1, &cparam) < 0) {
		ARLOGe("setupCamera(): Error loading parameter file %s for camera.\n", cparamName);
		arVideoClose();
		return false;
	}
	if (cparam.xsize != xsize || cparam.ysize != ysize) {
		ARLOGw("*** Camera Parameter resized from %d, %d. ***\n", cparam.xsize, cparam.ysize);
		arParamChangeSize(&cparam, xsize, ysize, &cparam);
	}
#ifdef DEBUG
	ARLOG("*** Camera Parameter ***\n");
	arParamDisp(&cparam);
#endif
	if ((*cparamLT_p = arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL) {
		ARLOGe("setupCamera(): Error: arParamLTCreate.\n");
		return false;
	}

	if ((*arhandle = arCreateHandle(*cparamLT_p)) == NULL) {
		ARLOGe("setupCamera(): Error: arCreateHandle.\n");
		return false;
	}
	if (arSetPixelFormat(*arhandle, pixFormat) < 0) {
		ARLOGe("setupCamera(): Error: arSetPixelFormat.\n");
		return false;
	}
	if (arSetDebugMode(*arhandle, AR_DEBUG_DISABLE) < 0) {
		ARLOGe("setupCamera(): Error: arSetDebugMode.\n");
		return false;
	}
	if ((*ar3dhandle = ar3DCreateHandle(&cparam)) == NULL) {
		ARLOGe("setupCamera(): Error: ar3DCreateHandle.\n");
		return false;
	}

	if (arVideoCapStart() != 0) {
		ARLOGe("setupCamera(): Unable to begin camera data capture.\n");
		return false;
	}

	return true;
}

bool ARTApp::setupMarker(const char *pattName, int *patt_id, ARHandle *arhandle, ARPattHandle **pattHandle_p)
{
	if ((*pattHandle_p = arPattCreateHandle()) == NULL) {
		ARLOGe("setupMarker(): Error: arPattCreateHandle.\n");
		return false;
	}

	// Loading only 1 pattern in this example.
	if ((*patt_id = arPattLoad(*pattHandle_p, pattName)) < 0) {
		ARLOGe("setupMarker(): Error loading pattern file %s.\n", pattName);
		arPattDeleteHandle(*pattHandle_p);
		return false;
	}

	arPattAttach(arhandle, *pattHandle_p);

	return true;
}

bool ARTApp::init(const char *cparamName, const char *pattName, const char *objModelFile, float pattWidth, float modelScale)
{
	if (arHandle) //has initialized
		return false;

	if (!setupCamera(cparamName, "", &cParam, &arHandle, &ar3DHandle)) {
		return false;
	}

	if (!setupMarker(pattName, &pattID, arHandle, &pattHandle)) {
		return false;
	}

	{
		objModel = glmReadOBJ((char*)objModelFile);
		if (!objModel)
		{
			ARLOGe("Unable to load obj model file.\n");
			return false;
		}
		glmUnitize(objModel);
		glmScale(objModel, pattWidth*modelScale);
	}
	this->pattWidth = pattWidth;

	return true;
}

bool ARTApp::detectMarkerAndPose(ARUint8 *image, ARHandle *arHandle, AR3DHandle *ar3DHandle, int pattID, int pattWidth, ARdouble pattTrans[3][4])
{
	// Detect the markers in the video frame.
	if (arDetectMarker(arHandle, image) < 0) {
		return false;
	}

	// Check through the marker_info array for highest confidence
	// visible marker matching our preferred pattern.
	int k = -1;
	for (int j = 0; j < arHandle->marker_num; j++) {
		if (arHandle->markerInfo[j].id == pattID) {
			if (k == -1) k = j; // First marker detected.
			else if (arHandle->markerInfo[j].cf > arHandle->markerInfo[k].cf) k = j; // Higher confidence marker detected.
		}
	}

	if (k != -1) {
		// Get the transformation between the marker and the real camera into pattTans.
		arGetTransMatSquare(ar3DHandle, &(arHandle->markerInfo[k]), pattWidth, pattTrans);
		return true;
	}

	return false;
}

void ARTApp::process(ARUint8 *image)
{
	bool pattFound = detectMarkerAndPose(image, arHandle, ar3DHandle, pattID, pattWidth, pattTrans);

	display(image, cParam, arglSettings, pattFound? objModel : NULL, pattTrans);
}

void ARTApp::setWindowSize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

#define VIEW_DISTANCE_MIN		40.0        // Objects closer to the camera than this will not be displayed. OpenGL units.
#define VIEW_DISTANCE_MAX		10000.0     // Objects further away from the camera than this will not be displayed. OpenGL units.
#define VIEW_SCALEFACTOR		1.0         // Units received from ARToolKit tracking will be multiplied by this factor before being used in OpenGL drawing.


void ARTApp::display(ARUint8 *arImage, ARParamLT *arParam, ARGL_CONTEXT_SETTINGS_REF arSettings, GLMmodel *objModel, const ARdouble pattTrans[3][4])
{
	ARdouble p[16];
	ARdouble m[16];

	// Select correct buffer for this context.
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers for new frame.

	arglDispImage(arImage, &(arParam->param), 1.0, arSettings);	// zoom = 1.0.

	// Projection transformation.
	arglCameraFrustumRH(&(arParam->param), VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, p);
	glMatrixMode(GL_PROJECTION);
	
	glLoadMatrixd(p);

	glMatrixMode(GL_MODELVIEW);

	// Viewing transformation.
	glLoadIdentity();
	// Lighting and geometry that moves with the camera should go here.
	// (I.e. must be specified before viewing transformations.)
	//none
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	if (objModel) 
	{
		// Calculate the camera position relative to the marker.
		// Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
		arglCameraViewRH(pattTrans, m, VIEW_SCALEFACTOR);
		glLoadMatrixd(m);

		// All lighting and geometry to be drawn relative to the marker goes here.
		glPushMatrix(); // Save world coordinate system.
		glTranslatef(0.0f, 0.0f, pattWidth / 2.0); // Place base of object on marker surface.

		glmDraw(objModel, GLM_SMOOTH | GLM_MATERIAL);
		glPopMatrix();    // Restore world coordinate system.
	} 

	// Any 2D overlays go here.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (GLdouble)windowWidth, 0, (GLdouble)windowHeight, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

ARTApp::~ARTApp()
{
	arPattDetach(arHandle);
	arPattDeleteHandle(pattHandle);
	arVideoCapStop();
	ar3DDeleteHandle(&ar3DHandle);
	arDeleteHandle(arHandle);
	arParamLTFree(&cParam);
	arVideoClose();

	if (objModel)
		glmDelete(objModel);
}