var ringcommDevice;
var mainGattServer;
var mainService;
var mainChar1;

var mainChar1Data = [];
var charData = "";
var currentCmd = "";

var textEnabled = false;
var cameraOn = false;

const cmdToAction = {
  2: sendText,
  "(0,1)": takePicture,
};

function sendText() {
  if (textEnabled) {
    const msg = $("#text-preset-input").val();
    // console.log("SENDING TEXT: " + msg);
    const url = "/send-sms?msg=" + msg;
    fetch(url);
  }
}

function onPairDeviceClicked() {
  console.log("Requesting Ring Comm...");
  navigator.bluetooth
    .requestDevice({
      filters: [{ services: [MAIN_SERVICE_UUID] }, { name: "Ring Comm" }],
    })
    .then((device) => {
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
      return service.getCharacteristic(MAIN_CHAR1_UUID);
    })
    .then((characteristic) => {
      mainChar1 = characteristic;
      render();
      return characteristic.startNotifications().then((_) => {
        console.log("> Started notifications");
        characteristic.addEventListener(
          "characteristicvaluechanged",
          handleNotification
        );
      });
    })
    .catch((error) => {
      console.log("Argh! " + error);
    });
}

function onClearDataClicked() {
  mainChar1Data = [];
  charData = "";
  render();
}

function onEnableTextClicked() {
  textEnabled = !textEnabled;

  $("#text-preset-input").css("display", textEnabled ? "block" : "none");
}

function handleNotification(event) {
  const value = event.target.value;
  const char = String.fromCharCode(value.getUint8());
  // console.log(char);
  charData += char;
  // mainChar1Data.push(String.fromCharCode(value.getUint8()));

  if (char === "\n") {
    handleDataActions(currentCmd);
    currentCmd = "";
  } else {
    currentCmd += char;
  }

  // handleDataActions(value);
  render();
}

function handleDataActions(cmd) {
  console.log("action: ", cmd);
  // const cmd = data.getUint8(0);
  if (cmdToAction[cmd]) {
    cmdToAction[cmd]();
  }
}

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
  }
  // render data container
  $("#data-container").empty();
  // console.log(String.fromCharCode(...mainChar1Data));
  for (const strData of charData.split("\n")) {
    // var strData = "0x";

    // for (let i = 0; i < d.byteLength; i++) {
    //   // strData += ("00" + d.getUint8(i).toString(16)).slice(-2);
    //   console.log(d.getUint8(i));
    // }
    // console.log(d.getUint8(d.byteLength - 1));
    $("#data-container").append(`<div>${strData}<div>`);
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
    cameraOn = true;
  });

  navigator.mediaDevices.getUserMedia(constraints).then((stream) => {
    video.srcObject = stream;
    const { height, width } = stream.getVideoTracks()[0].getSettings();
    canvas.width = width;
    canvas.height = height;
  });
}

function takePicture() {
  if (cameraOn) {
    context.drawImage(video, 0, 0, canvas.width, canvas.height);
    $("#user-video").css("display", "none");
    cameraOn = false;
  }
}

console.log(navigator);
console.log(navigator.MediaDevices);
console.log("supported: ", "mediaDevices" in navigator);

setupCamera();
