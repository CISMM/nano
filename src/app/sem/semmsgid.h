//---------- Message structure ----------------------
#ifndef SEM_MESSAGE_H
#define SEM_MESSAGE_H
extern "C" {
	typedef struct tagSEM_MESSAGE {
		long kv;
		long mag;
		long spot;
		long wd;   
		long contr;    
		long bright;
		long scanType;
		long scanLines;
		long scanTime;
		long beamOnOf;
		long kvOnOf;
		long detIndex;
		long stgX;
		long stgY;
		long stgZ;
		long stgT;
		long stgR;
		long param[4];    //reserved
		} SEM_MESSAGE;
	};
#endif