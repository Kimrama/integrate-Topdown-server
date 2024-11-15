/* eslint-disable react/prop-types */
import {
    ResponsiveContainer,
    LineChart,
    CartesianGrid,
    XAxis,
    YAxis,
    Tooltip,
    Legend,
    Line,
} from "recharts";

const Pressure = ({ zoomedData }) => {
    return (
        <div className="mt-2">
            <h2 className="text-2xl ml-[50px] bg-slate-300 w-fit p-2">
                Pressure
            </h2>
            <ResponsiveContainer width="100%" height={400}>
                <LineChart data={zoomedData} syncId="anyId">
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis
                        dataKey="date"
                        angle={45}
                        textAnchor="start"
                        height={120}
                        tick={{ stroke: "black", strokeWidth: 0.5 }}
                    />
                    <YAxis
                        domain={["auto", "auto"]}
                        tick={{ stroke: "black", strokeWidth: 0.5 }}
                    />
                    <Tooltip />
                    <Legend verticalAlign="top" />
                    <Line
                        type="monotone"
                        dataKey="Pressure"
                        stroke="#82ca9d"
                        strokeWidth={1.5}
                    />
                </LineChart>
            </ResponsiveContainer>
        </div>
    );
};
export default Pressure;
