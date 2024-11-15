import { useEffect, useState, useRef } from "react";
import dayjs from "dayjs";

import Energy from "./graphs/energy";
import Pressure from "./graphs/pressure";
import Position from "./graphs/position";
import Force from "./graphs/force";
import ReadSound from "./graphs/readSound";

function App() {
    const [data, setData] = useState([]);
    const [isConnected, setIsConnected] = useState(false);
    const [paused, setPaused] = useState(false);
    const [zoomLevel, setZoomLevel] = useState(1);
    const socketRef = useRef(null);
    const API_KEY = "89159ab4ca6d8bd2347a7871def5ac46";
    const chartDataRef = useRef([]);
    const RECONNECT_DELAY = 5000;
    const isPausedRef = useRef(false);

    useEffect(() => {
        console.log("Paused state changed:", paused);
        isPausedRef.current = paused;
    }, [paused]);

    const connectWebSocket = () => {
        console.log("connect");
        socketRef.current = new WebSocket("ws://technest.ddns.net:8001/ws");

        socketRef.current.onopen = () => {
            console.log("WebSocket connected");
            setIsConnected(true);
            socketRef.current.send(API_KEY);
        };

        socketRef.current.onmessage = (event) => {
            if (isPausedRef.current) return;

            try {
                const parsedData = JSON.parse(event.data);
                const newData = {
                    Energy: parsedData["Energy Consumption"]?.Power,
                    Cycle_Count: parsedData?.["Cycle Count"],
                    Pressure: parsedData?.Pressure,
                    Position_of_the_Punch:
                        parsedData?.["Position of the Punch"],
                    Force: parsedData?.Force,
                    date: dayjs().format("YYYY-MM-DD HH:mm:ss"),
                };
                if (chartDataRef.current.length >= 200) {
                    chartDataRef.current = [];
                }
                chartDataRef.current = [...chartDataRef.current, newData];
                setData(chartDataRef.current);
            } catch (error) {
                console.error("Error parsing JSON:", error);
            }
        };

        socketRef.current.onclose = () => {
            console.log(
                "WebSocket connection closed, attempting to reconnect..."
            );
            setIsConnected(false);
            setTimeout(connectWebSocket, RECONNECT_DELAY);
        };

        socketRef.current.onerror = (error) => {
            console.error("WebSocket error:", error);
            socketRef.current.close();
        };
    };

    useEffect(() => {
        connectWebSocket();

        return () => {
            if (socketRef.current) {
                socketRef.current.close();
            }
        };
    }, []);
    const zoomIn = () => {
        setZoomLevel((prev) => Math.min(prev + 1, 5));
    };

    const zoomOut = () => {
        setZoomLevel((prev) => Math.max(prev - 1, 1));
    };

    const zoomedData = data.slice(
        Math.max(0, data.length - 50 * zoomLevel),
        data.length
    );

    const togglePause = () => {
        setPaused((prev) => !prev);
    };

    return (
        <>
            <div className="flex items-center">
                <div>
                    <h1 className="text-5xl text-blue-500 font-bold m-[30px]">
                        MACHINE DASHBOARD
                    </h1>
                    <div className="flex">
                        <div
                            className={`text-3xl p-3 ml-[30px] w-[250px] rounded ${
                                isConnected ? "bg-green-400" : "bg-red-500"
                            } mr-[100px]`}
                        >
                            {isConnected ? "CONNECTED" : "DISCONNECT"}
                        </div>
                    </div>
                </div>
                <div>
                    <ReadSound />
                </div>
            </div>
            {isConnected && (
                <>
                    <div
                        className={`text-3xl p-3 ml-[30px] w-[250px] rounded bg-orange-200 m-2`}
                    >
                        {`PUNCH: ${
                            data.length > 0
                                ? data[data.length - 1].Cycle_Count
                                : "N/A"
                        }`}
                        <button
                            onClick={togglePause}
                            className="bg-blue-500 text-white p-2 m-4 rounded"
                        >
                            {paused ? "Resume Graph" : "Pause Graph"}
                        </button>
                    </div>
                    <br />
                    <Energy zoomedData={zoomedData} />
                    <Pressure zoomedData={zoomedData} />
                    <Position zoomedData={zoomedData} />
                    <Force zoomedData={zoomedData} />

                    <div className="flex justify-center mt-4">
                        <button
                            onClick={zoomIn}
                            className="px-4 py-2 mx-2 bg-blue-500 text-white rounded"
                        >
                            Zoom In
                        </button>
                        <button
                            onClick={zoomOut}
                            className="px-4 py-2 mx-2 bg-blue-500 text-white rounded"
                        >
                            Zoom Out
                        </button>
                    </div>
                </>
            )}
        </>
    );
}

export default App;
