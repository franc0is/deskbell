var deviceId = "";
var accessToken = "";

/**************** Pebble helpers ****************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== "undefined";
};

var getValue = function(dict, key) {
  if(hasKey(dict, key)) {
    return "" + dict.payload[key];
  } else {
    console.log("ERROR: Key '" + key + "' does not exist in received dictionary");
    return undefined;
  }
};

/************** Configuration *******************/
Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  Pebble.openURL('http://francois-mbp.hq.getpebble.com:8000/configurable.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));
  accessToken = options.spark_token;
  console.log("token : " + accessToken);
  deviceId = options.spark_id;
  console.log("id : " + deviceId);
});

/****************** XHR *************************/

//Spark.publish("channel", agrs) reports that args in int(String args) is 'null' with this method (why?)
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload =  function () {
      callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var importjQuery = function() {
  var script = document.createElement('script');
  script.src = 'http://code.jquery.com/jquery-latest.min.js';
  script.type = 'text/javascript';
  document.getElementsByTagName('head')[0].appendChild(script);
};

/****************** Main ************************/

Pebble.addEventListener("ready",
  function(e) {
    importjQuery();
    console.log("Pebble JS ready!");
  }
);

Pebble.addEventListener("appmessage",
  function(dict) {
    var url = "https://api.spark.io/v1/devices/" + deviceId + "/on?access_token=" + accessToken;
    var busy_coming = "";
    var valid = false;

    if(hasKey(dict, "COMING")) {
      busy_coming = "coming";
      valid = true;
    }

    else if(hasKey(dict, "BUSY")) {
      busy_coming = "busy";
      valid = true;
    }

    //Invalid
    else {
      console.log("Invalid key received!");
    }

    if(valid === true)
    {
      //Send with XHR
      // xhrRequest(url, "POST", 
      //   function(responseText) {
      //     console.log("Response text: " + responseText);

      //     var json = JSON.parse(responseText);

      //     Pebble.sendAppMessage(
      //       {"PIN_EVENT":json.return_value},
      //       function(e) {
      //         console.log("Pin " + json.return_value + " updated on Pebble");
      //       },
      //       function(e) {
      //         console.log("FAILED to update pin on Pebble!");
      //       }
      //     );
      //   }
      // );

      console.log("ajax");

      //Send with jQuery
      $.ajax({
        type: "POST",
        url: url,
        data: {"args":busy_coming},
        dataType: "json"
      });
    }
  }
);
