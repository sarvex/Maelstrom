#ifndef _MacDialogLabel_h
#define _MacDialogLabel_h

#include "screenlib/UIElementLabel.h"

class MacDialogLabel : public UIElementLabel
{
DECLARE_TYPESAFE_CLASS(UIElementLabel)
public:
	MacDialogLabel(UIBaseElement *parent, const char *name = "");
};

#endif // _MacDialogLabel_h
