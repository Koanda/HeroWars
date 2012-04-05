/**
 * Copyright (C) 2012 Jean Guegant (Jiwan)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
