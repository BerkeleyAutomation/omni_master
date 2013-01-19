//////////////////////////////////////////
// 
// FILE: siform.cpp
// Author: Kosy
//
//   Modified 4/21/2006 by Hawkeye
//
//////////////////////////////////////////

#include <QtGui>
#include <QProcess>
#include "siform.h"

SIForm::SIForm(QDialog *parent)
    : QDialog(parent)
{
    setupUi(this);
}


ServerNForm::ServerNForm( Q_UINT16 port )
{
    ///// ** KeyPad Define for Authorization ** ///////////////////
    keypad = new KeyPad();
    keypad->show();

    ///// ** Initializing ETC ** //////////////////////////////////
	simServer = new QTcpServer(this);
	if(!simServer->listen(QHostAddress::Any, port))
	{
		qDebug("****** Unable to start the server ******");
		simServer->close();
	}
	else
	{
		qDebug("SimpleServer is initialized ");
	}
    newClientFlag = -1;

    infoText->append( tr("I made the Server. \nAnd Wait the Client Connection \n"));
    qDebug("I made the Server. \nAnd Wait the Client Connection ");
    RB_View_Endoscope->setChecked(1);//select endoscopic view as a default
    slotSelectView();

    cB_LeftTool->insertItem("Dissector"); 
    cB_LeftTool->insertItem("Cautery"); 
    cB_LeftTool->insertItem("Grasper"); 
    cB_LeftTool->insertItem("Clip Applier"); 
    cB_LeftTool->insertItem("Scissors"); 

    cB_RightTool->insertItem("Dissector"); 
    cB_RightTool->insertItem("Cautery"); 
    cB_RightTool->insertItem("Grasper"); 
    cB_RightTool->insertItem("Clip Applier"); 
    cB_RightTool->insertItem("Scissors"); 

    UI2Madata.flag01 &= (ALL_ONES - BASIC_LOGIN) ;  //m_logined = FALSE;
    m_surgeon_name = "";
    

    ///// ** Initializing IP Address ** //////////////////////////////////
    if(QFile::exists("./Configuration/IPSetting.inf") )
      m_curr_directory = "./";
    else if(QFile::exists("./GUI_Server/Configuration/IPSetting.inf") )
      m_curr_directory = "./GUI_Server/";
    else if(QFile::exists("../Configuration/IPSetting.inf") )
      m_curr_directory = "../";
    else
      m_curr_directory = "";


    if(m_curr_directory != "" )
    {
      FILE *fp = fopen(m_curr_directory+"Configuration/IPSetting.inf","r");

      //Later, modify using new.. ^^
      QString temp;
      char tmp[100];
      if(fp != NULL)
      {
        fscanf(fp,"%d\n", &m_ip_max_no);
        ipname = new QString[m_ip_max_no];
        ip = new unsigned char[m_ip_max_no][4];
        for(int i=0;i<m_ip_max_no;i++)
        {
          fscanf(fp,"%s %d %d %d %d",tmp,&(ip[i][0]),&(ip[i][1]),&(ip[i][2]),&(ip[i][3]));
          ipname[i]=tmp;
          temp = ipname[i]+" : "+QString("%1.%2.%3.%4").arg(ip[i][0]).arg(ip[i][1]).arg(ip[i][2]).arg(ip[i][3]);
          cB_IP->insertItem(temp); 
        }
      }
      
      QPixmap pm_logo;
      pm_logo.load(m_curr_directory+"Configuration/BioRobotics.bmp");
      logoButton->setPixmap(pm_logo);

      // choose the default ip address
      fscanf(fp,"%s",tmp);
      temp = tmp;
      for (int j=0;j<m_ip_max_no;j++)
      {
		if(QString::compare(ipname[j],temp)==0) 
		{
			cB_IP->setCurrentItem(j);
			UI2Madata.UDPaddr = (ip[j][0]<<24) + (ip[j][1]<<16) + (ip[j][2]<<8) + ip[j][3];
			break;
		}
      }
      fclose(fp);
    }
	else
	{
		cB_IP->insertItem("There is no file");
		UI2Madata.UDPaddr = (192<<24) + (168<<16) + (0<<8) + 100 ; // Buttercup
	}

	//  ** Upload the images of the Stop/Start Buttons ** //
	pm_stop03.load(m_curr_directory+"Pixmaps/Paused.bmp");
	pm_stop02.load(m_curr_directory+"Pixmaps/Operating.bmp");
	pm_stop01.load(m_curr_directory+"Pixmaps/Initializing.bmp");
	pm_stop00.load(m_curr_directory+"Pixmaps/EStop.bmp");
	pm_start.load(m_curr_directory+"Pixmaps/Start.bmp");

	///// ** Connecting signals to slots** //////////////////////
	// button-related slot(function)s
	connect( startButton,		SIGNAL(clicked()),		this, SLOT(slotStart()) );
	connect( okButton,			SIGNAL(clicked()),		this, SLOT(slotClose()) );
	connect( A_startButton,		SIGNAL(clicked()),		this, SLOT(slotStart()) );
	connect( A_okButton,		SIGNAL(clicked()),		this, SLOT(slotClose()) );
	connect( this,				SIGNAL(closeEvent()),	this, SLOT(slotClose()) );
	connect( scaleDecreaseButton, SIGNAL(clicked()),	this, SLOT(slotScaleDecrease()) );
	connect( scaleIncreaseButton, SIGNAL(clicked()),	this, SLOT(slotScaleIncrease()) );
	connect( &m_timer,			SIGNAL(timeout()),		this, SLOT(slotUpdateTimer()) );
	connect( tabWidget,			SIGNAL(currentChanged(QWidget*)), this, SLOT(slotTabChange()) );

	connect(loginButton, SIGNAL(clicked()), this, SLOT(slotCheckLogin()));

	connect(DC_dictateButton,SIGNAL(toggled(bool)),this, SLOT(slotDCDictate(bool)));
	connect(DC_newButton,    SIGNAL( clicked() ),  this, SLOT(slotDCNew()));
	connect(DC_saveButton,   SIGNAL(toggled(bool)),this, SLOT(slotDCStart(bool)));

	connect(RB_View_Endoscope, SIGNAL(clicked()), this, SLOT(slotSelectView()));
	connect( RB_View_OR,       SIGNAL(clicked()), this, SLOT(slotSelectView()));
	connect( RB_View_OR2,      SIGNAL(clicked()), this, SLOT(slotSelectView()));

	connect( cB_IP, SIGNAL(activated(int)), this, SLOT(slotIPChanged(int)) );
	connect( cB_RightTool, SIGNAL(activated(int)), this, SLOT(slotRightToolChanged(int)) );
	connect( cB_LeftTool,  SIGNAL(activated(int)), this, SLOT(slotLeftToolChanged(int)) );
	connect( camAngle1,  SIGNAL(valueChanged(double)), this, SLOT(slotCamAngleChanged()) );
	connect( camAngle2,  SIGNAL(valueChanged(double)), this, SLOT(slotCamAngleChanged()) );
	connect( camAngle3,  SIGNAL(valueChanged(double)), this, SLOT(slotCamAngleChanged()) );
	connect( ITPCheck,  SIGNAL(stateChanged(int)), this, SLOT(slotITPCheckBox(int)) );


	// communication-realted slots
	connect(simServer, SIGNAL(newConnection()), SLOT(slotGUInewConnect()) );
	connect(keypad, SIGNAL(signalAuthorized()), SLOT(slotKeyPadAuthorized()) );
	
	// initialize HTTP connection to log server
	connect(&http,SIGNAL(done(bool)), this, SLOT(slotHttpRead(bool)));
	http.setHost("brl.ee.washington.edu");

    slotKeyPadAuthorized();
}

