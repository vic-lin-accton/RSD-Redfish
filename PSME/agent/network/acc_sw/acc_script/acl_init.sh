#!/bin/bash

DBG=

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [acl_init.sh $1 ] >> /var/log/SDK_CMD_LOG 

BCM_SDK_PBMASK=0x7fffffffffffff
OPENNSL_PBMASK=0x222223ffffffffffff

. /usr/local/bin/tools.sh

case "${1}" in

"start")
    ACL_COUNT=`acl.sh get acl_count`

    for (( c=0; c < "${ACL_COUNT}"; c++ ))
    do  
        ACL_Index=`acl.sh  get acl_index_content $c`
        ACL_PBMP=`uci get ACL.ACL${ACL_Index}.PBMP`

        ${DBG:+echo ========ACL[${ACL_Index}]========} 
        GRP_ID=${ACL_Index}

        if [ "${ACL_PBMP}" != "NA" ];then

            ${DBG:+echo show ACL_PBMP[${ACL_PBMP}]} 
            #Hex_ACL_PBMP=`printf '%x' $((2#$ACL_PBMP))`
 	    Hex_ACL_PBMP=$(mapping_bin2hexport convert ALL ${ACL_PBMP}) 
            ${DBG:+echo "ACL_PBMP_H[0x$Hex_ACL_PBMP]"}

            # To Check if any Rule bind to these port #
            ACL_RULE=`uci get ACL.ACL${ACL_Index}.RULE`


            if [ "${ACL_RULE}" != "NA" ];then

                ACL_RULE_A=($(echo $ACL_RULE |  sed -e 's/^ *//'  | sed -e 's/ \+/ /g'))
                ACL_RULE_A_C=${#ACL_RULE_A[@]}

                for STAGE in "QSET" "CRT_GRP_ENTRY" "QSET_ADD" "INSTALL"   #STAGE loop 
                do

                if [ "$STAGE" = "QSET" ];then
                    result=`acc_sw 'fp qset add stageingress'`
                    ${DBG:+echo "acc_sw 'fp qset add stageingress'"}
                    result=`acc_sw 'fp qset add InPorts'`
                    ${DBG:+echo "acc_sw 'fp qset add InPorts'"}
                fi
######################### SET QSET FIRST START ###################################

${DBG:+echo "acc_sw #############################QSET###################################"}

                for (( cc=0; cc < "${ACL_RULE_A_C}"; cc++ ))
                do  
                    ${DBG:+echo show RULE${ACL_RULE_A[$cc]}} 

                    # Need fp qset XXX first then set reltaed value #

                    IPSource=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".IPSource`
                    ${DBG:+echo IPSource[$IPSource]} 

                    if [ "${IPSource}" != "NA" ];then
                        if [ "$STAGE" = "QSET" ];then
                            result=`acc_sw "fp qset add SrcIp"`
                            ${DBG:+echo "acc_sw "fp qset add SrcIp""}
                        fi
                    fi

                    IPDestination=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".IPDestination`
                    ${DBG:+echo IPDestination[$IPDestination]} 

                    if [ "${IPDestination}" != "NA" ];then
                        if [ "$STAGE" = "QSET" ];then
                            result=`acc_sw "fp qset add DstIp"`
                            ${DBG:+echo "acc_sw "fp qset add DstIp""}
                        fi
                    fi

                    MACSource=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MACSource`
                    ${DBG:+echo MACSource[$MACSource]} 

                    if [ "${MACSource}" != "NA" ];then
                        if [ "$STAGE" = "QSET" ];then
                            result=`acc_sw "fp qset add SrcMac"`
                        fi
                    fi

                    MACDestination=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MACDestination`
                    ${DBG:+echo MACDestination[$MACDestination]} 

                    if [ "${MACDestination}" != "NA" ];then
                        if [ "$STAGE" = "QSET" ];then
                            result=`acc_sw "fp qset add DstMac"`
                            ${DBG:+echo "acc_sw "fp qset add DstMac""}
                        fi
                    fi

                    VLANID=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".VLANID`
                    ${DBG:+echo VLANID[$VLANID]} 

                    if [ "${VLANID}" != "NA" -a "${VLANID}" != "0" ];then
                        if [ "$STAGE" = "QSET" ];then
                            result=`acc_sw "fp qset add OuterVlanId"`
                            ${DBG:+echo "acc_sw "fp qset add OuterVlanId""}
                        fi
                    fi

                    L4SourcePort=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".L4SourcePort`
                    ${DBG:+echo L4SourcePort[$L4SourcePort]} 

                    if [ "${L4SourcePort}" != "NA" -a "${L4SourcePort}" != "0" ];then
                        if [ "$STAGE" = "QSET" ];then
                            result=`acc_sw "fp qset add L4SrcPort"`
                            ${DBG:+echo "acc_sw "fp qset add L4SrcPort""}
                        fi
                    fi

                    L4DestPort=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".L4DestPort`
                    ${DBG:+echo L4DestPort[$L4DestPort]} 

                    if [ "${L4DestPort}" != "NA" -a "${L4DestPort}" != "0" ];then
                        if [ "$STAGE" = "QSET" ];then
                            result=`acc_sw "fp qset add L4DstPort"`
                            ${DBG:+echo "acc_sw "fp qset add L4DstPort""}
                        fi
                    fi

                done  #Rule loop end

######################### SET QSET FIRST END ###################################

######################### CREATE GROUP START ###################################


                for (( cc=0; cc < "${ACL_RULE_A_C}"; cc++ ))
                do  
                    ${DBG:+echo show RULE${ACL_RULE_A[$cc]}} 
                    cc1=$(($cc+1))
                    Entry_ID=$(($c*20 + $cc1))

                    # Create Group/Entry in CRT_GRP_ENTRY state
                    if [ "$STAGE" = "CRT_GRP_ENTRY" ];then

${DBG:+echo "#############################GROUP SET###################################"}

#                      result=`acc_sw "fp group create 1 ${GRP_ID} auto 0x${Hex_ACL_PBMP}"`
#                      ${DBG:+echo "acc_sw "fp group create 1 ${GRP_ID} auto 0x${Hex_ACL_PBMP}""}

                       result=`acc_sw "fp group create 1 ${GRP_ID}"`
                       ${DBG:+echo "acc_sw "fp group create 1 ${GRP_ID}""}

                       result=`acc_sw "fp entry create ${GRP_ID} ${Entry_ID}"` 
                       ${DBG:+echo "acc_sw "fp entry create ${GRP_ID} ${Entry_ID}""}

# ??                   result=`acc_sw "fp qual  ${Entry_ID} InPorts 0x${Hex_ACL_PBMP} 0x${Hex_ACL_PBMP}" ` 
                       result=`acc_sw "fp qual  ${Entry_ID} InPorts 0x${Hex_ACL_PBMP} $OPENNSL_PBMASK" ` 
                       ${DBG:+echo "acc_sw "fp qual ${Entry_ID} InPorts 0x${Hex_ACL_PBMP} $OPENNSL_PBMASK""}
                    fi 

                done  #Rule loop end


######################### CREATE GROUP END   ###################################


######################### SET QSET CONDITION START #############################

                for (( cc=0; cc < "${ACL_RULE_A_C}"; cc++ ))
                do  
                    ${DBG:+echo show RULE${ACL_RULE_A[$cc]}} 
                    cc1=$(($cc+1))
                    Entry_ID=$(($c*20 + $cc1))

                    IPSource=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".IPSource`
                    ${DBG:+echo IPSource[$IPSource]} 

if [ "$STAGE" = "QSET_ADD" ];then
     ${DBG:+echo "acc_sw #############################CONDITION SET###################################"}
fi


                    if [ "${IPSource}" != "NA" ];then
                        if [ "$STAGE" = "QSET_ADD" ];then
                            IPSourceMASK=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".IPSourceMASK`
                            ${DBG:+echo IPSourceMASK[$IPSourceMASK]} 
                            result=`acc_sw "fp qual ${Entry_ID} SrcIp "${IPSource}" "${IPSourceMASK}""`
                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} SrcIp "${IPSource}" "${IPSourceMASK}"""}
                        fi
                    fi

                    IPDestination=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".IPDestination`
                    ${DBG:+echo IPDestination[$IPDestination]} 

                    if [ "${IPDestination}" != "NA" ];then
                        if [ "$STAGE" = "QSET_ADD" ];then
                            IPDestinationMASK=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".IPDestinationMASK`
                            ${DBG:+echo IPDestinationMASK[$IPDestinationMASK]} 
                            result=`acc_sw "fp qual ${Entry_ID} DstIp "${IPDestination}" "${IPDestinationMASK}""`
                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} DstIp "${IPDestination}" "${IPDestinationMASK}"""}
                        fi
                    fi

                    MACSource=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MACSource`
                    ${DBG:+echo MACSource[$MACSource]} 

                    if [ "${MACSource}" != "NA" ];then
                        if [ "$STAGE" = "QSET_ADD" ];then
                            MACSourceMASK=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MACSourceMASK`
                            ${DBG:+echo MACSourceMASK[$MACSourceMASK]} 
                            result=`acc_sw "fp qual ${Entry_ID} SrcMac "${MACSource}" "${MACSourceMASK}""`
                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} SrcMac "${MACSource}" "${MACSourceMASK}"""}
                        fi
                    fi

                    MACDestination=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MACDestination`
                    ${DBG:+echo MACDestination[$MACDestination]} 

                    if [ "${MACDestination}" != "NA" ];then
                        if [ "$STAGE" = "QSET_ADD" ];then
                            MACDestinationMASK=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MACDestinationMASK`
                            ${DBG:+echo MACDestinationMASK[$MACDestinationMASK]} 
                            result=`acc_sw "fp qual ${Entry_ID} DstMac "${MACDestination}" "${MACDestinationMASK}""`
