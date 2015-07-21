
#pragma once

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#  define snprintf _snprintf
#endif
#include <stdlib.h>					// malloc(), free()
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>			// arParamDisp()

#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include "GLM.h"           // load and draw obj model file


class ARTApp
{
public:
	// Marker detection.
	ARHandle		*arHandle ;
	ARPattHandle	*pattHandle;

	// Transformation matrix retrieval.
	AR3DHandle	*ar3DHandle ;
	ARdouble	pattTrans[3][4];		// Per-marker, but we are using only 1 marker.
	int			pattID;				// Per-marker, but we are using only 1 marker.
	float       pattWidth;

	// Drawing.
	ARParamLT *cParam ;
	ARGL_CONTEXT_SETTINGS_REF arglSettings ;
	
	GLMmodel *objModel;

	int windowWidth, windowHeight;

private:
	
	//从文件cparamName加载摄像机标定参数，创建并设置摄像机
	bool setupCamera(const char *cparamName, char *vconf, ARParamLT **cparamLT_p, ARHandle **arhandle, AR3DHandle **ar3dhandle);

	//从文件pattName创建marker
	bool setupMarker(const char *pattName, int *patt_id, ARHandle *arhandle, ARPattHandle **pattHandle_p);

	//从当前帧图像image中检测marker，并计算摄像机姿态参数矩阵pattTrans
	bool detectMarkerAndPose(ARUint8 *image, ARHandle *arHandle, AR3DHandle *ar3DHandle, int pattID, int pattWidth, ARdouble pattTrans[3][4]);

	//显示当前图像image，及由objModel表示的三维虚拟物体
	//可修改此函数实现你想要的效果
	void display(ARUint8 *arImage, ARParamLT *arParam, ARGL_CONTEXT_SETTINGS_REF arSettings, GLMmodel *objModel, const ARdouble pattTrans[3][4]);

public:

	ARTApp()
		:arHandle(NULL), pattHandle(NULL), ar3DHandle(NULL), cParam(NULL), arglSettings(NULL), objModel(NULL)
	{}

	~ARTApp();
	
	/* 初始化：调用setupCamera创建摄像机，调用setupMarker创建marker，使用GLM.h中的功能加载模型…

	@cparamName : 摄像机标定参数文件
	@pattName   : marker文件
	@objModelFile : obj三维模型文件
	@pattWidth  : marker的大小
	@modelScale : 对三维模型的缩放因子
	*/
	bool init(const char *cparamName, const char *pattName, const char *objModelFile, float pattWidth, float modelScale=1.0);

	/* 处理当前帧： 调用detectMarkerAndPose估计摄像机姿态，调用display渲染

	@image  : 当前帧的图像
	*/
	void process(ARUint8 *image);

	//设置窗口大小，当窗口大小改变时被调用
	void setWindowSize(int width, int height);
};
