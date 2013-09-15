To get syslog and nginx to work correctly:
* Update nginx with the latest config file
* Copy the config file to /etc/rsyslog.d/
* Restart rsyslog with /etc/init.d/rsyslog restart
* chmod the log file: chmod 644 /var/log/mctxserv.log
