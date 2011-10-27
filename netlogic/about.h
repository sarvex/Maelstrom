
#ifndef _about_h
#define _about_h

#include "../Maelstrom_Globals.h"

class Object;

class AboutPanelDelegate : public UIPanelDelegate
{
public:
	AboutPanelDelegate(UIPanel *panel) : UIPanelDelegate(panel) {
		numsprites = 0;
	}
	virtual ~AboutPanelDelegate() {
		assert(numsprites == 0);
	}

	virtual void OnShow();
	virtual void OnHide();
	virtual void OnDraw();

protected:
	int numsprites;
	Object *objects[MAX_SPRITES];
};

#endif // _about_h
