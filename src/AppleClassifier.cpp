#include "AppleClassifier.h"
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <algorithm>
#include <cmath>

// TODO: После настройки Conan добавить:
// #include <mlpack/core.hpp>
// #include <mlpack/methods/logistic_regression/logistic_regression.hpp>
// #include <mlpack/methods/random_forest/random_forest.hpp>

AppleClassifier::AppleClassifier()
    : m_trained(false)
    , m_model(nullptr)
    , m_goodThreshold(0.0)
    , m_badThreshold(0.0)
    , m_centroidDistance(0.0)
{
    qDebug() << "AppleClassifier initialized";
}

AppleClassifier::~AppleClassifier()
{
    // TODO: Освободить память модели MLPack
}

float AppleClassifier::train(const std::vector<std::vector<double>> &features,
                             const std::vector<int> &labels)
{
    qDebug() << "Training classifier with" << features.size() << "samples";

    if (features.empty() || labels.empty()) {
        qWarning() << "Empty training data";
        return 0.0f;
    }

    if (features.size() != labels.size()) {
        qWarning() << "Features and labels size mismatch";
        return 0.0f;
    }

    // TODO: Реализовать обучение с MLPack после настройки зависимостей
    // Сейчас используем упрощенную версию - центроидный классификатор
    
    qDebug() << "[AppleClassifier::train] Computing class centroids...";
    
    size_t numFeatures = features[0].size();
    m_goodMean.resize(numFeatures, 0.0);
    m_badMean.resize(numFeatures, 0.0);
    
    int goodCount = 0;
    int badCount = 0;
    
    // Вычисляем средние значения для каждого класса
    for (size_t i = 0; i < features.size(); ++i) {
        if (labels[i] == GOOD) {
            for (size_t j = 0; j < numFeatures; ++j) {
                m_goodMean[j] += features[i][j];
            }
            goodCount++;
        } else if (labels[i] == BAD) {
            for (size_t j = 0; j < numFeatures; ++j) {
                m_badMean[j] += features[i][j];
            }
            badCount++;
        }
    }
    
    // Нормализуем
    if (goodCount > 0) {
        for (size_t j = 0; j < numFeatures; ++j) {
            m_goodMean[j] /= goodCount;
        }
    }
    if (badCount > 0) {
        for (size_t j = 0; j < numFeatures; ++j) {
            m_badMean[j] /= badCount;
        }
    }
    
    qDebug() << "[AppleClassifier::train] Computed centroids for" << goodCount << "good and" << badCount << "bad samples";
    qDebug() << "[AppleClassifier::train] Feature dimensionality:" << numFeatures;
    
    // Вычисляем пороги (максимальное расстояние + запас)
    double maxGoodDist = 0.0;
    double maxBadDist = 0.0;
    
    for (size_t i = 0; i < features.size(); ++i) {
        double dist = 0.0;
        if (labels[i] == GOOD) {
            for (size_t j = 0; j < numFeatures; ++j) {
                double diff = features[i][j] - m_goodMean[j];
                dist += diff * diff;
            }
            dist = std::sqrt(dist);
            if (dist > maxGoodDist) maxGoodDist = dist;
        } else if (labels[i] == BAD) {
            for (size_t j = 0; j < numFeatures; ++j) {
                double diff = features[i][j] - m_badMean[j];
                dist += diff * diff;
            }
            dist = std::sqrt(dist);
            if (dist > maxBadDist) maxBadDist = dist;
        }
    }
    
    // Устанавливаем пороги с запасом 50%
    m_goodThreshold = (maxGoodDist > 0) ? maxGoodDist * 1.5 : 10.0;
    m_badThreshold = (maxBadDist > 0) ? maxBadDist * 1.5 : 10.0;
    
    qDebug() << "[AppleClassifier::train] Max distances - Good:" << maxGoodDist << "Bad:" << maxBadDist;
    qDebug() << "[AppleClassifier::train] Thresholds set: Good=" << m_goodThreshold << "Bad=" << m_badThreshold;
    
    // Вычисляем расстояние между центроидами
    m_centroidDistance = 0.0;
    for (size_t j = 0; j < numFeatures; ++j) {
        double diff = m_goodMean[j] - m_badMean[j];
        m_centroidDistance += diff * diff;
    }
    m_centroidDistance = std::sqrt(m_centroidDistance);
    qDebug() << "[AppleClassifier::train] Distance between centroids:" << m_centroidDistance;

    m_trained = true;
    
    // Простая валидация - проверяем на тренировочных данных
    int correct = 0;
    for (size_t i = 0; i < features.size(); ++i) {
        float confidence;
        AppleQuality predicted = predict(features[i], confidence);
        if ((int)predicted == labels[i]) {
            correct++;
        }
    }
    
    float accuracy = static_cast<float>(correct) / features.size();

    qDebug() << "[AppleClassifier::train] Training completed with accuracy:" << accuracy;

    return accuracy;
}

