var deviceId = localStorage.getItem("spark_id");
var accessToken = localStorage.getItem("spark_token");

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
  console.log("response : " + e.reponse);
  var options = JSON.parse(decodeURIComponent(e.response));
  accessToken = options.spark_token;
  deviceId = options.spark_id;
  console.log("id : " + deviceId);
  console.log("token : " + accessToken);
  localStorage.setItem("spark_id", deviceId);
  localStorage.setItem("spark_token", accessToken);
});

/****************** XHR *************************/

var success = function(json) {
  console.log("Response JSON: " + JSON.stringify(json));
};

/****************** Main ************************/

Pebble.addEventListener("ready",
  function(e) {
    console.log("Pebble JS ready!");
  }
);

Pebble.addEventListener("appmessage",
  function(dict) {
    var url = "https://api.spark.io/v1/devices/" + deviceId + "/led?access_token=" + accessToken;
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
      console.log("ajax, url: " + url);

      //Send with jQuery
      ajax({
        method: "post",
        url: url,
        data: {"args": busy_coming},
        type: "json"
      });
    }
  }
);
