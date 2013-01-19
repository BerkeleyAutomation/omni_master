#include <QApplication>
#include <QProcess>

#include <iostream>

#include "siform.h"

int main(int argc, char *argv[])
{
    QProcess* omni_client = new QProcess();
    std::cout << "starting omni client" << std::endl;
    omni_client->start("\"C:\\omni_master\\client\\Release\\Omni_Client.exe\"");
    omni_client->waitForStarted();
    std::cout << "started " << omni_client->pid() << std::endl;

    //connect(this,SIGNAL(signalFinalClose()),omni_client,SLOT(kill()));

    QApplication app(argc, argv);

//    Q3Canvas canvas(350,25);
//    canvas.setAdvancePeriod(250);
//    ServerNForm serverF( c_port_UI, canvas );
    ServerNForm serverF( c_port_UI);
//    app.setMainWidget( &serverF );
    /////////////////////////////
    // for entering 'sudo password', the GUI is shown after first connection.
//    serverF.show();



    int app_ret = app.exec();

    std::cout << "Closing omni client" << std::endl;
    omni_client->kill();
    omni_client->waitForFinished();
    std::cout << "Done" << std::endl;

    return app_ret;
}
