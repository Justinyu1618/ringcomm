var ringcommDevice;
var mainGattServer;
var mainService;
var mainChar1;

var allData = [];

var heartRateNormal = true;
var calibrating = false;

var slidesLoaded = true;
var slideN = 1;
var slideURL =
  // "https://docs.google.com/presentation/d/e/2PACX-1vTzEvwGb-7xGt08LGmmguJcHoKfGyFUmAfN388o4v67clbOU1NqCHD1nYeJzsZGZQybKmECs5a85wid/embed?start=false&loop=false&delayms=6000000&slide=";
  "https://docs.google.com/presentation/d/e/2PACX-1vQOGnbZtc3LLzxaMI0vuaU9z5aW0l9QvJU1Bcamx37yIBmntTlzVuAn8xjfren074cok3GiZf0FyKgv/embed?start=false&loop=false&delayms=60000000&slide=";
var cameraOn = false;

const INPUTS_PARAMS = [
  {
    uuid: MAIN_CHAR1_UUID,
    name: "Twist Sensor",
    onValue: onTwistSensor,
  },
  {
    uuid: MAIN_CHAR2_UUID,
    name: "Strain Sensor",
    onValue: onStrainSensor,
  },
  {
    uuid: MAIN_CHAR3_UUID,
    name: "Touch Sensor",
    onValue: onTouchSensor,
  },
];

function onTwistSensor(val) {
  console.log("TWIST SENSOR: ", val);
  // if (val === 0) {
  //   sendText();
  // }
  if (currentPage === "Text") {
    sendText();
  } else if (slidesLoaded && currentPage === "Slides") {
    nextSlide();
  } else if (cameraOn && currentPage === "Camera") {
    takePicture();
  }
}

function onStrainSensor(val) {
  updateHeartRatePlot(val);
}

function onTouchSensor(val) {
  if (val === 0) {
    previousTab();
  } else if (val === 2) {
    nextTab();
  } else if (val === 1) {
    if (slidesLoaded && currentPage === "Slides") {
      nextSlide();
    } else if (cameraOn && currentPage === "Camera") {
      takePicture();
    }
  }
}

const inputs = [];
class Characteristic {
  constructor({ uuid, name, onValue }) {
    this.uuid = uuid;
    this.name = name;
    this.handleNotification = (event) => {
      const val = event.target.value.getUint8();
      console.log(this.name + ": " + val);
      allData.unshift(`(${allData.length}) ${name}: ${val}`);
      if (!calibrating) {
        onValue(val);
      }
      render();
    };
  }

  initialize(BLEService) {
    BLEService.getCharacteristic(this.uuid).then((characteristic) => {
      this.characteristic = characteristic;
      this.characteristic.startNotifications().then((_) => {
        console.log("> Started notifications for " + this.name);
        this.characteristic.addEventListener(
          "characteristicvaluechanged",
          this.handleNotification
        );
      });
    });
  }
}

// pairing sequence for bluetooth device
function onPairDeviceClicked() {
  console.log("Requesting Ring Comm...");
  navigator.bluetooth
    .requestDevice({
      filters: [{ services: [MAIN_SERVICE_UUID] }, { name: "Ring Comm" }],
    })
    .then((device) => {
      $("#pair-device-button").addClass("loading");
      console.log("> Requested " + device.name + " (" + device.id + ")");
      ringcommDevice = device;
      return device.gatt.connect();
    })
    .then((server) => {
      console.log("> Connected to GATT Server ");
      mainGattServer = server;
      return server.getPrimaryService(MAIN_SERVICE_UUID);
    })
    .then((service) => {
      console.log("> Found service");
      mainService = service;

      for (const inp_param of INPUTS_PARAMS) {
        const newInput = new Characteristic(inp_param);
        newInput.initialize(service);
        inputs.push(newInput);
      }
      render();
      $("#pair-device-button").removeClass("loading");
      setConnectionStatus(true);
      startCalibration();
    })
    .catch((error) => {
      console.log("Argh! " + error);
      $("#pair-device-button").removeClass("loading");
    });
}

countdown = 0;
function startCalibration() {
  if (calibrating) {
    return;
  }

  calibrating = true;
  $("#calibration").css("display", "block");
  countdown = 5;
  $("#calibration-countdown").text(countdown);
  $("#calibration-countdown").css("display", "block");
  countdownInterval = setInterval(() => {
    if (countdown > 0) {
      countdown -= 1;
      $("#calibration-countdown").text(countdown);
    }
  }, [1000]);

  $("#calibration-instructions").text(
    "Put on ringcomm, calibrating strain sensor..."
  );
  setTimeout(() => {
    $("#calibration-instructions").text("Leave ring untwisted");
    countdown = 10;
    setTimeout(() => {
      $("#calibration-instructions").text("Twist the ring");
      countdown = 10;
      setTimeout(() => {
        $("#calibration-instructions").text("Calibration Complete!");
        clearInterval(countdownInterval);
        $("#calibration-countdown").css("display", "none");
        $("#calibration").css("background-color", "lightgreen");
        setTimeout(() => {
          stopCalibration();
        }, 3000);
      }, 10000);
    }, [10000]);
  }, [5000]);
}

function stopCalibration() {
  countdown = 0;
  calibrating = false;
  $("#calibration").css("display", "none");
  $("#calibration").css("background-color", "lightsteelblue");
}

// ***** ACTIONS *****