ServerNForm::~ServerNForm()
{
    if(keypad)
        delete keypad;
}


void ServerNForm::keyPressEvent(QKeyEvent *e) 
{
	int key = e->key();
	switch(key)
	{
		case Qt::Key_A: // Left Down
		{
			if(UI2Madata.flag01&BASIC_START) UI2Madata.flag01 |= FPEDAL_LEFT;
			break;
		}
		case Qt::Key_S: // Middle Down
        {
			if(UI2Madata.flag01&BASIC_START) UI2Madata.flag01 |= FPEDAL_MIDDLE;
			break;
		}
		case Qt::Key_D: // Right Down
		{
			if(UI2Madata.flag01&BASIC_START) {
				UI2Madata.flag01 |= FPEDAL_RIGHT;
				writeLog("Pedal down", "3");
				scaleDecreaseButton->setEnabled(FALSE);  // Disallow scale change while running
				scaleIncreaseButton->setEnabled(FALSE);   //   ""
				camAngle1->setEnabled(FALSE);
				camAngle2->setEnabled(FALSE);
				camAngle3->setEnabled(FALSE);
			}
			break;
		}
		case Qt::Key_Q: // Left Up
		{
			UI2Madata.flag01 &= (ALL_ONES - FPEDAL_LEFT);
		break;
		}
		case Qt::Key_W: // Middle Up
		{
			UI2Madata.flag01 &= (ALL_ONES - FPEDAL_MIDDLE);
			break;
		}
		case Qt::Key_E: // Right Up
		{
			if(UI2Madata.flag01&BASIC_START &&
				UI2Madata.flag01&FPEDAL_RIGHT ) {
				writeLog("Pedal up", "4");
				scaleDecreaseButton->setEnabled(TRUE);  // Disallow scale change while running
				scaleIncreaseButton->setEnabled(TRUE);   //   ""
				camAngle1->setEnabled(TRUE);
				camAngle2->setEnabled(TRUE);
				camAngle3->setEnabled(TRUE);
			}
			UI2Madata.flag01 &= (ALL_ONES - FPEDAL_RIGHT);
			break;
		}
        case Qt::Key_Space:
        {
            if(UI2Madata.flag01&BASIC_START) {
                if (UI2Madata.flag01&FPEDAL_RIGHT && false) {
                    writeLog("Pedal up", "4");
                    scaleDecreaseButton->setEnabled(TRUE);  // Disallow scale change while running
                    scaleIncreaseButton->setEnabled(TRUE);   //   ""
                    camAngle1->setEnabled(TRUE);
                    camAngle2->setEnabled(TRUE);
                    camAngle3->setEnabled(TRUE);
                    UI2Madata.flag01 &= (ALL_ONES - FPEDAL_RIGHT);
                } else {
                    UI2Madata.flag01 |= FPEDAL_RIGHT;
                    writeLog("Pedal down", "3");
                    scaleDecreaseButton->setEnabled(FALSE);  // Disallow scale change while running
                    scaleIncreaseButton->setEnabled(FALSE);   //   ""
                    camAngle1->setEnabled(FALSE);
                    camAngle2->setEnabled(FALSE);
                    camAngle3->setEnabled(FALSE);
                }
            }
            break;
        }
		default:
		{
			break;
		}
	} // switch
    SIForm::keyPressEvent(e);
	updateMaster();
}

