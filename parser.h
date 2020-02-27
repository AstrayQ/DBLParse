#ifndef PARSER_H
#define PARSER_H

#include "recordparser.h"

#include <QThread>
#include <QMap>
#include <QMap>
#include <QTime>
#include <QVariant>

class QXmlStreamReader;

class Parser : public QThread
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = nullptr);
    void clear();
    void setReader(QXmlStreamReader *r);
    bool hasReader() const;
    void run() override;
    QString documentVersion() const;
    void setDocumentVersion(QString s);
    QString documentEncoding() const;
    void setDocumentEncoding(QString s);
    QString dtdName() const;
    void setDtdName(QString s);
    QString dtdSystemId() const;
    void setDtdSystemId(QString s);
    int count() const;
    int count(QString recordName) const;
    void setCount(int cnt);
    QStringList recordNames() const;
    QMap<QString,QVariant> recordCount() const;
    void setRecordCount(QMap<QString,QVariant> x);
    QHash<QString,QVariant> authorIndex() const;
    void setAuthorIndex(QHash<QString,QVariant> x);
    void addRecordCount(QString recordName, int value = 1);
    void abortParse();
    int parseCostMsec() const;
    void setParseCostMsec(int x);
    QTime parseCostTime() const;
    QList<QVariant> getOffsetsByAuthorName(QString authorName) const;
    
signals:
    void done(Parser *parser);
    void countChanged(double ratio);
    
private:
    QXmlStreamReader *reader;
    QString documentVersion_;
    QString documentEncoding_;
    QString dtdName_;
    QString dtdSystemId_;
    int count_;
    QMap<QString,QVariant> recordCount_;
    QHash<QString,QVariant> authorIndex_;
    QTime parseTiming;
    int parseCostMsecs_;
    bool abortFlag;
    
    void parseRecords();
};

#endif // PARSER_H