function previousTab() {
  currentPageN =
    (pages.length + ((currentPageN - 1) % pages.length)) % pages.length;
  changePage(pages[currentPageN]);
  // Plotly.deleteTraces("graph", 0);
  // Plotly.addTraces("graph", { y: [0] });
}

function nextTab() {
  currentPageN =
    (pages.length + ((currentPageN + 1) % pages.length)) % pages.length;
  changePage(pages[currentPageN]);
  // Plotly.deleteTraces("graph", 0);
  // Plotly.addTraces("graph", { y: [0] });
}

function sendText() {
  const msg = $("#text-preset-input").val();
  const number = $("#text-number").val();
  if (msg && number) {
    console.log("SENDING TEXT: " + msg);
    const url = "/send-sms?msg=" + msg + "&number=" + number;
    fetch(url);
  }
}

function nextSlide() {
  if (slidesLoaded) {
    slideN += 1;
    const newUrl = slideURL + slideN;
    $("#slides")[0].src = newUrl;
  }
}

function previousSlide() {
  if (slidesLoaded) {
    slideN -= 1;
    const newUrl = slideURL + slideN;
    $("#slides")[0].src = newUrl;
  }
}

function onClearDataClicked() {
  allData = [];
  render();
}

// function onEnableTextClicked() {
//   textEnabled = !textEnabled;

//   $("#text-preset-input").css("display", textEnabled ? "block" : "none");
// }

const dec = new TextDecoder("utf-8");

function render() {
  // render device info
  if (ringcommDevice != undefined) {
    $("#device-info").css("display", "block");
    $("#no-device-info").css("display", "none");

    $("#device-info #name").text(ringcommDevice.name);
    $("#device-info #id").text(ringcommDevice.id);
    $("#device-info #connected").text(
      ringcommDevice.gatt.connected ? "true" : "false"
    );

    $("#heartrate-status").css("display", "block");
  }

  const hrStatus = $("#heartrate-status");
  if (heartRateNormal) {
    hrStatus.css("color", "green");
    hrStatus.text("Normal");
  } else {
    hrStatus.css("color", "red");
    hrStatus.text("Accelerated");
  }

  // render data container
  $("#data-container").empty();
  // console.log(String.fromCharCode(...allData));
  for (const data of allData) {
    // var strData = "0x";

    // for (let i = 0; i < d.byteLength; i++) {
    //   // strData += ("00" + d.getUint8(i).toString(16)).slice(-2);
    //   console.log(d.getUint8(i));
    // }
    // console.log(d.getUint8(d.byteLength - 1));
    $("#data-container").append(`<div>${data}<div>`);
  }
}

function getKeyCode(character) {
  switch (character) {
    case "enter":
      return 13;
    default:
      return character.charCodeAt(0);
  }
}

const canvas = document.getElementById("user-picture");
const context = canvas.getContext("2d");
const video = document.getElementById("user-video");

function setupCamera() {
  const button = document.getElementById("take-picture");

  const constraints = {
    video: true,
  };

  button.addEventListener("click", () => {
    $("#user-video").css("display", "initial");
    $("#user-picture").css("display", "none");
    $("#camera-container").css("background-color", "none");
    $("#camera-text").css("display", "none");
    $("#camera-download").css("display", "none");
    cameraOn = true;
  });

  const downloadButton = document.getElementById("camera-download");
  downloadButton.addEventListener("click", downloadPicture);

  navigator.mediaDevices.getUserMedia(constraints).then((stream) => {
    video.srcObject = stream;
    const { height, width } = stream.getVideoTracks()[0].getSettings();
    console.log(height, width);
    canvas.width = width;
    canvas.height = height;
  });
}

function takePicture() {
  if (cameraOn) {
    context.drawImage(video, 0, 0, canvas.width, canvas.height);
    $("#user-video").css("display", "none");
    $("#user-picture").css("display", "block");
    cameraOn = false;

    $("#camera-text").css("display", "block");
    $("#camera-download").css("display", "block");
  }
}

function downloadPicture() {
  if (!cameraOn) {
    const image = canvas
      .toDataURL("image/png", 1.0)
      .replace("image/png", "image/octet-stream");
    const link = document.createElement("a");
    link.download = "ringcomm_pic.png";
    link.href = image;
    link.click();
  }
}

// console.log(navigator);
// console.log(navigator.MediaDevices);
// console.log("supported: ", "mediaDevices" in navigator);

setupCamera();

function setConnectionStatus(connected) {
  if (connected) {
    $("#status-connected").css("display", "block");
    $("#status-disconnected").css("display", "none");
  } else {
    $("#status-connected").css("display", "none");
    $("#status-disconnected").css("display", "block");
  }
}

// const MAX_DATA_LEN = 30;
// var heartRateData = [0];
function setupHeartRatePlot() {
  const time = new Date();
  const initData = [
    { x: [time], y: [0], mode: "lines", line: { color: "#80CAF6" } },
  ];
  Plotly.newPlot("graph", initData);
}

setupHeartRatePlot();

function updateHeartRatePlot(val) {
  const time = new Date();
  const update = {
    x: [[time]],
    y: [[val]],
  };

  const start = time.setSeconds(time.getSeconds() - 10);
  const end = time.setSeconds(time.getSeconds() + 10);

  const minutesView = {
    xaxis: {
      type: "date",
      range: [start, end],
    },
  };

  Plotly.relayout("graph", minutesView);
  Plotly.extendTraces("graph", update, [0], 200);
}
