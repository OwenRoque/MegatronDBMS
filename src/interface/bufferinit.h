#ifndef BUFFERINIT_H
#define BUFFERINIT_H

#include <QDialog>

namespace Ui {
class BufferInit;
}

class BufferInit : public QDialog
{
    Q_OBJECT

public:
    explicit BufferInit(QWidget *parent = nullptr);
    ~BufferInit();
    int getBufferSize() const;
    QString getPolicy() const;

private slots:
    void onOkButtonClicked();

private:
    Ui::BufferInit *ui;
    int bufferSize;
    QString policy;

};

#endif // BUFFERINIT_H