void ServerNForm::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    switch(key)
    {
    case Qt::Key_Space:
    {
        if(UI2Madata.flag01&BASIC_START) {
            if (UI2Madata.flag01&FPEDAL_RIGHT || true) {
                writeLog("Pedal up", "4");
                scaleDecreaseButton->setEnabled(TRUE);  // Disallow scale change while running
                scaleIncreaseButton->setEnabled(TRUE);   //   ""
                camAngle1->setEnabled(TRUE);
                camAngle2->setEnabled(TRUE);
                camAngle3->setEnabled(TRUE);
                UI2Madata.flag01 &= (ALL_ONES - FPEDAL_RIGHT);
            } else {
                UI2Madata.flag01 |= FPEDAL_RIGHT;
                writeLog("Pedal down", "3");
                scaleDecreaseButton->setEnabled(FALSE);  // Disallow scale change while running
                scaleIncreaseButton->setEnabled(FALSE);   //   ""
                camAngle1->setEnabled(FALSE);
                camAngle2->setEnabled(FALSE);
                camAngle3->setEnabled(FALSE);
            }
        }
        break;
    }
    }
    SIForm::keyReleaseEvent(e);
    updateMaster();
}

void ServerNForm::slotRightToolChanged(int index)
{
  tabWidget->setFocus();

  //if(1)
  if(QMessageBox::information(0,"Right Tool Changing","Do you want to change the right hand tool \'"+cB_RightTool->text(index)+" \'??", "OK", "Cancel", QString::null, 0,1)  == 0  ) 
  {
    infoText->append( tr("Right Tool Changed %1 \n").arg(index) );
    UI2Madata.flag01 &= (ALL_ONES - TOOL_RIGHT);
    UI2Madata.flag01 |= (index << MOVEBIT_TOOL_RIGHT);
  }
  else
  {
    int before_tool = 0x00f&((UI2Madata.flag01&TOOL_RIGHT)>>MOVEBIT_TOOL_RIGHT);
    cB_RightTool->setCurrentItem(before_tool);
  }
	updateMaster();
}

void ServerNForm::slotLeftToolChanged(int index)
{
  tabWidget->setFocus();

  //if(1)
  if(QMessageBox::information(0,"Left Tool Changing","Do you want to change the left hand tool into \'"+cB_LeftTool->text(index)+"\' ??", "OK", "Cancel", QString::null, 0,1)  == 0  ) 
  {
    infoText->append( tr("Left Tool Changed %1 \n").arg(cB_LeftTool->text(index)) );
    UI2Madata.flag01 &= (ALL_ONES - TOOL_LEFT);
    UI2Madata.flag01 |= (index << MOVEBIT_TOOL_LEFT);
  }
  else
  {
    int before_tool = 0x00f&( (UI2Madata.flag01&TOOL_LEFT)>>MOVEBIT_TOOL_LEFT);
    cB_LeftTool->setCurrentItem(before_tool);
  }
	updateMaster();
}

void ServerNForm::slotIPChanged(int index)
{
  tabWidget->setFocus();

  infoText->append( tr("IP Changed %1 \n").arg(index) );
  UI2Madata.UDPaddr = (ip[index][0]<<24) + (ip[index][1]<<16) + (ip[index][2]<<8) + ip[index][3];

  updateMaster();
}

