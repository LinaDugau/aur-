# Использование YOLO11-segm и YOLACT для анализа изображений

## Обзор

Проект теперь поддерживает анализ изображений с использованием:
- **YOLO11-segm** - сегментация объектов с помощью YOLO11
- **YOLACT** - сегментация объектов с помощью YOLACT

Оба метода работают через **ONNX Runtime** без использования OpenCV4.

## Структура классов

### ONNXInference (базовый класс)
- `ONNXInference.h/cpp` - базовый класс для работы с ONNX моделями
- Предобработка изображений (letterbox resize, нормализация)
- Постобработка результатов

### YOLO11Segmentation
- `YOLO11Segmentation.h/cpp` - класс для работы с YOLO11-segm
- Детекция и сегментация объектов
- Non-Maximum Suppression (NMS)
- Извлечение масок и полигонов

### YOLACTInference
- `YOLACTInference.h/cpp` - класс для работы с YOLACT
- Детекция и сегментация объектов
- Декодирование масок из прототипов

## Использование в коде

### Загрузка моделей

```cpp
ImageProcessor* processor = new ImageProcessor();

// Загрузка YOLO11-segm модели
bool success = processor->loadYOLO11Model("/path/to/yolo11-segm.onnx");

// Загрузка YOLACT модели
bool success = processor->loadYOLACTModel("/path/to/yolact.onnx");
```

### Выполнение сегментации

```cpp
// Сегментация с YOLO11-segm
QVector<ONNXInference::SegmentationResult> results = 
    processor->segmentWithYOLO11("/path/to/image.jpg");

for (const auto& result : results) {
    QRectF bbox = result.detection.bbox;
    float confidence = result.detection.confidence;
    QImage mask = result.mask;
    QVector<QPointF> polygon = result.polygon;
    
    qDebug() << "Found object:" << result.detection.className
             << "confidence:" << confidence;
}

// Сегментация с YOLACT
QVector<ONNXInference::SegmentationResult> results = 
    processor->segmentWithYOLACT("/path/to/image.jpg");
```

### Прямое использование классов

```cpp
// YOLO11-segm
YOLO11Segmentation yolo11;
yolo11.loadModel("/path/to/yolo11-segm.onnx");
QVector<ONNXInference::SegmentationResult> results = 
    yolo11.segmentImage("/path/to/image.jpg", 0.25f, 0.45f);

// YOLACT
YOLACTInference yolact;
yolact.loadModel("/path/to/yolact.onnx");
QVector<ONNXInference::SegmentationResult> results = 
    yolact.segmentImage("/path/to/image.jpg", 0.15f);
```

## Формат результатов

```cpp
struct SegmentationResult {
    Detection detection;        // Детекция с bbox
    QImage mask;                // Маска сегментации
    QVector<QPointF> polygon;   // Полигон из маски
};

struct Detection {
    QRectF bbox;                // Bounding box
    float confidence;           // Уверенность детекции
    int classId;                // ID класса
    QString className;          // Имя класса
};
```

## Зависимости

Проект использует:
- **ONNX Runtime** (через Conan: `onnxruntime/1.16.3`)
- **Qt** для обработки изображений (без OpenCV)

## Примечания

1. **ONNX Runtime интеграция**: Текущая реализация содержит заглушки для ONNX Runtime. 
   Для полной функциональности необходимо:
   - Установить ONNX Runtime через Conan
   - Раскомментировать и реализовать методы `runInference()` в классах
   - Настроить правильную структуру входных/выходных тензоров

2. **Предобработка**: Изображения автоматически обрабатываются:
   - Letterbox resize (сохранение соотношения сторон)
   - Нормализация [0, 255] -> [0, 1]
   - Преобразование HWC -> CHW

3. **Постобработка**: 
   - YOLO11-segm использует NMS для фильтрации дубликатов
   - Маски конвертируются в полигоны для дальнейшей обработки

## Пример интеграции в AppleDetector

```cpp
// В AppleDetector можно использовать:
QVector<QRectF> boxes = m_imageProcessor->detectApplesYOLO(imagePath);
// Метод автоматически использует загруженные модели YOLO11-segm или YOLACT
```

