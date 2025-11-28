#ifndef ONNXINFERENCE_H
#define ONNXINFERENCE_H

#include <QString>
#include <QImage>
#include <QVector>
#include <QRectF>
#include <vector>
#include <memory>

/**
 * @brief Базовый класс для работы с ONNX моделями
 * 
 * Предоставляет базовую функциональность для загрузки и выполнения
 * ONNX моделей без использования OpenCV
 */
class ONNXInference
{
public:
    struct Detection {
        QRectF bbox;                    // Bounding box
        float confidence;               // Уверенность детекции
        int classId;                    // ID класса
        QString className;              // Имя класса
    };

    struct SegmentationResult {
        Detection detection;             // Детекция с bbox
        QImage mask;                    // Маска сегментации
        QVector<QPointF> polygon;       // Полигон из маски
    };

    ONNXInference();
    virtual ~ONNXInference();

    /**
     * @brief Загружает ONNX модель
     * @param modelPath Путь к .onnx файлу
     * @return true если успешно
     */
    virtual bool loadModel(const QString &modelPath);

    /**
     * @brief Проверяет, загружена ли модель
     */
    bool isModelLoaded() const { return m_modelLoaded; }

    /**
     * @brief Предобрабатывает изображение для модели
     * @param image Входное изображение
     * @param targetSize Целевой размер (обычно 640x640 для YOLO)
     * @return Предобработанные данные в формате float32 [1, 3, H, W]
     */
    std::vector<float> preprocessImage(const QImage &image, int targetSize = 640);

    /**
     * @brief Выполняет инференс модели
     * @param inputData Предобработанные данные
     * @param inputShape Форма входных данных [batch, channels, height, width]
     * @return Выходные данные модели
     */
    virtual std::vector<float> runInference(const std::vector<float> &inputData, 
                                           const std::vector<int64_t> &inputShape) = 0;

    /**
     * @brief Постобрабатывает результаты модели
     * @param outputData Выходные данные модели
     * @param originalSize Оригинальный размер изображения
     * @param modelSize Размер модели (обычно 640)
     * @param confThreshold Порог уверенности
     * @return Список детекций
     */
    virtual QVector<Detection> postprocessDetections(
        const std::vector<float> &outputData,
        const QSize &originalSize,
        int modelSize = 640,
        float confThreshold = 0.25f) = 0;

protected:
    bool m_modelLoaded;
    QString m_modelPath;
    
    // Вспомогательные методы
    QImage letterboxImage(const QImage &image, int targetSize, 
                         float &scale, int &padX, int &padY);
    QRectF scaleBbox(const QRectF &bbox, float scale, int padX, int padY, 
                    const QSize &originalSize);
    
    /**
     * @brief Конвертирует маску в полигон
     */
    static QVector<QPointF> extractPolygonFromMask(const QImage &mask, float threshold = 0.5f);
};

#endif // ONNXINFERENCE_H

