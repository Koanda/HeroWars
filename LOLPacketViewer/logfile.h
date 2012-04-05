#ifndef LOGFILE_H
#define LOGFILE_H
#include <QVector>
#include <QObject>
#include <Windows.h>

class LogFile : QObject
{
public:
    typedef enum
    {
        PT_SERVER_TO_CLIENT,
        PT_CLIENT_TO_SERVER
    } PACKET_TYPE;

    struct Packet
    {
        PACKET_TYPE type;
        quint32 length;
        SYSTEMTIME time;
        QByteArray data;
    };

public:
    void load(const QString& path);
    void reload();

    QVector<Packet> getPackets()
    {
        return m_packets;
    }

private:
    QString m_path;
    QVector<Packet> m_packets;
};

#endif // LOGFILE_H
