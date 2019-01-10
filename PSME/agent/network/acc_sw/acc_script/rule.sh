#!/bin/bash

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

help_p()
{
echo "get rule_from_acl [acl_index 1.2.3...] [rule_index 1.2.3...] in following params" 
echo "   Action IPSource IPSourceMASK IPDestination IPDestinationMASK MACSource MACSourceMASK"
echo "   MACDestination MACDestinationMASK VLANID VLANIDMASK L4SourcePort L4SourcePortMASK "
echo "   L4DestPort L4DestPortMASK L4ProtocolForwardMirrorInterface MirrorPortRegion MirrorType"
echo " "
echo "set rule_to_acl [acl_index 1.2.3...] [rule_index 1.2.3...] in following params "
echo "   Action IPSource IPSourceMASK IPDestination IPDestinationMASK MACSource MACSourceMASK"
echo "   MACDestination MACDestinationMASK VLANID VLANIDMASK L4SourcePort L4SourcePortMASK "
echo "   L4DestPort L4DestPortMASK L4Protocol ForwardMirrorInterface MirrorPortRegion MirrorType "
echo " "
echo "Note: If don't modify old value , use \"\" as input value"
echo "      If modify old value to NA, use "NA" as input value"
}

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [rule.sh $1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} ${14} ${15} ${16} ${17} ${18} ${19} ${20} ${21} ${22} ${23}] >> /var/log/SDK_CMD_LOG 
case "${1}" in

"get")
        case "${2}" in
# To get ACL member #
        "rule_from_acl")
        acl_index=$3
        rule_index=$4
        Action=`uci get RULE.RULE"$acl_index"_"$rule_index".Action`
        IPSource=`uci get RULE.RULE"$acl_index"_"$rule_index".IPSource`
        IPSourceMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".IPSourceMASK`
        IPDestination=`uci get RULE.RULE"$acl_index"_"$rule_index".IPDestination`
        IPDestinationMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".IPDestinationMASK`
        MACSource=`uci get RULE.RULE"$acl_index"_"$rule_index".MACSource`
        MACSourceMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".MACSourceMASK`
        MACDestination=`uci get RULE.RULE"$acl_index"_"$rule_index".MACDestination`
        MACDestinationMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".MACDestinationMASK`
        VLANID=`uci get RULE.RULE"$acl_index"_"$rule_index".VLANID`
        VLANIDMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".VLANIDMASK`
        L4SourcePort=`uci get RULE.RULE"$acl_index"_"$rule_index".L4SourcePort`
        L4SourcePortMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".L4SourcePortMASK`
        L4DestPort=`uci get RULE.RULE"$acl_index"_"$rule_index".L4DestPort`
        L4DestPortMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".L4DestPortMASK`
        L4Protocol=`uci get RULE.RULE"$acl_index"_"$rule_index".L4Protocol`
        ForwardMirrorInterface=`uci get RULE.RULE"$acl_index"_"$rule_index".ForwardMirrorInterface`
        MirrorPortRegion=`uci get RULE.RULE"$acl_index"_"$rule_index".MirrorPortRegion`
        MirrorType=`uci get RULE.RULE"$acl_index"_"$rule_index".MirrorType`

        echo "$Action $IPSource $IPSourceMASK $IPDestination $IPDestinationMASK $MACSource $MACSourceMASK $MACDestination $MACDestinationMASK $VLANID $VLANIDMASK $L4SourcePort $L4SourcePortMASK $L4DestPort $L4DestPortMASK $L4Protocol $ForwardMirrorInterface $MirrorPortRegion $MirrorType"
	;;
        esac
	;;