AppleClassifier::AppleQuality AppleClassifier::predict(
    const std::vector<double> &features,
    float &confidence)
{
    if (!m_trained) {
        qWarning() << "Model not trained";
        confidence = 0.0f;
        return NOT_APPLE;
    }

    if (features.empty()) {
        qWarning() << "Empty features";
        confidence = 0.0f;
        return NOT_APPLE;
    }

    // TODO: Реализовать предсказание с MLPack
    // Сейчас используем центроидный классификатор
    
    if (m_goodMean.empty() || m_badMean.empty()) {
        qWarning() << "[AppleClassifier::predict] Model centroids are empty!";
        confidence = 0.5f;
        return BAD;
    }
    
    // Проверяем размерность признаков
    if (features.size() != m_goodMean.size() || features.size() != m_badMean.size()) {
        qWarning() << "[AppleClassifier::predict] Feature dimension mismatch! Features:" 
                   << features.size() << "GoodMean:" << m_goodMean.size() << "BadMean:" << m_badMean.size();
    }
    
    // Вычисляем евклидово расстояние до центроида каждого класса
    double distGood = 0.0;
    double distBad = 0.0;
    
    size_t minSize = std::min({features.size(), m_goodMean.size(), m_badMean.size()});
    for (size_t i = 0; i < minSize; ++i) {
        double diffGood = features[i] - m_goodMean[i];
        double diffBad = features[i] - m_badMean[i];
        distGood += diffGood * diffGood;
        distBad += diffBad * diffBad;
    }
    
    distGood = std::sqrt(distGood);
    distBad = std::sqrt(distBad);
    
    // Нормализуем расстояния по порогам для более справедливого сравнения
    double normalizedDistGood = (m_goodThreshold > 0) ? distGood / m_goodThreshold : distGood;
    double normalizedDistBad = (m_badThreshold > 0) ? distBad / m_badThreshold : distBad;
    
    qDebug() << "[AppleClassifier::predict] Distances - Good:" << distGood 
             << "Bad:" << distBad << "Normalized - Good:" << normalizedDistGood 
             << "Bad:" << normalizedDistBad << "CentroidDistance:" << m_centroidDistance;
    
    // Упрощенная и более надежная логика классификации
    
    double minDist = std::min(distGood, distBad);
    double maxDist = std::max(distGood, distBad);
    
    // Если центроиды очень близки (плохо разделены), используем более строгую проверку
    bool poorlySeparated = (m_centroidDistance > 0 && m_centroidDistance < 0.5);
    
    // ШАГ 1: Проверка на "Не яблоко" - сначала проверяем нормализованные расстояния
    // Если оба нормализованных расстояния большие - объект далеко от обоих классов
    if (normalizedDistGood > 1.5 && normalizedDistBad > 1.5) {
        confidence = 0.85f;
        qDebug() << "[AppleClassifier::predict] Classified as NOT_APPLE (both normalized distances too large: Good=" 
                 << normalizedDistGood << ", Bad=" << normalizedDistBad << ")";
        return NOT_APPLE;
    }
    
    // Дополнительная проверка с использованием абсолютных расстояний
    if (m_centroidDistance > 0) {
        // Если минимальное расстояние намного больше расстояния между центроидами
        if (minDist > m_centroidDistance * 4.0) {
            confidence = 0.85f;
            qDebug() << "[AppleClassifier::predict] Classified as NOT_APPLE (minDist=" 
                     << minDist << " >> centroidDist=" << m_centroidDistance << ")";
            return NOT_APPLE;
        }
        
        // Если оба расстояния большие и близки (объект на равном расстоянии от обоих классов)
        // И при этом далеко от обоих - это не яблоко
        double distRatio = maxDist / (minDist + 1e-6);
        if (distRatio < 1.3 && minDist > m_centroidDistance * 2.8) {
            confidence = 0.8f;
            qDebug() << "[AppleClassifier::predict] Classified as NOT_APPLE (equidistant and far: minDist=" 
                     << minDist << ", ratio=" << distRatio << ")";
            return NOT_APPLE;
        }
    }
    
    // ШАГ 2: Определяем класс (GOOD или BAD)
    // Используем нормализованные расстояния для выбора между классами
    AppleQuality quality;
    
    if (normalizedDistGood < normalizedDistBad) {
        quality = GOOD;
        // Уверенность зависит от разницы нормализованных расстояний
        double diff = normalizedDistBad - normalizedDistGood;
        confidence = std::min(0.99f, std::max(0.5f, static_cast<float>(0.5 + diff * 0.5)));
        
        // Если уверенность очень низкая (почти нет разницы) - возможно, это не яблоко
        if (confidence < 0.55 && m_centroidDistance > 0 && minDist > m_centroidDistance * 1.5) {
            confidence = 0.75f;
            qDebug() << "[AppleClassifier::predict] Classified as NOT_APPLE (very low confidence for GOOD, ambiguous)";
            return NOT_APPLE;
        }
        
        qDebug() << "[AppleClassifier::predict] Classified as GOOD, confidence:" << confidence 
                 << " (normGood:" << normalizedDistGood << " < normBad:" << normalizedDistBad 
                 << ", distGood:" << distGood << ", distBad:" << distBad << ")";
    } else {
        quality = BAD;
        // Уверенность зависит от разницы нормализованных расстояний
        double diff = normalizedDistGood - normalizedDistBad;
        confidence = std::min(0.99f, std::max(0.5f, static_cast<float>(0.5 + diff * 0.5)));
        
        // Если уверенность очень низкая (почти нет разницы) - возможно, это не яблоко
        if (confidence < 0.55 && m_centroidDistance > 0 && minDist > m_centroidDistance * 1.5) {
            confidence = 0.75f;
            qDebug() << "[AppleClassifier::predict] Classified as NOT_APPLE (very low confidence for BAD, ambiguous)";
            return NOT_APPLE;
        }
        
        qDebug() << "[AppleClassifier::predict] Classified as BAD, confidence:" << confidence 
                 << " (normBad:" << normalizedDistBad << " < normGood:" << normalizedDistGood 
                 << ", distBad:" << distBad << ", distGood:" << distGood << ")";
    }

    return quality;
}

