/**
 * @file ImgSlicer.cpp
 * @author TonyWu
 */

#include "ImgSlicer.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QSettings>
#include <QThread>
#include <g3log/g3log.hpp>

#include "ImgSlicer_config.h"
#include "./ui_ImgSlicer.h"

namespace
{
    bool isSupportedImgFormat(const QString& suffix)
    {
        static const QSet<QString> supportedFormats = {"bmp", "png", "jpg", "jpeg"};
        return supportedFormats.contains(suffix.toLower());
    }

    void updateFileList(const QString& dirPath, QVector<QFileInfo>& imgList)
    {
        if (dirPath.isEmpty()) {
            imgList.clear();
            return;
        }

        QDir dir(dirPath);
        auto fileList = dir.entryInfoList(QDir::Files | QDir::Readable, QDir::Name);

        imgList.clear();
        for (const auto& fileInfo : fileList) {
            QString suffix = fileInfo.completeSuffix().toLower();
            if (isSupportedImgFormat(suffix)) {
                imgList.push_back(fileInfo);
            }
        }
    }

    bool createDir(const QString& path)
    {
        QDir dir;
        if (!dir.exists(path) && !dir.mkpath(path)) {
            return false;
        }
        return true;
    }
}

ImgSlicer::ImgSlicer(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::ImgSlicer)
{
    ui->setupUi(this);
    ui->version->setText(PROJECT_VERSION);

    QSettings settings;
    ui->inDirBox->setText(settings.value("inDir", "").toString());
    ui->outDirBox->setText(settings.value("outDir", "").toString());
    ui->camCntBox->setValue(settings.value("camCnt", 1).toInt());

    connect(this, &ImgSlicer::updateProgress, ui->progressBar, &QProgressBar::setValue);
}

ImgSlicer::~ImgSlicer()
{
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->requestInterruption();
        m_workerThread->wait();
    }
    delete ui;
}

void ImgSlicer::on_inDirButton_released()
{
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        ui->inDirBox->text(),
        QFileDialog::DontUseNativeDialog | QFileDialog::DontResolveSymlinks | QFileDialog::ReadOnly);

    if (!dirPath.isEmpty()) {
        ui->inDirBox->setText(dirPath);
        QSettings settings;
        settings.setValue("inDir", dirPath);
    }
}

void ImgSlicer::on_outDirButton_released()
{
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        ui->outDirBox->text(),
        QFileDialog::DontUseNativeDialog | QFileDialog::DontResolveSymlinks | QFileDialog::ReadOnly);

    if (!dirPath.isEmpty()) {
        ui->outDirBox->setText(dirPath);
        QSettings settings;
        settings.setValue("outDir", dirPath);
    }
}

void ImgSlicer::on_camCntBox_valueChanged(int val)
{
    QSettings settings;
    settings.setValue("camCnt", val);
}

void ImgSlicer::on_inDirBox_editingFinished()
{
    QSettings settings;
    settings.setValue("inDir", ui->inDirBox->text());
}

void ImgSlicer::on_outDirBox_editingFinished()
{
    QSettings settings;
    settings.setValue("outDir", ui->outDirBox->text());
}

void ImgSlicer::on_runSwitch_released()
{
    if (ui->runSwitch->text() == "Run") {
        startWork();
    }
    else {
        terminateWork();
    }
}

void ImgSlicer::run(const QString& outDir, int camCnt)
{
    for (int imgIndex = 0; imgIndex < m_imgList.size(); ++imgIndex) {
        emit updateProgress(imgIndex * 100 / m_imgList.size());
        if (QThread::currentThread()->isInterruptionRequested()) {
            m_wasInterrupted = true;
            LOG(INFO) << "Processing interrupted by user.";
            break;
        }

        const auto& fileInfo = m_imgList[imgIndex];
        LOG(INFO) << "Start slicing image: " << fileInfo.filePath().toStdString();

        QImageReader reader(fileInfo.filePath());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        reader.setAllocationLimit(0);
#endif
        if (!reader.read(&m_img)) {
            ++m_failedImgCnt;
            LOG(WARNING) << "Failed image: " << fileInfo.filePath().toStdString()
                         << ". Reason: Failed to read. " << reader.errorString().toStdString();
            continue;
        }

        if (m_img.width() % camCnt != 0) {
            ++m_failedImgCnt;
            LOG(WARNING) << "Failed image: " << fileInfo.filePath().toStdString()
                         << ". Reason: Image width is not divisible by camera count."
                         << " Image width: " << m_img.width() << ", Camera count: " << camCnt;
            continue;
        }

        int partWidth = m_img.width() / camCnt;
        bool allPartitionSaved = true;
        for (int camIndex = 0; camIndex < camCnt; ++camIndex) {
            QImage partition = m_img.copy(partWidth * camIndex, 0, partWidth, m_img.height());
            QDir cameraDir(QDir(outDir).filePath(QString("Cam%1").arg(camIndex + 1)));
            QString outputPath = cameraDir.filePath(fileInfo.fileName());
            if (!partition.save(outputPath)) {
                allPartitionSaved = false;
                LOG(WARNING) << "Failed image: " << fileInfo.filePath().toStdString()
                             << " (Camera " << (camIndex + 1) << "). Reason: Failed to save output.";
                break;
            }
        }

        if (!allPartitionSaved) {
            ++m_failedImgCnt;
            LOG(WARNING) << "Failed image: " << fileInfo.filePath().toStdString()
                << ". Reason: Failed to save all partitions.";
        }
        else {
            LOG(INFO) << "Image slicing completed: " << fileInfo.filePath().toStdString();
        }        
    }

    if (QThread::currentThread()->isInterruptionRequested() || m_failedImgCnt != 0) {
        emit updateProgress(0);
    }
    else {
        emit updateProgress(100);
    }
}

void ImgSlicer::startWork()
{
    ui->progressBar->reset();
    m_failedImgCnt = 0;
    m_wasInterrupted = false;

    QString inDir = ui->inDirBox->text();
    QString outDir = ui->outDirBox->text();
    if (inDir.isEmpty() || outDir.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("The directory is empty. Please fill the directory."));
        msgBox.exec();
        return;
    }

    updateFileList(inDir, m_imgList);
    if (m_imgList.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("The input directory doesn't contain any supported images. Please reselect the input directory."));
        msgBox.exec();
        return;
    }

    int camCnt = ui->camCntBox->value();
    for (int camIndex = 0; camIndex < camCnt; ++camIndex) {
        QString dirPath = QDir(outDir).filePath(QString("Cam%1").arg(camIndex + 1));
        if (!createDir(dirPath)) {
            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Failed to create output directory: %1. Please check the permissions.").arg(dirPath));
            msgBox.exec();
            return;
        }
    }

    m_workerThread = QThread::create([this, outDir, camCnt]() { run(outDir, camCnt); });
    connect(m_workerThread, &QThread::finished, this, [this]() {
        ui->runSwitch->setText("Run");

        QMessageBox msgBox(this);
        if (m_wasInterrupted) {
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("Image slicing was stopped by user."));
        }
        else if (m_failedImgCnt == 0) {
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("All images processed successfully."));
        }
        else {
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Some errors have occurred. Failed to process %1 images. Please check the log for details.").arg(m_failedImgCnt));
        }
        msgBox.exec();

        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    });

    m_workerThread->start();
    ui->runSwitch->setText("Stop");
}

void ImgSlicer::terminateWork()
{
    if (!m_workerThread) {
        return;
    }

    m_workerThread->requestInterruption();
    m_workerThread->wait();
}
