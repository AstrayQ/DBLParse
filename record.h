#ifndef RECORD_H
#define RECORD_H

#include <QObject>
#include <QDate>

class Record : public QObject
{
    Q_OBJECT
public:
    explicit Record(const QString &s, QObject *parent = nullptr);
    QString title() const;
    QString mdate() const;
    QString key() const;
    QString name() const;
    QStringList authors() const;
    void clearCoauthors();
    QStringList coauthors() const;
private:
    QString m_name;
    QString m_key;
    QString m_mdate;
    QStringList m_authors;
    QString m_title;
    QStringList m_coauthors;
};

#endif // RECORD_H
