import axios from "axios";
import dotenv from "dotenv";
import inquirer from "inquirer";

dotenv.config();

const SERVER_URL = process.env.POLICE_API_URL;

if (!SERVER_URL) {
  console.error("\x1b[31m!! ERROR: POLICE_API_URL is not defined in .env file\x1b[0m");
  process.exit(1);
}

interface PoliceResponse {
  ticket_id: string;
  processed_at: string;
  violation_details: {
    status: string;
    excess_speed: number;
  };
  payment_info: {
    amount: number;
    currency: string;
    deadline?: string;
  };
}

const sendViolationData = async (plate: string, speed: number) => {
  const cameraPayload = {
    plate,
    detectedSpeed: speed,
    speedLimit: 110,
    timestamp: new Date().toISOString(),
  };

  console.log("\n-----------------------------------------");
  console.log(`Sending Data to Police Station at [${SERVER_URL}]`);
  console.log("-----------------------------------------");

  try {
    const { data } = await axios.post<PoliceResponse>(SERVER_URL, cameraPayload);

    console.log("==> RESPONSE RECEIVED!\n");
    console.log(`Ticket ID:       ${data.ticket_id}`);
    console.log(`Processed At:    ${data.processed_at}`);
    console.log(`Status:          ${data.violation_details.status}`);
    console.log(`Excess Speed:    ${data.violation_details.excess_speed} km/h`);

    if (data.payment_info.amount > 0) {
      console.log(`FINE AMOUNT:     ${data.payment_info.amount} ${data.payment_info.currency}`);
      if (data.payment_info.deadline) {
        console.log(`Deadline:        ${data.payment_info.deadline}`);
      }
    } else {
      console.log("No fine issued.");
    }
    console.log("-----------------------------------------\n");
  } catch (error: any) {
    if (axios.isAxiosError(error)) {
      if (error.code === "ECONNREFUSED") {
        console.error("\x1b[31m!! CONNECTION FAILED: Police Server unreachable.\x1b[0m");
        console.error("   Check server IP/port and if the server is running.\x1b[0m");
      } else if (error.response) {
        console.error(`\x1b[31m!! SERVER ERROR: ${error.response.status} ${error.response.statusText}\x1b[0m`);
      } else {
        console.error("\x1b[31m!! NETWORK ERROR:\x1b[0m", error.message);
      }
    } else {
      console.error("\x1b[31m!! UNKNOWN ERROR:\x1b[0m", error);
    }
  }
};

const startCamera = async () => {
  console.log("======= HIGHWAY SPEED CAMERA STARTED =======\n");

  while (true) {
    const { plate, speed } = await inquirer.prompt([
      {
        type: "input",
        name: "plate",
        message: 'Enter License Plate (or type "exit" to quit):',
        validate: (input) => input.trim() !== "" || "Plate cannot be empty",
      },
      {
        type: "number",
        name: "speed",
        message: "Enter Detected Speed (km/h):",
        when: (ans) => ans.plate.toLowerCase() !== "exit",
        validate: (value) => {
          if (typeof value !== "number" || isNaN(value)) {
            return "Speed must be a number";
          }
          return value > 0 ? true : "Speed must be a positive number";
        },
      },
    ]);

    if (plate.toLowerCase() === "exit") {
      console.log("Shutting down camera...");
      break;
    }

    if (typeof speed === "number" && !isNaN(speed)) {
      await sendViolationData(plate.trim().toUpperCase(), speed);
    }
  }
};

startCamera();
