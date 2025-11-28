#include "YOLO11Segmentation.h"
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <cmath>
#include <algorithm>

YOLO11Segmentation::YOLO11Segmentation()
{
    m_modelLoaded = false;
}

YOLO11Segmentation::~YOLO11Segmentation()
{
}

bool YOLO11Segmentation::loadModel(const QString &modelPath)
{
    qDebug() << "Loading YOLO11-segm model from:" << modelPath;
    
    if (!ONNXInference::loadModel(modelPath)) {
        return false;
    }

    // TODO: Реальная загрузка через ONNX Runtime
    // Здесь должна быть инициализация ONNX Runtime сессии
    
    qDebug() << "YOLO11-segm model loaded successfully";
    return true;
}

QVector<ONNXInference::SegmentationResult> YOLO11Segmentation::segmentImage(
    const QString &imagePath, float confThreshold, float iouThreshold)
{
    QImage image(imagePath);
    if (image.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        return QVector<SegmentationResult>();
    }
    
    return segmentImage(image, confThreshold, iouThreshold);
}

QVector<ONNXInference::SegmentationResult> YOLO11Segmentation::segmentImage(
    const QImage &image, float confThreshold, float iouThreshold)
{
    QVector<SegmentationResult> results;
    
    if (!m_modelLoaded) {
        qWarning() << "YOLO11-segm model not loaded";
        return results;
    }

    if (image.isNull()) {
        qWarning() << "Input image is null";
        return results;
    }

    QSize originalSize = image.size();
    
    // Предобработка
    std::vector<float> inputData = preprocessImage(image, MODEL_SIZE);
    std::vector<int64_t> inputShape = {1, 3, MODEL_SIZE, MODEL_SIZE};
    
    // Инференс
    std::vector<float> outputData = runInference(inputData, inputShape);
    
    if (outputData.empty()) {
        qWarning() << "Inference returned empty output";
        return results;
    }

    // Постобработка детекций
    QVector<Detection> detections = postprocessDetections(outputData, originalSize, 
                                                          MODEL_SIZE, confThreshold);
    
    // Применяем NMS
    detections = applyNMS(detections, iouThreshold);
    
    // Извлекаем маски
    QVector<QImage> masks = extractMasks(outputData, detections, originalSize, MODEL_SIZE);
    
    // Объединяем детекции и маски
    for (int i = 0; i < detections.size() && i < masks.size(); ++i) {
        SegmentationResult result;
        result.detection = detections[i];
        result.mask = masks[i];
        result.polygon = maskToPolygon(masks[i]);
        results.append(result);
    }
    
    qDebug() << "YOLO11-segm found" << results.size() << "objects";
    return results;
}

std::vector<float> YOLO11Segmentation::runInference(
    const std::vector<float> &inputData, const std::vector<int64_t> &inputShape)
{
    // TODO: Реальный инференс через ONNX Runtime
    // Ort::Value inputTensor = Ort::Value::CreateTensor<float>(...);
    // auto outputTensors = session.Run(Ort::RunOptions{nullptr}, ...);
    
    qDebug() << "YOLO11-segm inference placeholder";
    qDebug() << "Input shape:" << inputShape[0] << inputShape[1] 
             << inputShape[2] << inputShape[3];
    
    // Заглушка: возвращаем пустой вектор
    // В реальной реализации здесь будет вызов ONNX Runtime
    std::vector<float> outputData;
    
    return outputData;
}

QVector<ONNXInference::Detection> YOLO11Segmentation::postprocessDetections(
    const std::vector<float> &outputData,
    const QSize &originalSize,
    int modelSize,
    float confThreshold)
{
    QVector<Detection> detections;
    
    if (outputData.empty()) {
        return detections;
    }

    // YOLO11-segm выходной формат:
    // [batch, num_detections, 4 + num_classes + mask_channels]
    // Где 4 - это bbox (x, y, w, h), num_classes - вероятности классов,
    // mask_channels - каналы для маски
    
    // TODO: Парсинг реальных выходных данных ONNX модели
    // Здесь нужно знать точную структуру выходных тензоров
    
    qDebug() << "YOLO11-segm postprocessing placeholder";
    qDebug() << "Output size:" << outputData.size();
    qDebug() << "Original image size:" << originalSize;
    
    // Заглушка для демонстрации структуры
    // В реальной реализации здесь будет парсинг тензоров
    
    return detections;
}

QVector<QImage> YOLO11Segmentation::extractMasks(
    const std::vector<float> &outputData,
    const QVector<Detection> &detections,
    const QSize &originalSize,
    int modelSize)
{
    QVector<QImage> masks;
    
    // TODO: Извлечение масок из выходных данных модели
    // YOLO11-segm возвращает прототипы масок, которые нужно обработать
    
    for (const auto &detection : detections) {
        // Создаем пустую маску
        QImage mask(originalSize.width(), originalSize.height(), QImage::Format_Grayscale8);
        mask.fill(0);
        masks.append(mask);
    }
    
    return masks;
}

QVector<ONNXInference::Detection> YOLO11Segmentation::applyNMS(
    const QVector<Detection> &detections, float iouThreshold)
{
    if (detections.isEmpty()) {
        return detections;
    }

    // Сортируем по уверенности (от большего к меньшему)
    QVector<Detection> sorted = detections;
    std::sort(sorted.begin(), sorted.end(),
              [](const Detection &a, const Detection &b) {
                  return a.confidence > b.confidence;
              });

    QVector<Detection> result;
    QVector<bool> suppressed(sorted.size(), false);

    for (int i = 0; i < sorted.size(); ++i) {
        if (suppressed[i]) {
            continue;
        }

        result.append(sorted[i]);

        // Подавляем перекрывающиеся детекции
        for (int j = i + 1; j < sorted.size(); ++j) {
            if (suppressed[j]) {
                continue;
            }

            float iou = calculateIoU(sorted[i].bbox, sorted[j].bbox);
            if (iou > iouThreshold) {
                suppressed[j] = true;
            }
        }
    }

    return result;
}

float YOLO11Segmentation::calculateIoU(const QRectF &box1, const QRectF &box2)
{
    float x1 = std::max(box1.x(), box2.x());
    float y1 = std::max(box1.y(), box2.y());
    float x2 = std::min(box1.x() + box1.width(), box2.x() + box2.width());
    float y2 = std::min(box1.y() + box1.height(), box2.y() + box2.height());

    if (x2 <= x1 || y2 <= y1) {
        return 0.0f;
    }

    float intersection = (x2 - x1) * (y2 - y1);
    float area1 = box1.width() * box1.height();
    float area2 = box2.width() * box2.height();
    float unionArea = area1 + area2 - intersection;

    if (unionArea <= 0) {
        return 0.0f;
    }

    return intersection / unionArea;
}

QVector<QPointF> YOLO11Segmentation::maskToPolygon(const QImage &mask, float threshold)
{
    // Используем метод из базового класса
    return ONNXInference::extractPolygonFromMask(mask, threshold);
}

QImage YOLO11Segmentation::processMaskProto(
    const std::vector<float> &maskProto,
    const Detection &detection,
    const QSize &originalSize,
    int modelSize)
{
    // TODO: Обработка прототипов масок YOLO11-segm
    // Это требует знания структуры выходных данных модели
    
    QImage mask(originalSize.width(), originalSize.height(), QImage::Format_Grayscale8);
    mask.fill(0);
    return mask;
}

