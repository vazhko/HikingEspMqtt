#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H


namespace  mqttClient {
    void init();
    void handle();
    bool publish(const char* topic, const char* payload);
};

#endif