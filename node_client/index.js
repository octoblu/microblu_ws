var SkynetSerialPort = require('skynet-serial').SerialPort;
var skynet = require('skynet');
var firmata = require('firmata');

// CHANGE THIS value !
// it needs to be the uuid of the device you want to send commands to:
var sendId = 'f5f3e920-c5a7-11e3-8ee5-3511496cbd23';

var firm;
var pinState = 1;

function togglePin(){
  if(pinState){
    pinState = 0;
  }else{
    pinState = 1;
  }
  firm.digitalWrite(6, pinState);
  setTimeout(togglePin, 650);
}

var conn = skynet.createConnection({});

conn.on('ready', function(data){

  console.log('connected to skynet', data);

  var serialPort = new SkynetSerialPort(conn, sendId);

  firm = new firmata.Board(serialPort, {samplingInterval:60000}, function (err, ok) {
    if (err){
      console.log('could not connect to board----' , err);
    }
    console.log("board loaded", ok);
    togglePin();

  });

});


