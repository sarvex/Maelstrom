
#include "UIElementLabel.h"
#include "Maelstrom_Globals.h"

UIElementType UIElementLabel::s_elementType;


UIElementLabel::UIElementLabel(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_fontName = NULL;
	m_fontSize = 0;
	m_fontStyle = UIFONT_STYLE_NORMAL;
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
	m_text = NULL;
	m_texture = NULL;
}

UIElementLabel::~UIElementLabel()
{
	if (m_fontName) {
		SDL_free(m_fontName);
	}
	if (m_text) {
		SDL_free(m_text);
	}
	if (m_texture) {
		GetUI()->FreeText(m_texture);
	}
}

static UIFontStyle ParseStyle(const char *text)
{
	if (strcasecmp(text, "BOLD") == 0) {
		return UIFONT_STYLE_BOLD;
	} else if (strcasecmp(text, "UNDERLINE") == 0) {
		return UIFONT_STYLE_UNDERLINE;
	} else if (strcasecmp(text, "ITALIC") == 0) {
		return UIFONT_STYLE_UNDERLINE;
	}
	return UIFONT_STYLE_NORMAL;
}

bool
UIElementLabel::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("fontName", 0, false);
	if (attr) {
		if (m_fontName) {
			SDL_free(m_fontName);
		}
		m_fontName = SDL_strdup(attr->value());
	}

	attr = node->first_attribute("fontSize", 0, false);
	if (attr) {
		m_fontSize = SDL_atoi(attr->value());
	}

	attr = node->first_attribute("fontStyle", 0, false);
	if (attr) {
		m_fontStyle = ParseStyle(attr->value());
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
	SDL_Texture *texture;

	if (!m_fontName || !m_fontSize) {
		SetError("You must set a font first");
		return;
	}

	if (m_text && SDL_strcmp(text, m_text) == 0) {
		return;
	}

	if (m_text) {
		SDL_free(m_text);
	}
	if (m_texture) {
		GetUI()->FreeText(m_texture);
	}

	m_text = SDL_strdup(text);
	m_texture = GetUI()->CreateText(m_text, m_fontName, m_fontSize, m_fontStyle, m_color);

	m_rect.w = m_screen->GetImageWidth(m_texture);
	m_rect.h = m_screen->GetImageHeight(m_texture);
	CalculateAnchor();
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
			GetUI()->FreeText(m_texture);
		}
		m_texture = GetUI()->CreateText(m_text, m_fontName, m_fontSize, m_fontStyle, m_color);
	}
}

void
UIElementLabel::Draw()
{
	if (m_texture) {
		m_screen->QueueBlit(m_rect.x, m_rect.y, m_texture, NOCLIP);
	}
}
