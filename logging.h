#pragma once
#ifndef LOGGING_H
#define LOGGING_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>


class logging : public QObject
{
    Q_OBJECT

public:
    enum LoggingMode {
        Daily, Hourly, Minutely, Secondly

    };
    explicit logging(QObject* parent = nullptr);
    //initialize file path (json)
    bool loadConfig(const QString& configFilePath);

    void logMessage(const QString& message);

private:
    QString m_logDirectory;
    QString m_filePrefix;
    QString m_dateFormat;
    LoggingMode m_loggingMode;
    int m_maxLogCount;
    bool m_timeZoneUTC;

    //for active log file
    QFile m_currentLogFile;
    QDateTime m_currentLogPeriodStart;

    void openNewLogFile();
    QString generateLogFileName(const QDateTime& dateTime)const;
    bool isNewLogPeriod(const QDateTime& now)const;
    void removeOldLogsIfNeeded();
    LoggingMode stringToLoggingMode(const QString& modeStr)const;

};
#endif //finish .h
