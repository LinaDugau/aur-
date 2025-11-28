#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H

#include <QObject>
#include <QCamera>
#include <QCameraImageCapture>
#include <QImage>
#include <QVideoFrame>

/**
 * @brief Класс для работы с камерой устройства
 */
class CameraHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool hasCamera READ hasCamera NOTIFY hasCameraChanged)

public:
    explicit CameraHandler(QObject *parent = nullptr);
    ~CameraHandler();

    bool isActive() const { return m_isActive; }
    bool hasCamera() const { return m_hasCamera; }

public slots:
    /**
     * @brief Запускает камеру
     */
    void startCamera();

    /**
     * @brief Останавливает камеру
     */
    void stopCamera();

    /**
     * @brief Делает снимок
     */
    void captureImage();

    /**
     * @brief Устанавливает viewfinder для предпросмотра
     */
    void setViewfinder(QObject *viewfinder);

signals:
    /**
     * @brief Сигнал захвата изображения
     */
    void imageCaptured(const QString &filePath);

    /**
     * @brief Сигнал обновления кадра (для real-time анализа)
     */
    void frameAvailable(const QImage &frame);

    void isActiveChanged();
    void hasCameraChanged();
    void errorOccurred(const QString &error);

private slots:
    void onImageCaptured(int id, const QImage &image);
    void onCameraError(QCamera::Error error);

private:
    QCamera *m_camera;
    QCameraImageCapture *m_imageCapture;

    bool m_isActive;
    bool m_hasCamera;

    QString m_captureDir;
};

#endif // CAMERAHANDLER_H
