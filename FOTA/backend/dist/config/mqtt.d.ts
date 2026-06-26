export declare const mqttConfig: {
    brokerUrl: string;
    options: {
        clientId: string;
        clean: boolean;
        connectTimeout: number;
        reconnectPeriod: number;
        keepalive: number;
    };
    topics: {
        subscribe: string[];
        publish: {
            firmwareUpdate: string;
            firmwareNotify: string;
            deviceResponse: (deviceId: string) => string;
        };
    };
};
//# sourceMappingURL=mqtt.d.ts.map