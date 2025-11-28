#include "AppleDetector.h"
#include "ImageProcessor.h"
#include "AppleClassifier.h"
#include "CameraHandler.h"
#include <QDebug>
#include <QThread>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>
#include <QFile>

AppleDetector::AppleDetector(QObject *parent)
    : QObject(parent)
    , m_imageProcessor(new ImageProcessor())
    , m_classifier(new AppleClassifier())
    , m_cameraHandler(new CameraHandler(this))
    , m_isProcessing(false)
    , m_modelTrained(false)
    , m_useSegmentation(false)
    , m_realtimeAnalysis(false)
{
    // Подключаем сигналы от камеры
    connect(m_cameraHandler, &CameraHandler::imageCaptured,
            this, &AppleDetector::analyzeImage);
    connect(m_cameraHandler, &CameraHandler::frameAvailable,
            this, &AppleDetector::onCameraFrameAvailable);
    connect(m_cameraHandler, &CameraHandler::errorOccurred,
            this, &AppleDetector::onCameraError);
    
    qDebug() << "[AppleDetector::constructor] ========== AppleDetector initialized ==========";
    
    // Пытаемся автоматически загрузить сохранённую модель
    QString modelPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                       + "/apple_model.dat";
    
    qDebug() << "[AppleDetector::constructor] Checking for saved model at:" << modelPath;
    
    if (QFileInfo::exists(modelPath)) {
        qDebug() << "[AppleDetector::constructor] ✓ Found saved model file, attempting to load...";
        loadModel(modelPath);
    } else {
        qDebug() << "[AppleDetector::constructor] ⚠ No saved model found. Training will be required.";
    }
}

AppleDetector::~AppleDetector()
{
    delete m_imageProcessor;
    delete m_classifier;
}

void AppleDetector::analyzeImage(const QString &imagePath)
{
    qDebug() << "Analyzing image:" << imagePath;

    if (m_isProcessing) {
        emit errorOccurred("Already processing another image");
        return;
    }

    if (!QFileInfo::exists(imagePath)) {
        emit errorOccurred("Image file not found: " + imagePath);
        return;
    }

    if (!m_classifier->isTrained()) {
        emit errorOccurred("Model not trained. Please train or load a model first.");
        return;
    }

    setIsProcessing(true);

    try {
        // Проверяем наличие яблока на изображении
        if (!m_imageProcessor->detectApple(imagePath)) {
            setLastResult("не яблоко");
            emit analysisComplete("не яблоко", 0.95f);
            setIsProcessing(false);
            return;
        }

        // Извлекаем признаки
        std::vector<double> features = m_imageProcessor->extractFeatures(imagePath, m_useSegmentation);

        // Классифицируем
        float confidence = 0.0f;
        AppleClassifier::AppleQuality quality = m_classifier->predict(features, confidence);

        QString result = AppleClassifier::qualityToString(quality);
        setLastResult(result);

        qDebug() << "Analysis result:" << result << "confidence:" << confidence;

        emit analysisComplete(result, confidence);

    } catch (const std::exception &e) {
        emit errorOccurred(QString("Error during analysis: %1").arg(e.what()));
    }

    setIsProcessing(false);
}

