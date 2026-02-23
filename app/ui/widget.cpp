#include "widget.h"
#include "ui_widget.h"

#include <iostream>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
      , ui(new Ui::Widget) {
    ui->setupUi(this);
    this->setLayout(ui->verticalLayout);
    ui->widgetBottom->setLayout(ui->horizontalLayout_2);

    this->SetupConnections();
}

Widget::~Widget() {
    delete ui;
}

void Widget::ViewSlot() {
    std::cout << "hello world" << std::endl;
}

void Widget::SetupConnections() {
    connect(this, SIGNAL(ViewSig()), this, SLOT(ViewSlot()));
    connect(ui->btnOpen, SIGNAL(clicked()), this, SLOT(ViewSlot()));
}
