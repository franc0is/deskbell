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

// Build the URL.  Since it's possible to accidentally have an
// extra / or two in $_SERVER['QUERY_STRING], replace "//" with "/"
// using str_replace().  This also appends the access token to the URL.
$url = 'https://api.pushover.net/1/messages.json'.'?token='.TOKEN.'&user='.USER.'&timestamp='.time().str_replace('//', '/', $_SERVER['QUERY_STRING']);

echo file_get_contents('php://input');
echo "\n\n";

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