void ServerNForm::slotSelectView()
{
	tabWidget->setFocus();
	//VIEW_ENDOSCOPE _ORVIEW _ORVIEW2
	if(RB_View_Endoscope->isChecked()) {
		UI2Madata.flag01 &= (ALL_ONES - VIEW_FLAG);
		UI2Madata.flag01 |= VIEW_ENDOSCOPE;
	} else if(RB_View_OR->isChecked()) {
		UI2Madata.flag01 &= (ALL_ONES - VIEW_FLAG);
		UI2Madata.flag01 |= VIEW_ORVIEW;
	} else if(RB_View_OR2->isChecked()) {
		UI2Madata.flag01 &= (ALL_ONES - VIEW_FLAG);
		UI2Madata.flag01 |= VIEW_ORVIEW2;
	} else {
		UI2Madata.flag01 &= (ALL_ONES - VIEW_FLAG);
	}

	updateMaster();
}


void ServerNForm::slotCamAngleChanged()
{
//  tabWidget->setFocus();

  infoText->append( tr("Setting camera angle (%1,%2,%3).\n").arg(camAngle1->value()).arg(camAngle2->value()).arg(camAngle3->value()) );
  UI2Madata.camAngle[0] = camAngle1->value();
  UI2Madata.camAngle[1] = camAngle2->value();
  UI2Madata.camAngle[2] = camAngle3->value();

  updateMaster();
}

void ServerNForm::slotITPCheckBox(int check)
{
  infoText->append( tr("Setting ITP check to %1.\n").arg(check) );
  UI2Madata.useITP = check;

  updateMaster();
}

void ServerNForm::slotCheckLogin()
{
	tabWidget->setFocus();
	if( !(UI2Madata.flag01&BASIC_LOGIN) ) {
		keypad->setStatus(KP_SHOW);
		keypad->show();	//showing the keypad
		hide();		//hiding the main GUI
	}
	else //(m_logined == TRUE) // Surgeon wants to logoff
	{
		if(QMessageBox::information(0,"Login Button","Do you want to log-off??", "OK", "Cancel", QString::null, 0,1)  == 0  ) // asking to logoff really.
		{
			EnableMainGUI(false);
			writeLog("Surgeon logged out","2");
		}
	}
	// No updateMaster() call, since UI2Ma ain't changed.
}
void ServerNForm::slotDCNew()
{
	tabWidget->setFocus();

	static int pressed = 0;
	if(pressed) { pressed = 0; UI2Madata.flag01 &= (ALL_ONES - DC_NEWFILE); }
	else        { pressed = 1; UI2Madata.flag01 |= DC_NEWFILE;              }
	updateMaster();
}

void ServerNForm::slotDCStart(bool toggle)
{
	tabWidget->setFocus();
	if(toggle) {
		UI2Madata.flag01|= DC_STARTSTOP;	// starting = 1, stopped = 0
		DC_saveButton->setText("I'm Saving \n Press to Stop");
	} else {
		UI2Madata.flag01&= (ALL_ONES-DC_STARTSTOP);	// starting = 1, stopped = 0
		DC_saveButton->setText("Press to Save");
	}
	updateMaster();
}
void ServerNForm::slotDCDictate(bool toggle)
{
	tabWidget->setFocus();
	if(toggle) {
		UI2Madata.flag01|= DC_DICTATE;	// starting = 1, stopped = 0
		DC_dictateButton->setText("I'm Dictating\nPress to Stop");
	} else {
		UI2Madata.flag01&= (ALL_ONES-DC_DICTATE);	// starting = 1, stopped = 0
		DC_dictateButton->setText("Press to Dictate");
	}
	updateMaster();
}


void ServerNForm::mousePressEvent(QMouseEvent* e)
{
	tabWidget->setFocus();
	if(e->button() == Qt::RightButton) {
		UI2Madata.flag01 |= FPEDAL_RIGHT;
		infoText->append( tr("A Right Button in a Mouse is Clicked\n") );
		updateMaster();
	}
} 

void ServerNForm::mouseReleaseEvent(QMouseEvent* e)
{
	tabWidget->setFocus();
	if(e->button() == Qt::RightButton) {
		UI2Madata.flag01 &= (ALL_ONES - FPEDAL_RIGHT);
		infoText->append( tr("A Right Button in a Mouse is Released\n") );
		updateMaster();
	}
}
void ServerNForm::slotTabChange(){}

#define SHOW_BUTTON_TEXT false

