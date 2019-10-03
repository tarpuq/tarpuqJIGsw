#include "gtablewidget.h"
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

GTableWidget::GTableWidget(QWidget *parent)
    : QTableWidget (parent)
{

}

void GTableWidget::copy()
{
    QItemSelectionModel * selection = selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();

    if(indexes.size() < 1)
        return;

    // QModelIndex::operator < sorts first by row, then by column.
    // this is what we need
    //    std::sort(indexes.begin(), indexes.end());
    qSort(indexes);

    // You need a pair of indexes to find the row changes
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text_as_html;
    QString selected_text;
    selected_text_as_html.prepend("<html><style>br{mso-data-placement:same-cell;}</style><table><tr><td>");
    QModelIndex current;

    Q_FOREACH(current, indexes)
    {
        QVariant data = model()->data(previous);
        QString text = data.toString();
        selected_text.append(text);
        text.replace("\n","<br>");
        // At this point `text` contains the text in one cell
        selected_text_as_html.append(text);

        // If you are at the start of the row the row number of the previous index
        // isn't the same.  Text is followed by a row separator, which is a newline.
        if (current.row() != previous.row())
        {
            selected_text_as_html.append("</td></tr><tr><td>");
            selected_text.append(QLatin1Char('\n'));
        }
        // Otherwise it's the same row, so append a column separator, which is a tab.
        else
        {
            selected_text_as_html.append("</td><td>");
            selected_text.append(QLatin1Char('\t'));
        }
        previous = current;
    }

    // add last element
    selected_text_as_html.append(model()->data(current).toString());
    selected_text.append(model()->data(current).toString());
    selected_text_as_html.append("</td></tr>");
    QMimeData * md = new QMimeData;
    md->setHtml(selected_text_as_html);
    //    qApp->clipboard()->setText(selected_text);
    md->setText(selected_text);
    qApp->clipboard()->setMimeData(md);

    //    selected_text.append(QLatin1Char('\n'));
    //    qApp->clipboard()->setText(selected_text);
}

void GTableWidget::paste()
{
    qDebug() << "paste";
}

void GTableWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Copy) )
    {
        copy();
    }
    else if(event->matches(QKeySequence::Paste) )
    {
        paste();
    }
    else
    {
        QTableWidget::keyPressEvent(event);
    }
}
