#pragma once

#include <QWidget>

namespace Ui
{
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);

    ~Widget() override;

    // connect signal and slot
    void SetupConnections();

signals:

public slots:

private:
    Ui::Widget *ui;
};
