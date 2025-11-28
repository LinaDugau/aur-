# Инструкции по сборке AurСад

## Быстрый старт

### 1. Подключение к Aurora Build Engine

```bash
# В PowerShell (Windows)
ssh -p 2222 -i C:/AuroraOS/vmshare/ssh/private_keys/sdk mersdk@localhost
```

### 2. Подготовка проекта

```bash
# В SSH-сессии Build Engine
cd ~/aurcad  # или путь к вашему проекту

# Клонирование (если нужно)
git clone https://github.com/LinaDugau/aurcad.git
cd aurcad
```

### 3. Сборка приложения

```bash
# Инициализация и сборка
mb2 build-init
mb2 build
mb2 rpm

# Установка на эмулятор
mb2 deploy-rpm
```

## Установка зависимостей (опционально)

**Примечание**: Conan используется для управления ML-библиотеками (OpenCV, MLPack). Если пакеты недоступны, приложение будет работать с базовой функциональностью.

### Настройка Conan

```bash
# Войдите в sb2 sdk-install
sb2 -t AuroraOS-5.1.6.110-MB2-x86_64 -m sdk-install -R

# Настройка remotes
conan remote add omprussiapublic https://releases.omprussia.ru/conan/stable || true
conan remote add conancenter https://center.conan.io || true

# Создание профиля
conan profile detect --force

# Настройка компилятора (если нужно)
cat >> ~/.conan2/profiles/default << 'EOF'
compiler=gcc
compiler.version=11
compiler.libcxx=libstdc++11
compiler.cppstd=17
EOF

# Установка зависимостей
cd ~/aurcad
conan install . -s build_type=Release --build=missing

exit
```

**Примечание**: Если получаете ошибку `404: Not Found` - это нормально. Пакеты могут быть недоступны, приложение будет работать без них.

## Сборка для разных архитектур

```bash
# armv7hl (по умолчанию)
mb2 -t AuroraOS-5.1.6.110-base-armv7hl build

# aarch64
mb2 -t AuroraOS-5.1.6.110-base-aarch64 build

# i486 (эмулятор)
mb2 -t AuroraOS-5.1.6.110-base-i486 build
```

## Установка на устройство

### Через mb2

```bash
mb2 -d <device-ip> deploy-rpm
```

### Через SCP

```bash
# Скопируйте RPM
scp RPMS/ru.auroraos.aurcad-1.0.0-1.armv7hl.rpm defaultuser@<device-ip>:~

# На устройстве
ssh defaultuser@<device-ip>
devel-su
rpm -i ru.auroraos.aurcad-1.0.0-1.armv7hl.rpm
```

## Полезные команды

```bash
# Очистка сборки
mb2 clean

# Пересборка с нуля
mb2 build --clean

# Проверка спецификации
mb2 build --check

# Список целей сборки
mb2 list targets
```

## Отладка

### Логи сборки

```bash
mb2 -v build 2>&1 | tee build.log
```

### Запуск с отладкой

```bash
# На устройстве
QT_LOGGING_RULES="*.debug=true" ru.auroraos.aurcad

# Просмотр логов
journalctl -f | grep aurcad
```

## Проблемы и решения

### Ошибка: "conan: command not found"

Conan не установлен. Можно пропустить установку зависимостей - приложение будет работать с базовой функциональностью.

### Ошибка: "Qt modules not found"

```bash
mb2 -t AuroraOS-5.1.6.110-base-armv7hl build --fix-version
```

### Приложение не запускается

Проверьте зависимости и права доступа:

```bash
rpm -q sailfishsilica-qt5
chmod +x /usr/bin/ru.auroraos.aurcad
journalctl -u ru.auroraos.aurcad
```

---

**Готово!** Теперь вы можете собрать и запустить AurСад на вашем устройстве с ОС Аврора! 🚀
