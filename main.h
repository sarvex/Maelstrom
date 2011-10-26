
#ifndef _main_h
#define _main_h

class MainPanelDelegate : public UIPanelDelegate
{
public:
	MainPanelDelegate(UIPanel *panel) : UIPanelDelegate(panel) { }

	virtual bool OnLoad();
	virtual void OnTick();
};

#endif // _main_h
