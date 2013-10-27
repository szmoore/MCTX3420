<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

if (isUserLoggedIn()) {

echo '
        <div id="sidebar">
 ';

 //Is admin
if ($loggedInUser->checkPermission(array(2))){
    echo '
          <div class="widget">
            <div class="title">Site Administration</div>
            <div id="sidebar-menu" class="nav-menu">
              <ul>
                <li><a href="index.php"><span>Home</span></a></li>
                <li><a href="admin_users.php"><span>Manage user list</span></a></li>
                <li><a href="admin_upload_users.php"><span>Upload new users</span></a></li>
                <li><a href="admin_pages.php"><span>Manage visible pages</span></a></li>
                <li><a href="admin_configuration.php"><span>Manage site details</span></a></li>
              </ul>
            </div>
          </div>
    ';
}


echo $custom_sidebar.'
       </div>
        <!-- End sidebar -->
';

}
?>
