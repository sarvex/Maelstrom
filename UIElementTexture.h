#ifndef _UIElementTexture_h
#define _UIElementTexture_h

#include "screenlib/UIElement.h"


class UIElementTexture : public UIElement
{
public:
	UIElementTexture(UIPanel *panel, const char *name = "");
	virtual ~UIElementTexture();

	void SetTexture(SDL_Texture *texture);

	virtual void Draw();

private:
	SDL_Texture *m_texture;
};

#endif // _UIElementTexture_h
