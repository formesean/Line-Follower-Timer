"use client";

import { useState, useEffect } from "react";
import axios from "axios";

export default function Home() {
  const [time, setTime] = useState("00:00:00");

  useEffect(() => {
    const fetchTime = async () => {
      try {
        const response = await axios.get("http://localhost:3001/timer");
        setTime(response.data.time);
      } catch (error) {
        console.error("Failed to fetch timer:", error);
      }
    };

    // Fetch time every 100ms
    const interval = setInterval(fetchTime, 100);

    return () => clearInterval(interval);
  }, []);

  return (
    <div className="flex items-center justify-center min-h-screen bg-gray-900 text-white">
      <h1 className="text-6xl font-bold">{time}</h1>
    </div>
  );
}
