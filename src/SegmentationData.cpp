#include "SegmentationData.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QDebug>
#include <QImage>
#include <QRectF>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <cmath>

SegmentationData::SegmentationData()
{
}

bool SegmentationData::loadFromJson(const QString &jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open JSON file:" << jsonPath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON format";
        return false;
    }

    QJsonObject root = doc.object();

    // Извлекаем информацию об изображении
    m_annotation.imagePath = root["imagePath"].toString();
    m_annotation.imageWidth = root["imageWidth"].toInt();
    m_annotation.imageHeight = root["imageHeight"].toInt();

    // Извлекаем полигоны
    QJsonArray shapes = root["shapes"].toArray();
    for (const QJsonValue &shapeValue : shapes) {
        QJsonObject shapeObj = shapeValue.toObject();

        Polygon polygon;
        polygon.label = shapeObj["label"].toString();

        // Извлекаем точки полигона
        QJsonArray pointsArray = shapeObj["points"].toArray();
        for (const QJsonValue &pointValue : pointsArray) {
            QJsonArray point = pointValue.toArray();
            if (point.size() >= 2) {
                double x = point[0].toDouble();
                double y = point[1].toDouble();
                polygon.points.append(QPointF(x, y));
            }
        }

        if (!polygon.points.isEmpty()) {
            polygon.boundingBox = SegmentationData::calculateBoundingBox(polygon.points);
            polygon.area = SegmentationData::calculatePolygonArea(polygon.points);
            m_annotation.polygons.append(polygon);
        }
    }

    qDebug() << "Loaded" << m_annotation.polygons.size() << "polygons from" << jsonPath;

    return true;
}

QMap<QString, SegmentationData::ImageAnnotation> SegmentationData::loadFromDirectory(const QString &dirPath)
{
    QMap<QString, ImageAnnotation> annotations;

    QDir dir(dirPath);
    QStringList jsonFiles = dir.entryList(QStringList() << "*.json", QDir::Files);

    for (const QString &jsonFile : jsonFiles) {
        QString jsonPath = dir.absoluteFilePath(jsonFile);
        SegmentationData segData;

        if (segData.loadFromJson(jsonPath)) {
            // Извлекаем имя изображения из пути
            QString imageName = QFileInfo(segData.annotation().imagePath).fileName();
            annotations[imageName] = segData.annotation();
        }
    }

    qDebug() << "Loaded" << annotations.size() << "annotations from directory";

    return annotations;
}

bool SegmentationData::hasApple() const
{
    for (const Polygon &polygon : m_annotation.polygons) {
        if (polygon.label.toLower() == "apple") {
            return true;
        }
    }
    return false;
}

QImage SegmentationData::createMaskFromPolygon(const Polygon &polygon, int width, int height)
{
    QImage mask(width, height, QImage::Format_Grayscale8);
    mask.fill(Qt::black);

    QPainter painter(&mask);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);

    QPolygonF qPolygon(polygon.points);
    painter.drawPolygon(qPolygon);

    return mask;
}

double SegmentationData::calculateIoU(const Polygon &p1, const Polygon &p2)
{
    // Упрощенная версия: сравниваем bounding boxes
    QRectF box1 = p1.boundingBox;
    QRectF box2 = p2.boundingBox;

    QRectF intersection = box1.intersected(box2);
    double intersectionArea = intersection.width() * intersection.height();

    double unionArea = p1.area + p2.area - intersectionArea;

    if (unionArea == 0) {
        return 0.0;
    }

    return intersectionArea / unionArea;
}

QRectF SegmentationData::calculateBoundingBox(const QVector<QPointF> &points)
{
    if (points.isEmpty()) {
        return QRectF();
    }

    double minX = static_cast<double>(points[0].x());
    double minY = static_cast<double>(points[0].y());
    double maxX = minX;
    double maxY = minY;

    for (const QPointF &point : points) {
        minX = std::min(minX, static_cast<double>(point.x()));
        minY = std::min(minY, static_cast<double>(point.y()));
        maxX = std::max(maxX, static_cast<double>(point.x()));
        maxY = std::max(maxY, static_cast<double>(point.y()));
    }

    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

double SegmentationData::calculatePolygonArea(const QVector<QPointF> &points)
{
    if (points.size() < 3) {
        return 0.0;
    }

    // Shoelace formula
    double area = 0.0;
    int n = points.size();

    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += points[i].x() * points[j].y();
        area -= points[j].x() * points[i].y();
    }

    return std::abs(area) / 2.0;
}
