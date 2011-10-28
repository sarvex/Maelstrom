
#include "UIElementLabel.h"
#include "Maelstrom_Globals.h"

UIElementType UIElementLabel::s_elementType;


UIElementLabel::UIElementLabel(UIPanel *panel, const char *name) :
	UIElement(panel, name)
{
	m_font = NULL;
	m_style = STYLE_NORM;
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
	m_text = NULL;
	m_texture = NULL;
}

UIElementLabel::~UIElementLabel()
{
	if (m_text) {
		delete[] m_text;
	}
	if (m_texture) {
		fontserv->FreeText(m_texture);
	}
}

static Uint8 ParseStyle(const char *text)
{
	Uint8 style = STYLE_NORM;

	if (strcasecmp(text, "BOLD") == 0) {
		style = STYLE_BOLD;
	} else if (strcasecmp(text, "UNDERLINE") == 0) {
		style = STYLE_ULINE;
	} else if (strcasecmp(text, "ITALIC") == 0) {
		style = STYLE_ITALIC;
	}
	return style;
}

bool
UIElementLabel::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;
	const char *fontName = NULL;
	int fontSize = 0;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("fontName", 0, false);
	if (attr) {
		fontName = attr->value();
	}

	attr = node->first_attribute("fontSize", 0, false);
	if (attr) {
		fontSize = SDL_atoi(attr->value());
	}

	if (fontName && fontSize) {
		m_font = fontserv->NewFont(fontName, fontSize);
		if (!m_font) {
			SetError("Element 'Label' couldn't find font %s:%d", fontName, fontSize);
			return false;
		}
	} else if (!m_font) {
		SetError("Element 'Label' missing attribute 'fontName' or 'fontSize'");
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

	return true;
}

void
UIElementLabel::SetText(const char *text)
{
	if (m_text && strcmp(text, m_text) == 0) {
		return;
	}

	if (m_text) {
		delete[] m_text;
	}
	if (m_texture) {
		fontserv->FreeText(m_texture);
	}

	m_text = new char[strlen(text)+1];
	strcpy(m_text, text);
	m_texture = fontserv->TextImage(m_text, m_font, m_style, m_color);
	m_rect.w = m_screen->GetImageWidth(m_texture);
	m_rect.h = m_screen->GetImageHeight(m_texture);
	CalculateAnchor();
#ifdef UI_DEBUG
if (m_rect.x)
printf("Label: '%s' %d,%d\n", m_text, m_rect.x, m_rect.y);
#endif
}

void
UIElementLabel::SetTextColor(Uint8 R, Uint8 G, Uint8 B)
{
	Uint32 color;

	color = m_screen->MapRGB(R, G, B);
	if (color == m_color) {
		return;
	}
	m_color = color;

	if (m_text) {
		if (m_texture) {
			fontserv->FreeText(m_texture);
		}
		m_texture = fontserv->TextImage(m_text, m_font, m_style, m_color);
	}
}

void
UIElementLabel::Draw()
{
	if (m_texture) {
		m_screen->QueueBlit(m_rect.x, m_rect.y, m_texture, NOCLIP);
	}
}
