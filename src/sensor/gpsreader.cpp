// gpsreader.cpp

#include <QDebug>
#include "gpsreader.h"

GPSReaderThread::GPSReaderThread(QObject *parent)
    : QThread(parent), m_stopRequested(false), m_gpsActive(false)
{
    m_gpsLocation.quality = 0;
}

GPSReaderThread::~GPSReaderThread()
{
    m_gpsLocation.quality = 0;
    m_gpsActive = false;
    m_stopRequested = true;
    wait();
}

bool GPSReaderThread::open(const QString &portName, qint32 baudRate)
{
    close();

    m_serialPort.setPortName(portName);
    m_serialPort.setBaudRate(baudRate);

    if (m_serialPort.open(QIODevice::ReadOnly))
    {
        m_gpsActive = true;
        return true;
    }
    else
    {
        emit messageReceived("Failed to open serial port: " + m_serialPort.errorString());
        return false;
    }
}

void GPSReaderThread::close()
{
    if (m_serialPort.isOpen())
    {
        m_serialPort.close();
        m_gpsActive = false;
    }
}

void GPSReaderThread::run()
{
    while (!m_stopRequested)
    {
        if (m_serialPort.waitForReadyRead(500))
        {
            m_mutex.lock();
            while (m_serialPort.canReadLine())
            {
                m_receivedData += m_serialPort.readLine();
                if (m_receivedData.endsWith('\n'))
                {
                    if (m_receivedData.mid(3,3)=="GGA")
                    {
                        // $GNGGA,202059.00,4915.76956,N,12255.06830,W,1,12,0.70,115.5,M,-17.4,M,,*79
                        if ( validateChecksum(m_receivedData) )
                        {
                            setGGA(m_receivedData);
                            m_gpsLocation.time      = getUTC();
                            m_gpsLocation.latitude  = getLatitude();
                            m_gpsLocation.longitude = getLongitude();
                            m_gpsLocation.altitude  = getAltitude();
                            m_gpsLocation.quality   = getQuality();
                            m_gpsLocation.satellites= getSatellites();
                            m_gpsLocation.HDOP      = getHDOP();
                            m_gpsLocation.GeoidSeparation = getGeoidSeparation();
                            m_gpsLocation.ReferenceID= getReferenceId();

                            emit messageReceived(QString::fromLatin1(m_receivedData));
                        }
                    }
                    m_receivedData.clear();
                }
            }
            m_mutex.unlock();
        }
    }
}

bool GPSReaderThread::validateChecksum( QByteArray& receivedData )
{
    const QByteArray hexValues = "0123456789ABCDEF";
    int checksum = 0;
    int endPos = receivedData.indexOf("*");
    if ( endPos < 0 )
        return false;

    for ( int i=1; i<endPos; i++ )
    {
        checksum = checksum ^ receivedData[i];
    }
    if ( checksum != ( (hexValues.indexOf(receivedData[endPos+1]) << 4) + (hexValues.indexOf(receivedData[endPos+2])) ) )
    {
        return false;
    }
    return true;
}

void GPSReaderThread::setGGA( QByteArray& receivedData )
{
    int startPos    = 1;        // Skip the $ infront of the message
    int endPos      = 0;
    int fieldNum    = 0;
    int moreFields  = 1;

    m_receivedData = receivedData;
    while(moreFields)
    {
        m_fieldStart[fieldNum]=startPos;
        endPos = receivedData.indexOf(',',startPos);
        if ( endPos < 0 )
        {
            endPos = receivedData.indexOf('*',2);
            moreFields = 0;
        }
        m_fieldLength[fieldNum]=endPos-startPos;
        startPos = endPos + 1;

        if ( fieldNum++ > 31 )
            moreFields = 0;
    }
    return;
}