void AppleDetector::trainModel(const QString &datasetPath, bool usePolygonData)
{
    qDebug() << "[AppleDetector::trainModel] ========== TRAINING STARTED ==========";
    qDebug() << "[AppleDetector::trainModel] Dataset path:" << datasetPath;
    qDebug() << "[AppleDetector::trainModel] Use polygon data:" << usePolygonData;

    if (m_isProcessing) {
        qWarning() << "[AppleDetector::trainModel] ❌ Already processing";
        emit errorOccurred("Already processing");
        return;
    }

    qDebug() << "[AppleDetector::trainModel] Checking if dataset directory exists...";
    if (!QDir(datasetPath).exists()) {
        qWarning() << "[AppleDetector::trainModel] ❌ Dataset directory not found:" << datasetPath;
        emit errorOccurred("Dataset directory not found: " + datasetPath);
        return;
    }
    qDebug() << "[AppleDetector::trainModel] ✓ Dataset directory exists";

    setIsProcessing(true);
    emit trainingProgress(0, "Loading dataset...");

    try {
        qDebug() << "[AppleDetector::trainModel] Loading dataset...";
        // Загружаем датасет
        QDir datasetDir(datasetPath);
        QStringList imageFiles = datasetDir.entryList(QStringList() << "*.jpg" << "*.png", QDir::Files);

        qDebug() << "[AppleDetector::trainModel] Found" << imageFiles.size() << "images";

        if (imageFiles.isEmpty()) {
            qWarning() << "[AppleDetector::trainModel] ❌ No images found in dataset directory";
            emit errorOccurred("No images found in dataset directory");
            setIsProcessing(false);
            return;
        }

        // Если используем polygon данные, загружаем аннотации
        if (usePolygonData) {
            QString annotationsDir = datasetDir.absoluteFilePath("../labelme");
            if (QDir(annotationsDir).exists()) {
                m_imageProcessor->setAnnotationsDirectory(annotationsDir);
                qDebug() << "Annotations directory set:" << annotationsDir;
            } else {
                qWarning() << "Annotations directory not found, falling back to regular feature extraction";
                usePolygonData = false;
            }
        }

        // Загружаем метки из classes.txt
        QString classesFile = datasetDir.absoluteFilePath("../classes.txt");
        qDebug() << "[AppleDetector::trainModel] Looking for classes file:" << classesFile;
        QFile file(classesFile);
        QMap<QString, int> imageLabels;

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "[AppleDetector::trainModel] ✓ Classes file opened successfully";
            QTextStream in(&file);
            int goodLabels = 0, badLabels = 0;
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty()) {
                    QStringList parts = line.split(QRegExp("\\s+"));
                    if (parts.size() >= 2) {
                        QString imageName = parts[0];
                        QString label = parts[1];
                        int labelValue = (label == "хорошее") ? 0 : 1;
                        imageLabels[imageName] = labelValue;
                        if (labelValue == 0) goodLabels++;
                        else badLabels++;
                    }
                }
            }
            file.close();
            qDebug() << "[AppleDetector::trainModel] Loaded" << imageLabels.size() << "image labels (Good:" 
                     << goodLabels << "Bad:" << badLabels << ")";
        } else {
            qWarning() << "[AppleDetector::trainModel] ⚠ Could not open classes file:" << classesFile;
        }

        qDebug() << "[AppleDetector::trainModel] Starting feature extraction...";
        emit trainingProgress(20, "Extracting features...");

        // Извлекаем признаки из всех изображений
        std::vector<std::vector<double>> features;
        std::vector<int> labels;

        int processedImages = 0;
        for (const QString &imageFile : imageFiles) {
            QString imagePath = datasetDir.absoluteFilePath(imageFile);

            try {
                // Проверяем наличие метки перед извлечением признаков
                int label = imageLabels.value(imageFile, -1); // -1 = нет метки
                if (label == -1) {
                    qWarning() << "[AppleDetector::trainModel] ⚠ No label found for image:" << imageFile << ", skipping";
                    continue;
                }
                
                if (processedImages == 0 || processedImages % 10 == 0) {
                    qDebug() << "[AppleDetector::trainModel] Processing image" << (processedImages + 1) << "/" << imageFiles.size() 
                             << ":" << imageFile << "label:" << (label == 0 ? "GOOD" : "BAD");
                }
                
                std::vector<double> imageFeatures = m_imageProcessor->extractFeatures(imagePath, usePolygonData);
                features.push_back(imageFeatures);
                labels.push_back(label);

                processedImages++;
                int progress = 20 + (processedImages * 50 / imageFiles.size());
                emit trainingProgress(progress, QString("Processing %1/%2 images...")
                                     .arg(processedImages).arg(imageFiles.size()));

            } catch (const std::exception &e) {
                qWarning() << "[AppleDetector::trainModel] Error processing image" << imageFile << ":" << e.what();
            }
        }

        qDebug() << "[AppleDetector::trainModel] Feature extraction complete.";
        qDebug() << "[AppleDetector::trainModel] Extracted features from" << features.size() << "images";

        if (features.empty()) {
            qWarning() << "[AppleDetector::trainModel] ❌ Failed to extract features from any image";
            emit errorOccurred("Failed to extract features from any image");
            setIsProcessing(false);
            return;
        }

        qDebug() << "[AppleDetector::trainModel] Starting model training...";
        emit trainingProgress(70, "Training model...");

        // Обучаем модель
        float accuracy = m_classifier->train(features, labels);

        qDebug() << "[AppleDetector::trainModel] ✓ Training completed! Accuracy:" << accuracy;
        setModelTrained(true);
        
        // Автоматически сохраняем обученную модель
        QString modelPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                           + "/apple_model.dat";
        QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        
        qDebug() << "[AppleDetector::trainModel] Auto-saving model...";
        qDebug() << "[AppleDetector::trainModel] AppDataLocation:" << appDataDir;
        qDebug() << "[AppleDetector::trainModel] Model path:" << modelPath;
        
        // Создаём директорию, если её нет
        if (!QDir(appDataDir).exists()) {
            qDebug() << "[AppleDetector::trainModel] Creating directory:" << appDataDir;
            QDir().mkpath(appDataDir);
        }
        
        if (m_classifier->saveModel(modelPath)) {
            qDebug() << "[AppleDetector::trainModel] ✅ Model auto-saved successfully to:" << modelPath;
        } else {
            qWarning() << "[AppleDetector::trainModel] ❌ Failed to auto-save model to:" << modelPath;
        }

        emit trainingProgress(100, "Training complete");
        emit trainingComplete(true, accuracy);

        qDebug() << "Training completed with accuracy:" << accuracy;

    } catch (const std::exception &e) {
        emit errorOccurred(QString("Error during training: %1").arg(e.what()));
        emit trainingComplete(false, 0.0f);
    }

    setIsProcessing(false);
}

