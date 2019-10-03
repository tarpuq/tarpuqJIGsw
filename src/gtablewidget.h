#ifndef GTABLEWIDGET_H
#define GTABLEWIDGET_H

#include <QTableWidget>
#include <QKeyEvent>

class GTableWidget : public QTableWidget
{
public:
    explicit GTableWidget(QWidget *parent = nullptr);

private:
    void copy();
    void paste();

public slots:
    void keyPressEvent(QKeyEvent *event);
};

#endif // GTABLEWIDGET_H
