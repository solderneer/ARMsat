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
    .uint16('dustconc')
    .uint16('heading')
    .uint16('voltage_cell1')
    .uint16('voltage_cell2')
    .uint16('voltage_cell3')
    .uint16('current')
    .uint8('endbyte')
    .uint8('startbyte1')
    .uint8('startbyte2');

export var SerialInit = function (/*selBaudrate, selCOMPort*/attitude, heading, altimeter){
    /*
    var port = new SerialPort(selCOMport, { baudRate: selBaudrate, parser: SerialPort.parsers.byteDelimiter([171,205])}, function (err) {
        if (err) {
            return console.log('Error:', err.message);
        }
    });

    port.on('data', function (data) {
        data = toHexString(data);
        var buf = new Buffer(data, 'hex');
        console.log(payload.parse(buf));
    });*/

    var buf = new Buffer('117109a40002041e0000182f010329e68feddb69495155abcd', 'hex');

    setInterval(function(){
        console.log(payload.parse(buf));
        currentDataPoint = payload.parse(buf);
        tableUpdate(attitude, heading, altimeter);
    }, 500);

};


export var getDataPoint = function () {
    return currentDataPoint;
};

var tableUpdate = function (attitude, heading, altimeter) {
    document.getElementById('cell1').innerText = currentDataPoint.voltage_cell1 + " V";
    document.getElementById('cell2').innerText = currentDataPoint.voltage_cell2 + " V";
    document.getElementById('cell3').innerText = currentDataPoint.voltage_cell3 + " V";
    document.getElementById('cell4').innerText = "-";

    document.getElementById('total-current').innerHTML = currentDataPoint.current + " A";
    document.getElementById('total-cell').innerHTML = currentDataPoint.voltage_cell3 + " V";

    document.getElementById('pressure').innerText = currentDataPoint.pressure + " Pa";
    document.getElementById('altitude').innerText = currentDataPoint.altitude + " m";
    document.getElementById('temperature').innerText = currentDataPoint.temperature + " Celsius";
    document.getElementById('windspeed').innerText = "-" ;
    document.getElementById('gas').innerText = currentDataPoint.dustconc  + " ppm";
    document.getElementById('humidity').innerText = currentDataPoint.humidity  + " .rel";
    document.getElementById('yaw').innerText = currentDataPoint.heading + " deg";

    /*
    attitude.setRoll(30*Math.sin(increment/10));
    attitude.setPitch(50*Math.sin(increment/20));*/
    
    // Heading update
    heading.setHeading(currentDataPoint.heading);

    // Altimeter update
    altimeter.setAltitude(currentDataPoint.altitude);
    altimeter.setPressure(currentDataPoint.pressure);

};