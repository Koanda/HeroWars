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
    file.close();
}

void LogFile::reload()
{

}
