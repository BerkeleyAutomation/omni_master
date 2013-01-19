#include "keypad.h"
#include <qpushbutton.h>
#include <qfile.h>
#include <QtGui>

#define KP_SHOW			0
#define KP_HIDE_FAIL		1
#define KP_HIDE_AUTHORIZED	2
//#define _DATA_DISPLAY

KeyPad::KeyPad(QDialog *parent)
    : QDialog(parent)
{
  //This is required defaultly
  setupUi(this);
  
  ///  ** Connecting ** ///////////////////
  connect(pushButton_KP0, SIGNAL(clicked()),this, SLOT(slotKP_0()));
  connect(pushButton_KP1, SIGNAL(clicked()),this, SLOT(slotKP_1()));
  connect(pushButton_KP2, SIGNAL(clicked()),this, SLOT(slotKP_2()));
  connect(pushButton_KP3, SIGNAL(clicked()),this, SLOT(slotKP_3()));
  connect(pushButton_KP4, SIGNAL(clicked()),this, SLOT(slotKP_4()));
  connect(pushButton_KP5, SIGNAL(clicked()),this, SLOT(slotKP_5()));
  connect(pushButton_KP6, SIGNAL(clicked()),this, SLOT(slotKP_6()));
  connect(pushButton_KP7, SIGNAL(clicked()),this, SLOT(slotKP_7()));
  connect(pushButton_KP8, SIGNAL(clicked()),this, SLOT(slotKP_8()));
  connect(pushButton_KP9, SIGNAL(clicked()),this, SLOT(slotKP_9()));
  connect(ButtonOk, SIGNAL(clicked()),this, SLOT(slotCheckPIN()));
  connect(pushButton_KP_BS, SIGNAL(clicked()),this, SLOT(slotKP_BS()));
  connect(pushButton_KP_CLS, SIGNAL(clicked()),this, SLOT(slotKP_CLS()));
  
  m_PIN = 0;
  m_PIN_Digit = 0;
  m_MaxDigit = MAXDIGIT;
  m_UserName = "";
  m_UserPath = "";
  m_iKeyPadStatus = KP_SHOW;
  
  QString file_path;
  if(QFile::exists("./Configuration/Surgeon_PIN.inf"))
  {
	  file_path = "./Configuration/Surgeon_PIN.inf";
  }
  else if(QFile::exists("./GUI_Server/Configuration/Surgeon_PIN.inf"))
  {
	  file_path = "./GUI_Server/Configuration/Surgeon_PIN.inf";
  }
  else if(QFile::exists("../Configuration/Surgeon_PIN.inf"))
  {
	  file_path = "../Configuration/Surgeon_PIN.inf";
  }
  else
	  file_path = "";

  if(file_path != "")
  {
    FILE *fp = fopen(file_path, "r");
    fscanf(fp,"%d\n",&m_PinInfo_Number);
    m_PinInfo = new PIN_INFO[m_PinInfo_Number];
    for(int i=0;i<m_PinInfo_Number;i++)
    {
      fscanf(fp,"%s %d %d\n", m_PinInfo[i].name, &m_PinInfo[i].pin_digit, &m_PinInfo[i].pin);
      strcpy(m_PinInfo[i].path, m_PinInfo[i].name);
      qDebug("%d\n", m_PinInfo[i].pin);
    }
  }
  else // If there is no files
  {
    m_PinInfo_Number = 1;
    m_PinInfo = new PIN_INFO[m_PinInfo_Number];
    m_PinInfo[0].pin = 1;
    m_PinInfo[0].pin_digit = 1;
    strcpy(m_PinInfo[0].name, "kosy");
    strcpy(m_PinInfo[0].path, "kosy");
    lineEdit_MSG->setText("I can't find the PIN information!!");
    qDebug("I can't find the PIN information!!");
  }
 
}

KeyPad::~KeyPad() { if(m_PinInfo) delete m_PinInfo; }

void KeyPad::slotKP_1() { addNumber(1); displayPIN(); }
void KeyPad::slotKP_2() { addNumber(2); displayPIN(); }
void KeyPad::slotKP_3() { addNumber(3); displayPIN(); }
void KeyPad::slotKP_4() { addNumber(4); displayPIN(); }
void KeyPad::slotKP_5() { addNumber(5); displayPIN(); }
void KeyPad::slotKP_6() { addNumber(6); displayPIN(); }
void KeyPad::slotKP_7() { addNumber(7); displayPIN(); }
void KeyPad::slotKP_8() { addNumber(8); displayPIN(); }
void KeyPad::slotKP_9() { addNumber(9); displayPIN(); }
void KeyPad::slotKP_0() { addNumber(0); displayPIN(); }

