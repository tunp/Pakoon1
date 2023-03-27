#ifndef DLG_ON_SCREEN_KBD_H
#define DLG_ON_SCREEN_KBD_H

#include "Dialog.h"
#include "KbdButton.h"

#include "../Pakoon1View.h"

class DlgOnScreenKbd : public Dialog {
private:
	CPakoon1View *pView;
  int esc_code;
public:
	DlgOnScreenKbd(CPakoon1View *pView);
	~DlgOnScreenKbd();
	bool onMousePress(int x, int y);
	bool onMouseRelease(int x, int y);
	bool onMouseMove(int x, int y);
  bool onFingerDown(int x, int y, int finger_id);
	static void keyPressCallback(void *p, Button *b);
	static void keyReleaseCallback(void *p, Button *b);
	void keyPress(int *key_press);
	void keyRelease(int *key_press);
  bool needsRedraw();
};

#endif
