// Create Temperature Gauge
var gaugeTemp = new LinearGauge({
  renderTo: 'gauge-temperature',
  width: 150,
  height: 450,
  units: "RPM ℃",
  minValue: 0,
  startAngle: 90,
  ticksAngle: 180,
  maxValue: 5500,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
      "0",
      "20",
      "40",
      "60",
      "80",
      "100",
      "120",
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 100,
          "to": 120,
          "color": "rgba(200, 50, 50, .75)"
      }
  ],
  colorPlate: "#fff",
  colorBarProgress: "#CC2936",
  colorBarProgressEnd: "#049faa",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  needleCircleSize: 7,
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
  barWidth: 10,
}).draw();
  
// Create Pressure Gauge
var gaugeRMP = new RadialGauge({
  renderTo: 'gauge-rpm',
  width: 300,
  height: 300,
  units: "RPM",
  minValue: 0,
  maxValue: 5500,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 2,
  majorTicks: [
      "0",
      "1000",
      "2000",
      "3000",
      "4000",
      "5000",
      "5500"

  ],
  minorTicks: 1,
  strokeTicks: true,
  highlights: [
      {
          "from": 5000,
          "to": 5500,
          "color": "#007F80"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007474",
  colorNeedleEnd: "#800000",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Create Speed Gauge
var gaugeSpeed = new RadialGauge({
  renderTo: 'gauge-speed',
  width: 300,
  height: 300,
  units: "Speed(kph)",
  minValue: 0,
  maxValue: 260,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 2,
  majorTicks: [
      "0",
      "20",
      "40",
      "60",
      "80",
      "100", 
      "120",
      "140",
      "160",
      "180",
      "200",
      "220",
      "240",
      "260"
  ],
  minorTicks: 1,
  strokeTicks: true,
  highlights: [
      {
          "from": 260,
          "to": 260,
          "color": "#007F80"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007474",
  colorNeedleEnd: "#800000",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();


var Socket;

function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onmessage = function(event) {
    onMessages(event);
  };
}

// function setRelayStatus(key, status)
// {
//   var relay_element = document.getElementById(key+"-status");
//   relay_element.className = '';
//   relay_element.classList.add('ConnectionIndicator');
//   if (status == 1)
//   {
//     var element = document.getElementById("sys_status");
//     element.innerHTML = "Connections state: Connected";
//     relay_element.classList.add('ConnectionIndicator--connected');
//   } else {
//     var element = document.getElementById("sys_status");
//     element.innerHTML = "Connection state: Disconnected";
//     relay_element.classList.add('ConnectionIndicator--disconnected');
//   }
// }
function onMessages(event) {
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);
    for (var i = 0; i < keys.length; i++){
      var key = keys[i];
      var fValue = parseInt(myObj[key]);
      if (key == "speed"){
        
        gaugeSpeed.value = fValue;
      } else if (key === "rmp") {
        gaugeRMP.value = fValue;
      } else if ( key === "coolantTemperature") {
        gaugeTemp.value = fValue; 
      }
      // } else if(key === "sys") {
      //   setRelayStatus(key,parseInt(myObj[key]));
      // }
	}
}

window.onload = function(event) {
  init();
}