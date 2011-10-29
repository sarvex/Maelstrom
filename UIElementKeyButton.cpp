
#include "UIElementKeyButton.h"
#include "screenlib/UIElementLabel.h"

UIElementType UIElementKeyButton::s_elementType;

UIElementKeyButton::UIElementKeyButton(UIBaseElement *parent, const char *name) :
	UIElementButton(parent, name)
{
}

UIElementKeyButton::~UIElementKeyButton()
{
}

bool
UIElementKeyButton::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	if (!UIElementButton::Load(node, templates)) {
		return false;
	}

	if (m_hotkey != SDLK_UNKNOWN) {
		UIElementLabel *label;
		const char *text;

		text = SDL_GetKeyName(m_hotkey);
		label = GetElement<UIElementLabel>("label");
		if (label) {
			label->SetText(text);
		}
		label = GetElement<UIElementLabel>("shadow");
		if (label) {
			label->SetText(text);
		}
	}
	return true;
}
