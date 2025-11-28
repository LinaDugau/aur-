#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>
#include <QImage>
#include <vector>
#include "SegmentationData.h"
#include "YOLO11Segmentation.h"
#include "YOLACTInference.h"

/**
 * @brief Класс для предобработки изображений яблок
 *
 * Обработка изображений без OpenCV:
 * - Нормализация
 * - Извлечение признаков (цвет, текстура, форма)
 * - Подготовка для ML-модели
 * - Работа с polygon сегментацией
 * - Интеграция YOLO11-segm и YOLACT через ONNX Runtime
 */
class ImageProcessor
{
public:
    ImageProcessor();
    ~ImageProcessor();

    /**
     * @brief Извлекает признаки из изображения
     * @param imagePath Путь к изображению
     * @param useSegmentation Использовать ли polygon данные
     * @return Вектор признаков
     */
    std::vector<double> extractFeatures(const QString &imagePath, bool useSegmentation = false);

    /**
     * @brief Извлекает признаки с использованием polygon маски
     */
    std::vector<double> extractFeaturesWithMask(const QString &imagePath,
                                                const SegmentationData::Polygon &polygon);

    /**
     * @brief Предобрабатывает изображение
     * @param imagePath Путь к изображению
     * @return QImage обработанное изображение
     */
    QImage preprocessImage(const QString &imagePath);

    /**
     * @brief Извлекает цветовые признаки (RGB/HSV гистограммы)
     */
    std::vector<double> extractColorFeatures(const QString &imagePath);

    /**
     * @brief Извлекает цветовые признаки из области маски
     */
    std::vector<double> extractColorFeaturesFromMask(const QImage &image, const QImage &mask);

    /**
     * @brief Извлекает текстурные признаки (edges, contrast, LBP)
     */
    std::vector<double> extractTextureFeatures(const QString &imagePath);

    /**
     * @brief Извлекает признаки формы из polygon
     */
    std::vector<double> extractShapeFeatures(const SegmentationData::Polygon &polygon);

    /**
     * @brief Проверяет наличие яблока на изображении (YOLO/YOLACT)
     */
    bool detectApple(const QString &imagePath);

    /**
     * @brief Детекция яблок с использованием YOLO11
     * @return Список bounding boxes найденных яблок
     */
    QVector<QRectF> detectApplesYOLO(const QString &imagePath);

    /**
     * @brief Загружает YOLO11-segm модель
     */
    bool loadYOLO11Model(const QString &modelPath);

    /**
     * @brief Загружает YOLACT модель
     */
    bool loadYOLACTModel(const QString &modelPath);

    /**
     * @brief Выполняет сегментацию с использованием YOLO11-segm
     * @param imagePath Путь к изображению
     * @return Список результатов сегментации
     */
    QVector<ONNXInference::SegmentationResult> segmentWithYOLO11(const QString &imagePath);

    /**
     * @brief Выполняет сегментацию с использованием YOLACT
     * @param imagePath Путь к изображению
     * @return Список результатов сегментации
     */
    QVector<ONNXInference::SegmentationResult> segmentWithYOLACT(const QString &imagePath);

    /**
     * @brief Устанавливает директорию с labelme аннотациями
     */
    void setAnnotationsDirectory(const QString &dirPath);

private:
    static const int IMAGE_SIZE = 224;  // Стандартный размер для нейросетей
    static const int FEATURE_DIM = 512; // Размерность вектора признаков

    QMap<QString, SegmentationData::ImageAnnotation> m_annotations;
    QString m_annotationsDir;

    // Модели сегментации
    YOLO11Segmentation* m_yolo11Segm;
    YOLACTInference* m_yolact;

    // Вспомогательные методы
    QImage convertToGrayscale(const QImage &image);
    double calculateCircularity(const SegmentationData::Polygon &polygon);
    double calculateAspectRatio(const QRectF &rect);
};

#endif // IMAGEPROCESSOR_H
