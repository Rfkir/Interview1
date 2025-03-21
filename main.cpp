#include "logging.h"
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Interview1_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // Set up code that uses the Qt event loop here.
    // Call a.quit() or a.exit() to quit the application.
    // A not very useful example would be including
    // #include <QTimer>
    // near the top of the file and calling
    // QTimer::singleShot(5000, &a, &QCoreApplication::quit);
    // which quits the application after 5 seconds.

    // If you do not need a running Qt event loop, remove the call
    // to a.exec() or use the Non-Qt Plain C++ Application template.

    QCoreApplication app(argc, argv);

    logging logger;
    logger.loadConfig("C:/Users/remzi/Documents/Interview1/config.json");
    // config.json yükle
    if(!logger.loadConfig("config.json")) {
        qWarning() << "Config yüklenemedi, varsayılan değerler kullanılacak...";
    }

    // Test logları
    logger.logMessage("Program başlatıldı.");
    logger.logMessage("İşlem 1 yapıldı.");
    logger.logMessage("İşlem 2 yapıldı.");

    // Birkaç saniyede bir log atmak için örnek döngü
    for(int i = 0; i < 3; i++) {
        logger.logMessage(QString("Döngü index: %1").arg(i));

    }

    return 0;
}
