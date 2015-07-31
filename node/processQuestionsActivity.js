'use strict';

var https = require('https');
var async = require('async');
var dgram = require('dgram');

//////// CONSTANTS ///////
var DEFAULT_HOST_NAME = '10.1.31.13';
var DEFAULT_PORT = 2811;

// timeout in ms for when to requery db
var REQUERY_TIMEOUT_MS = 5000; // 5 seconds

// TODO: tweak these values
var NUM_RECENT_DATA_TO_SAVE = 10;
var DIST_FROM_AVG_MULTIPLIER = 20;

// url for making metrics queries
var QUERY_URL = 'https://pipe.creativelive.com/api/metrics/rolling/';

// questions-related query endpoints
var ENDPOINT_ASK_QUESTIONS = 'Ask+Question';
var ENDPOINT_VOTE = 'Vote+On+Question';
var ENDPOINT_TOGGLE = 'Toggle+Questions+Tool';

// servo speeds
var SERVO_ZERO = 92;
var SERVO_THRESHOLD = SERVO_ZERO + 5;
///////////////////////////

var hostAddress = DEFAULT_HOST_NAME;
var port = DEFAULT_PORT;
// trackers for calculating peak
var recentData = [];
var prevTotal = 0;
var prevPrevTotal = 0;

//////////////////
// entry point
parseArguments();
var client = dgram.createSocket('udp4');
interval();
/////////////////

/** parse flag+argument pairs from command line */
function parseArguments() {
  var args = process.argv;
  var hostFlagIndex = args.indexOf("-h");
  var portFlagIndex = args.indexOf("-p");
  if (hostFlagIndex != -1) {
    hostAddress = args[hostFlagIndex + 1];
  }
  if (portFlagIndex != -1) {
    port = args[portFlagIndex + 1];
  }
}

/** gets the amount of activity we have around questions */
function getQuestionsActivity(callback) {
  // new segments query
  async.parallel([
    function(callback) {
      return makeQuery(QUERY_URL + ENDPOINT_ASK_QUESTIONS, callback);
    },
    function(callback) {
      return makeQuery(QUERY_URL + ENDPOINT_VOTE, callback);
    },
    function(callback) {
      return makeQuery(QUERY_URL + ENDPOINT_TOGGLE, callback);
    }
  ], function(err, results) {
    if (err) {
      console.log('[ERROR]', err);
    }
    var sum = 0;
    results.forEach(function(count) {
      sum += count;
    })
    var ratio = updateDataAndGetRatio(sum);
    return sendSpeed(mapRatioToServoSpeed(ratio), callback);
  });
}

/** helper function to make queries to the metrics endpoint */
function makeQuery(url, callback) {
  https.get(url, function(res) {
    res.on('data', function(chunk) {
      var data = JSON.parse(chunk.toString());
      if (data.values && data.values.length > 0) {
        return callback(null, data.values[0].count);
      } else {
        return callback('invalid https get object for url: ' + url, 0);
      }
    });
    res.on('error', function(err) {
      return callback(err.message, 0);
    });
  })
}

/** estimate total activity's percentage of peak activity */
function updateDataAndGetRatio(totalActivity) {
  // update data
  if (recentData.length == NUM_RECENT_DATA_TO_SAVE) {
    recentData.shift();
  }
  recentData.push(totalActivity);

  // find ratio
  var avg = getAverage();
  var distFromAvg = (totalActivity - avg) * DIST_FROM_AVG_MULTIPLIER;
  console.log('dist:', distFromAvg);
  if (distFromAvg < 0) {
    distFromAvg = 0;
  }
  var estimatedPeak = avg * 2;
  var ratio = distFromAvg >= estimatedPeak ? 1 : distFromAvg / estimatedPeak;
  return ratio;
}

/** gets the average of recent peaks */
function getAverage() {
  var numData = recentData.length;
  if (numData == 0) {
    return 1;
  } else {
    var sum = 0;
    for (var i = 0; i < numData; i++) {
      sum += recentData[i];
    }
    return sum / numData;
  }
}

/** maps ratio of activity to servo speed */
function mapRatioToServoSpeed(ratio) {
  if (ratio == 0) {
    return SERVO_ZERO;
  }

  var servoSpeed = stripDecimal((180 - SERVO_THRESHOLD) * Math.pow(ratio, 2) + SERVO_THRESHOLD);
  console.log('ratio:', ratio, 'speed:', servoSpeed);

  if (servoSpeed < SERVO_THRESHOLD) {
    console.log('Below Threshold!\n');
    return SERVO_ZERO;
  }
  return servoSpeed;
}

/** sends the speed to the device */
function sendSpeed(speed, callback) {
  var message;
  if (speed == SERVO_ZERO) {
    message = new Buffer('clservo-stop:::;');
  } else {
    message = new Buffer('clservo-set:' + speed + '::;');
  }
  client.send(message, 0, message.length, port, hostAddress, function(err, bytes) {
      if (err) {
        throw err;
      }
      console.log('UDP message "' + message.toString() + '" sent to ' + hostAddress +':'+ port);
      callback(null, speed);
  });
}

/** helper for stripping the decimals from a number */
function stripDecimal(n) {
  return n | 0;
}
/** interval function that kicks off the query/write process every X seconds */
function interval() {
  getQuestionsActivity(function(err, results) {
    if (err) {
      throw err;
      return;
    }
    setTimeout(function() { interval(); }, REQUERY_TIMEOUT_MS);
  });
}