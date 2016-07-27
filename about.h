#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include "base64.h"
#include "global.h"

using namespace Global;

namespace Ui {
class About;
}

class QUrl;


static const unsigned char key[16] = {
  0x5a,0x55,0x5b,0x83,
  0x99,0xa9,0x4f,0x7c,
  0xa1,0x23,0xae,0x9e,
  0xfa,0xa4,0x5c,0x4a
};

static const unsigned char iv[16] = {
  0x1e,0xc2,0xc9,0x92,
  0x3a,0xaf,0x4c,0xf2,
  0xa9,0x88,0x9b,0xb2,
  0xc9,0xfe,0x39,0x3d
};

class About : public QDialog
{
    Q_OBJECT

public:
    About(QWidget *parent = 0,Qt::WindowFlags f = 0);
    ~About();

protected:
    QString link(const QString & url,QString name = "");
    QString getMac();
    QString getappID();

protected slots:
    void openlink(const QUrl & link);
    void Exitwidget();

private:
    Ui::About *ui;
};

#endif // ABOUT_H
