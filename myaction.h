#ifndef MYACTION_H
#define MYACTION_H

#include <QAction>
#include <QString>
#include <QIcon>
#include <QKeySequence>

class MyAction : public QAction
{
public:
    //!Create a new MyAction with name.If autoadd is true
    //!the action will be added to the parent
    MyAction(QObject *parent,const char *name,bool autoadd = true);

    //!Create a new MyAction.If autoadd is true
    //!the action will be added to the parent
    MyAction(QObject *parent,bool autoadd = true);

    MyAction(const QString & text,QKeySequence accel,QObject *parent,
             const char * name = "",bool autoadd = true);

    MyAction(QKeySequence accel,QObject *parent,const char *name = "",
             bool autoadd = true);

    ~MyAction();

    void addShortcut(QKeySequence key);

    //!Change the icon and text of the action
    void change(const QIcon & icon,const QString & text);

    //!Change the text of the action
    void change(const QString & text);

protected:
    //!Checks if the parent is a QWidget and adds the action to it
    void addActionToParent();
};

#endif // MYACTION_H
