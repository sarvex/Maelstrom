
#ifndef _game_h
#define _game_h

class UIElement;
class UIElementIcon;
class UIElementLabel;

class GamePanelDelegate : public UIPanelDelegate
{
public:
	GamePanelDelegate(UIPanel *panel) : UIPanelDelegate(panel) { }

	virtual bool OnLoad();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnTick();
	virtual void OnDraw();

protected:
	void DrawStatus(Bool first);
	void DoHousekeeping();
	void DoBonus();
	void NextWave();

protected:
	bool m_showingBonus;

	UIElementLabel *m_score;
	UIElement *m_shield;
	UIElementLabel *m_wave;
	UIElementLabel *m_lives;
	UIElementLabel *m_bonus;

	UIElementIcon *m_multiplier[4];
	UIElementIcon *m_autofire;
	UIElementIcon *m_airbrakes;
	UIElementIcon *m_lucky;
	UIElementIcon *m_triplefire;
	UIElementIcon *m_longfire;
};

#endif // _game_h
