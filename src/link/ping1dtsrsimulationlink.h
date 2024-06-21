#pragma once

#include "simulationlink.h"
#include <QTimer>

/**
 * @brief Link that simulates Ping sensor behaviour
 *
 */
class Ping1DTSRSimulationLink : public SimulationLink {
public:
    /**
     * @brief Construct a new Ping1D Simulation Link object
     *
     * @param parent
     */
    Ping1DTSRSimulationLink(QObject* parent = nullptr);

    /**
     * @brief Generates random data
     *
     */
    void randomUpdate();

private:
    QTimer _randomUpdateTimer;
};
