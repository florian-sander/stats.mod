<?
#### stats.php
#
## a little script to relay stats to your normal homepage
#


# Configuration

# the server where stats.mod is running (you can use "localhost" if
# the stats.php is located on the same server)
$STATSERVER = "broken.eggheads.org";

# the port
$STATPORT = 8033;

# time to wait before the script gives up
$STATTIMEOUT = 10;












###################################################################
###################################################################
#
## Don't touch anything below unless you know what you are doing!
#
#
#





$MAXLEN = 4096;
$statpath = $HTTP_GET_VARS["statpath"];
$password = $HTTP_POST_VARS["password"];
$languages = $HTTP_SERVER_VARS["HTTP_ACCEPT_LANGUAGE"];
$useragent = $HTTP_SERVER_VARS["HTTP_USER_AGENT"];
$xforwardedfor = $HTTP_SERVER_VARS["HTTP_X_FORWARDED_FOR"];
$remoteaddr = $HTTP_SERVER_VARS["REMOTE_ADDR"];
$cookies = $HTTP_SERVER_VARS["HTTP_COOKIE"];
$scriptname = $HTTP_SERVER_VARS["SCRIPT_NAME"];
$postparams = "";

if ($statpath == "") {
	$statpath = $HTTP_SERVER_VARS["PATH_INFO"];
	if ($statpath == "") {
		$statpath = "/";
	}
}

function my_array_keys ($arr, $term="") {
    $t = array();
    while (list($k,$v) = each($arr)) {
        if ($term && $v != $term)
            continue;
            $t[] = $k;
        }
        return $t;
}


if ($password != "") {
	$keys = my_array_keys($HTTP_POST_VARS);
	for ($i = 0; $i < count($keys); $i++) {
		$param = urlencode($HTTP_POST_VARS[$keys[$i]]);
		$postparams .= "&$keys[$i]=$param";
	}
	$postparams = substr($postparams, 1);
}

function transform_url($url) {
	global $scriptname;
	global $statpath;

	if (!strcasecmp(substr($url, 0, 7), "http://") ||
		!strcasecmp(substr($url, 0, 7), "mailto:") ||
		!strcasecmp(substr($url, 0, 6), "ftp://")) {
		// global URL, don't touch it
		$retstr = $url;
	} else if ($url[0] == "/") {
		// absolute URL, simply remap it
		$retstr = "$scriptname$url";
	} else {
		// relative URL, process any '../'s
		$parts = explode("/", $url);
		$backs = 0;
		$newurl = "";
		for ($i = 0; $i < count($parts); $i++) {
			if (!strcasecmp($parts[$i], "..")) {
				$backs++;
			} else {
				$tmp = "$newurl/$parts[$i]";
				$newurl = $tmp;
			}
		}
		$newurl = substr($newurl, 1);
		$parts = explode("/", $statpath);
		$newpath = "";
		for ($i = 0; $i < (count($parts) - $backs - 1); $i++) {
			$tmp = "${newpath}/$parts[$i]";
			$newpath = $tmp;
		}
		$newpath = substr($newpath, 1);
		$retstr = "$scriptname$newpath/$newurl";
	}
	return $retstr;
}

function do_transform($line) {
	$words = explode(" ", $line);
	for ($i = 0; $i < count($words); $i++) {
		if (!strcasecmp(substr($words[$i], 0, 5), "href=")) {
			$urlq = strstr($words[$i], "\"");
			$urlq = substr($urlq, 1);
			$urlparts = explode("\"", $urlq);
			$rest = $urlparts[1];
			$newurl = transform_url($urlparts[0]);
			echo "href=\"$newurl\"$rest ";
		} else if (!strcasecmp(substr($words[$i], 0, 7), "action=")) {
			$urlq = strstr($words[$i], "\"");
			$urlq = substr($urlq, 1);
			$urlparts = explode("\"", $urlq);
			$rest = $urlparts[1];
			$newurl = transform_url($urlparts[0]);
			echo "action=\"$newurl\"$rest ";
		} else {
			echo "$words[$i] ";
		}
	}
}

$httpbody = false;

$idx = fsockopen($STATSERVER, $STATPORT, $errno, $errstr, $STATTIMEOUT);
if (!$idx) {
	echo "<html><head><title>$errno: $errstr</title></head>";
	echo "<body><H1>Connection to statistics server FAILED!</H1><br>";
	echo "$errno: $errstr";
} else {
	if ($postparams == "") {
		fputs ($idx, "GET $statpath HTTP/1.0\r\n");
	} else {
		$len = strlen($postparams);
		fputs ($idx, "POST $statpath HTTP/1.0\r\n");
		fputs ($idx, "Content-Length: $len\r\n");
	}
	if ($languages != "") {
		fputs ($idx, "Accept-Language: $languages\r\n");
	}
	if ($useragent != "") {
		fputs ($idx, "User-Agent: $useragent\r\n");
	}
	if ($remoteaddr != "") {
		fputs ($idx, "X-Relayed-For: $remoteaddr\r\n");
	}
	if ($xforwardedfor != "") {
		fputs ($idx, "X-Forwarded-For: $xforwardedfor\r\n");
	}
	if ($cookies != "") {
		fputs ($idx, "Cookie: $cookies\r\n");
	}

	fputs ($idx, "\r\n");

	if ($postparams != "") {
		fputs ($idx, $postparams);
	}

	while (!feof($idx)) {
		$buf = fgets($idx, $MAXLEN);
		if ($httpbody) {
			do_transform($buf);
		} else if ($buf == "" || $buf == "\n") {
			$httpbody = true;
		} else {
			$buf = eregi_replace("Location: ", "Location: $scriptname?statpath=", $buf);
			header($buf);
		}
	}
	fclose($idx);
}


########### old regex experiments
//	echo eregi_replace("href=\"(\/.*\/)*\"/", "relay.php?statpath=", $buf);
//	echo eregi_replace("href=\"(http\:\/\/{0,0})", "relay.php?statpath=", $buf);
//        $buf = eregi_replace("href=\"", "href=\"relay.php?statpath=", $buf);
//	$buf = eregi_replace("(href=\")([^\"]*)(\")", "href=\"$scriptname.php?path=$statpath{transform_remote_url(\\2)}\"", $buf);
//	$buf = eregi_replace("(href=\"){1,}", "href=\"$scriptname?statpath=$statpath", $buf);
//	$buf = eregi_replace("(href=\")[[:alpha:]]*[^\ ]*http\:\/\/", "href=\"http://", $buf);
//	echo $buf;
//	echo pregi_replace("href=\"(^http)", "href=\"relay.php?statpath=", $buf);
//      for ($i = 0; $i < strlen($buf); $i++) {
//        if (!strncasecmp($buf[$i], "href=\"", 6) && !(!strncasecmp($buf[$i], "href=\"http://", 13)) {
//          echo "href=\"$scriptname?statpath=$statpath/";
//          $i = $i + 5;
//        } else {
//          echo $buf[i];
//        }
//      }
//      echo eregi_replace("(href\=\")(http\:\/\/){,0}", "href=\"$scriptname?statpath=", $buf);
//      echo $buf;

?>