#!/bin/bash
MAXNUMRECS=500
LOGPATH="/var/log/rf_server.log"
TMPSJON="/tmp/entrylog.json"
LOGSRVSTATE="/etc/psme/nosrvlog"

FAN_MAX=`psme.sh get max_fan_num`
THERMAL_NUM=`psme.sh  get thermal_sensor_num`

help_p()
{
reset
echo ""
echo "////////////////////////////////////////////////////////////////////////////////////////////////"
echo "logsrv.sh <get> <All/EntriesCount/EntryContent/MaxNumberOfRecords/DateTime/DateTimeLocalOffset/ServiceEnabled/Entry/Status> <1~$MAXNUMRECS>"
echo "logsrv.sh <set> <Reset/Fan_PlugIn/Fan_UnPlug/Psu_PlugIn/Psu_UnPlug/Thermal_Critical/Thermal_Fatal/Status> <[1~$FAN_MAX]/[1~2]/[1~$THERMAL_NUM]>/[1/0]"
echo ""
echo "NOTE:"
echo ""
echo "get All"
echo "get EntriesCount"
echo "get EntryContent [INDEX: 1~$MAXNUMRECS]"
echo "get MaxNumberOfRecords"
echo "get DateTime"
echo "get DateTimeLocalOffset"
echo "get ServiceEnabled"
echo "get Entry"
echo "get Status"

echo "set Reset"
echo "set Fan_PlugIn [FAN ID: 1~$FAN_MAX]"
echo "set Fan_UnPlug [FAN ID: 1~$FAN_MAX]"
echo "set Psu_PlugIn [PSU ID: 1~2]"
echo "set Psu_UnPlug [PSU ID: 1~2]"
echo "set Thermal_Critical [Thermal Sensor: 1~$THERMAL_NUM]"
echo "set Thermal_Fatal [Thermal Sensor: 1~$THERMAL_NUM]"
echo "set Status [Enable: 0~1]"
}

rsyslog_en()
{
	status=`service rsyslog status | grep running`
	if [ "$status" = "rsyslogd is running." ];then
		echo 1
	else 
		echo 0
	fi
}

log_status()
{
	if [ -f $LOGSRVSTATE ];then
		echo 0
	else
		echo 1
	fi
}



create_entry_log_json()
{
   echo "{ \"ENTRY\":{\""EntryType"\":\"$1\",\""OemRecordFormat"\":\"$2\",\""Severity"\":\"$3\",\""Created"\":\"$4\",\""EntryCode"\":\"$5\",\""SensorType"\":\"$6\",\""SensorNumber"\":$7,\""Message"\":\"$8\"}}" >> ${LOGPATH}
}

if [ $# -eq 0 ];then
        help_p
        exit 99
fi

case "${1}" in


"get")

        case "${2}" in
        "All")
	DATETIME=`date +"%Y-%m-%dT%H:%M:%S%:z"`
	DATETIMEOFFSET=`date +"%:z"`
	if [ -f "/etc/logrotate.d/rfsrvlog" ];then
		OVERWRITEP=1
	else
		OVERWRITEP=0
	fi

	SERVICE_ENABLE=$(log_status)

	echo $DATETIME $DATETIMEOFFSET $MAXNUMRECS $OVERWRITEP $SERVICE_ENABLE
        ;;

        "MaxNumberOfRecords")
	echo $MAXNUMRECS 
        ;;

        "EntryContent")
        Index=$(($3-1))
        `tail -n $MAXNUMRECS /var/log/rf_server.log > "$TMPSJON"`
	IFS=$'\n'
        LOG=($(cat $TMPSJON))
        CT=${LOG[$Index]}
        echo $CT
        echo $CT > $TMPSJON
        ;;


        "EntriesCount")
        `tail -n $MAXNUMRECS /var/log/rf_server.log > "$TMPSJON"`
	IFS=$'\n'
        LOG=($(cat $TMPSJON))
        COUNT=${#LOG[@]}
        echo $COUNT
        ;;

        "DateTime")
	DATETIME=`date +"%Y-%m-%dT%H:%M:%S%:z"`
	echo $DATETIME
        ;;

        "DateTimeLocalOffset")
	DATETIMEOFFSET=`date +"%:z"`
	echo $DATETIMEOFFSET
        ;;

        "ServiceEnabled")
	#Always enable
	rsyslog_en
        ;;

        "Status")
	if [ -f $LOGSRVSTATE ];then
 	    echo 0
	else
 	    echo 1
	fi
	;;


        esac
        ;;

"set")
	DATETIME=`date +"%Y-%m-%dT%H:%M:%S%:z"`

	state=$(rsyslog_en)

	if [ "$state" = "0" ];then
		exit
	fi

        case "${2}" in

        "Reset")
        `rm /var/log/rf_server.log` 
        ;;

        "Fan_PlugIn")
	SERVICE_ENABLE=$(log_status)
	if [ "$SERVICE_ENABLE" = "1" ];then
		create_entry_log_json "Event" "NULL" "OK" "$DATETIME" "Assert" "Fan" "${3}" "FAN plug in."
	fi	
        ;;

        "Fan_UnPlug")
        SERVICE_ENABLE=$(log_status)
        if [ "$SERVICE_ENABLE" = "1" ];then
        	create_entry_log_json "Event" "NULL" "Warning" "$DATETIME" "Assert" "Fan" "${3}" "FAN unplug."
	fi
        ;;

        "Psu_PlugIn")
        SERVICE_ENABLE=$(log_status)
        if [ "$SERVICE_ENABLE" = "1" ];then
        	create_entry_log_json "Event" "NULL" "OK" "$DATETIME" "Assert" "PowerUnit" "${3}" "PSU plug in."
	fi
        ;;

        "Psu_UnPlug")
        SERVICE_ENABLE=$(log_status)
        if [ "$SERVICE_ENABLE" = "1" ];then
        	create_entry_log_json "Event" "NULL" "Warning" "$DATETIME" "Assert" "PowerUnit" "${3}" "PSU unplug."
	fi
        ;;

        "Thermal_Critical")
        SERVICE_ENABLE=$(log_status)
        if [ "$SERVICE_ENABLE" = "1" ];then
		create_entry_log_json "Event" "NULL" "Warning" "$DATETIME" "Assert" "Temperature" "${3}" "Thermal Sensor over critical temp."
	fi
        ;;

        "Thermal_Fatal")
        SERVICE_ENABLE=$(log_status)
        if [ "$SERVICE_ENABLE" = "1" ];then
        	create_entry_log_json "Event" "NULL" "Warning" "$DATETIME" "Assert" "Temperature" "${3}" "Thermal Sensor over fatal temp."
	fi
        ;;

        "Status")
	Status=$3

	if [ "$Status" = "1" ];then
		rm $LOGSRVSTATE > /dev/null 2>&1 
	elif [ "$Status" = "0" ];then
		touch $LOGSRVSTATE 
	fi
        ;;

        esac
	;;	
*)
help_p
;;
esac
