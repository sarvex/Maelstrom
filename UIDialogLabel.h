#ifndef _UIDialogLabel_h
#define _UIDialogLabel_h

#include "screenlib/UIElementLabel.h"

class UIDialogLabel : public UIElementLabel
{
public:
	UIDialogLabel(UIBaseElement *parent, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElementLabel::IsA(type) || type == GetType();
	}

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

#endif // _UIDialogLabel_h
