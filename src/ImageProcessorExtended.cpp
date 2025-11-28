// Расширенные методы для ImageProcessor - использование polygon данных

#include "ImageProcessor.h"
#include "SegmentationData.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <cmath>

void ImageProcessor::setAnnotationsDirectory(const QString &dirPath)
{
    m_annotationsDir = dirPath;

    // Загружаем все аннотации
    SegmentationData segData;
    m_annotations = segData.loadFromDirectory(dirPath);

    qDebug() << "Loaded" << m_annotations.size() << "annotations";
}

std::vector<double> ImageProcessor::extractFeaturesWithMask(
    const QString &imagePath,
    const SegmentationData::Polygon &polygon)
{
    qDebug() << "Extracting features with polygon mask from:" << imagePath;

    std::vector<double> features;

    // Загружаем изображение
    QImage image(imagePath);
    if (image.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        features.resize(FEATURE_DIM, 0.0);
        return features;
    }

    // Создаем маску из полигона
    QImage mask = SegmentationData::createMaskFromPolygon(polygon, image.width(), image.height());

    // 1. Цветовые признаки из области маски (256 признаков)
    std::vector<double> colorFeatures = extractColorFeaturesFromMask(image, mask);
    features.insert(features.end(), colorFeatures.begin(), colorFeatures.end());

    // 2. Текстурные признаки из области маски (128 признаков)
    // Применяем маску к изображению
    QImage maskedImage = image.copy();
    for (int y = 0; y < maskedImage.height(); ++y) {
        for (int x = 0; x < maskedImage.width(); ++x) {
            if (qGray(mask.pixel(x, y)) == 0) {
                maskedImage.setPixel(x, y, qRgb(0, 0, 0));
            }
        }
    }

    // Сохраняем временно для анализа текстуры
    QString tempPath = "/tmp/aurcad_masked.jpg";
    maskedImage.save(tempPath);
    std::vector<double> textureFeatures = extractTextureFeatures(tempPath);
    features.insert(features.end(), textureFeatures.begin(), textureFeatures.end());

    // 3. Признаки формы (10 признаков)
    std::vector<double> shapeFeatures = extractShapeFeatures(polygon);
    features.insert(features.end(), shapeFeatures.begin(), shapeFeatures.end());

    // 4. Дополняем до FEATURE_DIM нулями
    while (features.size() < FEATURE_DIM) {
        features.push_back(0.0);
    }

    // Обрезаем до FEATURE_DIM если превысили
    if (features.size() > FEATURE_DIM) {
        features.resize(FEATURE_DIM);
    }

    qDebug() << "Extracted" << features.size() << "features with mask";

    return features;
}

std::vector<double> ImageProcessor::extractColorFeaturesFromMask(
    const QImage &image,
    const QImage &mask)
{
    std::vector<double> colorFeatures;

    // Гистограммы RGB только для области внутри маски
    std::vector<int> histR(64, 0);
    std::vector<int> histG(64, 0);
    std::vector<int> histB(64, 0);

    int pixelCount = 0;
    double sumR = 0, sumG = 0, sumB = 0;

    for (int y = 0; y < image.height() && y < mask.height(); ++y) {
        for (int x = 0; x < image.width() && x < mask.width(); ++x) {
            // Проверяем, находится ли пиксель внутри маски
            if (qGray(mask.pixel(x, y)) > 128) {
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

                sumR += r;
                sumG += g;
                sumB += b;
                pixelCount++;
            }
        }
    }

    if (pixelCount == 0) {
        pixelCount = 1; // Избегаем деления на ноль
    }

    // Нормализуем гистограммы
    for (int i = 0; i < 64; ++i) {
        colorFeatures.push_back(static_cast<double>(histR[i]) / pixelCount);
        colorFeatures.push_back(static_cast<double>(histG[i]) / pixelCount);
        colorFeatures.push_back(static_cast<double>(histB[i]) / pixelCount);
    }

    // Средние значения цветов
    colorFeatures.push_back((sumR / pixelCount) / 255.0);
    colorFeatures.push_back((sumG / pixelCount) / 255.0);
    colorFeatures.push_back((sumB / pixelCount) / 255.0);

    // Дополнительно: HSV статистики
    // Конвертируем средний цвет в HSV
    QColor avgColor(sumR / pixelCount, sumG / pixelCount, sumB / pixelCount);
    colorFeatures.push_back(avgColor.hsvHue() / 360.0);
    colorFeatures.push_back(avgColor.hsvSaturation() / 255.0);
    colorFeatures.push_back(avgColor.value() / 255.0);

    return colorFeatures;
}

