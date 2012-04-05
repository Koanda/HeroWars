#include <QFile>
#include <QDataStream>
#include "logfile.h"

void LogFile::load(const QString& path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) return;

    m_path = path;

    QDataStream stream(&file);

    m_packets.clear();
    unsigned int packetCount;
    file.read(reinterpret_cast<char*>(&packetCount),sizeof(packetCount));

    for(quint32 i = 0 ; i < packetCount ; ++i)
    {
        Packet p;
        file.read(reinterpret_cast<char*>(&p.type),sizeof(packetCount)); // I hate Qt casting challenge.
        file.read(reinterpret_cast<char*>(&p.time),sizeof(p.time));
        unsigned int length;
        file.read(reinterpret_cast<char*>(&length),sizeof(length));
        p.length = length;
        p.data = file.read(length);
        m_packets.append(p);
    }
}

void LogFile::reload()
{

}