void KeyPad::slotCheckPIN() 
{
  int checkResult = FALSE;

  for(int i=0;i<m_PinInfo_Number;i++)
  {
    if(m_PinInfo[i].pin==m_PIN && m_PinInfo[i].pin_digit==m_PIN_Digit )
    {
      checkResult = TRUE;
      m_UserName = m_PinInfo[i].name;
      m_UserPath = m_PinInfo[i].path;

      break;
    }
  }

  if(checkResult == TRUE)
  {
    //m_iKeyPadStatus = KP_HIDE_RB;
    slotKP_CLS();
    lineEdit_MSG->setText("Your PIN is authorized.!!");
	signalAuthorized();
    // Originally, it is necessary here to hide the keypad and to show the main GUI
    // But, I put these tasks in communcation. 
    //accept();
  }
  else
  {
    lineEdit_MSG->setText("Your PIN is not authorized.!!");
    slotKP_CLS();
  }
}

void KeyPad::slotKP_BS()
{
  if( m_PIN_Digit == 0 || m_PIN_Digit == 1 )
  {
    m_PIN = 0;
    m_PIN_Digit = 0;
  }
  else
  {
    m_PIN = m_PIN/10;
    m_PIN_Digit--;
  }
  
  
#ifdef _DATA_DISPLAY
  QString msg=QString("%1, %2").arg(m_PIN).arg(m_PIN_Digit);
  lineEdit_MSG->setText(msg);
#endif
  displayPIN();
}


void KeyPad::slotKP_CLS() 
{
  m_PIN = 0;
  m_PIN_Digit=0;

#ifdef _DATA_DISPLAY
  QString msg=QString("%1, %2").arg(m_PIN).arg(m_PIN_Digit);
  lineEdit_MSG->setText(msg);
#endif

  displayPIN();
}

int KeyPad::addNumber(int no)
{
  QString msg;
  if(m_PIN_Digit >= MAXDIGIT)
  {
    msg=QString("Too many digits!! Maximum digit is %1").arg(m_PIN_Digit);
    //lineEdit_MSG->setText("Too many digits!!");
    lineEdit_MSG->setText(msg);
    return FALSE;
  }
  
  if(m_PIN_Digit == 0)
  {
    m_PIN = no;
    m_PIN_Digit = 1;
  }
  else
  {
    m_PIN = m_PIN * 10 + no;
    m_PIN_Digit++;
  }

#ifdef _DATA_DISPLAY
  msg=QString("%1, %2").arg(m_PIN).arg(m_PIN_Digit);
  lineEdit_MSG->setText(msg);
#endif

  return TRUE;
}

int KeyPad::displayPIN()
{
  QString strPIN="";
  for(int i=0;i<m_PIN_Digit;i++)
  {
    strPIN+=" *";
  }

  lineEdit_PIN->setText(strPIN);

  if(m_PIN_Digit == 1)
      lineEdit_MSG->setText("Please Enter Your PIN!!");
    

  return TRUE;
}

int KeyPad::getPIN()
{
  return m_PIN;
}
QString KeyPad::getUserName()
{
  return m_UserName;
}
int KeyPad::getStatus()
{
  return m_iKeyPadStatus;
}
void KeyPad::setStatus(int status)
{
  m_iKeyPadStatus = status;
}

void KeyPad::setEnabledAll(bool enabled)
{
  pushButton_KP0->setEnabled(enabled);
  pushButton_KP1->setEnabled(enabled);
  pushButton_KP2->setEnabled(enabled);
  pushButton_KP3->setEnabled(enabled);
  pushButton_KP4->setEnabled(enabled);
  pushButton_KP5->setEnabled(enabled);
  pushButton_KP6->setEnabled(enabled);
  pushButton_KP7->setEnabled(enabled);
  pushButton_KP8->setEnabled(enabled);
  pushButton_KP9->setEnabled(enabled);
  pushButton_KP_BS->setEnabled(enabled);
  pushButton_KP_CLS->setEnabled(enabled);
  ButtonOk->setEnabled(enabled);
  lineEdit_MSG->setEnabled(enabled);
  lineEdit_PIN->setEnabled(enabled);
  lineEdit_MSG->setText("Please Enter Your PIN!!");
  
}