#                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} DstMac "${MACDestination}" "${MACDestinationMASK}"""}

                        fi
                    fi

                    VLANID=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".VLANID`
                    ${DBG:+echo VLANID[$VLANID]} 

                    if [ "${VLANID}" != "NA" -a "${VLANID}" != "0" ];then
                        if [ "$STAGE" = "QSET_ADD" ];then
                            VLANIDMASK=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".VLANIDMASK`
                            ${DBG:+echo VLANIDMASK[$VLANIDMASK]} 
                            result=`acc_sw "fp qual ${Entry_ID} OuterVlanId "${VLANID}" "${VLANIDMASK}""`
#                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} OuterVlanId "${VLANID}" "${VLANIDMASK}"""}
                        fi
                    fi

                    L4SourcePort=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".L4SourcePort`
                    ${DBG:+echo L4SourcePort[$L4SourcePort]} 

                    if [ "${L4SourcePort}" != "NA" -a "${L4SourcePort}" != "0" ];then
                        if [ "$STAGE" = "QSET_ADD" ];then
                            L4SourcePortMASK=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".L4SourcePortMASK`
                            ${DBG:+echo L4SourcePortMASK[$L4SourcePortMASK]} 
                            result=`acc_sw "fp qual ${Entry_ID} OuterVlanId "${L4SourcePort}" "${L4SourcePortMASK}""`
