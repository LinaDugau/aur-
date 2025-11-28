#ifndef YOLO11SEGMENTATION_H
#define YOLO11SEGMENTATION_H

#include "ONNXInference.h"
#include <QImage>
#include <QVector>
#include <QPointF>

/**
 * @brief Класс для работы с YOLO11-segm (сегментация)
 * 
 * Реализует детекцию и сегментацию объектов с использованием YOLO11-segm модели
 */
class YOLO11Segmentation : public ONNXInference
{
public:
    YOLO11Segmentation();
    ~YOLO11Segmentation() override;

    /**
     * @brief Загружает YOLO11-segm модель
     */
    bool loadModel(const QString &modelPath) override;

    /**
     * @brief Выполняет сегментацию на изображении
     * @param imagePath Путь к изображению
     * @param confThreshold Порог уверенности (по умолчанию 0.25)
     * @param iouThreshold Порог IoU для NMS (по умолчанию 0.45)
     * @return Список результатов сегментации
     */
    QVector<SegmentationResult> segmentImage(const QString &imagePath,
                                             float confThreshold = 0.25f,
                                             float iouThreshold = 0.45f);

    /**
     * @brief Выполняет сегментацию на QImage
     */
    QVector<SegmentationResult> segmentImage(const QImage &image,
                                             float confThreshold = 0.25f,
                                             float iouThreshold = 0.45f);

    /**
     * @brief Выполняет инференс модели
     */
    std::vector<float> runInference(const std::vector<float> &inputData, 
                                    const std::vector<int64_t> &inputShape) override;

    /**
     * @brief Постобрабатывает результаты YOLO11-segm
     */
    QVector<Detection> postprocessDetections(
        const std::vector<float> &outputData,
        const QSize &originalSize,
        int modelSize = 640,
        float confThreshold = 0.25f) override;

    /**
     * @brief Извлекает маски сегментации из выходных данных
     */
    QVector<QImage> extractMasks(const std::vector<float> &outputData,
                                 const QVector<Detection> &detections,
                                 const QSize &originalSize,
                                 int modelSize = 640);

    /**
     * @brief Применяет Non-Maximum Suppression (NMS)
     */
    QVector<Detection> applyNMS(const QVector<Detection> &detections,
                               float iouThreshold = 0.45f);

    /**
     * @brief Конвертирует маску в полигон (использует метод базового класса)
     */
    static QVector<QPointF> maskToPolygon(const QImage &mask, float threshold = 0.5f);

private:
    static const int MODEL_SIZE = 640;  // Стандартный размер для YOLO11
    static const int NUM_CLASSES = 80;  // COCO датасет
    
    // Вспомогательные методы
    float calculateIoU(const QRectF &box1, const QRectF &box2);
    QImage processMaskProto(const std::vector<float> &maskProto,
                           const Detection &detection,
                           const QSize &originalSize,
                           int modelSize);
};

#endif // YOLO11SEGMENTATION_H

