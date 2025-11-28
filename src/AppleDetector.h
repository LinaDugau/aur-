#ifndef APPLEDETECTOR_H
#define APPLEDETECTOR_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QVariantMap>

class ImageProcessor;
class AppleClassifier;
class CameraHandler;

/**
 * @brief Основной класс для детекции и классификации яблок
 *
 * Интегрирует обработку изображений и ML-классификацию
 * Поддерживает работу с камерой и polygon сегментацией
 * Предоставляет Qt-интерфейс для взаимодействия с QML
 */
class AppleDetector : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)
    Q_PROPERTY(QString lastResult READ lastResult NOTIFY lastResultChanged)
    Q_PROPERTY(bool modelTrained READ modelTrained NOTIFY modelTrainedChanged)
    Q_PROPERTY(bool useSegmentation READ useSegmentation WRITE setUseSegmentation NOTIFY useSegmentationChanged)
    Q_PROPERTY(CameraHandler* cameraHandler READ cameraHandler CONSTANT)

public:
    explicit AppleDetector(QObject *parent = nullptr);
    ~AppleDetector();

    bool isProcessing() const { return m_isProcessing; }
    QString lastResult() const { return m_lastResult; }
    bool modelTrained() const { return m_modelTrained; }
    bool useSegmentation() const { return m_useSegmentation; }
    CameraHandler* cameraHandler() const { return m_cameraHandler; }

    void setUseSegmentation(bool use);

public slots:
    /**
     * @brief Анализирует изображение яблока
     * @param imagePath Путь к файлу изображения
     */
    void analyzeImage(const QString &imagePath);

    /**
     * @brief Анализирует изображение с использованием polygon данных
     */
    void analyzeImageWithSegmentation(const QString &imagePath);

    /**
     * @brief Обучает модель на датасете
     * @param datasetPath Путь к папке с датасетом
     * @param usePolygonData Использовать polygon данные для обучения
     */
    void trainModel(const QString &datasetPath, bool usePolygonData = false);

    /**
     * @brief Загружает предобученную модель
     * @param modelPath Путь к файлу модели
     */
    void loadModel(const QString &modelPath);

    /**
     * @brief Сохраняет обученную модель
     * @param modelPath Путь для сохранения
     */
    void saveModel(const QString &modelPath);

    /**
     * @brief Анализирует кадр с камеры
     */
    void analyzeCameraFrame(const QImage &frame);

    /**
     * @brief Включает/выключает режим real-time анализа
     */
    void setRealtimeAnalysis(bool enabled);

signals:
    /**
     * @brief Сигнал завершения анализа
     * @param result Результат: "хорошее", "плохое", "не яблоко"
     * @param confidence Уверенность модели (0.0 - 1.0)
     */
    void analysisComplete(const QString &result, float confidence);

    /**
     * @brief Сигнал прогресса обучения
     * @param progress Прогресс (0-100)
     * @param message Сообщение о текущем шаге
     */
    void trainingProgress(int progress, const QString &message);

    /**
     * @brief Сигнал завершения обучения
     * @param success Успешность обучения
     * @param accuracy Точность модели
     */
    void trainingComplete(bool success, float accuracy);

    void isProcessingChanged();
    void lastResultChanged();
    void modelTrainedChanged();
    void useSegmentationChanged();
    void errorOccurred(const QString &error);

    /**
     * @brief Сигнал обнаружения яблок (для real-time)
     */
    void applesDetected(int count, const QVariantList &locations);

private slots:
    void onCameraFrameAvailable(const QImage &frame);
    void onCameraError(const QString &error);

private:
    void setIsProcessing(bool processing);
    void setLastResult(const QString &result);
    void setModelTrained(bool trained);

    ImageProcessor *m_imageProcessor;
    AppleClassifier *m_classifier;
    CameraHandler *m_cameraHandler;

    bool m_isProcessing;
    QString m_lastResult;
    bool m_modelTrained;
    bool m_useSegmentation;
    bool m_realtimeAnalysis;
};

#endif // APPLEDETECTOR_H
