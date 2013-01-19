#ifndef __KEYPAD_H__
#define __KEYPAD_H__

//#include "keypad.h"
#include "ui_keypad.h"
#include <qlineedit.h>
#include <qpushbutton.h>
#define MAXDIGIT 6

typedef struct
{
  int pin;
  int pin_digit;
  char name[20];
  char path[20];
}PIN_INFO;

class KeyPad : public QDialog, private Ui::KeyPad
{
  Q_OBJECT

private:
  int m_PIN;
  int m_PIN_Digit;
  int m_MaxDigit;
  int m_PinInfo_Number;
  PIN_INFO *m_PinInfo;
  QString m_UserName;
  QString m_UserPath;
  int m_iKeyPadStatus;

public:
  KeyPad(QDialog *parent = 0);
  ~KeyPad();
  int addNumber(int no);
  int displayPIN();
  int getPIN();
  QString getUserName();
  int getStatus();
  void setStatus(int status);
  void setEnabledAll(bool enabled);

private slots:
  void slotKP_1();
  void slotKP_2();
  void slotKP_3();
  void slotKP_4();
  void slotKP_5();
  void slotKP_6();
  void slotKP_7();
  void slotKP_8();
  void slotKP_9();
  void slotKP_0();
  void slotKP_BS();
  void slotKP_CLS();
  void slotCheckPIN();

signals:
  void signalAuthorized();
};

#endif
