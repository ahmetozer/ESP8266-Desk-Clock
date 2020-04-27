<?php
// Linux Interface statistics service for ESP8266
//
function formatBytes($bytes, $unit = "", $decimals = 2) {
	$units = array('B' => 0, 'KB' => 1, 'MB' => 2, 'GB' => 3, 'TB' => 4,
			'PB' => 5, 'EB' => 6, 'ZB' => 7, 'YB' => 8);

	$value = 0;
	if ($bytes > 0) {
		// Generate automatic prefix by bytes
		// If wrong prefix given
		if (!array_key_exists($unit, $units)) {
			$pow = floor(log($bytes)/log(1024));
			$unit = array_search($pow, $units);
		}

		// Calculate byte value by prefix
		$value = ($bytes/pow(1024,floor($units[$unit])));
	}

	// If decimals is not numeric or decimals is less than 0
	// then set default value
	if (!is_numeric($decimals) || $decimals < 0) {
		$decimals = 2;
	}

	// Format output
	return sprintf('%.' . $decimals . 'f '.$unit, $value);
  }

//function speed($inf){
$ppp0rx1=file_get_contents('/sys/class/net/ppp0/statistics/rx_bytes');
$ppp0tx1=file_get_contents('/sys/class/net/ppp0/statistics/tx_bytes');
$sit1rx1=file_get_contents('/sys/class/net/sit1/statistics/rx_bytes');
$sit1tx1=file_get_contents('/sys/class/net/sit1/statistics/tx_bytes');
//sleep(1);
usleep(125000); //saniyenin 1/8
$ppp0rx2=file_get_contents('/sys/class/net/ppp0/statistics/rx_bytes');
$ppp0tx2=file_get_contents('/sys/class/net/ppp0/statistics/tx_bytes');
$sit1rx2=file_get_contents('/sys/class/net/sit1/statistics/rx_bytes');
$sit1tx2=file_get_contents('/sys/class/net/sit1/statistics/tx_bytes');

$ppp0rx = ($ppp0rx2-$ppp0rx1)*64;
$ppp0tx = ($ppp0tx2-$ppp0tx1)*64;
$sit1rx = ($sit1rx2-$sit1rx1)*64;
$sit1tx = ($sit1tx2-$sit1tx1)*64;

//}
function formatBytes2($bytes, $unit = "", $decimals = 2) {
	$units = array('B' => 0, 'Kb' => 1, 'Mb' => 2, 'Gb' => 3, 'Tb' => 4, 'Pb' => 5, 'Eb' => 6, 'Zb' => 7, 'Yb' => 8);

	$value = 0;
	if ($bytes > 0) {
		// Generate automatic prefix by bytes
		// If wrong prefix given
		if (!array_key_exists($unit, $units)) {
			$pow = floor(log($bytes)/log(1024));
			$unit = array_search($pow, $units);
		}

		// Calculate byte value by prefix
		$value = ($bytes/pow(1024,floor($units[$unit])));
	}

	// If decimals is not numeric or decimals is less than 0
	// then set default value
	if (!is_numeric($decimals) || $decimals < 0) {
		$decimals = 2;
	}

	// Format output
	return sprintf('%.' . $decimals . 'f '.$unit, $value);
  }
?>
{
"ppp0":{"down":"<?php echo  formatBytes(file_get_contents('/sys/class/net/ppp0/statistics/rx_bytes'));?>",
"up":"<?php echo  formatBytes(file_get_contents('/sys/class/net/ppp0/statistics/tx_bytes'));?>",
"rxs":"<?php echo formatBytes2($ppp0rx)?>",
"txs":"<?php echo formatBytes2($ppp0tx)?>"},
"sit1":{"down":"<?php echo  formatBytes(file_get_contents('/sys/class/net/sit1/statistics/rx_bytes'));?>",
"up":"<?php echo  formatBytes(file_get_contents('/sys/class/net/sit1/statistics/tx_bytes'));?>",
"rxs":"<?php echo formatBytes2($sit1rx)?>",
"txs":"<?php echo formatBytes2($sit1tx)?>"}
}
