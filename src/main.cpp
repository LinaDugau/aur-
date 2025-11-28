#include <auroraapp.h>
#include <QtQuick>
#include <QQmlContext>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "AppleDetector.h"
#include "CameraHandler.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> application(Aurora::Application::application(argc, argv));
    application->setOrganizationName(QStringLiteral("ru.auroraos"));
    application->setApplicationName(QStringLiteral("aurcad"));

    // Создаем детектор яблок
    AppleDetector appleDetector;

    // Инициализируем путь к датасету и аннотациям
    QString datasetPath = QDir::currentPath() + "/omsk";
    QString annotationsPath = datasetPath + "/labelme";

    qDebug() << "Dataset path:" << datasetPath;
    qDebug() << "Annotations path:" << annotationsPath;

    // Проверяем существование датасета
    if (QDir(datasetPath).exists()) {
        qDebug() << "Dataset found";
    } else {
        qWarning() << "Dataset not found at:" << datasetPath;
    }

    QScopedPointer<QQuickView> view(Aurora::Application::createView());

    // Регистрируем AppleDetector в QML context
    view->rootContext()->setContextProperty("appleDetector", &appleDetector);

    // Устанавливаем главный QML файл
    view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/aurcad.qml")));
    view->show();

    qDebug() << "AurCad application started";

    return application->exec();
}
