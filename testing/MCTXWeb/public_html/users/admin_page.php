<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

//what a bunch of spaghetti code

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}
$pageId = $_GET['id'];

//Check if selected pages exist
if(!pageIdExists($pageId)){
	header("Location: admin_pages.php"); die();	
}

$pageDetails = fetchPageDetails($pageId); //Fetch information specific to page

//Forms posted
if(!empty($_POST)){
	$update = 0;
	
	if(!empty($_POST['private'])){ $private = $_POST['private']; }
	
	//Toggle private page setting
	if (isset($private) AND $private == 'Yes'){
		if ($pageDetails['private'] == 0){
			if (updatePrivate($pageId, 1)){
				$successes[] = lang("PAGE_PRIVATE_TOGGLED", array("private"));
			}
			else {
				$errors[] = lang("SQL_ERROR");
			}
		}
	}
	elseif ($pageDetails['private'] == 1){
		if (updatePrivate($pageId, 0)){
			$successes[] = lang("PAGE_PRIVATE_TOGGLED", array("public"));
		}
		else {
			$errors[] = lang("SQL_ERROR");	
		}
	}
	
	//Remove permission level(s) access to page
	if(!empty($_POST['removePermission'])){
		$remove = $_POST['removePermission'];
		if ($deletion_count = removePage($pageId, $remove)){
			$successes[] = lang("PAGE_ACCESS_REMOVED", array($deletion_count));
		}
		else {
			$errors[] = lang("SQL_ERROR");	
		}
		
	}
	
	//Add permission level(s) access to page
	if(!empty($_POST['addPermission'])){
		$add = $_POST['addPermission'];
		if ($addition_count = addPage($pageId, $add)){
			$successes[] = lang("PAGE_ACCESS_ADDED", array($addition_count));
		}
		else {
			$errors[] = lang("SQL_ERROR");	
		}
	}
	
	$pageDetails = fetchPageDetails($pageId);
}

$pagePermissions = fetchPagePermissions($pageId);
$permissionData = fetchAllPermissions();

require_once("models/header.php");
startPage();

echo notificationBlock($errors, $successes);

echo '
<div class="widget">
<div class="right">
  Page ID '.$pageDetails['id'].'
</div>

<div class="title">Manage page "'.$pageDetails['page'].'"</div>

<div class="sub-title">Access control list</div>';
echo "
<form name='adminPage' class='nice clear' action='".$_SERVER['PHP_SELF']."?id=".$pageId."' method='post'>
<input type='hidden' name='process' value='1'>

<table class='admin left lines'>
<tr>
  <th class=''>Swap</th>
  <th class=''>Has access</th>
  <th class=''>Does not have access</th>
</tr>
";

foreach ($permissionData as $v) {
  echo "<tr>";
  if(isset($pagePermissions[$v['id']])) {
    echo "<td><input type='checkbox' name='removePermission[".$v['id']."]' id='removePermission[".$v['id']."]' value='".$v['id']."'></td>";
    echo "<td>".$v['name']."</td><td></td>";
  } else {
    echo "<td><input type='checkbox' name='addPermission[".$v['id']."]' id='addPermission[".$v['id']."]' value='".$v['id']."</td>'>";
    echo "<td></td><td>".$v['name']."</td>";
  }
  
  echo "</tr>";
}

echo "
</table>

<p class='left'>
<label for='private'>Private page:</label>";

//Display private checkbox
if ($pageDetails['private'] == 1){
	echo "<input type='checkbox' name='private' id='private' value='Yes' checked>";
}
else {
	echo "<input type='checkbox' name='private' id='private' value='Yes'>";	
}

echo "
</p>

<p class='right'>
<label>&nbsp;</label>
<input type='submit' value='Update' class='submit' />
</p>
</form>
";

echo "
</div>";

finishPage();

?>