void ServerNForm::slotStart()
{
	tabWidget->setFocus();

    if (SHOW_BUTTON_TEXT) startButton->setText("Waiting the connection of the Omni");

	if(!(UI2Madata.flag01&BASIC_START)) { 	// To convert cur state(=STOP) to START
		UI2Madata.flag01 |= BASIC_START;
		infoText->append( tr("Start button is clicked\n") );
				//startButton's image is chaged in "slotGUIupdateStopButtonImage()"
        A_startButton->setText("Press to STOP");

//		m_time.setHMS(0,0,0,0);
		m_timer.start(1000);
		m_time.start(); // I think this fn. should move after 

		okButton->setEnabled(FALSE);
		A_okButton->setEnabled(FALSE);
		loginButton->setEnabled(FALSE);
	} else if(UI2Madata.flag01&BASIC_START) {//To convert into STOP
		UI2Madata.flag01 &= (ALL_ONES-BASIC_START);
		infoText->append( tr("Stop button is clicked\n") );
		//startButton's image is chaged in "slotGUIupdateStopButtonImage()"
        A_startButton->setText("Press to START");
		m_timer.stop();
		//    animation->stop();
		scaleDecreaseButton->setEnabled(TRUE);  // Disallow scale change while running
		scaleIncreaseButton->setEnabled(TRUE);   //  ""
		camAngle1->setEnabled(TRUE);
		camAngle2->setEnabled(TRUE);
		camAngle3->setEnabled(TRUE);
		okButton->setEnabled(TRUE);
		A_okButton->setEnabled(TRUE);
		loginButton->setEnabled(TRUE);
	}
	UI2Madata.flag01 &= (ALL_ONES - FPEDAL_RIGHT - FPEDAL_MIDDLE - FPEDAL_LEFT);
	updateMaster();
}

// Run the timer.
void ServerNForm::slotUpdateTimer()
{
	int hr,min,sec,msec=m_time.elapsed();
	hr = msec/3600000;
	min = (msec%3600000)/60000;
	sec = (msec%60000)/1000;
	lineEdit_Time->setText(QString("Time : %1:%2:%3").arg(hr,2).arg(min,2).arg(sec,2));
}

void ServerNForm::slotClose()
{
	tabWidget->setFocus();
	if ( QMessageBox::information(0,"Close Button","Do you want to close this program??", 
				"OK", 
				"Cancel", 
				QString::null, 0,1) == 0 ) 
	{ // asking to logoff really.
		UI2Madata.flag01 &= (ALL_ONES - BASIC_PROGRAM);
		QMessageBox::information(0,"Closing Confirm","I sent the closing message to omni. The GUI will be closed.", "OK", QString::null,QString::null, 0,1);
		//emit signalFinalClose();
		this->close();
		//reject();
	}

	updateMaster();
}

void ServerNForm::slotScaleDecrease() 
{
	tabWidget->setFocus();
	UI2Madata.scale -= 5;
	if(UI2Madata.scale < 0)
		UI2Madata.scale = 0;
	scaleLineEdit->setText(QString("%1").arg((double)UI2Madata.scale/(double)SCALE_MAX));

	updateMaster();
}

void ServerNForm::slotScaleIncrease() 
{
	tabWidget->setFocus();
	UI2Madata.scale += 5;
	if(UI2Madata.scale > SCALE_MAX)
	UI2Madata.scale = SCALE_MAX;
	scaleLineEdit->setText(QString("%1").arg((double)UI2Madata.scale/(double)SCALE_MAX));

	updateMaster();
}

int ServerNForm::EnableMainGUI(int flag) 
{
	if(flag == true) {
		UI2Madata.flag01 |= BASIC_LOGIN ;  //m_logined = TRUE;
		m_surgeon_name = keypad->getUserName();
		loginButton->setText(m_surgeon_name + " logged-in \n Press to log out" );

		cB_RightTool->setEnabled(TRUE);
		cB_LeftTool->setEnabled(TRUE);
		startButton->setEnabled(TRUE);
		RB_View_OR->setEnabled(TRUE);
		RB_View_OR2->setEnabled(TRUE);
		RB_View_Endoscope->setEnabled(TRUE);
		DC_saveButton->setEnabled(TRUE);
		DC_newButton->setEnabled(TRUE);
		DC_dictateButton->setEnabled(TRUE);
		A_startButton->setEnabled(TRUE);
		rB_CM_PP->setEnabled(TRUE);
		rB_CM_1D->setEnabled(TRUE);
		cB_IP->setEnabled(TRUE);
		scaleDecreaseButton->setEnabled(TRUE);
		scaleIncreaseButton->setEnabled(TRUE);
		camAngle1->setEnabled(TRUE);
		camAngle2->setEnabled(TRUE);
		camAngle3->setEnabled(TRUE);
		connectionButton->setEnabled(TRUE);
	} else { //(flag == false)
		UI2Madata.flag01 &= (ALL_ONES - BASIC_LOGIN) ;  //m_logined = FALSE;
		m_surgeon_name = "";
		loginButton->setText("None logged in \n Press to login" );

		cB_RightTool->setEnabled(FALSE);
		cB_LeftTool->setEnabled(FALSE);
		startButton->setEnabled(FALSE);
		RB_View_OR->setEnabled(FALSE);
		RB_View_OR2->setEnabled(FALSE);
		RB_View_Endoscope->setEnabled(FALSE);
		DC_saveButton->setEnabled(FALSE);
		DC_newButton->setEnabled(FALSE);
		DC_dictateButton->setEnabled(FALSE);
		A_startButton->setEnabled(FALSE);
		rB_CM_PP->setEnabled(FALSE);
		rB_CM_1D->setEnabled(FALSE);
		cB_IP->setEnabled(FALSE);
		scaleDecreaseButton->setEnabled(FALSE);
		scaleIncreaseButton->setEnabled(FALSE);
		camAngle1->setEnabled(FALSE);
		camAngle2->setEnabled(FALSE);
		camAngle3->setEnabled(FALSE);
		connectionButton->setEnabled(FALSE);
	}

	updateMaster();
	return (flag);
}


