//
// Created by Ivan Kishchenko on 14.03.2021.
//

#ifndef ARDUINO_PROTOCOL_H
#define ARDUINO_PROTOCOL_H

#define SYNC "sync"
#define CONF "conf"

enum ProtoState {
    idle,
    opened,
    ready_sync,
    wait_sync,
    wait_conf,
    connected
};

#endif //ARDUINO_PROTOCOL_H
