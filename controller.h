#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>

class ControllerPrivate;

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);

signals:
    void filesNumberChanged(int nFiles);
    void filesProcessed(int nFiles);
    void finished();

public slots:
    void setInputDirectory(const QString &inputDir);
    void setOutputDirectory(const QString &outputDir);
    void setFileFilter(const QString &fileFilter);
    void setTool(const QString &tool);
    void setToolArguments(const QString &toolArgs);
    void start();
    void stop();

private:
    QScopedPointer<ControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Controller)

    Q_PRIVATE_SLOT(d_func(), void _q_threadFinished(qulonglong threadIdHash))
};


#endif // CONTROLLER_H
