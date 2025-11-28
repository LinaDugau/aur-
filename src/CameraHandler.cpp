#include "CameraHandler.h"
#include <QCameraInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>

CameraHandler::CameraHandler(QObject *parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_imageCapture(nullptr)
    , m_isActive(false)
    , m_hasCamera(false)
{
    // Проверяем доступность камеры
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    m_hasCamera = !cameras.isEmpty();

    if (m_hasCamera) {
        // Используем заднюю камеру по умолчанию
        QCameraInfo cameraInfo = QCameraInfo::defaultCamera();
        for (const QCameraInfo &info : cameras) {
            if (info.position() == QCamera::BackFace) {
                cameraInfo = info;
                break;
            }
        }

        m_camera = new QCamera(cameraInfo, this);
        m_imageCapture = new QCameraImageCapture(m_camera, this);

        // Настройка захвата
        m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);

        // Директория для сохранения снимков
        m_captureDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                       + "/AurCad";
        QDir().mkpath(m_captureDir);

        // Подключаем сигналы
        connect(m_imageCapture, &QCameraImageCapture::imageCaptured,
                this, &CameraHandler::onImageCaptured);
        // Используем старый синтаксис для совместимости с Qt 5.6
        connect(m_camera, SIGNAL(error(QCamera::Error)),
                this, SLOT(onCameraError(QCamera::Error)));

        qDebug() << "Camera initialized:" << cameraInfo.description();
    } else {
        qWarning() << "No camera available";
    }
}

CameraHandler::~CameraHandler()
{
    if (m_camera) {
        m_camera->stop();
    }
}

void CameraHandler::startCamera()
{
    if (!m_hasCamera || !m_camera) {
        emit errorOccurred("No camera available");
        return;
    }

    if (m_camera->state() == QCamera::ActiveState) {
        qDebug() << "Camera already active";
        return;
    }

    m_camera->start();
    m_isActive = true;
    emit isActiveChanged();

    qDebug() << "Camera started";
}

void CameraHandler::stopCamera()
{
    if (m_camera && m_camera->state() == QCamera::ActiveState) {
        m_camera->stop();
        m_isActive = false;
        emit isActiveChanged();

        qDebug() << "Camera stopped";
    }
}

void CameraHandler::captureImage()
{
    if (!m_imageCapture) {
        emit errorOccurred("Image capture not initialized");
        return;
    }

    if (!m_camera || m_camera->state() != QCamera::ActiveState) {
        emit errorOccurred("Camera not active");
        return;
    }

    // Генерируем имя файла
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName = QString("%1/apple_%2.jpg").arg(m_captureDir).arg(timestamp);

    m_imageCapture->capture(fileName);

    qDebug() << "Capturing image to:" << fileName;
}

void CameraHandler::setViewfinder(QObject *viewfinder)
{
    // Viewfinder is handled in QML using VideoOutput
    // This method is kept for API compatibility but does nothing
    Q_UNUSED(viewfinder);
    qDebug() << "Viewfinder is handled in QML, this method is deprecated";
}

void CameraHandler::onImageCaptured(int id, const QImage &image)
{
    Q_UNUSED(id);

    // Сохраняем изображение
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName = QString("%1/apple_%2.jpg").arg(m_captureDir).arg(timestamp);

    if (image.save(fileName, "JPEG", 95)) {
        qDebug() << "Image saved:" << fileName;
        emit imageCaptured(fileName);

        // Для real-time анализа
        emit frameAvailable(image);
    } else {
        emit errorOccurred("Failed to save image");
    }
}

void CameraHandler::onCameraError(QCamera::Error error)
{
    QString errorMsg;

    switch (error) {
    case QCamera::NoError:
        return;
    case QCamera::CameraError:
        errorMsg = "Camera error occurred";
        break;
    case QCamera::InvalidRequestError:
        errorMsg = "Invalid request";
        break;
    case QCamera::ServiceMissingError:
        errorMsg = "Camera service missing";
        break;
    case QCamera::NotSupportedFeatureError:
        errorMsg = "Feature not supported";
        break;
    }

    qWarning() << "Camera error:" << errorMsg;
    emit errorOccurred(errorMsg);
}
