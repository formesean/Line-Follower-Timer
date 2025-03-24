const express = require("express");
const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");
const cors = require("cors");

const app = express();
app.use(cors());

const PORT = 3001;
let serialPort;
let lastTime = "00:00:00";
let scanning = false;

// Function to find Arduino's COM port
async function findCOMPort() {
  try {
    const ports = await SerialPort.list();
    console.log("Detected Ports:", ports);

    return ports.find(
      (port) =>
        port.path &&
        (port.manufacturer?.includes("Arduino") ||
          port.friendlyName?.includes("USB") ||
          port.path.includes("ttyUSB") ||
          port.path.includes("ttyACM") ||
          port.path.includes("COM"))
    );
  } catch (err) {
    console.error("Error detecting serial port:", err);
    return null;
  }
}

// Function to initialize Serial Connection
async function initSerial() {
  if (scanning) return;
  scanning = true;

  while (true) {
    const arduinoPort = await findCOMPort();
    if (arduinoPort) {
      console.log(`Arduino found on: ${arduinoPort.path}`);
      connectSerial(arduinoPort.path);
      break;
    } else {
      console.log("No Arduino found! Retrying in 5s...");
      await new Promise((resolve) => setTimeout(resolve, 5000));
    }
  }

  scanning = false;
}

// Function to handle Serial Communication
function connectSerial(portPath) {
  serialPort = new SerialPort({ path: portPath, baudRate: 115200 });
  const parser = serialPort.pipe(new ReadlineParser({ delimiter: "\n" }));

  parser.on("data", (data) => {
    const trimmedData = data.trim();
    console.log("Received:", trimmedData);

    const [minutes, seconds, milliseconds] = trimmedData.split(":");

    if (minutes && seconds && milliseconds) {
      let adjustedMillis = parseInt(milliseconds, 10) + 4;

      let adjustedSeconds = parseInt(seconds, 10);
      let adjustedMinutes = parseInt(minutes, 10);

      if (adjustedMillis >= 100) {
        adjustedMillis -= 100;
        adjustedSeconds = (adjustedSeconds + 1) % 60;
        if (adjustedSeconds === 0) {
          adjustedMinutes = (adjustedMinutes + 1) % 60;
        }
      }

      lastTime = `${adjustedMinutes
        .toString()
        .padStart(2, "0")}:${adjustedSeconds
        .toString()
        .padStart(2, "0")}:${adjustedMillis.toString().padStart(2, "0")}`;
    }
  });

  serialPort.on("close", () => {
    console.error("🔌 Serial connection lost! Reconnecting...");
    setTimeout(initSerial, 5000);
  });

  serialPort.on("error", (err) => {
    console.error("Serial Port Error:", err);
    setTimeout(initSerial, 5000);
  });
}

// API Endpoint to get the timer value
app.get("/timer", (req, res) => {
  res.json({ time: lastTime });
});

// Start Server & Serial Connection
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
  initSerial();
});
