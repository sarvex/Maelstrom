#ifndef _MacDialogLabel_h
#define _MacDialogLabel_h

#include "screenlib/UIElementLabel.h"

class MacDialogLabel : public UIElementLabel
{
public:
	MacDialogLabel(UIBaseElement *parent, const char *name = "");

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

#endif // _MacDialogLabel_h
