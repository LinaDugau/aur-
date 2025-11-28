#ifndef APPLECLASSIFIER_H
#define APPLECLASSIFIER_H

#include <QString>
#include <vector>
#include <map>

/**
 * @brief Классификатор яблок на основе MLPack
 *
 * Реализует обучение и предсказание с использованием MLPack:
 * - Логистическая регрессия
 * - Random Forest
 * - Neural Network (опционально)
 */
class AppleClassifier
{
public:
    enum AppleQuality {
        GOOD = 0,      // Хорошее яблоко
        BAD = 1,       // Плохое яблоко
        NOT_APPLE = 2  // Не яблоко
    };

    AppleClassifier();
    ~AppleClassifier();

    /**
     * @brief Обучает модель на датасете
     * @param features Матрица признаков (N x D)
     * @param labels Метки классов (N)
     * @return Точность на валидационном наборе
     */
    float train(const std::vector<std::vector<double>> &features,
                const std::vector<int> &labels);

    /**
     * @brief Предсказывает качество яблока
     * @param features Вектор признаков
     * @param confidence Уверенность модели (выходной параметр)
     * @return Класс яблока
     */
    AppleQuality predict(const std::vector<double> &features,
                        float &confidence);

    /**
     * @brief Загружает модель из файла
     */
    bool loadModel(const QString &modelPath);

    /**
     * @brief Сохраняет модель в файл
     */
    bool saveModel(const QString &modelPath);

    /**
     * @brief Проверяет, обучена ли модель
     */
    bool isTrained() const { return m_trained; }

    /**
     * @brief Возвращает текстовое описание класса
     */
    static QString qualityToString(AppleQuality quality);

private:
    bool m_trained;
    void *m_model; // Указатель на модель MLPack (opaque pointer для скрытия деталей)
    
    // Простой классификатор: средние значения признаков для каждого класса
    std::vector<double> m_goodMean;
    std::vector<double> m_badMean;
    
    // Пороговые значения для определения "Не яблоко"
    double m_goodThreshold;
    double m_badThreshold;
    
    // Расстояние между центроидами классов
    double m_centroidDistance;
};

#endif // APPLECLASSIFIER_H