// Update GUI with info from master.
void ServerNForm::updateGUIFromMaster(int cnt)
{
	// Change the button's color according to the runlevel
	// Output proper runlevel 
	//if (cnt % 10 == 1) {
		slotGUIupdateStopButtonImage();
		slotGUIupdateConnectionButtonImage();
		GUIupdateLCDNumber();
	//}
	// Output some information
	if( Ma2UIdata.tick % 1000 == 1 || TRUE) {
		//infoText->append( tr("rd : t:%1 x1:%2 y1:%3 z1:%4 \tx2:%5 y2:%6 z2:%7 b:%8").arg(Ma2UIdata.tick).arg(Ma2UIdata.delx[0]).arg(Ma2UIdata.dely[0]).arg(Ma2UIdata.delz[0]).arg(Ma2UIdata.delx[1]).arg(Ma2UIdata.dely[1]).arg(Ma2UIdata.delz[1]).arg(Ma2UIdata.runlevel));
	}
	// *** Timer Update *** //
/*	if( cnt % 100 == 1 ){
		int hr,min,sec,msec=m_time.elapsed();
		hr = msec/3600000;
		min = (msec%3600000)/60000;
		sec = (msec%60000)/1000;
		lineEdit_Time->setText(QString("Time : %1:%2:%3").arg(hr,2).arg(min,2).arg(sec,2));
	}*/
}

// Updates the runlevel display in the GUI
void ServerNForm::GUIupdateLCDNumber()
{
	//lCDNumber2->setNumDigits(Ma2UIdata.runlevel);
	lCDNumber2->display(Ma2UIdata.runlevel);
}

void ServerNForm::slotKeyPadAuthorized()
{
	show();		//show the main GUI
	keypad->hide();  //hide the keypad
	keypad->setStatus(KP_HIDE_AUTHORIZED);
	EnableMainGUI(true);
	writeLog("Surgeon logged in.","1");
}

// This function is a slot activated by clientsocket when the image change is required.
void ServerNForm::slotGUIupdateStopButtonImage()
{
	QColor startColor(192,192,192);
	QColor stopColor_rl03(0,255,0);   // button up
	QColor stopColor_rl02(0,0,255);   // button down
	QColor stopColor_rl01(255,255,0); // Initializing
	QColor stopColor_rl00(255,0,0);   // Emergency

	static int prv_runlevel=3, prv_start=TRUE;
	if( (prv_start) && !(UI2Madata.flag01&BASIC_START) ) {
		startButton->setPixmap(pm_start);
		setPaletteBackgroundColor_ksy(A_startButton, startColor);
	} else if( !(prv_start) && (UI2Madata.flag01&BASIC_START) )	{
		if( Ma2UIdata.runlevel == 0) {
				startButton->setPixmap(pm_stop00);
				setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl00);
		} else if( Ma2UIdata.runlevel == 1) {
				startButton->setPixmap(pm_stop01);
				setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl01);
		} else if( Ma2UIdata.runlevel == 2) {
				startButton->setPixmap(pm_stop02);
				setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl02);
		} else if( Ma2UIdata.runlevel == 3) {
				startButton->setPixmap(pm_stop03);
				setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl03);
		}
	} else if( (prv_start) && (UI2Madata.flag01&BASIC_START) ) {
		if(prv_runlevel != 0 && Ma2UIdata.runlevel == 0) {
			startButton->setPixmap(pm_stop00);
			setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl00);
		} else if(prv_runlevel != 1 && Ma2UIdata.runlevel == 1) {
			startButton->setPixmap(pm_stop01);
			setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl01);
		} else if(prv_runlevel != 2 && Ma2UIdata.runlevel == 2) {
			startButton->setPixmap(pm_stop02);
			setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl02);
		}	else if(prv_runlevel != 3 && Ma2UIdata.runlevel == 3) {
			startButton->setPixmap(pm_stop03);
			setPaletteBackgroundColor_ksy(A_startButton, stopColor_rl03);
		}
	}
	prv_start = UI2Madata.flag01&BASIC_START;
	prv_runlevel = Ma2UIdata.runlevel;
}

