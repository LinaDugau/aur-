#ifndef YOLACTINFERENCE_H
#define YOLACTINFERENCE_H

#include "ONNXInference.h"
#include <QImage>
#include <QVector>

/**
 * @brief Класс для работы с YOLACT (You Only Look At Coefficients)
 * 
 * Реализует детекцию и сегментацию объектов с использованием YOLACT модели
 */
class YOLACTInference : public ONNXInference
{
public:
    YOLACTInference();
    ~YOLACTInference() override;

    /**
     * @brief Загружает YOLACT модель
     */
    bool loadModel(const QString &modelPath) override;

    /**
     * @brief Выполняет сегментацию на изображении
     * @param imagePath Путь к изображению
     * @param confThreshold Порог уверенности (по умолчанию 0.15)
     * @return Список результатов сегментации
     */
    QVector<SegmentationResult> segmentImage(const QString &imagePath,
                                           float confThreshold = 0.15f);

    /**
     * @brief Выполняет сегментацию на QImage
     */
    QVector<SegmentationResult> segmentImage(const QImage &image,
                                           float confThreshold = 0.15f);

    /**
     * @brief Выполняет инференс модели
     */
    std::vector<float> runInference(const std::vector<float> &inputData, 
                                    const std::vector<int64_t> &inputShape) override;

    /**
     * @brief Постобрабатывает результаты YOLACT
     */
    QVector<Detection> postprocessDetections(
        const std::vector<float> &outputData,
        const QSize &originalSize,
        int modelSize = 550,
        float confThreshold = 0.15f) override;

    /**
     * @brief Извлекает маски из выходных данных YOLACT
     */
    QVector<QImage> extractMasks(const std::vector<float> &outputData,
                                const QVector<Detection> &detections,
                                const QSize &originalSize,
                                int modelSize = 550);

private:
    static const int MODEL_SIZE = 550;  // Стандартный размер для YOLACT
    static const int NUM_CLASSES = 81;  // COCO датасет (80 классов + фон)
    static const int NUM_MASKS = 32;    // Количество прототипов масок
    
    // Вспомогательные методы
    QImage decodeMask(const std::vector<float> &maskCoeffs,
                     const std::vector<float> &maskProto,
                     const Detection &detection,
                     const QSize &originalSize,
                     int modelSize);
};

#endif // YOLACTINFERENCE_H

