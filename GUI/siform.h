#ifndef __SIFORMD___H
#define __SIFORMD___H



#include <iostream>
#include <qapplication.h>
//#include <q3textview.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <QMouseEvent>
#include <QPixmap>
#include <QKeyEvent>
#include <QtNetwork>

#include "CommonDS.h"
//#include "Communication.h"
//#include "Animation.h"
#include "keypad.h"

using namespace std;

extern stMA2UI_DATA Ma2UIdata;
extern stUI2MA_DATA UI2Madata;
extern unsigned short c_port_UI;
extern const char* c_addr_UI;

#define KP_SHOW			0
#define KP_HIDE_FAIL		1
#define KP_HIDE_RB		2
#define KP_HIDE_AUTHORIZED 	3

#include "ui_siform.h"

int checksumMA2UI(stMA2UI_DATA *ma2ui);
int checksumUI2MA(stUI2MA_DATA *ui2ma);

class SIForm : public QDialog, public Ui::SIForm
{
    Q_OBJECT

public:
    SIForm(QDialog *parent = 0);

};

//#endif




//===============================================================================================
//#define INFO_MESSAGE 
class ServerNForm : public SIForm 
{
    Q_OBJECT
public:
    ServerNForm(  Q_UINT16 port );
    //ServerNForm(  Q_UINT16, Q3Canvas&);
    ~ServerNForm();

	void writeLog(QString log_message, QString log_level); // Write out a log message to the HTTP log server

signals:
    void signalFinalClose();

private slots:
    void slotStart();
    void slotClose();
 
    void slotScaleDecrease();
    void slotScaleIncrease();

	void updateGUIFromMaster(int cnt);
    void slotGUInewConnect();
    void slotGUIupdateStopButtonImage();
    void slotGUIupdateConnectionButtonImage();
    void slotReadClientatGUI();
    void slotDataExchangeInit();
	void slotUpdateTimer();

    void slotTabChange();
    void slotCheckLogin();
    void slotSelectView();
    void slotDCNew();
    void slotDCStart(bool toggle);
    void slotDCDictate(bool toggle);

    void slotIPChanged(int index);
	void slotCamAngleChanged();
	void slotITPCheckBox(int check);
    void slotRightToolChanged(int index);
    void slotLeftToolChanged(int index);
	void slotKeyPadAuthorized();
	void updateMaster();
	void sendTCP(stUI2MA_DATA *data);
	void slotDisconnectMaster();

	// Get result of log action.
	void slotHttpRead(bool b);
	
	void closeEvent(QCloseEvent * event);
protected:
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);

    int EnableMainGUI(int flag);
    int ConnectionStatus(bool state);

    void dataExchangeLoop();
    void setPaletteBackgroundColor_ksy(QPushButton* pb, QColor c);
    void GUIupdateLCDNumber();


    QTcpSocket *newClient;	//ClientSocket *newClient
    int newClientFlag;

    QTcpServer *simServer;	
    
//    Q3Canvas& canvas;
//    CommCheckView *editor;
//    CommCheckSprite* animation;
    KeyPad* keypad;

    int m_ip_max_no;
    QString m_curr_directory;
    QString* ipname;
    unsigned char (*ip)[4];

    int m_logined;
    QString m_surgeon_name;
    QString m_surgeon_path;
    

    QTime m_time;
    QTimer m_timer;
    QPixmap pm_stop00, pm_stop01, pm_stop02, pm_stop03, pm_start;

	// HTTP connection to log server
	QHttp http;
};



#endif