QString GPSReaderThread::getPositionString()
{
    QString tempString;

    tempString = getUTCString();
    tempString += " ";
    tempString += getLatitudeString();
    tempString += " ";
    tempString += getLatitudeDirection();
    tempString += " ";
    tempString += getLongitudeString();
    tempString += " ";
    tempString += getLongitudeDirection();
    tempString += " ";
    tempString += getAltitudeString();
    tempString += " ";
    tempString += getAltitudeUnits();

    return tempString;
}

QString GPSReaderThread::getUTCString()
{
    return QString(m_receivedData.mid(m_fieldStart[1],m_fieldLength[1]));
}
double GPSReaderThread::getUTC()
{
    double temp;
    QString stemp = getUTCString();
    temp = stemp.toDouble();  // return in HHMMSS.nn[n]
    //temp = (stemp.mid(0,2).toDouble()*3600)+(stemp.mid(2,2).toDouble()*60)+(stemp.mid(4).toDouble()); // return in milliseconds
    return temp;
}
QString GPSReaderThread::getLatitudeString()
{
    return QString::number(getLatitude());
}
double GPSReaderThread::getLatitude()
{
    int s = m_fieldStart[2];
    int l = m_fieldLength[2];
    double temp = m_receivedData.mid(s,2).toDouble() + m_receivedData.mid(s+2,l-2).toDouble()/60;
    if (getLatitudeDirection() == "S")
        temp = -temp;
    return temp;
}

void GPSReaderThread::setLatitude( double val )
{}

QString GPSReaderThread::getLatitudeDirection()
{
    return QString(m_receivedData.mid(m_fieldStart[3],m_fieldLength[3]));
}
QString GPSReaderThread::getLongitudeString()
{
    return QString::number(getLongitude());
}
double GPSReaderThread::getLongitude()
{
    int s = m_fieldStart[4];
    int l = m_fieldLength[4];
    double temp = m_receivedData.mid(s,3).toDouble() + m_receivedData.mid(s+3,l-3).toDouble()/60;
    if (getLongitudeDirection() == "E")
        temp = -temp;
    return temp;
}

void GPSReaderThread::setLongitude( double val )
{}

QString GPSReaderThread::getLongitudeDirection()
{
    return QString(m_receivedData.mid(m_fieldStart[5],m_fieldLength[5]));
}

QString GPSReaderThread::getAltitudeString()
{
    return QString(m_receivedData.mid(m_fieldStart[9],m_fieldLength[9]));
}
double GPSReaderThread::getAltitude()
{
    double temp = getAltitudeString().toDouble();
    return temp;
}

void GPSReaderThread::setAltitude( double val )
{}

QString GPSReaderThread::getAltitudeUnits()
{
    return QString(m_receivedData.mid(m_fieldStart[10],m_fieldLength[10]));
}

bool GPSReaderThread::getQuality()
{
    return QString(m_receivedData.mid(m_fieldStart[6],m_fieldLength[6])).toInt();
}

QString GPSReaderThread::getSatellitesString()
{
    return QString(m_receivedData.mid(m_fieldStart[7],m_fieldLength[7]));
}

uint8_t GPSReaderThread::getSatellites()
{
    return getSatellitesString().toInt();
}

QString GPSReaderThread::getHDOPString()
{
    return QString(m_receivedData.mid(m_fieldStart[8],m_fieldLength[8]));
}

double GPSReaderThread::getHDOP()
{
    return getSatellitesString().toDouble();
}

QString GPSReaderThread::getGeoidSeparationString()
{
    return QString(m_receivedData.mid(m_fieldStart[11],m_fieldLength[11]));
}

double GPSReaderThread::getGeoidSeparation()
{
    return getSatellitesString().toDouble();
}

QString GPSReaderThread::getReferenceIdString()
{
    return QString(m_receivedData.mid(m_fieldStart[14],m_fieldLength[14]));
}

uint16_t GPSReaderThread::getReferenceId()
{
    return getSatellitesString().toDouble();
}

const GPSLocationType& GPSReaderThread::getLocation()
{
    return m_gpsLocation;
}

