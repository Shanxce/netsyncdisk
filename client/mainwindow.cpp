#include "mainwindow.h"
#include "ui_mainwindow.h"
enum STATUS{Login_PasswdFail,Login_UserFail,Login_sucess,Register_fail,Register_sucess,
            Register_Unstrong,Bind_Sourcenoexist,Bind_fail,Connect_dely};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    communicator=new Communicator();
    CommunicatorThread = new QThread();
    Dialog_Login=new Login();
    Dialog_Register=new Register();

    CommunicatorThread->start();
    communicator->moveToThread(CommunicatorThread);
    connect(this, &MainWindow::Signal_Connect, communicator, &Communicator::Connect);

    emit Signal_Connect();


    connect(Dialog_Login,&Login::Sigal_Login,communicator,&Communicator::Login);
    connect(Dialog_Register,&Register::Sigal_Register,communicator,&Communicator::Register);
    connect(communicator,&Communicator::Signal_Reply_Login,Dialog_Login,&Login::Reply_Login);
    connect(communicator,&Communicator::Signal_Reply_Register,Dialog_Register,&Register::Reply_Register);

    //绑定目录的信号传递
    connect(this,&this->Signal_SendSetBind,communicator,&Communicator::SendSetBind);
    connect(communicator,&Communicator::Signal_setBind,this,&MainWindow::setBind);
    connect(communicator,&Communicator::Signal_Reply_SendSetBind,this,&this->Reply_SetBind);

    //绑定连接状态信号传递
    connect(communicator,&Communicator::Signal_Connect_Status,this,&MainWindow::deal_Connect_Status);

    //绑定日志文件传递
    connect(communicator,&Communicator::Signal_Log,this,&MainWindow::Log);



    connect(this,&MainWindow::Signal_Listen,communicator,&Communicator::Listen);
    //设置显示初始目录
    this->ShowShareDir();
}

void MainWindow::ShowShareDir()
{
    QString qssTV = "QTableWidget::item:hover{background-color:rgb(92,188,227,200)}"
                    "QTableWidget::item:selected{background-color:#1B89A1}"
                    "QHeaderView::section,QTableCornerButton:section{ \
           padding:3px; margin:0px; color:#DCDCDC;  border:1px solid #242424; \
   border-left-width:0px; border-right-width:1px; border-top-width:0px; border-bottom-width:1px; \
background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 #646464,stop:1 #525252); }"
                    "QTableWidget{background-color:white;border:none;}";
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁止编辑
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);    //行头自适应表格
    ui->tableWidget->horizontalHeader()->setFont(QFont("song", 12));

    ui->tableWidget->horizontalHeader()->setHighlightSections(false);
    QFont font = ui->tableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->tableWidget->horizontalHeader()->setFont(font);
    ui->tableWidget->setStyleSheet(qssTV);
    ui->tableWidget->horizontalHeader()->setHighlightSections(false); //点击表头时不对表头光亮
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    //所有单元格的字体  设置成一样
    ui->tableWidget->setFont(QFont("song", 12));
    ui->tableWidget->setRowCount(6);    //设置行数
    ui->tableWidget->setColumnCount(1); //设置列数
    ui->tableWidget->setWindowTitle("共享文件夹");
    QStringList header;

    header << "可选共享文件夹";
    ui->tableWidget->setHorizontalHeaderLabels(header);

    //去掉默认行号 可以用horizontalHeader() ->setVisible(false)隐藏横向表头
    QHeaderView *header1 = ui->tableWidget->verticalHeader();
    header1->setHidden(true);
    //设置单元格大小
    //设置默认行头宽度
    ui->tableWidget->horizontalHeader()->setMinimumHeight(40);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(80); //设置默认宽度
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(40);   //设置一行默认高度
    ui->tableWidget->setColumnWidth(1, 110);

    ui->tableWidget->setSortingEnabled(true); //启动排序

    for (int crowCount = 0; crowCount < 6; ++crowCount)
        ui->tableWidget->setItem(crowCount,0,new QTableWidgetItem(QString("ShareDir")+QString::number(crowCount)));

}
void MainWindow::setBind(QString Source,QString Remote,int Status)
{
    this->ifBind=Status;
    if(ifBind)
    {
        SourcePath=Source;
        RemotePath=Remote;
        ui->Button_PathConnect->setText("解绑");
        ui->Input_Remotepath->setText(RemotePath);
        ui->Input_Sourcepath->setText(SourcePath);
        ui->Input_Remotepath->setEnabled(false);
        ui->Input_Sourcepath->setEnabled(false);
        cout<<"Signal_Listen"<<endl;
        emit Signal_Listen();
    }
    else 
    {
        ui->Button_PathConnect->setText("绑定");
        ui->Input_Remotepath->setText("");
        ui->Input_Sourcepath->setText("");
        ui->Input_Remotepath->setEnabled(true);
        ui->Input_Sourcepath->setEnabled(true);
    }
}
void MainWindow::Log(QString logcontent)
{
    ui->Log->append(logcontent);
}

void MainWindow::Reply_SetBind(QString Source, QString Remote,int Status)
{
    switch (Status) {
    case 1:
        this->ifBind=1;
        this->SourcePath=Source;
        this->RemotePath=Remote;
        ui->Button_PathConnect->setText("解绑");
        ui->Input_Remotepath->setText(RemotePath);
        ui->Input_Sourcepath->setText(SourcePath);
        ui->Input_Remotepath->setEnabled(false);
        ui->Input_Sourcepath->setEnabled(false);
        ui->Label_reminder->setText("绑定成功!");
        emit Signal_Listen();
        break;
    case 0:
        this->ifBind=0;
        ui->Button_PathConnect->setText("绑定");
        ui->Input_Remotepath->setEnabled(true);
        ui->Input_Sourcepath->setEnabled(true);
        ui->Label_reminder->setText("解绑成功!");
        break;
    case Bind_Sourcenoexist:
        ui->Label_reminder->setText("本机目录无法绑定，请检查目录是否存在以及是否有权限!");
        break;
    case Bind_fail:
        ui->Label_reminder->setText("远端目录错误!");
        break;
    case Connect_dely:
        ui->Label_reminder->setText("网络异常！");
        break;
    default:
        break;
    }
}



void MainWindow::on_Button_PathConnect_clicked()
{
    int _ifBind=ui->Button_PathConnect->text()=="绑定"?1:0;
    QString _SourcePath=ui->Input_Sourcepath->text();
    QString _RemotePath=ui->Input_Remotepath->text();
    std::cout<<_SourcePath.toStdString()<<' '<<_RemotePath.toStdString()<<endl;
    DEBUG(_ifBind);

    Share_ifBind=0;
    emit Signal_SendSetBind(_SourcePath,_RemotePath,_ifBind);
}
void MainWindow::deal_Connect_Status(int Status)
{
    // 如果没有连接成功应该要重新请求连接
    if(Status==0)
    {   
        emit Signal_Connect();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


