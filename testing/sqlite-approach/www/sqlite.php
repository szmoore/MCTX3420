<?php
	class MyDB extends SQLite3
	{
		function __construct()
		{
			$this->open('../db/data.db');
		}

		function getSensorValue($id)
		{
			//needs error checking, but you get the idea
			$ret = $this->query("select value from sensors where sensor_id={$id}");
			$row = $ret->fetchArray(SQLITE3_NUM);
			return $row[0];
		}
	}

	$db = new MyDB();
	if (!$db) {
		echo $db->lastErrorMsg();
	} else {
		echo "yay<br>\n";
	}

	$ret = $db->query('SELECT * from test');
	while ($row = $ret->fetchArray(SQLITE3_ASSOC)) {
		echo "NUM = ". $row['num'] . "<br>\n";
	}

	echo "Sensor 1 value: " . $db->getSensorValue(1). "<br>\n";
	$db->close();

?>