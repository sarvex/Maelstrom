#ifndef _UIPanels_h
#define _UIPanels_h

class UIManager;
class UIPanel;

UIPanel *CreateMaelstromUIPanel(UIManager *ui, const char *type, const char *name, const char *delegate);

#endif // _UIPanels_h
