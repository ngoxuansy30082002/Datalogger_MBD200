"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.mqttConfig = void 0;
const env_1 = require("./env");
exports.mqttConfig = {
    brokerUrl: env_1.env.MQTT_BROKER_URL,
    options: {
        clientId: `firmware-server-${Math.random().toString(16).slice(2)}`,
        clean: true,
        connectTimeout: 30000,
        reconnectPeriod: 5000,
        keepalive: 60,
    },
    topics: {
        subscribe: [
            'datalogger/+/firmware/query',
            'datalogger/+/status',
            'datalogger/+/heartbeat',
        ],
        publish: {
            firmwareUpdate: 'datalogger/firmware/update',
            firmwareNotify: 'datalogger/firmware/notify',
            deviceResponse: (deviceId) => `datalogger/${deviceId}/response`,
        },
    },
};
//# sourceMappingURL=mqtt.js.map