// Update color of a button to show the TCP status
void ServerNForm::slotGUIupdateConnectionButtonImage() 
{
	QColor GoodColor(0,255,255);
	QColor FailColor(255,0,255);   // button up
        if(Ma2UIdata.runlevel == 0) { //emergency state
		setPaletteBackgroundColor_ksy(connectionButton, FailColor);
		connectionButton->setText("Connection Failed");
	} else { //
		setPaletteBackgroundColor_ksy(connectionButton, GoodColor);
		connectionButton->setText("Good Connection");
	}
}

void ServerNForm::setPaletteBackgroundColor_ksy(QPushButton* pb, QColor c) {
	QPalette palette;
	palette.setColor(pb->backgroundRole(), c);
	pb->setPalette(palette);
}



///////////////////////////////////////////////////////////////////////
////////////////// BEGIN: NETWORKING FUNCTIONS ////////////////////////
///////////////////////////////////////////////////////////////////////



// This function is a slot activated by server when it got a new connection
void ServerNForm::slotGUInewConnect() 
{
	newClient = simServer->nextPendingConnection();
	newClientFlag = 0;
	infoText->append( tr("New connection \n"));
	qDebug("New connection in main.cpp");
	keypad->setEnabledAll(TRUE);
	connect(newClient, SIGNAL(readyRead()), SLOT(slotReadClientatGUI()) );
	//    connect(newClient, SIGNAL(disconnected()), newClient, SLOT(deleteLater()));
	connect(newClient, SIGNAL(disconnected()), newClient, SLOT(slotDisconnectMaster()));
}

// disconnected from master (never seems to get activated....)
void ServerNForm::slotDisconnectMaster()
{
	infoText->append( tr("Lost connection to master. \n"));
	newClientFlag = -1;
	newClient->deleteLater();
}

// Read data sent from the Omni.
void ServerNForm::slotReadClientatGUI() 
{
	if (newClientFlag < 0) {
		qDebug("ERROR: read attempt, but newClient not inited.\n");
	} else if (newClientFlag == 0) {   // The first packet is an init packet 
			cout << "calling init\n";
	newClientFlag++;
	slotDataExchangeInit();
	} else { // Subsequent packets are dataExchange packets
		dataExchangeLoop();
	}
}

// START: slotDataExchangeInit()
// This function is called when the "readyRead" event happens at first time. 
// SUI recives master state and esnds initial SUI state to master.
void ServerNForm::slotDataExchangeInit() 
{
	newClient->read((char *)&Ma2UIdata, sizeof(Ma2UIdata));
	qDebug("first receiveing: %d %d %d %d %d %d", Ma2UIdata.delx[0], Ma2UIdata.dely[0], Ma2UIdata.delz[0], Ma2UIdata.runlevel, (int)Ma2UIdata.tick, Ma2UIdata.checksum);

	if( Ma2UIdata.checksum == checksumMA2UI(&Ma2UIdata)) {
		UI2Madata.flag01 |= BASIC_PROGRAM;
		UI2Madata.flag01 &= (ALL_ONES-BASIC_START);
		UI2Madata.tick = 0;
		UI2Madata.scale = SCALE_INIT_VALUE; // will be divided by 100
		UI2Madata.checksum = checksumUI2MA(&UI2Madata);

		// if not already logged in, show login screen.	
		if( keypad->getStatus() != KP_HIDE_AUTHORIZED )
			keypad->show();
	} else {
		infoText->append( tr("Data was received strangely\n" ) );
		qDebug("Data was received strangely")  ;
		UI2Madata.flag01 &= (ALL_ONES-BASIC_PROGRAM);
		UI2Madata.flag01 &= (ALL_ONES-BASIC_START);
		UI2Madata.tick = 0;
		UI2Madata.scale = 0; // will be divided by 100
		UI2Madata.checksum = checksumUI2MA(&UI2Madata);
	}
	qDebug("sending init packet: s:%d, f:%d, a:%d, t:%d, check:%d", UI2Madata.scale, UI2Madata.flag01, UI2Madata.UDPaddr, (int)UI2Madata.tick, UI2Madata.checksum);
	infoText->append( tr("Initial data was sended %1\n").arg(UI2Madata.flag01));
	scaleLineEdit->setText(QString("%1").arg((double)UI2Madata.scale/(double)SCALE_MAX));

	updateMaster(); // Important: reply to init packet with initial state.
}//END: slotDataExchangeInit()

