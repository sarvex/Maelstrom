
#ifndef _controls_h
#define _controls_h

// Functions from controls.cc
#ifdef USE_JOYSTICK
extern void	CalibrateJoystick(char *joystick);
#endif
extern void	LoadControls(void);
extern void	SaveControls(void);
extern int	PollEvent(SDL_Event *event, int timeout);
extern void	HandleEvents(int timeout);
extern int	DropEvents(void);
extern void	ShowDawn(void);

/* Generic key control definitions */
#define THRUST_KEY	0x01
#define RIGHT_KEY	0x02
#define LEFT_KEY	0x03
#define SHIELD_KEY	0x04
#define FIRE_KEY	0x05
#define PAUSE_KEY	0x06
#define ABORT_KEY	0x07

/* The controls structure */
typedef struct {
	SDL_Keycode gPauseControl;
	SDL_Keycode gShieldControl;
	SDL_Keycode gThrustControl;
	SDL_Keycode gTurnRControl;
	SDL_Keycode gTurnLControl;
	SDL_Keycode gFireControl;
	SDL_Keycode gQuitControl;
} Controls;


class UIElementLabel;
class UIElementRadioGroup;

class ControlsDialogDelegate : public UIPanelDelegate
{
public:
	ControlsDialogDelegate(UIPanel *panel) : UIPanelDelegate(panel) { }

	virtual bool OnLoad();
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnTick();
	virtual bool HandleEvent(const SDL_Event &event);

protected:
	void ShowKeyLabel(int index);
	void ShowKeyLabels() {
		for (int i = 0; i < NUM_CTLS; ++i) {
			ShowKeyLabel(i);
		}
	}

	SDL_Keycode GetKeycode(int index);
	void SetKeycode(int index, SDL_Keycode keycode);

protected:
	enum {
		FIRE_CTL,
		THRUST_CTL,
		SHIELD_CTL,
		TURNR_CTL,
		TURNL_CTL,
		PAUSE_CTL,
		QUIT_CTL,
		NUM_CTLS
	};

	Controls m_controls;
	UIElementLabel *m_controlKeys[NUM_CTLS];
	UIElementRadioGroup *m_radioGroup;
	Uint32 m_keyinuseTimers[NUM_CTLS];
};

#endif /* _controls_h */
