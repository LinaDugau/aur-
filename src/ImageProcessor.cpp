#include "ImageProcessor.h"
#include "SegmentationData.h"
#include <QDebug>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <cmath>

// TODO: После настройки Conan добавить:
// #include <opencv2/opencv.hpp>
// #include <opencv2/imgproc.hpp>
// #include <opencv2/highgui.hpp>

ImageProcessor::ImageProcessor()
    : m_yolo11Segm(nullptr)
    , m_yolact(nullptr)
{
    qDebug() << "ImageProcessor initialized";
}

ImageProcessor::~ImageProcessor()
{
    delete m_yolo11Segm;
    delete m_yolact;
}

std::vector<double> ImageProcessor::extractFeatures(const QString &imagePath, bool useSegmentation)
{
    qDebug() << "Extracting features from:" << imagePath << "useSegmentation:" << useSegmentation;

    // Если используем сегментацию и есть аннотации
    if (useSegmentation && !m_annotations.isEmpty()) {
        QFileInfo fileInfo(imagePath);
        QString imageName = fileInfo.fileName();
        
        // Ищем аннотацию для этого изображения
        if (m_annotations.contains(imageName)) {
            const SegmentationData::ImageAnnotation &annotation = m_annotations[imageName];
            
            // Ищем полигон с меткой "apple"
            for (const SegmentationData::Polygon &polygon : annotation.polygons) {
                if (polygon.label.toLower() == "apple") {
                    // Используем расширенное извлечение признаков с маской
                    return extractFeaturesWithMask(imagePath, polygon);
                }
            }
        }
    }

    // Стандартное извлечение признаков
    std::vector<double> features;

    // Извлекаем цветовые признаки
    std::vector<double> colorFeatures = extractColorFeatures(imagePath);
    features.insert(features.end(), colorFeatures.begin(), colorFeatures.end());

    // Извлекаем текстурные признаки
    std::vector<double> textureFeatures = extractTextureFeatures(imagePath);
    features.insert(features.end(), textureFeatures.begin(), textureFeatures.end());

    qDebug() << "Extracted" << features.size() << "features";

    return features;
}

std::vector<double> ImageProcessor::extractColorFeatures(const QString &imagePath)
{
    std::vector<double> colorFeatures;

    // Загружаем и предобрабатываем изображение (кроп + ресайз)
    QImage image = preprocessImage(imagePath);
    if (image.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        // Возвращаем пустой вектор нужного размера
        colorFeatures.resize(256, 0.0);
        return colorFeatures;
    }

    // Вычисляем цветовые гистограммы (упрощенная версия без OpenCV)
    // HSV гистограммы для каждого канала

    // Гистограммы R, G, B (по 64 бина на канал = 192 признака)
    std::vector<int> histR(64, 0);
    std::vector<int> histG(64, 0);
    std::vector<int> histB(64, 0);

    int totalPixels = 0;

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            // Квантуем в 64 бина
            int binR = (r * 64) / 256;
            int binG = (g * 64) / 256;
            int binB = (b * 64) / 256;

            histR[binR]++;
            histG[binG]++;
            histB[binB]++;
            totalPixels++;
        }
    }

    // Нормализуем гистограммы
    for (int i = 0; i < 64; ++i) {
        colorFeatures.push_back(static_cast<double>(histR[i]) / totalPixels);
        colorFeatures.push_back(static_cast<double>(histG[i]) / totalPixels);
        colorFeatures.push_back(static_cast<double>(histB[i]) / totalPixels);
    }

    // Дополнительные статистики: средние значения, стандартное отклонение
    double meanR = 0, meanG = 0, meanB = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            meanR += qRed(pixel);
            meanG += qGreen(pixel);
            meanB += qBlue(pixel);
        }
    }

    meanR /= totalPixels;
    meanG /= totalPixels;
    meanB /= totalPixels;

    colorFeatures.push_back(meanR / 255.0);
    colorFeatures.push_back(meanG / 255.0);
    colorFeatures.push_back(meanB / 255.0);

    return colorFeatures;
}

std::vector<double> ImageProcessor::extractTextureFeatures(const QString &imagePath)
{
    std::vector<double> textureFeatures;

    QImage image = preprocessImage(imagePath);
    if (image.isNull()) {
        // Возвращаем пустой вектор
        textureFeatures.resize(128, 0.0);
        return textureFeatures;
    }

    // Упрощенные текстурные признаки без OpenCV
    // Вычисляем edge density, contrast, и т.д.

    // Простой edge detector (Sobel-подобный)
    int edgeCount = 0;
    for (int y = 1; y < image.height() - 1; ++y) {
        for (int x = 1; x < image.width() - 1; ++x) {
            int gx = qGray(image.pixel(x + 1, y)) - qGray(image.pixel(x - 1, y));
            int gy = qGray(image.pixel(x, y + 1)) - qGray(image.pixel(x, y - 1));
            int gradient = std::sqrt(gx * gx + gy * gy);
            if (gradient > 50) {
                edgeCount++;
            }
        }
    }

    double edgeDensity = static_cast<double>(edgeCount) /
                        ((image.width() - 2) * (image.height() - 2));
    textureFeatures.push_back(edgeDensity);

    // Контраст
    double minGray = 255, maxGray = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            int gray = qGray(image.pixel(x, y));
            minGray = std::min(minGray, static_cast<double>(gray));
            maxGray = std::max(maxGray, static_cast<double>(gray));
        }
    }
    double contrast = (maxGray - minGray) / 255.0;
    textureFeatures.push_back(contrast);

    // Дополняем до нужной размерности нулями
    while (textureFeatures.size() < 128) {
        textureFeatures.push_back(0.0);
    }

    return textureFeatures;
}

bool ImageProcessor::detectApple(const QString &imagePath)
{
    // Упрощенная проверка: предполагаем, что если изображение корректно загружается
    // и имеет достаточный размер, то это может быть яблоко
    // В полной версии здесь будет YOLO/YOLACT детекция

    QImage image(imagePath);
    if (image.isNull()) {
        return false;
    }

    // Проверяем минимальный размер
    if (image.width() < 50 || image.height() < 50) {
        return false;
    }

    // TODO: Добавить YOLO/YOLACT детекцию когда будет настроен OpenCV
    // Пока возвращаем true для всех корректных изображений

    return true;
}

QImage ImageProcessor::preprocessImage(const QString &imagePath)
{
    QImage image(imagePath);
    if (image.isNull()) {
        return QImage();
    }

    // Вырезаем центральную часть (60% от ширины и высоты)
    // Это помогает убрать фон (стол, стены) и сосредоточиться на яблоке
    int cropWidth = image.width() * 0.6;
    int cropHeight = image.height() * 0.6;
    int x = (image.width() - cropWidth) / 2;
    int y = (image.height() - cropHeight) / 2;
    
    QImage cropped = image.copy(x, y, cropWidth, cropHeight);

    // Resize до стандартного размера
    QImage resized = cropped.scaled(IMAGE_SIZE, IMAGE_SIZE,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);

    return resized;
}
