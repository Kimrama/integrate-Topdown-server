const WebSocket = require("ws");
const { MongoClient } = require("mongodb");
const { v4: uuidv4 } = require("uuid");

const apiKey = process.env.API_KEY || "a6d6b457cd4b3813f16e5b0d3816613d";
const wsUrl = process.env.WS_URL || "ws://technest.ddns.net:8001/ws";
const mongoUrl = process.env.MONGO_URL || "mongodb://localhost:27017/";
const dbName = "integration";

const client = new MongoClient(mongoUrl, {
    useNewUrlParser: true,
    useUnifiedTopology: true,
});

async function connectMongoDB() {
    try {
        await client.connect();
        console.log("Connected to MongoDB");
    } catch (err) {
        console.error("Error connecting to MongoDB:", err);
        setTimeout(connectMongoDB, 5000);
    }
}

async function insertData(parsedData) {
    try {
        const db = client.db(dbName);
        const collection = db.collection("machine");
        const now = new Date();
        const dataToInsert = {
            _id: uuidv4(),
            energy_consumption: parsedData["Energy Consumption"],
            voltage: parsedData["Voltage"],
            pressure: parsedData["Pressure"],
            force: parsedData["Force"],
            cycle_count: parsedData["Cycle Count"],
            position_of_the_punch: parsedData["Position of the Punch"],
            timestamp: now.toLocaleString("th-TH", {
                timeZone: "Asia/Bangkok",
                hour12: false,
            }),
        };
        console.log(dataToInsert);
        await collection.insertOne(dataToInsert);

        console.log("Data inserted into MongoDB");
    } catch (err) {
        console.error("Error inserting data into MongoDB:", err);
    }
}

const ws = new WebSocket(wsUrl);

ws.on("open", () => {
    console.log("Connected to WebSocket");
    ws.send(apiKey);
});

ws.on("message", async (data) => {
    const rawString = data.toString("utf8");
    try {
        const parsedData = JSON.parse(rawString);
        console.log("Parsed Data:", parsedData);

        await insertData(parsedData);
    } catch (err) {
        console.error("Error parsing or inserting data:", err);
    }
});

ws.on("close", () => {
    console.log("WebSocket closed");
    client.close();
});

ws.on("error", (err) => {
    console.error("WebSocket error:", err);
});

connectMongoDB();
