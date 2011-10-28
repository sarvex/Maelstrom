
#ifndef _game_h
#define _game_h

class UIElement;
class UIElementLabel;
class UIElementRect;

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

	UIElement *m_multiplier[4];
	UIElement *m_autofire;
	UIElement *m_airbrakes;
	UIElement *m_lucky;
	UIElement *m_triplefire;
	UIElement *m_longfire;

	UIElementLabel *m_multiplayerCaption;
	UIElementRect *m_multiplayerColor;
	UIElementLabel *m_fragsLabel;
	UIElementLabel *m_frags;
};

#endif // _game_h
