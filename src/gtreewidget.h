#ifndef GTREEWIDGET_H
#define GTREEWIDGET_H

#include <QTreeWidget>
#include <QKeyEvent>

class GTreeWidget : public QTreeWidget
{
public:
    explicit GTreeWidget(QWidget *parent = nullptr);

private:
    void copy();
    void paste();

public slots:
    void keyPressEvent(QKeyEvent *event);
};

#endif // GTREEWIDGET_H