#                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} OuterVlanId "${L4SourcePort}" "${L4SourcePortMASK}"""}
                        fi
                    fi

                    L4DestPort=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".L4DestPort`
                    ${DBG:+echo L4DestPort[$L4DestPort]} 

                    if [ "${L4DestPort}" != "NA" -a "${L4DestPort}" != "0" ];then
                        if [ "$STAGE" = "QSET_ADD" ];then
                            L4DestPortMASK=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".L4DestPortMASK`
                            ${DBG:+echo L4DestPortMASK[$L4DestPortMASK]} 
                            result=`acc_sw "fp qual ${Entry_ID} OuterVlanId "${L4DestPort}" "${L4DestPortMASK}""`
#                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} OuterVlanId "${L4DestPort}" "${L4DestPortMASK}"""}
                        fi
                    fi

                    L4Protocol=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".L4Protocol`
                    ${DBG:+echo L4Protocol[$L4Protocol]} 

                    if [ "${L4Protocol}" != "NA" -a "${L4Protocol}" != "0" ];then
                    echo "L4 NA"
                    #SDK NOT SUPPORT#
                    fi

                    ForwardMirrorInterface=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".ForwardMirrorInterface`
                    ${DBG:+echo ForwardMirrorInterface[$ForwardMirrorInterface]} 

                    MirrorPortRegion=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MirrorPortRegion`
                    ${DBG:+echo MirrorPortRegion[$MirrorPortRegion]} 

                    MirrorType=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".MirrorType`
                    ${DBG:+echo MirrorType[$MirrorType]} 

