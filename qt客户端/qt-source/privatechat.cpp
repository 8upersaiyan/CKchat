#include "privatechat.h"
#include "ui_privatechat.h"
#include <QCloseEvent>

PrivateChat::PrivateChat(QTcpSocket *s, QString u, QString f, Chatlist *c, QList<ChatWidgetInfo> *l, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    socket = s;
    userName = u;
    friendName = f;
    mainWidget = c;
    chatWidgetList = l;

    connect(mainWidget, &Chatlist::signal_to_sub_widget, this, &PrivateChat::show_text_slot);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}
//发送信息
void PrivateChat::on_sendButton_clicked()
{
    QString text = ui->lineEdit->text();
    QJsonObject obj;
    obj.insert("cmd", "private_chat");
    obj.insert("user_from", userName);
    obj.insert("user_to", friendName);
    obj.insert("text", text);
    QByteArray ba = QJsonDocument(obj).toJson();
    socket->write(ba);

    ui->lineEdit->clear();
    ui->textEdit->append(userName+":"+text);//发送内容 显示在对话框
     ui->textEdit->setAlignment(Qt::AlignRight); //靠右显示
    ui->textEdit->append("\n");
}

void PrivateChat::show_text_slot(QJsonObject obj)
{
    if (obj.value("cmd").toString() == "private_chat")
    {
        if (obj.value("user_from").toString() == friendName)
        {
            if (this->isMinimized()) //最小化
            {
                this->showNormal(); //正常化
            }
            this->activateWindow();  //显示活跃 在最前边
            ui->textEdit->append(friendName+":"+obj.value("text").toString());
            ui->textEdit->setAlignment(Qt::AlignLeft);
            ui->textEdit->append("\n");
        }
    }
}

void PrivateChat::closeEvent(QCloseEvent *event)
{
    for (int i = 0; i < chatWidgetList->size(); i++)
    {
        if (chatWidgetList->at(i).name == friendName)
        {
            chatWidgetList->removeAt(i);
            break;
        }
    }
    event->accept(); //点击x关闭窗口会用到
}

void PrivateChat::on_fileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "发送文件", QCoreApplication::applicationFilePath());
    if (fileName.isEmpty())
    {
        QMessageBox::warning(this, "发送文件提示", "请选择一个文件");
    }
    else
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QJsonObject obj;
        obj.insert("cmd", "send_file");
        obj.insert("from_user", userName);
        obj.insert("to_user", friendName);
        obj.insert("length", file.size());
        obj.insert("filename", fileName);
        QByteArray ba = QJsonDocument(obj).toJson();
        socket->write(ba);
    }
}