bool AppleClassifier::loadModel(const QString &modelPath)
{
    qDebug() << "[AppleClassifier::loadModel] Attempting to load model from:" << modelPath;

    QFile file(modelPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[AppleClassifier::loadModel] ❌ Cannot open model file:" << modelPath;
        return false;
    }

    qDebug() << "[AppleClassifier::loadModel] File opened successfully, size:" << file.size() << "bytes";

    // Проверяем размер файла (Magic 4 + Version 4 + Trained 1 = 9 bytes min)
    if (file.size() < 9) { 
        qWarning() << "[AppleClassifier::loadModel] ❌ Model file is too small or corrupted:" << file.size() << "bytes (minimum 9 bytes required)";
        file.close();
        return false;
    }

    // Читаем и проверяем данные модели
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_6);
    
    quint32 magic, version;
    bool trained;
    
    in >> magic >> version >> trained;
    
    // Проверяем магическое число
    if (magic != 0x41505043) { // "APPC"
        qWarning() << "[AppleClassifier::loadModel] ❌ Invalid model file format (magic number mismatch). Expected: 0x41505043, Got: 0x" << QString::number(magic, 16);
        file.close();
        return false;
    }
    
    qDebug() << "[AppleClassifier::loadModel] ✓ Magic number verified";
    
    // Проверяем версию
    if (version < 1 || version > 4) {
        qWarning() << "[AppleClassifier::loadModel] ❌ Unsupported model version:" << version;
        file.close();
        return false;
    }
    
    qDebug() << "[AppleClassifier::loadModel] ✓ Version verified (v" << version << ")";

    m_trained = trained;

    if (version >= 2 && m_trained) {
        quint32 goodSize, badSize;
        in >> goodSize;
        m_goodMean.resize(goodSize);
        for (quint32 i = 0; i < goodSize; ++i) {
            in >> m_goodMean[i];
        }
        
        in >> badSize;
        m_badMean.resize(badSize);
        for (quint32 i = 0; i < badSize; ++i) {
            in >> m_badMean[i];
        }
        
        qDebug() << "[AppleClassifier::loadModel] Loaded centroids: Good=" << goodSize << "Bad=" << badSize;
    }
    
    if (version >= 3 && m_trained) {
        in >> m_goodThreshold;
        in >> m_badThreshold;
        qDebug() << "[AppleClassifier::loadModel] Loaded thresholds: Good=" << m_goodThreshold << "Bad=" << m_badThreshold;
    } else {
        // Дефолтные значения для старых версий
        m_goodThreshold = 10.0;
        m_badThreshold = 10.0;
    }
    
    if (version >= 4 && m_trained) {
        in >> m_centroidDistance;
        qDebug() << "[AppleClassifier::loadModel] Loaded centroid distance:" << m_centroidDistance;
    } else {
        // Для старых версий вычисляем расстояние между центроидами
        if (m_trained && !m_goodMean.empty() && !m_badMean.empty()) {
            m_centroidDistance = 0.0;
            size_t minSize = std::min(m_goodMean.size(), m_badMean.size());
            for (size_t i = 0; i < minSize; ++i) {
                double diff = m_goodMean[i] - m_badMean[i];
                m_centroidDistance += diff * diff;
            }
            m_centroidDistance = std::sqrt(m_centroidDistance);
            qDebug() << "[AppleClassifier::loadModel] Computed centroid distance:" << m_centroidDistance;
        } else {
            m_centroidDistance = 0.0;
        }
    }

    file.close();
    qDebug() << "[AppleClassifier::loadModel] ✅ Model loaded successfully! Trained status:" << m_trained;
    return true;
}