std::vector<double> ImageProcessor::extractShapeFeatures(
    const SegmentationData::Polygon &polygon)
{
    std::vector<double> shapeFeatures;

    // 1. Площадь полигона (нормализованная)
    shapeFeatures.push_back(polygon.area / 1000000.0); // Нормализуем

    // 2. Периметр
    double perimeter = 0.0;
    for (int i = 0; i < polygon.points.size(); ++i) {
        int j = (i + 1) % polygon.points.size();
        QPointF p1 = polygon.points[i];
        QPointF p2 = polygon.points[j];
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        perimeter += std::sqrt(dx * dx + dy * dy);
    }
    shapeFeatures.push_back(perimeter / 1000.0); // Нормализуем

    // 3. Круглость (circularity)
    double circularity = calculateCircularity(polygon);
    shapeFeatures.push_back(circularity);

    // 4. Aspect ratio bounding box
    double aspectRatio = calculateAspectRatio(polygon.boundingBox);
    shapeFeatures.push_back(aspectRatio);

    // 5. Компактность (площадь / площадь bounding box)
    double bbArea = polygon.boundingBox.width() * polygon.boundingBox.height();
    double compactness = (bbArea > 0) ? (polygon.area / bbArea) : 0.0;
    shapeFeatures.push_back(compactness);

    // 6. Количество вершин (нормализованное)
    shapeFeatures.push_back(polygon.points.size() / 100.0);

    // 7-10. Резерв
    shapeFeatures.push_back(0.0);
    shapeFeatures.push_back(0.0);
    shapeFeatures.push_back(0.0);
    shapeFeatures.push_back(0.0);

    return shapeFeatures;
}

double ImageProcessor::calculateCircularity(const SegmentationData::Polygon &polygon)
{
    // Circularity = 4π * Area / Perimeter²
    // Для круга = 1.0

    double perimeter = 0.0;
    for (int i = 0; i < polygon.points.size(); ++i) {
        int j = (i + 1) % polygon.points.size();
        QPointF p1 = polygon.points[i];
        QPointF p2 = polygon.points[j];
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        perimeter += std::sqrt(dx * dx + dy * dy);
    }

    if (perimeter == 0) {
        return 0.0;
    }

    double circularity = (4.0 * M_PI * polygon.area) / (perimeter * perimeter);

    return std::min(circularity, 1.0); // Ограничиваем [0, 1]
}

double ImageProcessor::calculateAspectRatio(const QRectF &rect)
{
    if (rect.height() == 0) {
        return 1.0;
    }

    return rect.width() / rect.height();
}

QImage ImageProcessor::convertToGrayscale(const QImage &image)
{
    return image.convertToFormat(QImage::Format_Grayscale8);
}

QVector<QRectF> ImageProcessor::detectApplesYOLO(const QString &imagePath)
{
    QVector<QRectF> detections;

    // TODO: Интеграция YOLO11 через OpenCV DNN или ONNX Runtime
    // Пока возвращаем пустой список или используем polygon из labelme

    // Проверяем, есть ли аннотация для этого изображения
    QString imageName = QFileInfo(imagePath).fileName();

    if (m_annotations.contains(imageName)) {
        const auto &annotation = m_annotations[imageName];

        for (const auto &polygon : annotation.polygons) {
            if (polygon.label.toLower() == "apple") {
                detections.append(polygon.boundingBox);
            }
        }

        qDebug() << "Found" << detections.size() << "apples from annotations";
    }

    return detections;
}

bool ImageProcessor::loadYOLOModel(const QString &modelPath)
{
    // TODO: Загрузка YOLO11 модели
    // Потребуется OpenCV DNN или ONNX Runtime

    qDebug() << "YOLO model loading not yet implemented:" << modelPath;

    return false;
}
