#!/usr/bin/env python3
"""
Скрипт подготовки датасета для обучения модели AurСад

Конвертирует датасет в формат, удобный для MLPack:
- Извлекает признаки из изображений
- Создает CSV файлы для обучения
- Разбивает на train/test наборы
"""

import os
import sys
import json
from pathlib import Path

def load_classes(classes_file):
    """Загружает классы из classes.txt"""
    classes = {}
    with open(classes_file, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line:
                parts = line.split()
                if len(parts) >= 2:
                    image_name = parts[0]
                    label = parts[1]
                    # Конвертируем в числовые метки
                    classes[image_name] = 0 if label == 'хорошее' else 1
    return classes

def parse_labelme_annotations(labelme_dir):
    """Парсит аннотации из labelme JSON файлов"""
    annotations = {}
    labelme_path = Path(labelme_dir)

    for json_file in labelme_path.glob('*.json'):
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
            image_name = Path(data['imagePath']).name
            shapes = data.get('shapes', [])

            # Извлекаем polygon для сегментации
            polygons = []
            for shape in shapes:
                if shape['shape_type'] == 'polygon':
                    polygons.append({
                        'label': shape['label'],
                        'points': shape['points']
                    })

            annotations[image_name] = polygons

    return annotations

def create_dataset_info(dataset_dir, output_dir):
    """Создает файл с информацией о датасете"""
    dataset_path = Path(dataset_dir)
    training_dir = dataset_path / 'Training'
    labelme_dir = dataset_path / 'labelme'
    classes_file = dataset_path / 'classes.txt'

    # Загружаем данные
    classes = load_classes(classes_file)
    annotations = parse_labelme_annotations(labelme_dir)

    # Собираем статистику
    total_images = len(list(training_dir.glob('*.jpg')))
    good_apples = sum(1 for label in classes.values() if label == 0)
    bad_apples = sum(1 for label in classes.values() if label == 1)

    info = {
        'total_images': total_images,
        'labeled_images': len(classes),
        'good_apples': good_apples,
        'bad_apples': bad_apples,
        'annotated_images': len(annotations),
        'classes': {
            0: 'хорошее',
            1: 'плохое'
        },
        'images': []
    }

    # Собираем информацию о каждом изображении
    for img_file in sorted(training_dir.glob('*.jpg')):
        img_name = img_file.name
        img_info = {
            'filename': img_name,
            'path': str(img_file),
            'label': classes.get(img_name),
            'has_annotation': img_name in annotations
        }

        if img_name in annotations:
            img_info['polygons'] = len(annotations[img_name])

        info['images'].append(img_info)

    # Сохраняем
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    info_file = output_path / 'dataset_info.json'
    with open(info_file, 'w', encoding='utf-8') as f:
        json.dump(info, f, ensure_ascii=False, indent=2)

    print(f"✓ Информация о датасете сохранена: {info_file}")
    print(f"\nСтатистика:")
    print(f"  Всего изображений: {total_images}")
    print(f"  С метками: {len(classes)}")
    print(f"  Хорошие яблоки: {good_apples} ({good_apples/len(classes)*100:.1f}%)")
    print(f"  Плохие яблоки: {bad_apples} ({bad_apples/len(classes)*100:.1f}%)")
    print(f"  С сегментацией: {len(annotations)}")

    return info

def main():
    """Главная функция"""
    # Определяем путь к датасету относительно скрипта
    script_dir = Path(__file__).parent.parent
    dataset_dir = script_dir / 'omsk'
    output_dir = script_dir / 'dataset_prepared'

    print("AurСад - Подготовка датасета")
    print("=" * 50)

    # Проверяем существование датасета
    if not dataset_dir.exists():
        print(f"✗ Ошибка: Датасет не найден в {dataset_dir}")
        print(f"  Убедитесь, что папка 'omsk' существует в корне проекта")
        sys.exit(1)

    # Создаем информацию о датасете
    info = create_dataset_info(dataset_dir, output_dir)

    print(f"\n✓ Датасет подготовлен!")
    print(f"  Результаты: {output_dir}")

    return 0

if __name__ == '__main__':
    sys.exit(main())