void AppleDetector::loadModel(const QString &modelPath)
{
    qDebug() << "[AppleDetector::loadModel] ========== LOAD MODEL REQUEST ==========";
    qDebug() << "[AppleDetector::loadModel] Path:" << modelPath;

    if (!QFileInfo::exists(modelPath)) {
        qWarning() << "[AppleDetector::loadModel] ❌ Model file not found:" << modelPath;
        emit errorOccurred("Model file not found: " + modelPath);
        return;
    }

    qDebug() << "[AppleDetector::loadModel] File exists, delegating to AppleClassifier...";
    
    if (m_classifier->loadModel(modelPath)) {
        setModelTrained(true);
        qDebug() << "[AppleDetector::loadModel] ✅ Model loaded and ready for use!";
    } else {
        qWarning() << "[AppleDetector::loadModel] ❌ Failed to load model (AppleClassifier returned false)";
        emit errorOccurred("Failed to load model");
    }
}

void AppleDetector::saveModel(const QString &modelPath)
{
    qDebug() << "Saving model to:" << modelPath;

    if (!m_classifier->isTrained()) {
        emit errorOccurred("No trained model to save");
        return;
    }

    if (m_classifier->saveModel(modelPath)) {
        qDebug() << "Model saved successfully";
    } else {
        emit errorOccurred("Failed to save model");
    }
}

void AppleDetector::setIsProcessing(bool processing)
{
    if (m_isProcessing != processing) {
        m_isProcessing = processing;
        emit isProcessingChanged();
    }
}

void AppleDetector::setLastResult(const QString &result)
{
    if (m_lastResult != result) {
        m_lastResult = result;
        emit lastResultChanged();
    }
}

void AppleDetector::setModelTrained(bool trained)
{
    if (m_modelTrained != trained) {
        m_modelTrained = trained;
        emit modelTrainedChanged();
    }
}

void AppleDetector::setUseSegmentation(bool use)
{
    if (m_useSegmentation != use) {
        m_useSegmentation = use;
        emit useSegmentationChanged();
        qDebug() << "Use segmentation set to:" << use;
    }
}

void AppleDetector::analyzeImageWithSegmentation(const QString &imagePath)
{
    // Временно сохраняем текущее значение
    bool oldValue = m_useSegmentation;
    
    // Включаем сегментацию для этого анализа
    m_useSegmentation = true;
    
    // Выполняем анализ
    analyzeImage(imagePath);
    
    // Восстанавливаем значение
    m_useSegmentation = oldValue;
}

void AppleDetector::analyzeCameraFrame(const QImage &frame)
{
    if (!m_realtimeAnalysis || !m_modelTrained) {
        return;
    }

    if (m_isProcessing) {
        return; // Пропускаем кадр, если уже обрабатываем
    }

    try {
        // Сохраняем кадр во временный файл для анализа
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                          + "/aurcad_frame_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";
        
        if (frame.save(tempPath, "JPEG", 85)) {
            // Анализируем изображение
            analyzeImage(tempPath);
            
            // Удаляем временный файл
            QFile::remove(tempPath);
        }
    } catch (const std::exception &e) {
        qWarning() << "Error analyzing camera frame:" << e.what();
    }
}

void AppleDetector::setRealtimeAnalysis(bool enabled)
{
    if (m_realtimeAnalysis != enabled) {
        m_realtimeAnalysis = enabled;
        qDebug() << "Realtime analysis set to:" << enabled;
    }
}

void AppleDetector::onCameraFrameAvailable(const QImage &frame)
{
    // Перенаправляем на analyzeCameraFrame
    analyzeCameraFrame(frame);
}

void AppleDetector::onCameraError(const QString &error)
{
    // Перенаправляем ошибку камеры
    emit errorOccurred("Camera error: " + error);
}