"set")
        case "${2}" in

        "rule_to_acl")
        acl_index=$3
        rule_index=$4

        ACL_count=`acl.sh get acl_count`

        if [ "$ACL_count" = "0" ];then
            echo ERROR
            exit
        fi

        Action=$5
        pAction=`uci get RULE.RULE"$acl_index"_"$rule_index".Action`
        ipAction=${Action:-$pAction}
        uci set RULE.RULE"$acl_index"_"$rule_index".Action=$ipAction

        IPSource=$6
        pIPSource=`uci get RULE.RULE"$acl_index"_"$rule_index".IPSource`
        ipIPSource=${IPSource:-$pIPSource}
        uci set RULE.RULE"$acl_index"_"$rule_index".IPSource=$ipIPSource

        IPSourceMASK=$7
        pIPSourceMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".IPSourceMASK`
        ipIPSourceMASK=${IPSourceMASK:-$pIPSourceMASK}
        uci set RULE.RULE"$acl_index"_"$rule_index".IPSourceMASK=$ipIPSourceMASK

        IPDestination=$8
        pIPDestination=`uci get RULE.RULE"$acl_index"_"$rule_index".IPDestination`
        ipIPDestination=${IPDestination:-$pIPDestination}
        uci set RULE.RULE"$acl_index"_"$rule_index".IPDestination=$ipIPDestination

        IPDestinationMASK=$9
        pIPDestinationMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".IPDestinationMASK`
        ipIPDestinationMASK=${IPDestinationMASK:-$pIPDestinationMASK}
        uci set RULE.RULE"$acl_index"_"$rule_index".IPDestinationMASK=$ipIPDestinationMASK

        MACSource=${10}
        pMACSource=`uci get RULE.RULE"$acl_index"_"$rule_index".MACSource`
        ipMACSource=${MACSource:-$pMACSource}
        uci set RULE.RULE"$acl_index"_"$rule_index".MACSource=$ipMACSource

        MACSourceMASK=${11}
        pMACSourceMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".MACSourceMASK`
        ipMACSourceMASK=${MACSourceMASK:-$pMACSourceMASK}
        uci set RULE.RULE"$acl_index"_"$rule_index".MACSourceMASK=$ipMACSourceMASK

        MACDestination=${12}
        pMACDestination=`uci get RULE.RULE"$acl_index"_"$rule_index".MACDestination`
        ipMACDestination=${MACDestination:-$pMACDestination}
        uci set RULE.RULE"$acl_index"_"$rule_index".MACDestination=$ipMACDestination

        MACDestinationMASK=${13}
        pMACDestinationMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".MACDestinationMASK`
        ipMACDestinationMASK=${MACDestinationMASK:-$pMACDestinationMASK}
        uci set RULE.RULE"$acl_index"_"$rule_index".MACDestinationMASK=$ipMACDestinationMASK

        VLANID=${14}

        if [ "$VLANID" = 30600 ];then
            VLANID="NA"
        elif [ "$VLANID" = 0 ];then
            VLANID=""
        fi

        pVLANID=`uci get RULE.RULE"$acl_index"_"$rule_index".VLANID`
        ipVLANID=${VLANID:-$pVLANID}
        uci set RULE.RULE"$acl_index"_"$rule_index".VLANID=$ipVLANID

        VLANIDMASK=${15}

        if [ "$VLANIDMASK" = "0x7788" ];then
            VLANIDMASK="NA"
        elif [ "$VLANIDMASK" = "0x0" ];then
            VLANIDMASK=""
        fi

        pVLANIDMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".VLANIDMASK`
        ipVLANIDMASK=${VLANIDMASK:-$pVLANIDMASK}
        uci set RULE.RULE"$acl_index"_"$rule_index".VLANIDMASK=$ipVLANIDMASK

        L4SourcePort=${16}

        if [ "$L4SourcePort" = 30600 ];then
            L4SourcePort="NA"
        elif [ "$L4SourcePort" = 0 ];then
            L4SourcePort=""
        fi

        pL4SourcePort=`uci get RULE.RULE"$acl_index"_"$rule_index".L4SourcePort`
        ipL4SourcePort=${L4SourcePort:-$pL4SourcePort}
        uci set RULE.RULE"$acl_index"_"$rule_index".L4SourcePort=$ipL4SourcePort

        L4SourcePortMASK=${17}

        if [ "$L4SourcePortMASK" = "0x7788" ];then
            L4SourcePortMASK="NA"
        elif [ "$L4SourcePortMASK" = "0x0" ];then
            L4SourcePortMASK=""
        fi

        pL4SourcePortMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".L4SourcePortMASK`
        ipL4SourcePortMASK=${L4SourcePortMASK:-$pL4SourcePortMASK}
        uci set RULE.RULE"$acl_index"_"$rule_index".L4SourcePortMASK=$ipL4SourcePortMASK

        L4DestPort=${18}

        if [ "$L4DestPort" = 30600 ];then
            L4DestPort="NA"
        elif [ "$L4DestPort" = 0 ];then
            L4DestPort=""
        fi

        pL4DestPort=`uci get RULE.RULE"$acl_index"_"$rule_index".L4DestPort`
        ipL4DestPort=${L4DestPort:-$pL4DestPort}
        uci set RULE.RULE"$acl_index"_"$rule_index".L4DestPort=$ipL4DestPort

        L4DestPortMASK=${19}

        if [ "$L4DestPortMASK" = "0x7788" ];then
            L4DestPortMASK="NA"
        elif [ "$L4DestPortMASK" = "0x0" ];then
            L4DestPortMASK=""
        fi

        pL4DestPortMASK=`uci get RULE.RULE"$acl_index"_"$rule_index".L4DestPortMASK`
        ipL4DestPortMASK=${L4DestPortMASK:-$pL4DestPortMASK}
        uci set RULE.RULE"$acl_index"_"$rule_index".L4DestPortMASK=$ipL4DestPortMASK

        L4Protocol=${20}
        pL4Protocol=`uci get RULE.RULE"$acl_index"_"$rule_index".L4Protocol`
        ipL4Protocol=${L4Protocol:-$pL4Protocol}
        uci set RULE.RULE"$acl_index"_"$rule_index".L4Protocol=$ipL4Protocol

        ForwardMirrorInterface=${21}

        if [ "$ForwardMirrorInterface" = 0 ];then
            ForwardMirrorInterface=""
        fi

        pForwardMirrorInterface=`uci get RULE.RULE"$acl_index"_"$rule_index".ForwardMirrorInterface`
        ipForwardMirrorInterface=${ForwardMirrorInterface:-$pForwardMirrorInterface}
        uci set RULE.RULE"$acl_index"_"$rule_index".ForwardMirrorInterface=$ipForwardMirrorInterface

        MirrorPortRegion=${22}
        pMirrorPortRegion=`uci get RULE.RULE"$acl_index"_"$rule_index".MirrorPortRegion`
        ipMirrorPortRegion=${MirrorPortRegion:-$pMirrorPortRegion}
        uci set RULE.RULE"$acl_index"_"$rule_index".MirrorPortRegion=$ipMirrorPortRegion

        MirrorType=${23}
        pMirrorType=`uci get RULE.RULE"$acl_index"_"$rule_index".MirrorType`
        ipMirrorType=${MirrorType:-$pMirrorType}
        uci set RULE.RULE"$acl_index"_"$rule_index".MirrorType=$ipMirrorType

        uci commit
        ;;
        esac
        ;;
*)
help_p
;;

esac

