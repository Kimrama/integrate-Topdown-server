import { useState, useEffect } from "react";
import mqtt from "mqtt";

function ReadSound() {
    const [soundData, setSoundData] = useState("");

    useEffect(() => {
        const client = mqtt.connect("ws://localhost:9001", {
            username: "Topdown",
            password: "123",
        });

        client.on("connect", () => {
            console.log("Connected to MQTT broker");
            client.subscribe("Sound", (err) => {
                if (!err) {
                    console.log("Subscribed to topic: Sound");
                } else {
                    console.log(err);
                }
            });
        });

        client.on("message", (topic, message) => {
            if (topic === "Sound") {
                setSoundData(message.toString());
            }
        });

        return () => {
            client.end();
        };
    }, []);

    return (
        <div className="App">
            <h1>Data from rasPI</h1>
            <p>{soundData}</p>
        </div>
    );
}

export default ReadSound;
