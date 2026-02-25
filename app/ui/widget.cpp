#include "widget.h"
#include "ui_widget.h"

#include "../../x_video_player/src/x/x_log.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setLayout(ui->verticalLayout);
    ui->widgetBottom->setLayout(ui->horizontalLayout_2);

    this->SetupConnections();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::ViewSlot()
{
    XLOG_INFO("HELLO WORLD");
}

void Widget::SetupConnections()
{
    connect(this, SIGNAL(ViewSig()), this, SLOT(ViewSlot()));
    connect(ui->btnOpen, SIGNAL(clicked()), this, SLOT(ViewSlot()));
}
