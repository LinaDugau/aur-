#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QString>
#include <QImage>
#include <vector>
#include "SegmentationData.h"

/**
 * @brief Класс для предобработки изображений яблок
 *
 * Использует OpenCV для обработки изображений:
 * - Нормализация
 * - Извлечение признаков (цвет, текстура, форма)
 * - Подготовка для ML-модели
 * - Работа с polygon сегментацией
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
     * @brief Загружает YOLO модель
     */
    bool loadYOLOModel(const QString &modelPath);

    /**
     * @brief Устанавливает директорию с labelme аннотациями
     */
    void setAnnotationsDirectory(const QString &dirPath);

private:
    static const int IMAGE_SIZE = 224;  // Стандартный размер для нейросетей
    static const int FEATURE_DIM = 512; // Размерность вектора признаков

    QMap<QString, SegmentationData::ImageAnnotation> m_annotations;
    QString m_annotationsDir;

    // Вспомогательные методы
    QImage convertToGrayscale(const QImage &image);
    double calculateCircularity(const SegmentationData::Polygon &polygon);
    double calculateAspectRatio(const QRectF &rect);
};

#endif // IMAGEPROCESSOR_H
