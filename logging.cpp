#include "logging.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
//constructor
logging::logging(QObject *parent)
    : QObject(parent),
    m_loggingMode(Daily),
    m_maxLogCount(7),
    m_dateFormat("yyyyMMdd_hhmmss"),
    m_timeZoneUTC(true)
{
}

bool logging::loadConfig(const QString &configFilePath)
{
    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Config dosyası açılamadı:" << configFilePath;
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());



    if (!doc.isObject()) {
        qWarning() << "Config formatı geçersiz (JSON obje bekleniyor).";
        return false;
    }

    QJsonObject rootObj=doc.object();

    // rading parameters
    m_logDirectory = rootObj.value("logDirectory").toString("./logs");
    m_filePrefix   = rootObj.value("filePrefix").toString("myapp_log");
    QString mode   = rootObj.value("loggingMode").toString("daily");
    m_loggingMode  = stringToLoggingMode(mode);
    m_maxLogCount  = rootObj.value("maxLogCount").toInt(7);
    m_dateFormat   = rootObj.value("dateFormat").toString("yyyyMMdd_hhmmss");
    m_timeZoneUTC  = rootObj.value("timeZoneUTC").toBool(true);

    //create path if not
    QDir dir;
    if (!dir.exists(m_logDirectory)) {
        if (!dir.mkpath(m_logDirectory)) {
            qWarning() << "Log dizini oluşturulamadı:" << m_logDirectory;
        }
    }

    //open lof file
    openNewLogFile();

    return true;
}

void logging::logMessage(const QString &message)
{
    // take current time(UTC veya yerel)
    QDateTime now = m_timeZoneUTC ? QDateTime::currentDateTimeUtc()
                                  : QDateTime::currentDateTime();

    //check for the new period
    if (isNewLogPeriod(now)) {
        if (m_currentLogFile.isOpen()) {
            m_currentLogFile.close();
        }
        openNewLogFile();
    }

    // if file is open write log
    if (m_currentLogFile.isOpen()) {
        QTextStream out(&m_currentLogFile);
        out << now.toString(Qt::ISODate) << " - " << message << "\n";
        m_currentLogFile.flush();
    }
}

void logging::openNewLogFile()
{
    // start new period
    m_currentLogPeriodStart = m_timeZoneUTC ? QDateTime::currentDateTimeUtc()
                                            : QDateTime::currentDateTime();

    // create new file name
    QString fileName = generateLogFileName(m_currentLogPeriodStart);
    QString fullPath = QDir(m_logDirectory).filePath(fileName);

    // open file
    m_currentLogFile.setFileName(fullPath);
    if (!m_currentLogFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Log dosyası açılamadı:" << fullPath;
    }

    // clean old files
    removeOldLogsIfNeeded();
}

QString logging::generateLogFileName(const QDateTime &dateTime) const
{
    // <prefix>_<tarih_saat>.log
    QString dateStr = dateTime.toString(m_dateFormat);
    return QString("%1_%2.log").arg(m_filePrefix, dateStr);
}

bool logging::isNewLogPeriod(const QDateTime &now) const
{
    switch (m_loggingMode) {
    case Daily:
        return (m_currentLogPeriodStart.date() != now.date());
    case Hourly:
        // Hourly change checking it is controlled by config file
        return (m_currentLogPeriodStart.date() != now.date()) ||
               (m_currentLogPeriodStart.time().hour() != now.time().hour());
    case Minutely:
        return (m_currentLogPeriodStart.date() != now.date()) ||
               (m_currentLogPeriodStart.time().hour() != now.time().hour()) ||
               (m_currentLogPeriodStart.time().minute() != now.time().minute());
    case Secondly:
        return (m_currentLogPeriodStart.date() != now.date()) ||
               (m_currentLogPeriodStart.time().hour() != now.time().hour()) ||
               (m_currentLogPeriodStart.time().minute() != now.time().minute()) ||
               (m_currentLogPeriodStart.time().second() != now.time().second());
    }
    // Varsayılan (hiçbiri değilse Daily gibi davran)
    return false;
}

void logging::removeOldLogsIfNeeded()
{
    // Log dosyalarını dizinde bul
    QDir dir(m_logDirectory);
    QStringList filters;
    filters << QString("%1_*.log").arg(m_filePrefix);
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time | QDir::Reversed);

    QFileInfoList fileList = dir.entryInfoList();

    // maxLogCount'tan fazla dosya varsa, en eskilerini sil
    if (fileList.size() > m_maxLogCount) {
        int filesToRemove = fileList.size() - m_maxLogCount;
        for (int i = 0; i < filesToRemove; ++i) {
            QFile::remove(fileList.at(i).absoluteFilePath());
        }
    }
}

logging::LoggingMode logging::stringToLoggingMode(const QString &modeStr) const
{
    if (modeStr.compare("daily", Qt::CaseInsensitive) == 0) {
        return Daily;
    } else if (modeStr.compare("hourly", Qt::CaseInsensitive) == 0) {
        return Hourly;
    } else if (modeStr.compare("minutely", Qt::CaseInsensitive) == 0) {
        return Minutely;
    } else if (modeStr.compare("secondly", Qt::CaseInsensitive) == 0) {
        return Secondly;
    }
    // Varsayılan
    return Daily;
}
