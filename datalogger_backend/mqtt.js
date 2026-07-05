const { MongoClient } = require('mongodb');
const mqtt = require('mqtt');

const uri = "mongodb://heavystorm2708_db_user:matkhau123@ac-sxa3kv2-shard-00-00.jq8reyr.mongodb.net:27017,ac-sxa3kv2-shard-00-01.jq8reyr.mongodb.net:27017,ac-sxa3kv2-shard-00-02.jq8reyr.mongodb.net:27017/?ssl=true&replicaSet=atlas-nuwwp3-shard-0&authSource=admin&appName=Cluster0";
const MQTT_BROKER = 'mqtt://broker.emqx.io:1883';
const MQTT_TOPICS = ['/datalogger/sensor', '/datalogger/notif', '/datalogger/status'];

const client = new MongoClient(uri);

async function startMQTT() {
    try {
        await client.connect();
        console.log("[Database] Connected to MongoDB.");

        const db = client.db("Datalogger_MBD200");
        const sensorCollection = db.collection("SensorValues");
        const notificationCollection = db.collection("Notifications");
        const statusCollection = db.collection("SystemStatus");

        await sensorCollection.createIndex({ "timestamp": 1 }, { expireAfterSeconds: 604800 });
        await notificationCollection.createIndex({ "timestamp": 1 }, { expireAfterSeconds: 2592000 });
        await statusCollection.createIndex({ "timestamp": 1 }, { expireAfterSeconds: 604800 });

        const mqttClient = mqtt.connect(MQTT_BROKER);

        mqttClient.on('connect', () => {
            console.log("[MQTT] Connected to Broker.");
            mqttClient.subscribe(MQTT_TOPICS, (err) => {
                if (!err) console.log(`[MQTT] Subscribed to: ${MQTT_TOPICS.join(', ')}`);
            });
        });

        mqttClient.on('message', async (topic, payload) => {
            const rawPayload = payload.toString();
            try {
                const jsonData = JSON.parse(rawPayload);
                const currentTime = new Date();

                switch (topic) {
                    case '/datalogger/sensor':
                        if (Array.isArray(jsonData) && jsonData.length > 0) {
                            const sensorsToProcess = jsonData.slice(0, 20);
                            const bulkData = [];

                            for (const sensor of sensorsToProcess) {
                                if (sensor.name && sensor.value !== undefined) {
                                    bulkData.push({
                                        sensor_name: sensor.name,
                                        value: parseFloat(sensor.value),
                                        unit: sensor.unit || "",
                                        status_text: sensor.statusText || "Tốt",
                                        type: (sensor.type || "ANALOG").toUpperCase(), // Bóc tách trường chuẩn kết nối từ phần cứng
                                        timestamp: currentTime
                                    });
                                }
                            }

                            if (bulkData.length > 0) {
                                await sensorCollection.insertMany(bulkData);
                                console.log(`[DB - Sensor] Bulk saved ${bulkData.length} sensors with interface types.`);
                            }
                        }
                        break;

                    case '/datalogger/notif':
                        await notificationCollection.insertOne({
                            alert_type: jsonData.alert_type || "Cảnh báo",
                            target: jsonData.target || "Hệ thống",
                            detail: jsonData.detail || rawPayload,
                            severity: jsonData.severity || "INFO",
                            timestamp: currentTime
                        });
                        break;

                    case '/datalogger/status':
                        await statusCollection.insertOne({
                            cpuLoad: parseFloat(jsonData.cpuLoad) || 0,
                            memory: parseFloat(jsonData.memory) || 0,
                            storage: parseFloat(jsonData.storage) || 0,
                            uptime: jsonData.uptime || "0d 0h 0m",
                            services: {
                                mqttBroker: jsonData.services?.mqttBroker || "Inactive",
                                ftpServer: jsonData.services?.ftpServer || "Inactive",
                                rs485Modbus: jsonData.services?.rs485Modbus || "Inactive",
                                network: jsonData.services?.network || "Inactive",
                                sdCard: jsonData.services?.sdCard || "No insert"
                            },
                            timestamp: currentTime
                        });
                        break;
                }
            } catch (err) {
                console.error(`[Worker] Error decoding payload on topic ${topic}:`, err.message);
            }
        });

    } catch (error) {
        console.error("[System] Severe backend initialization failed:", error);
    }
}

startMQTT();