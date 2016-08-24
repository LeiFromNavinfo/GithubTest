#include <jni.h>
#include "Editor/Editor.h"

class JNIProgress : public Editor::ProgressEventer
{
	virtual int         OnStarted() { return 0;}

	virtual int         OnFinished() { return 0;}

	virtual int         OnFailed() { return 0;}

	virtual int         OnProgress(std::string description, double progress);

public:
	JNIProgress();
	~JNIProgress();

	static void			DisposeProgress();
};

