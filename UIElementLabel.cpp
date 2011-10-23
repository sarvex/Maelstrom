
#include "UIElementLabel.h"
#include "Maelstrom_Globals.h"


UIElementLabel::UIElementLabel(UIPanel *panel, const char *name) :
	UIElement(panel, name)
{
	m_font = NULL;
	m_style = STYLE_NORM;
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
	m_texture = NULL;
}

UIElementLabel::~UIElementLabel()
{
	if (m_texture) {
		fontserv->FreeText(m_texture);
	}
}

static Uint8 ParseStyle(const char *text)
{
	Uint8 style = STYLE_NORM;

	if (strcasecmp(text, "bold") == 0) {
		style = STYLE_BOLD;
	} else if (strcasecmp(text, "underline") == 0) {
		style = STYLE_ULINE;
	} else if (strcasecmp(text, "italic") == 0) {
		style = STYLE_ITALIC;
	}
	return style;
}

bool
UIElementLabel::Load(rapidxml::xml_node<> *node)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;
	const char *fontName;
	int fontSize;

	attr = node->first_attribute("fontName", 0, false);
	if (!attr) {
		SetError("Element 'Label' missing attribute 'font'");
		return false;
	}
	fontName = attr->value();

	attr = node->first_attribute("fontSize", 0, false);
	if (!attr) {
		SetError("Element 'Label' missing attribute 'fontSize'");
		return false;
	}
	fontSize = SDL_atoi(attr->value());

	m_font = fontserv->NewFont(fontName, fontSize);
	if (!m_font) {
		SetError("Element 'Label' couldn't find font %s:%d", fontName, fontSize);
		return false;
	}

	attr = node->first_attribute("fontStyle", 0, false);
	if (attr) {
		m_style = ParseStyle(attr->value());
	}

	child = node->first_node("color", 0, false);
	if (child) {
		m_color = LoadColor(child);
	}

	attr = node->first_attribute("text", 0, false);
	if (attr) {
		SetText(attr->value());
	}

	return UIElement::Load(node);
}

void
UIElementLabel::SetText(const char *text)
{
	if (m_texture) {
		fontserv->FreeText(m_texture);
	}

	m_texture = fontserv->TextImage(text, m_font, m_style, m_color);
	m_rect.w = m_screen->GetImageWidth(m_texture);
	m_rect.h = m_screen->GetImageHeight(m_texture);
	CalculateAnchor();
}

void
UIElementLabel::Draw()
{
	if (m_texture) {
		m_screen->QueueBlit(m_rect.x, m_rect.y, m_texture, NOCLIP);
	}
}
