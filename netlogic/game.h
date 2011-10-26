
#ifndef _game_h
#define _game_h

class GamePanelDelegate : public UIPanelDelegate
{
public:
	GamePanelDelegate(UIPanel *panel) : UIPanelDelegate(panel) { }

	virtual bool OnLoad();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnTick();
	virtual void OnDraw();
};

#endif // _game_h
