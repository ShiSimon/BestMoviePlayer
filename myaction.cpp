#include "myaction.h"
#include <QWidget>

MyAction::MyAction(QObject * parent,const char * name,bool autoadd)
    :QAction(parent)
{
    setObjectName(name);
    if(autoadd) addActionToParent();
}

MyAction::MyAction(QObject *parent, bool autoadd)
    :QAction(parent)
{
    if(autoadd) addActionToParent();
}

MyAction::MyAction(const QString &text, QKeySequence accel, QObject *parent,
                   const char *name, bool autoadd)
    :QAction(parent)
{
    setObjectName(name);
    setText(text);
    setShortcut(accel);
    if(autoadd) addActionToParent();
}

MyAction::MyAction(QKeySequence accel, QObject *parent, const char *name,bool autoadd)
    :QAction(parent)
{
    setObjectName(name);
    setShortcut(accel);
    if(autoadd) addActionToParent();
}

MyAction::~MyAction(){
}

void MyAction::addShortcut(QKeySequence key){
    setShortcut(key);
}

void MyAction::addActionToParent(){
    if(parent()){
        if(parent()->inherits("QWidget")){
            QWidget *w=static_cast<QWidget*>(parent());
            w->addAction(this);
        }
    }
}

void MyAction::change(const QIcon &icon, const QString &text){
    setIcon(icon);
    change(text);
}

void MyAction::change(const QString &text){
    setText(text);

    QString accel_text = shortcut().toString();

    QString s = text;
    s.replace("&","");
    if(!accel_text.isEmpty()){
        setToolTip(s + accel_text + ")");
        setIconText(s);
    }
}
