<?
// Set your token here
define('TOKEN', 'adnnmc76K9ea4E96U1sLQv2CLobyFo');
// Set your token here
define('USER', 'u7hLeioVq2SJaCEbeDnFhEggw4GL6e');

// All responses should be JSON
header('Content-type: application/json');

// Security check!
// Set "Referer" = "technobly.com" in request headers
if(!isset($_SERVER['HTTP_REFERER']) || substr_count($_SERVER['HTTP_REFERER'], $_SERVER['SERVER_NAME'])==0)
        die(json_encode(array(
                'error' => 'Invalid request'
        )));

$timestamp_log = "log.txt";
if (strtoupper($_SERVER['REQUEST_METHOD'])=='GET') {
  echo (!file_exists($timestamp_log)) ? json_encode("") : json_encode(explode("\n", file_get_contents($timestamp_log))); 
  exit();
}

// Build the URL.  Since it's possible to accidentally have an
// extra / or two in $_SERVER['QUERY_STRING], replace "//" with "/"
// using str_replace().  This also appends the access token to the URL.
$url = 'https://api.pushover.net/1/messages.json'.'?token='.TOKEN.'&user='.USER.'&timestamp='.time().str_replace('//', '/', $_SERVER['QUERY_STRING']);

echo file_get_contents('php://input');
echo "\n\n";

//
// stash the timestamp in a file for a subsequent GET request. (Only logs
// $max_stamps most recent entries)

//
// pebble log directory & number of entries to stash
$max_stamps = 5;

date_default_timezone_set('America/Los_Angeles');
$date = new DateTime();
$timestr = (file_exists($timestamp_log) ? "\n" : "").$date->getTimestamp();
file_put_contents($timestamp_log, $timestr, FILE_APPEND);

$num_stamps = 0;
if (file_exists($timestamp_log)) {
  $timestamps = explode("\n", file_get_contents($timestamp_log));
  $num_stamps = count($timestamps);
 }

//echo "There are ".$num_stamps." timestamps<br/>";

$st = ($num_stamps <= $max_stamps) ? 0 : ($num_stamps - $max_stamps);
$start_pt = $num_stamps;

if ($num_stamps >= $max_stamps) {
  //  echo "unlinking <br\>";
  $start_pt = 0;
  unlink($timestamp_log);
}

//echo "start at ".$start_pt."<br/>";
for ($i=$st; $i < $num_stamps; $i++) {
  //  echo $timestamps[$i]."<---".$i."<br/>"."\n";
  if (($i >= $start_pt)) {
    file_put_contents($timestamp_log, ((file_exists($timestamp_log)) ? "\n" : "").$timestamps[$i], FILE_APPEND);
  }
}

//$url = 'https://api.pushover.net/1/messages.json'.'?token='.'adnnmc76K9ea4E96U1sLQv2CLobyFo'.'&user='.'u7hLeioVq2SJaCEbeDnFhEggw4GL6e'.'&title='.'php'.'&message='.'hello';

// HTTP POST requires the use of cURL
if (strtoupper($_SERVER['REQUEST_METHOD'])=='POST') {
        $c = curl_init();

        curl_setopt_array($c, array(
                // Set the URL to access
                CURLOPT_URL => $url,
                // Tell cURL it's an HTTP POST request
                CURLOPT_POST => TRUE,
                // Include the POST data
                // $HTTP_RAW_POST_DATA may work on some servers, but it's deprecated in favor of php://input
                CURLOPT_POSTFIELDS => file_get_contents('php://input'),
                // Return the output to a variable instead of automagically echoing it (probably a little redundant)
                CURLOPT_RETURNTRANSFER => TRUE
        ));

        // Make the cURL call and echo the response
        echo curl_exec($c);

        // Close the cURL resource
        curl_close($c);
}
?>