#ifndef _UIElementTexture_h
#define _UIElementTexture_h

#include "UIElement.h"


class UIElementTexture : public UIElement
{
public:
	UIElementTexture(UIPanel *panel, const char *name = "");
	virtual ~UIElementTexture();

	virtual bool IsA(UIElementType type) {
		return UIElement::IsA(type) || type == GetType();
	}

	void SetTexture(SDL_Texture *texture);

	virtual void Draw();

private:
	SDL_Texture *m_texture;

protected:
	static UIElementType s_elementType;

public:
	static UIElementType GetType() {
		if (!s_elementType) {
			s_elementType = GenerateType();
		}
		return s_elementType;
	}
};

#endif // _UIElementTexture_h
