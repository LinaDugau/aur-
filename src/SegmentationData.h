#ifndef SEGMENTATIONDATA_H
#define SEGMENTATIONDATA_H

#include <QtCore>
#include <QtGui>
#include <QString>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QFileInfo>
#include <QImage>

/**
 * @brief Класс для работы с данными сегментации из LabelMe
 */
class SegmentationData
{
public:
    struct Polygon {
        QString label;              // "apple"
        QVector<QPointF> points;    // Точки полигона
        QRectF boundingBox;         // Bounding box (QRectF from QtCore)
        double area;                // Площадь полигона
    };

    struct ImageAnnotation {
        QString imagePath;
        int imageWidth;
        int imageHeight;
        QVector<Polygon> polygons;
    };

    SegmentationData();

    /**
     * @brief Загружает аннотации из JSON файла LabelMe
     */
    bool loadFromJson(const QString &jsonPath);

    /**
     * @brief Загружает все аннотации из директории
     */
    QMap<QString, ImageAnnotation> loadFromDirectory(const QString &dirPath);

    /**
     * @brief Проверяет, содержит ли изображение яблоко
     */
    bool hasApple() const;

    /**
     * @brief Возвращает аннотацию
     */
    ImageAnnotation annotation() const { return m_annotation; }

    /**
     * @brief Извлекает маску из полигона
     */
    static QImage createMaskFromPolygon(const Polygon &polygon, int width, int height);

    /**
     * @brief Вычисляет IoU (Intersection over Union) между двумя полигонами
     */
    static double calculateIoU(const Polygon &p1, const Polygon &p2);

private:
    ImageAnnotation m_annotation;

    static QRectF calculateBoundingBox(const QVector<QPointF> &points);
    static double calculatePolygonArea(const QVector<QPointF> &points);
};

#endif // SEGMENTATIONDATA_H