bool AppleClassifier::saveModel(const QString &modelPath)
{
    qDebug() << "[AppleClassifier::saveModel] Attempting to save model to:" << modelPath;

    if (!m_trained) {
        qWarning() << "[AppleClassifier::saveModel] ❌ No trained model to save";
        return false;
    }

    qDebug() << "[AppleClassifier::saveModel] Model is trained, proceeding to save...";

    QFile file(modelPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[AppleClassifier::saveModel] ❌ Cannot open file for writing:" << modelPath;
        qWarning() << "[AppleClassifier::saveModel] Error:" << file.errorString();
        return false;
    }

    qDebug() << "[AppleClassifier::saveModel] File opened for writing";


    // Сохраняем данные модели
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_6);
    
    // Записываем магическое число и версию формата
    out << quint32(0x41505043); // "APPC" - Apple Classifier
    out << quint32(4);          // Версия формата (v4: добавлено расстояние между центроидами)
    out << m_trained;           // Флаг обученности
    
    // Сохраняем центроиды
    out << static_cast<quint32>(m_goodMean.size());
    for (double val : m_goodMean) {
        out << val;
    }
    
    out << static_cast<quint32>(m_badMean.size());
    for (double val : m_badMean) {
        out << val;
    }
    
    // Сохраняем пороги (v3+)
    out << m_goodThreshold;
    out << m_badThreshold;
    
    // Сохраняем расстояние между центроидами (v4+)
    out << m_centroidDistance;
    
    qDebug() << "[AppleClassifier::saveModel] Saved" << m_goodMean.size() << "features for GOOD class";
    qDebug() << "[AppleClassifier::saveModel] Saved" << m_badMean.size() << "features for BAD class";

    file.flush();
    qint64 bytesWritten = file.pos();
    file.close();

    qDebug() << "[AppleClassifier::saveModel] ✅ Model saved successfully!" << bytesWritten << "bytes written to" << modelPath;
    return true;
}

QString AppleClassifier::qualityToString(AppleQuality quality)
{
    switch (quality) {
    case GOOD:
        return "хорошее";
    case BAD:
        return "плохое";
    case NOT_APPLE:
        return "не яблоко";
    default:
        return "неизвестно";
    }
}
