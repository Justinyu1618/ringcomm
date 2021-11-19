var ringcommDevice;
var mainGattServer;
var mainService;
var mainChar1;

var allData = [];

var heartRateNormal = true;

var slidesLoaded = true;
var slideN = 1;
var slideURL =
  "https://docs.google.com/presentation/d/e/2PACX-1vQZVdiAEErHgcn2qnHv-Dx8sK1L7O8xA4LgOu32_kZgxN4EiZfJFBq6DW8pWoxCWoRxPJcmN1mNENdw/embed?start=false&loop=false&delayms=6000000&slide=";
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

const inputs = [];
class Characteristic {
  constructor({ uuid, name, onValue }) {
    this.uuid = uuid;
    this.name = name;
    this.handleNotification = (event) => {
      const val = event.target.value; //.getUint8();
      console.log(this.name + ": " + val);
      allData.unshift(`(${allData.length}) ${name}: ${val}`);
      onValue(val);
      render();
    };
  }

  initialize(BLEService) {
    if (!BLEService) {
      window.addEventListener("keypress", (event) => {
        console.log(event.key, this.name[2]);
        if (event.key === this.name[2]) {
          this.handleNotification({ target: { value: 1 } });
        }
      });
      return;
    }

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

function onTwistSensor(val) {
  console.log("TWIST SENSOR: ", val);
  if (val === 1) {
    if (slidesLoaded && currentPage === "Slides") {
      nextSlide();
    } else if (cameraOn && currentPage === "Camera") {
      takePicture();
    }
  }
}

function onStrainSensor(val) {
  if (val === 1) {
    heartRateNormal = false;
  } else if (val === 0) {
    heartRateNormal = true;
  }
}

function onTouchSensor(val) {
  if (val === 1) {
    sendText();
  }
}

function onPairDeviceClicked() {
  // for (const inp_param of INPUTS_PARAMS) {
  //   const newInput = new Characteristic(inp_param);
  //   newInput.initialize(false);
  //   inputs.push(newInput);
  // }
  // render();

  // return;

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
    })
    .catch((error) => {
      console.log("Argh! " + error);
      $("#pair-device-button").removeClass("loading");
    });
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
    cameraOn = true;
  });

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
