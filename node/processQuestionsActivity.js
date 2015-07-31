'use strict';

var net = require('net');
var https = require('https');
var async = require('async');

//////// CONSTANTS ///////
var DEFAULT_HOST_NAME = '10.1.31.13';
var DEFAULT_PORT = 7;

// timeout in ms for when to requery db
var REQUERY_TIMEOUT_MS = 5000; // 5 seconds

// TODO: tweak these values
var INITIAL_PEAK_AVG = 10;
var NUM_RECENT_PEAKS_TO_SAVE = 5;

// url for making metrics queries
var QUERY_URL = 'https://pipe.creativelive.com/api/metrics/rolling/';

// questions-related query endpoints
var ENDPOINT_ASK_QUESTIONS = 'Ask+Question';
var ENDPOINT_VOTE = 'Vote+On+Question';
var ENDPOINT_TOGGLE = 'Toggle+Questions+Tool';

// servo speeds
var SERVO_ZERO = 98;
var SERVO_THRESHOLD = SERVO_ZERO + 5;
///////////////////////////

var hostAddress = DEFAULT_HOST_NAME;
var port = DEFAULT_PORT;
// trackers for calculating peak
var recentPeaks = [INITIAL_PEAK_AVG];
var prevTotal = 0;
var prevPrevTotal = 0;

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
    console.log('per metric:', results);
    if (err) {
      console.log('[ERROR]', err);
    }
    var sum = 0;
    results.forEach(function(count) {
      sum += count;
    })
    var percent = calculatePeakAndGetPercent(sum);
    return sendSpeed(mapPercentToServoSpeed(percent), callback);
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

/** uses the total activity to calculate/update new peak if needed and get percentage of activity  */
function calculatePeakAndGetPercent(totalActivity) {
  // try update peak info
  if (prevPrevTotal <= prevTotal && prevTotal > totalActivity) {
    // previous total was a peak
    if (recentPeaks.length == NUM_RECENT_PEAKS_TO_SAVE) {
      recentPeaks.shift();
    }
    recentPeaks.push(prevTotal);
  }
  // update previous trackers
  prevPrevTotal = prevTotal;
  prevTotal = totalActivity;

  var peakAvg = getPeakAverage();
  var percent = totalActivity >= peakAvg ? 100 : stripDecimal((totalActivity / peakAvg) * 100);
  console.log('total:', totalActivity, 'avg:', peakAvg, 'percent:', percent);
  return percent;
}

/** gets the average of recent peaks */
function getPeakAverage() {
  var numPeaks = recentPeaks.length;
  if (numPeaks == 0) {
    return INITIAL_PEAK_AVG;
  } else {
    var sum = 0;
    for (var i = 0; i < numPeaks; i++) {
      sum += recentPeaks[i];
    }
    return sum / numPeaks;
  }
}

/** maps percent of activity to servo speed */
function mapPercentToServoSpeed(percent) {
  var x = ((percent / 100) * 65) / 100;
  var servoSpeed = stripDecimal((180 - SERVO_THRESHOLD) * Math.pow(x, 3) + SERVO_THRESHOLD);

  if (servoSpeed < SERVO_THRESHOLD) {
    console.log('Below Threshold!\n');
    return servoSpeed = SERVO_ZERO;
  }
  return servoSpeed;
}

/** sends the speed to the device */
function sendSpeed(speed, callback) {
  client.write('servo:' + speed + '::;', function() {
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

parseArguments();

/** TCP connection */
var client = net.connect({
  host: hostAddress,
  port: port
}, function() {
  console.log('connected to server');
  interval();
});
client.on('data', function(data) {
  console.log(data.toString());
});
client.on('end', function() {
  console.log('disconnected from server');
});