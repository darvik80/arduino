
#include <etl/platform.h>
#include <etl/vector.h>
#include <etl/string.h>
#include <Arduino.h>

#include "protocol.h"

ProtoState state = idle;
unsigned long lastTimeUpdate = millis();

class Operation {
public:
    virtual void reset() = 0;
    virtual int process(int ch) = 0;
};

class Handshake : public Operation {
    const char* _cmd;
    int _idx;
    int _len;
public:
    explicit Handshake(const char* cmd) {
        _cmd = cmd;
        _idx = 0;
        _len = strlen(cmd);
    }

    void reset() override {
        _idx = 0;
    }

    int process(int ch) override {
        if (ch != _cmd[_idx]) {
            return -1;
        }

        return (int)(++_idx == _len);
    }
};

class Proxy : public Operation {
public:
    void reset() override {
    }

    int process(int ch) override {
        if (ch < 0) {
            return ch;
        }

        Serial.write(ch);
        return 1;
    }
};

Handshake syncCmd(SYNC);
Handshake confCmd(CONF);
Proxy proxy;

Operation* operation = &syncCmd;

void setup() {
    Serial.begin(57600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }

    Serial.write("test");
    state = ready_sync;

    operation = &syncCmd;
    operation->reset();
}

void loop() {
    unsigned long now = millis();
    unsigned long delta = now - lastTimeUpdate;
    if (delta > 5000 && state != ready_sync) {
        state = ready_sync;
        operation = &syncCmd;
        operation->reset();
        Serial.write("disc");
    }

    while (Serial.available()) {
        lastTimeUpdate = now;
        int res = operation->process(Serial.read());
        if (res < 0) {
            state = ready_sync;
            operation = &syncCmd;
            operation->reset();
            continue;
        }
        switch (state) {
            case ready_sync:
                if (res == 1) {
                    state = wait_conf;
                    operation = &confCmd;
                    operation->reset();
                    Serial.write(CONF);
                }
                break;
            case wait_conf:
                if (res == 1) {
                    state = connected;
                    Serial.write("conn\r\n");
                    operation = &proxy;
                }
                break;
            default:
                break;
        }
    }
}