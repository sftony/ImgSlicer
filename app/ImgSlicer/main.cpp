/**
 * @file main.cpp
 * @autho TonyWu
 */

#include <QApplication>
#include <QTranslator>
#include <QCommandLineParser>
#include <Logger/LogRotate.h>
#include <g3log/logworker.hpp>

#include "ImgSlicer.h"
#include "ImgSlicer_config.h"

int main(int argc, char* argv[])
{
    auto logger = g3::LogWorker::createLogWorker();
    auto sinkHandle = logger->addSink(std::make_unique<LogRotate>("ImgSlicer", "./"), &LogRotate::save);
    g3::initializeLogging(logger.get());

    std::string version(PROJECT_VERSION);
    LOG(INFO) << "ImgSlicer version: " << version;

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("sftony");
    QCoreApplication::setApplicationName("ImgSlicer");
    QCoreApplication::setApplicationVersion(QString::fromStdString(version));

    QTranslator translator;
    if (!translator.load("ImgSlicer_zh_TW.qm")) {
        LOG(WARNING) << "Failed to load translation file: ImgSlicer_zh_TW.qm. The application will use the default language.";
    }
    a.installTranslator(&translator);

    QCommandLineParser parser;
    parser.setApplicationDescription("Image slicer that splits images into per-camera outputs. This tool only support bmp, png, jpg, and jpeg image formats.");
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(a);

    ImgSlicer w;
    w.show();
    return a.exec();
}