######################### INSTALL RULE        #############################
                    if [ "$STAGE" = "QSET_ADD" ];then
                        Action=`uci get RULE.RULE"${ACL_RULE_A[$cc]}".Action`
                        ${DBG:+echo Action[$Action]} 
                        if [ "${Action}" = "Deny" ];then

                            result=`acc_sw "fp action add ${Entry_ID} Drop"` 
                            ${DBG:+echo "acc_sw "fp action add ${Entry_ID} Drop""}

                            result=`acc_sw "fp qual ${Entry_ID} InPorts 0x${Hex_ACL_PBMP} $OPENNSL_PBMASK" ` 
                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} InPorts 0x${Hex_ACL_PBMP} $OPENNSL_PBMASK""}

                        elif [ "${Action}" = "Mirror" ];then

                            result=`acc_sw "fp qual ${Entry_ID} InPorts ${MirrorPortRegion} $OPENNSL_PBMASK" ` 
                            ${DBG:+echo "acc_sw "fp qual ${Entry_ID} InPorts ${MirrorPortRegion} ${MirrorPortRegion}""} 

                            result=`acc_sw "fp action add ${Entry_ID} MirrorIngress 0 ${ForwardMirrorInterface}"` 
                            ${DBG:+echo "acc_sw "fp action add ${Entry_ID} MirrorIngress 0 ${ForwardMirrorInterface}""}

                        elif [ "${Action}" = "Permit" ];then

                            result=`acc_sw "fp action ports add ${Entry_ID} RedirectPbmp ${MirrorPortRegion}"` 
                            ${DBG:+echo "acc_sw "fp action ports add ${Entry_ID} RedirectPbmp $MirrorPortRegion""}

                        elif [ "${Action}" = "Forward" ];then

                            result=`acc_sw "fp qual ${Entry_ID} InPorts ${MirrorPortRegion} $OPENNSL_PBMASK" ` 
                            Hex_ForwardMirrorInterface=`printf '%x' $((10#$ForwardMirrorInterface))`
                            result=`acc_sw "fp action ports add ${Entry_ID} RedirectPbmp ${ForwardMirrorInterface}"` 
                            ${DBG:+echo "acc_sw "fp action ports add ${Entry_ID} RedirectPbmp ${ForwardMirrorInterface}""}

                        fi

                            result=`acc_sw "fp entry install ${Entry_ID}"`
                            ${DBG:+echo "acc_sw "fp entry install ${Entry_ID}""}
                            result=`acc_sw "fp qset clear"`
                            ${DBG:+echo "acc_sw "fp qset clear""}
                        ${DBG:+echo "==========INSTALL ACL RULE========"}
                    fi

               done  #Rule loop end



              done # Stage loop end 
            fi

        fi

    done #ACL loop end
    ;;

"clean")
    result=`acc_sw "fp init"`
    result=`acc_sw "fp qset clear"`
    ;;
*)
;;

esac

