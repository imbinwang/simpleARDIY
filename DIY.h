
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
	
	//���ļ�cparamName����������궨���������������������
	bool setupCamera(const char *cparamName, char *vconf, ARParamLT **cparamLT_p, ARHandle **arhandle, AR3DHandle **ar3dhandle);

	//���ļ�pattName����marker
	bool setupMarker(const char *pattName, int *patt_id, ARHandle *arhandle, ARPattHandle **pattHandle_p);

	//�ӵ�ǰ֡ͼ��image�м��marker���������������̬��������pattTrans
	bool detectMarkerAndPose(ARUint8 *image, ARHandle *arHandle, AR3DHandle *ar3DHandle, int pattID, int pattWidth, ARdouble pattTrans[3][4]);

	//��ʾ��ǰͼ��image������objModel��ʾ����ά��������
	//���޸Ĵ˺���ʵ������Ҫ��Ч��
	void display(ARUint8 *arImage, ARParamLT *arParam, ARGL_CONTEXT_SETTINGS_REF arSettings, GLMmodel *objModel, const ARdouble pattTrans[3][4]);

public:

	ARTApp()
		:arHandle(NULL), pattHandle(NULL), ar3DHandle(NULL), cParam(NULL), arglSettings(NULL), objModel(NULL)
	{}

	~ARTApp();
	
	/* ��ʼ��������setupCamera���������������setupMarker����marker��ʹ��GLM.h�еĹ��ܼ���ģ�͡�

	@cparamName : ������궨�����ļ�
	@pattName   : marker�ļ�
	@objModelFile : obj��άģ���ļ�
	@pattWidth  : marker�Ĵ�С
	@modelScale : ����άģ�͵���������
	*/
	bool init(const char *cparamName, const char *pattName, const char *objModelFile, float pattWidth, float modelScale=1.0);

	/* ����ǰ֡�� ����detectMarkerAndPose�����������̬������display��Ⱦ

	@image  : ��ǰ֡��ͼ��
	*/
	void process(ARUint8 *image);

	//���ô��ڴ�С�������ڴ�С�ı�ʱ������
	void setWindowSize(int width, int height);
};
