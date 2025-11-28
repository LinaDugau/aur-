#include "ONNXInference.h"
#include <QDebug>
#include <QPainter>
#include <QFileInfo>
#include <cmath>
#include <algorithm>

// Заглушка для ONNX Runtime - в реальной реализации нужно подключить onnxruntime
// Для компиляции без ONNX Runtime используем заглушку
#ifndef ONNXRUNTIME_AVAILABLE
#define ONNXRUNTIME_AVAILABLE 0
#endif

ONNXInference::ONNXInference()
    : m_modelLoaded(false)
{
}

ONNXInference::~ONNXInference()
{
}

bool ONNXInference::loadModel(const QString &modelPath)
{
    m_modelPath = modelPath;
    
    // Проверяем существование файла
    if (!QFileInfo::exists(modelPath)) {
        qWarning() << "ONNX model file not found:" << modelPath;
        m_modelLoaded = false;
        return false;
    }

    // TODO: Реальная загрузка ONNX модели через ONNX Runtime
    // Ort::Env env;
    // Ort::Session session(env, modelPath.toStdString().c_str(), ...);
    
    qDebug() << "ONNX model loading placeholder for:" << modelPath;
    qDebug() << "Note: Full ONNX Runtime integration required";
    
    m_modelLoaded = true;
    return true;
}

std::vector<float> ONNXInference::preprocessImage(const QImage &image, int targetSize)
{
    std::vector<float> preprocessed;
    
    if (image.isNull()) {
        qWarning() << "Cannot preprocess null image";
        return preprocessed;
    }

    // Letterbox resize (сохраняет соотношение сторон)
    float scale;
    int padX, padY;
    QImage resized = letterboxImage(image, targetSize, scale, padX, padY);

    // Конвертируем в RGB если нужно
    QImage rgbImage = resized.convertToFormat(QImage::Format_RGB888);

    // Размер: [1, 3, targetSize, targetSize]
    preprocessed.resize(1 * 3 * targetSize * targetSize);

    // Нормализация: [0, 255] -> [0, 1]
    // И перестановка: HWC -> CHW
    int idx = 0;
    for (int c = 0; c < 3; ++c) {
        for (int y = 0; y < targetSize; ++y) {
            for (int x = 0; x < targetSize; ++x) {
                QRgb pixel = rgbImage.pixel(x, y);
                float value = 0.0f;
                
                if (c == 0) value = qRed(pixel) / 255.0f;      // R
                else if (c == 1) value = qGreen(pixel) / 255.0f; // G
                else value = qBlue(pixel) / 255.0f;              // B
                
                preprocessed[idx++] = value;
            }
        }
    }

    return preprocessed;
}

QImage ONNXInference::letterboxImage(const QImage &image, int targetSize, 
                                    float &scale, int &padX, int &padY)
{
    int origWidth = image.width();
    int origHeight = image.height();
    
    // Вычисляем масштаб
    float scaleW = static_cast<float>(targetSize) / origWidth;
    float scaleH = static_cast<float>(targetSize) / origHeight;
    scale = std::min(scaleW, scaleH);
    
    int newWidth = static_cast<int>(origWidth * scale);
    int newHeight = static_cast<int>(origHeight * scale);
    
    // Resize с сохранением соотношения сторон
    QImage resized = image.scaled(newWidth, newHeight, 
                                  Qt::KeepAspectRatio, 
                                  Qt::SmoothTransformation);
    
    // Вычисляем отступы для центрирования
    padX = (targetSize - newWidth) / 2;
    padY = (targetSize - newHeight) / 2;
    
    // Создаем изображение нужного размера с серым фоном (128, 128, 128)
    QImage result(targetSize, targetSize, QImage::Format_RGB888);
    result.fill(qRgb(114, 114, 114)); // Серый цвет для padding
    
    // Копируем resized изображение в центр
    QPainter painter(&result);
    painter.drawImage(padX, padY, resized);
    painter.end();
    
    return result;
}

QRectF ONNXInference::scaleBbox(const QRectF &bbox, float scale, int padX, int padY, 
                               const QSize &originalSize)
{
    // Преобразуем bbox из координат модели обратно в координаты оригинала
    float x = (bbox.x() - padX) / scale;
    float y = (bbox.y() - padY) / scale;
    float w = bbox.width() / scale;
    float h = bbox.height() / scale;
    
    // Ограничиваем границами изображения
    x = std::max(0.0f, std::min(x, static_cast<float>(originalSize.width())));
    y = std::max(0.0f, std::min(y, static_cast<float>(originalSize.height())));
    w = std::max(0.0f, std::min(w, static_cast<float>(originalSize.width()) - x));
    h = std::max(0.0f, std::min(h, static_cast<float>(originalSize.height()) - y));
    
    return QRectF(x, y, w, h);
}

QVector<QPointF> ONNXInference::extractPolygonFromMask(const QImage &mask, float threshold)
{
    QVector<QPointF> polygon;
    
    if (mask.isNull()) {
        return polygon;
    }

    // Простой алгоритм извлечения контура из маски
    int thresholdValue = static_cast<int>(threshold * 255);
    
    // Находим границы маски
    int minX = mask.width(), minY = mask.height();
    int maxX = 0, maxY = 0;
    
    for (int y = 0; y < mask.height(); ++y) {
        for (int x = 0; x < mask.width(); ++x) {
            if (qGray(mask.pixel(x, y)) > thresholdValue) {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }
    
    // Если маска пустая, возвращаем пустой полигон
    if (minX >= maxX || minY >= maxY) {
        return polygon;
    }
    
    // Создаем упрощенный прямоугольный полигон
    // В реальной реализации можно использовать более сложные алгоритмы
    // (например, алгоритм следования контура или упрощение полигона)
    polygon.append(QPointF(minX, minY));
    polygon.append(QPointF(maxX, minY));
    polygon.append(QPointF(maxX, maxY));
    polygon.append(QPointF(minX, maxY));
    
    return polygon;
}

