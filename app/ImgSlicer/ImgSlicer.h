/**
 * @file ImgSlicer.h
 * @author TonyWu
 */

#ifndef IMGSLICER_H
#define IMGSLICER_H

#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class ImgSlicer; }
class QFileInfo;
class QImage;
class QThread;
QT_END_NAMESPACE

class ImgSlicer : public QMainWindow
{
    Q_OBJECT

public:
    ImgSlicer(QWidget* parent = nullptr);
    ~ImgSlicer();

private slots:
    void on_inDirButton_released();
    void on_outDirButton_released();
    void on_camCntBox_valueChanged(int val);
    void on_inDirBox_editingFinished();
    void on_outDirBox_editingFinished();
    void on_runSwitch_released();

signals:
    void updateProgress(int val);

private:
    Ui::ImgSlicer* ui;

    QThread* m_workerThread = nullptr;
    QVector<QFileInfo> m_imgList;
    int m_failedImgCnt = 0;
    bool m_wasInterrupted = false;
    QImage m_img;
    bool m_isRunning = false;

    void run(const QString& outDir, int camCnt);
    void startWork();
    void terminateWork();
};
#endif
