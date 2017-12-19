#include "controller.h"

#include <QThread>
#include <QDir>
#include <QFileInfoList>
#include <QQueue>

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <random>

class ControllerPrivate
{
public:
    ControllerPrivate(Controller *controller)
        : q_ptr(controller)
        , nFiles(0)
        , nFilesProcessed(0)
        , finderFinished(false)
        , stop(false)
    {

    }

    Controller *q_ptr;

    std::list<std::thread> threadPool;
    QString inputDir;
    QString outputDir;
    QString fileFilter;
    QString tool;
    QString toolArgs;
    volatile bool stop;

    QQueue<QString> fileQueue;
    int nFiles;
    std::atomic<int> nFilesProcessed;
    std::mutex mtxFileList;
    std::condition_variable cvFileList;
    bool finderFinished;

    void EnqueueFile(const QString &file)
    {
        std::lock_guard<std::mutex> lock(mtxFileList);
        fileQueue << file;
        ++nFiles;
        emit q_ptr->filesNumberChanged(nFiles);
        cvFileList.notify_one();
    }

    QString DequeueFile()
    {
        std::unique_lock<std::mutex> lock(mtxFileList);
        cvFileList.wait(lock, [this](){ return this->stop || !this->fileQueue.empty() || this->finderFinished;});
        if(fileQueue.empty())
            return QString();
        return fileQueue.dequeue();
    }

    void FindFiles(const QString &subDir)
    {
        QDir dir(subDir, fileFilter, QDir::Name|QDir::IgnoreCase, QDir::Files|QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Readable);
        QFileInfoList list = dir.entryInfoList();
        for(auto fileInfo : list) {
            if(stop) {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if(fileInfo.isFile()) {
                EnqueueFile(fileInfo.absoluteFilePath());
            }
            else if(fileInfo.isDir()) {
                FindFiles(fileInfo.absoluteFilePath());
            }
        }
    }

    void Finder()
    {
        finderFinished = false;
        FindFiles(inputDir);
        finderFinished = true;
        FinishTask();
    }

    void ProcessFile(const QString &file)
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(500,5000);
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(mt)));

        emit q_ptr->filesProcessed(++nFilesProcessed);
    }

    void Processor()
    {
        while(!stop) {
            QString file = DequeueFile();
            if(!file.isEmpty()) {
                ProcessFile(file);
            }
            else if (finderFinished) {
                break;
            }
        }
        FinishTask();
    }

    void FinishTask()
    {
        QMetaObject::invokeMethod(q_ptr, "_q_threadFinished", Qt::QueuedConnection, Q_ARG(qulonglong, std::this_thread::get_id().hash()));
    }
    void _q_threadFinished(qulonglong threadIdHash)
    {
        auto it = std::find_if(threadPool.begin(), threadPool.end(), [&threadIdHash](const std::thread &thread){
            return threadIdHash == thread.get_id().hash();
        });
        if(it != threadPool.end()) {
            it->join();
            threadPool.erase(it);
        }

        if(threadPool.empty()) {
            emit q_ptr->finished();
        }
    }
};

Controller::Controller(QObject *parent) : QObject(parent)
{
    d_ptr.reset(new ControllerPrivate(this));
}

void Controller::setInputDirectory(const QString &inputDir)
{
    Q_D(Controller);
    d->inputDir = inputDir;
}

void Controller::setOutputDirectory(const QString &outputDir)
{
    Q_D(Controller);
    d->outputDir = outputDir;
}

void Controller::setFileFilter(const QString &fileFilter)
{
    Q_D(Controller);
    d->fileFilter = fileFilter;
}

void Controller::setTool(const QString &tool)
{
    Q_D(Controller);
    d->tool = tool;
}

void Controller::setToolArguments(const QString &toolArgs)
{
    Q_D(Controller);
    d->toolArgs = toolArgs;
}

void Controller::start()
{
    Q_D(Controller);
    d->nFiles = 0;
    d->nFilesProcessed = 0;
    d->fileQueue.clear();

    int nThreads = qMax(QThread::idealThreadCount(), 2);
    d->threadPool.push_back(std::thread(&ControllerPrivate::Finder, d));
    for(int i=1; i<nThreads; ++i) {
        d->threadPool.emplace_back(&ControllerPrivate::Processor, d);
    }
}

void Controller::stop()
{
    Q_D(Controller);
    std::lock_guard<std::mutex> lock(d->mtxFileList);
    d->stop = true;
    d->cvFileList.notify_all();
}

#include "moc_controller.cpp"
