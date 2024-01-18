// gpsreader.h
#pragma once

#include <QMutex>
#include <QThread>
#include <QtSerialPort/QSerialPort>

#pragma pack(push, 1)
typedef struct {
    double time;            // UTC time
    double latitude;        // [-] decimal latitude
    double longitude;       // [-] decimal longitude
    double altitude;        // meters
    double HDOP;            // horizontal precision
    double GeoidSeparation; // adjustment in meters
    uint16_t ReferenceID;   // reference station ID (0..4095)
    uint8_t quality;        // 0 - not valid, >0 valid (see NEMA definitions)
    uint8_t satellites;     // 0..24
} GPSLocationType;
#pragma pack(pop)

// $GNGGA,202057.00,4915.76973,N,12255.06849,W,1,12,0.70,115.3,M,-17.4,M,,*78
class GPSReaderThread : public QThread
{
    Q_OBJECT

public:
    explicit GPSReaderThread(QObject *parent = nullptr);
    ~GPSReaderThread();

    bool open( const QString& portName, qint32 baudRate );
    void close();

    bool validateChecksum( QByteArray& receivedData );

    void setGGA( QByteArray& receivedData );

    QObject* get();
    bool getQuality();
    QString getUTCString();
    double getUTC();
    QString getLatitudeString();
    double getLatitude();
    void setLatitude( double val );
    QString getLatitudeDirection();
    QString getLongitudeString();
    double getLongitude();
    void setLongitude( double val );
    QString getLongitudeDirection();
    QString getAltitudeString();
    double getAltitude();
    void setAltitude( double val );
    QString getAltitudeUnits();
    QString getSatellitesString();
    uint8_t getSatellites();
    QString getHDOPString();
    double getHDOP();
    QString getGeoidSeparationString();
    double getGeoidSeparation();
    QString getReferenceIdString();
    uint16_t getReferenceId();

    const GPSLocationType& getLocation();

    QString getPositionString();

signals:
    void messageReceived(const QString& message);

protected:
    void run() override;

private:
    QSerialPort m_serialPort;
    QByteArray m_receivedData;
    QMutex m_mutex;
    bool m_stopRequested;
    int m_fieldStart[32];
    int m_fieldLength[32];
    bool m_gpsActive;
    GPSLocationType m_gpsLocation;
};
