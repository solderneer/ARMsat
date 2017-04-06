var SerialPort = require("serialport");
var jquery = require('jquery');
var Parser = require('binary-parser').Parser;
window.jQuery = window.$ = require('jquery');

var currentDataPoint;

var payload = new Parser()
    .endianess('big')
    .uint16('humidity')
    .uint16('temperature')
    .uint32('pressure')
    .uint16('altitude')
    .uint16('heading')
    .uint16('dustconc')
    .uint16('windspeed')
    .uint16('voltage_cell1')
    .uint16('voltage_cell2')
    .uint16('voltage_cell3')
    .uint16('current')
    .uint8('endbyte')
    .uint8('startbyte1')
    .uint8('startbyte2');

function toHexString(byteArray) {
  return byteArray.map(function(byte) {
    return ('0' + (byte & 0xFF).toString(16)).slice(-2);
  }).join('')
}


export var SerialInit = function (selBaudrate, selCOMPort, attitude, heading, altimeter){
    /*var port = new SerialPort(selCOMPort, { baudRate: selBaudrate, parser: SerialPort.parsers.byteDelimiter([171,205])}, function (err) {
        if (err) {
            return console.log('Error:', err.message);
        }
    });

    port.on('data', function (data) {
        data = toHexString(data);
        console.log(data);
        var buf = new Buffer(data, 'hex');
        console.log(payload.parse(buf));
        currentDataPoint = payload.parse(buf);
        tableUpdate(attitude, heading, altimeter);
    });*/

    var buf = new Buffer('117109a40002041e0000182f010329e68feddb694951000055abcd', 'hex');

    setInterval(function(){
        console.log(payload.parse(buf));
        currentDataPoint = payload.parse(buf);
        tableUpdate(attitude, heading, altimeter);
    }, 500);

    var port = 1;
    return port; 
};


export var getDataPoint = function () {
    return currentDataPoint;
};

var tableUpdate = function (attitude, heading, altimeter) {
    document.getElementById('cell1').innerText = (currentDataPoint.voltage_cell1/1000) + " V";
    document.getElementById('cell2').innerText = (currentDataPoint.voltage_cell2/1000) + " V";
    document.getElementById('cell3').innerText = (currentDataPoint.voltage_cell3/1000) + " V";
    document.getElementById('cell4').innerText = "-";

    document.getElementById('total-current').innerHTML = currentDataPoint.current + " A";
    document.getElementById('total-cell').innerHTML = ((currentDataPoint.voltage_cell3 + currentDataPoint.voltage_cell2 + currentDataPoint.voltage_cell1)/1000) + " V";

    document.getElementById('pressure').innerText = currentDataPoint.pressure + " Pa";
    document.getElementById('altitude').innerText = currentDataPoint.altitude + " m";
    document.getElementById('temperature').innerText = (currentDataPoint.temperature/100) + " Celsius";
    document.getElementById('windspeed').innerText = "-" ;//stubbed out
    document.getElementById('gas').innerText = "-";//stubbed out
    document.getElementById('humidity').innerText = (currentDataPoint.humidity/100)  + " %";
    document.getElementById('yaw').innerText = currentDataPoint.heading + " deg";

    /*
    attitude.setRoll(30*Math.sin(increment/10));
    attitude.setPitch(50*Math.sin(increment/20));*/
    
    // Heading update
    heading.setHeading(currentDataPoint.heading);

    // Altimeter update
    altimeter.setAltitude(currentDataPoint.altitude * 3.28084);
    altimeter.setPressure(currentDataPoint.pressure/100);

};