void ServerNForm::dataExchangeLoop() 
{
	static stMA2UI_DATA ma2ui;
	static int iCount;
	iCount ++;

	newClient->read((char *)&ma2ui, sizeof(ma2ui));   	// Get the data.
	if( ma2ui.checksum == checksumMA2UI(&ma2ui) ) { 	  // check data validity
		memcpy(&Ma2UIdata, &ma2ui, sizeof(ma2ui));        // store the valid data
		UI2Madata.tick = Ma2UIdata.tick;
		// update the gui.
		updateGUIFromMaster(iCount);
	} else {                                            // Data was bad!
		infoText->append( tr("Data was received strangely\n" ) );
	}
	return;
} // END : dataExchangeLoop()

////// CHECKSUMS ///////
int checksumMA2UI(stMA2UI_DATA *ma2ui) 
{
	int chk=0;
	chk = ma2ui->delx[0] + ma2ui->dely[0] + ma2ui->delz[0] + ma2ui->runlevel + (int)ma2ui->tick;
	return chk;
}

int checksumUI2MA(stUI2MA_DATA *ui2ma) 
{
	int chk=0;
	chk = ui2ma->scale + ui2ma->flag01 + ui2ma->UDPaddr + (int)ui2ma->tick;
	return chk;
}

// Send current SUI state to master.
void ServerNForm::updateMaster()
{
	stUI2MA_DATA data;
	memcpy(&data, &UI2Madata, sizeof(stUI2MA_DATA));
	if( keypad->getStatus() != KP_HIDE_AUTHORIZED ){
		// set zeros
		data.scale = 0;
		data.tick = 0;
	}
	data.checksum = checksumUI2MA(&data);
	sendTCP(&data);
	return;
}

void ServerNForm::sendTCP(stUI2MA_DATA *data)
{
	qint64 ret=0;
	if ( newClientFlag > 0){
		try{
			ret = newClient->write((char *)data, sizeof(stUI2MA_DATA));
		} catch (int SE){
			qDebug("Socket Error(%d) whilest sending_TCP().",SE);
			return;
		}
		qDebug("sendTCP okay!");	
	} else {
		qDebug("no TCP connex.  Cannot send.");	
	}

	return;
}
///////////////////////////////////////////////////////////////////////
/////////////////// END: NETWORKING FUNCTIONS /////////////////////////
///////////////////////////////////////////////////////////////////////


// writeLog()
//
//  Write data to the HTTP log server
//  Log messages:
//    1 : logged in
//    2 : logged out
//    3 : pedal dn
//    4 : pedal up
//    5 : software terminating normally
//    6 : software terminating abnormally
//    7 : Software error (i.e. Omni connection lost)
//
void ServerNForm::writeLog(QString log_message, QString log_level)
{
	infoText->append( "Wrote to log: " + log_message +"." );

	QTime t; t.start();
	QDate d = QDate::currentDate();
	QString surgeon_name(m_surgeon_name);
	QString time=t.toString("hh:mm:ss.zzz");
	QString date;
	date=d.toString("dd-MMMM-yyyy");
	QString logline(date + "\t"
		 + time + "\t" 
		 + surgeon_name + "\t" 
		 + log_level + "\t" 
		 + log_message );

	QUrl::encode(surgeon_name);
	QUrl::encode(log_message);
	QUrl::encode(time);

	// format log message: time, user, log_level, message
	QString B( "timestamp=" + time + 
		"&user=" + surgeon_name +
		"&log_level=" + log_level +
		"&log_message=" + log_message );

	QHttpRequestHeader header("POST", "/log_server/index.php");
	header.setValue("Host", "brl.ee.washington.edu");
	header.setContentType("application/x-www-form-urlencoded");
	http.setHost("brl.ee.washington.edu");
	http.request(header, B.utf8());

	// TODO:  Also write to a text file here
	QStringList lines;
	lines += logline.toLatin1(); 
	QFile file( "logfile.txt" );
    if ( file.open( IO_WriteOnly | IO_Append ) ) {
        QTextStream stream( &file );
        for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it )
			stream << *it << "\n";
		file.close();
    }

}
void ServerNForm::slotHttpRead(bool b)
{
	QString result(http.readAll());
	if (b) { // error!
		infoText->append("HTTP error! " + http.errorString() );
	} else { // no error, completed request
//		infoText->append("HTTP response:" + result);
	} 
}

void ServerNForm::closeEvent(QCloseEvent * event)
{
	static int close_really;

	if ( close_really==0 && (UI2Madata.flag01&BASIC_LOGIN) ){
		writeLog("Surgeon logged out","2");
		disconnect(&http,SIGNAL(done(bool)), this, SLOT(slotHttpRead(bool)));
		connect(&http,SIGNAL(done(bool)), this, SLOT(close()));
		event->ignore();
		close_really=1;
	} else {
			event->accept();
	}
}
