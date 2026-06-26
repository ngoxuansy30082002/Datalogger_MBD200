import { env } from './env';

export const mqttConfig = {
  brokerUrl: env.MQTT_BROKER_URL,
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
      deviceResponse: (deviceId: string) => `datalogger/${deviceId}/response`,
    },
  },
};
