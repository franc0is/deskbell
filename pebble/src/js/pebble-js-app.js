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
  Pebble.openURL('http://technobly.com/pebble/config.html');
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

    var url = "https://api.spark.io/v1/devices/" + deviceId + "/msg?access_token=" + accessToken;
    var coming_busy = "";
    var msg = "";

    if (hasKey(dict, "QUERY")) {
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function () {
	    if (xmlhttp.readyState === 4) {
		console.log(xmlhttp.responseText);
		var timestamps = JSON.parse(xmlhttp.responseText);
		timestamps.reverse();
		var text = timestamps.join(",");
		Pebble.sendAppMessage( { "TIMESTR" : text });
		
	    }
	};
	xmlhttp.setRequestHeader("Referer", "technobly.com");
	xmlhttp.open('GET', "http://technobly.com/pebble/pushover.php", true);

	xmlhttp.send(null);

      return;
    }

    if (hasKey(dict, "COMING")) {
      coming_busy = "1";
      msg = "I'm coming now!";
    } else if(hasKey(dict, "BUSY")) {
      coming_busy = "2";
      msg = "I'm busy now!";
    } else {
      console.log("Invalid key received!");
      return;
    }

    if (hasKey(dict, "MESSAGE")) {
      msg = getValue(dict, "MESSAGE");
    }

    var args = coming_busy + msg;
    console.log("ajax, url: " + url + ", args: " + args);

    //Send with jQuery
    ajax({
      method: "post",
      url: url,
      data: {"args": args},
      type: "json"
    });
  }
);
