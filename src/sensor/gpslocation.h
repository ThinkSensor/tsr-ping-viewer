#pragma once
#include "qobject.h"
#include "gpsreader.h"

class GPSLocation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double   time            READ time            WRITE settime             NOTIFY timeChanged)
    Q_PROPERTY(double   latitude        READ latitude        WRITE setlatitude         NOTIFY latitudeChanged)
    Q_PROPERTY(double   longitude       READ longitude       WRITE setlongitude        NOTIFY longitudeChanged)
    Q_PROPERTY(double   altitude        READ altitude        WRITE setaltitude         NOTIFY altitudeChanged)
    Q_PROPERTY(double   HDOP            READ HDOP            WRITE setHDOP             NOTIFY HDOPChanged)
    Q_PROPERTY(double   GeoidSeparation READ GeoidSeparation WRITE setGeoidSeparation  NOTIFY GeoidSeparationChanged)
    Q_PROPERTY(uint16_t ReferenceID     READ ReferenceID     WRITE setReferenceID      NOTIFY ReferenceIDChanged)
    Q_PROPERTY(uint8_t  quality         READ quality         WRITE setquality          NOTIFY qualityChanged)
    Q_PROPERTY(uint8_t  satellites      READ satellites      WRITE setsatellites       NOTIFY satellitesChanged)

public:
    explicit GPSLocation(QObject* parent = nullptr);

    double time();
    void settime(double val);

    double latitude();
    void setlatitude(double val);

    double longitude();
    void setlongitude(double val);

    double altitude();
    void setaltitude(double val);

    double HDOP();
    void setHDOP(double val);

    double GeoidSeparation();
    void setGeoidSeparation(double val);

    uint16_t ReferenceID();
    void setReferenceID(uint16_t val);

    uint8_t quality();
    void setquality(uint8_t val);

    uint8_t satellites();
    void setsatellites(uint8_t val);

signals:
    void locationChanged(GPSLocationType location);
    void timeChanged(double newValue);
    void latitudeChanged(double newValue);
    void longitudeChanged(double newValue);
    void altitudeChanged(double newValue);
    void HDOPChanged(double newValue);
    void GeoidSeparationChanged(double newValue);
    void ReferenceIDChanged(uint16_t newValue);
    void qualityChanged(uint8_t newValue);
    void satellitesChanged(uint8_t newValue);

private:
    double m_time = 0;
    double m_latitude = 0;
    double m_longitude = 0;
    double m_altitude = 0;
    double m_hdop = 0;
    double m_geoidseparation = 0;
    uint16_t m_referenceid = 0;
    uint8_t m_quality = 0;
    uint8_t m_satellites = 0;
    int m_valid = 0;
};
