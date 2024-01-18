#include "gpslocation.h"

GPSLocation::GPSLocation(QObject* parent) {}

double GPSLocation::time() { return m_time; }
void GPSLocation::settime(double val) {
    if (m_altitude != val) {
        m_altitude = val;
        emit timeChanged(val);
    }
}

double GPSLocation::latitude() { return m_latitude; }
void GPSLocation::setlatitude(double val) {
    if (m_latitude != val) {
        m_latitude = val;
        emit latitudeChanged(val);
    }
}

double GPSLocation::longitude()  { return m_longitude; }
void GPSLocation::setlongitude(double val) {
    if (m_longitude != val) {
        m_longitude = val;
        emit longitudeChanged(val);
    }
}

double GPSLocation::altitude() { return m_altitude; }
void GPSLocation::setaltitude(double val) {
    if (m_altitude != val) {
        m_altitude = val;
        emit altitudeChanged(val);
    }
}

double GPSLocation::HDOP() { return m_hdop; }
void GPSLocation::setHDOP(double val) {
    if (m_altitude != val) {
        m_altitude = val;
        emit HDOPChanged(val);
    }
}

double GPSLocation::GeoidSeparation() { return m_geoidseparation; }
void GPSLocation::setGeoidSeparation(double val) {
    if (m_altitude != val) {
        m_altitude = val;
        emit GeoidSeparationChanged(val);
    }
}

uint16_t GPSLocation::ReferenceID() { return m_referenceid; }
void GPSLocation::setReferenceID(uint16_t val) {
    if (m_altitude != val) {
        m_altitude = val;
        emit ReferenceIDChanged(val);
    }
}

uint8_t GPSLocation::quality() { return m_quality; }
void GPSLocation::setquality(uint8_t val) {
    if (m_altitude != val) {
        m_altitude = val;
        emit qualityChanged(val);
    }
}

uint8_t GPSLocation::satellites() { return m_satellites; }
void GPSLocation::setsatellites(uint8_t val) {
    if (m_altitude != val) {
        m_altitude = val;
        emit satellitesChanged(val);
    }
}
