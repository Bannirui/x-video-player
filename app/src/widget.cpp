#include "widget.h"
#include "ui_widget.h"

#include "x/x_log.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->SetupConnections();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::SetupConnections()
{
}
