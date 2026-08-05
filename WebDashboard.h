#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <WebServer.h>
#include "Measurement.h"
#include "Energy.h"
#include "Relay.h"
#include "Config.h"

class WebDashboard
{
public:
    void begin();
    void handleClient(Relay relay[NUM_SOCKETS], Energy energy[NUM_CHANNELS], Measurement measurements[NUM_CHANNELS]);

private:
    WebServer server{80};
};

extern WebDashboard webDashboard;

#endif
