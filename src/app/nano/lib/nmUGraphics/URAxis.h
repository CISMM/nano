#ifndef URAXIS_H
#define URAXIS_H

#include "URender.h"

class URAxis:public URender{
private:
public:
	URAxis();
	~URAxis();
	int Render(void *userdata=NULL);

	int SetVisibilityAll(void *userdata=NULL);
	int SetProjTextAll(void *userdata=NULL);
	int SetClampAll(void *userdata=NULL);
	int ScaleAll(void *userdata=NULL);
	int SetTransxAll(void *userdata=NULL);
	int SetTransyAll(void *userdata=NULL);
	int SetTranszAll(void *userdata=NULL);
	int SetRotxAll(void *userdata=NULL);
	int SetRotyAll(void *userdata=NULL);
	int SetRotzAll(void *userdata=NULL);
	int SetColorAll(void *userdata=NULL);
	int SetAlphaAll(void *userdata=NULL);

	int ChangeStaticFile(void *userdata=NULL);
};

#endif
