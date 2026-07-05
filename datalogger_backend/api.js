const { MongoClient } = require('mongodb');
const express = require('express');
const cors = require('cors');

const uri = "mongodb://heavystorm2708_db_user:matkhau123@ac-sxa3kv2-shard-00-00.jq8reyr.mongodb.net:27017,ac-sxa3kv2-shard-00-01.jq8reyr.mongodb.net:27017,ac-sxa3kv2-shard-00-02.jq8reyr.mongodb.net:27017/?ssl=true&replicaSet=atlas-nuwwp3-shard-0&authSource=admin&appName=Cluster0";
const client = new MongoClient(uri);
const app = express();

app.use(cors());

async function startAPI() {
    try {
        await client.connect();
        console.log("[Database] Connected to MongoDB.");

        const notificationCollection = client.db("Datalogger_MBD200").collection("Notifications");
        const sensorCollection = client.db("Datalogger_MBD200").collection("SensorValues");

        app.get('/', (req, res) => {
            res.send('Datalogger MBD-200 Backend is running');
        });

        app.get('/api/sync', async (req, res) => {
            try {
                const lastSyncStr = req.query.lastSyncTime;
                let query = {};

                if (lastSyncStr && lastSyncStr !== "0") {
                    const lastSyncDate = new Date(parseInt(lastSyncStr));
                    query = { timestamp: { $gt: lastSyncDate } };
                }

                const history = await notificationCollection.find(query).sort({ timestamp: -1 }).limit(50).toArray();
                res.json(history);
            } catch (err) {
                res.status(500).json({ error: "Internal Server Error" });
            }
        });

        app.get('/api/sensors/:name', async (req, res) => {
            try {
                const sensorName = req.params.name;
                const sensorData = await sensorCollection.find({ sensor_name: sensorName })
                    .sort({ timestamp: -1 })
                    .limit(60)
                    .toArray();
                res.json(sensorData.reverse());
            } catch (e) {
                res.status(500).json({ error: "Internal Server Error" });
            }
        });

        const PORT = process.env.PORT || 3000;
        app.listen(PORT, '0.0.0.0', () => {
            console.log(`[Server] API is listening on port ${PORT}.`);
        });

    } catch (error) {
        console.error("[System] Initialization failed:", error);
    }
}

startAPI();