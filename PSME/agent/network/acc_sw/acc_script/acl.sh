#!/bin/bash

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

help_p()
{
echo "get acl_count                                                             "
echo "get acl_rule_count [acl_index 1.2.3...]                                                             "
echo "get acl_bindport [acl_index 1.2.3...] //ex: 1101 , port 1 3 4 bind to acl "
echo "get acl_index_content [index 0.1.2...]                                    "
echo "get acl_include_rule_count [acl_index 1.2.3..]                            "
echo "get acl_include_rule [acl_index 1.2.3..] [index 0.1.2.3..]                "

echo "set add_acl                                                               " 
echo "set del_acl      [acl_index 1.2.3...]                                     "
echo "set bind_acl     [acl_index 1.2.3...] [port list [1 2 3 4..54]]           "
echo "set unbind_acl   [acl_index 1.2.3...] [port list [1 2 3 4..54]]           "
echo "set modify_acl_rule [acl_index 1.2.3...] [rule_list [1 2 3 4...]          "
echo "set add_acl_rule [acl_index 1.2.3...] [rule_index \"\" 1 2 3]             "
echo "set del_acl_rule [acl_index 1.2.3...] [rule_index 1.2.3... ]              "
}

Add_Default_Rule()
{
    ACL_INDEX=$1
    RULE_INDEX=$2
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX"=RULE
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".Action='Deny'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".ForwardMirrorInterface='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".MirrorPortRegion='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".MirrorType='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".IPSource='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".IPSourceMASK='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".IPDestination='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".IPDestinationMASK='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".MACSource='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".MACSourceMASK='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".MACDestination='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".MACDestinationMASK='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".VLANID='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".VLANIDMASK='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".L4SourcePort='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".L4SourcePortMASK='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".L4DestPort='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".L4DestPortMASK='NA'
    uci set RULE.RULE"$ACL_INDEX"_"$RULE_INDEX".L4Protocol='NA'
    uci commit
}

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [acl.sh $1 $2 $3 $4 $5 $6] >> /var/log/SDK_CMD_LOG 

case "${1}" in

"get")
        case "${2}" in
# To get ACL member #
        "acl_count")
        result=`uci show ACL |  grep -E 'ACL[0-9][0-9]=|ACL[0-9]=' | awk -F'=' '{print $1}' | sed -e "s/ACL.ACL//g" > /tmp/ACL`
        content=($(cat /tmp/ACL))
        count=`uci show ACL |  grep -E 'ACL[0-9][0-9]=|ACL[0-9]=' | wc -l`

        IFS=$'\n'
        scontent=($(sort <<< "${content[*]}"))
        `rm /tmp/ACL`
	for (( c=0; c < "${count}"; c++ ))
  	do 
            `echo "${scontent[$c]}" >> /tmp/ACL`
	done

        echo $count
        ;;

        "acl_rule_count")

        IFS=$' '
        scount=($(uci get ACL.ACL"$3".RULE))
        count=${#scount[@]}

        if [ ${scount[0]} = "NA" ];then
            echo 0
        else
            echo $count
        fi
        ;;



        "acl_index_content")
        index=$3
        content=($(cat /tmp/ACL))
        echo ${content[$index]} 
        ;;
##acc_sw get acl_bindport in decimal mode 
##
        "acl_bindport")
        acl_index="${3}"

        bMEM=`uci show | grep ACL.ACL"$acl_index".PBMP`

        if [ "$bMEM" = "" ];then
           echo ""
           exit
        fi

        MEMBER=`uci get ACL.ACL"$acl_index".PBMP`

        if [ "$MEMBER" = "NA" ];then
            echo ""
        elif [ "$((2#$MEMBER))" = "0" ];then
            echo ""
        else
            echo "$((2#$MEMBER))"
        fi

	;;

# To get ACL index rule member count #
        "acl_include_rule_count")
        acl_index="${3}"

        result=`uci get ACL.ACL$acl_index.RULE | sed -e  's/NA//g' | sed -e 's/ \+/ /g'  > /tmp/ACL"$acl_index"_RULE`
        IFS=$' '
        content=($(cat /tmp/ACL"$acl_index"_RULE))
        count=${#content[@]}
        echo $count
        ;;

# To get ACL index rule member#
        "acl_include_rule")
        acl_index="${3}"
        content_index="${4}"

        IFS=$' '
        content=($(cat /tmp/ACL"$acl_index"_RULE))
        ff=${content[$content_index]}
        fff=`echo $ff | sed -e s/"$acl_index"_//g`
        echo $fff
        ;;

        esac
	;;
"set")
        case "${2}" in

        "add_acl")
        result=`uci show ACL | grep "ACL.ACL.=" | awk -F'=' '{print $1}' | sed -e "s/ACL.ACL//g" > /tmp/ACL`
        count=`uci show ACL | grep "ACL.ACL.=" | wc -l`

        content=($(cat /tmp/ACL))
        IFS=$'\n'
        scontent=($(sort <<< "${content[*]}"))

	for (( c=1; c <= "${count}"; c++ ))
  	    do 
            if [ "${scontent[$(($c-1))]}" != "$c" ];then
                `uci set ACL.ACL"$c"=ACL`
                `uci set ACL.ACL"$c".RULE='NA'`
                `uci set ACL.ACL"$c".PBMP='NA'`
                `uci commit`
                echo $c
                exit 
            fi
	done
        `uci set ACL.ACL"$c"=ACL`
        `uci set ACL.ACL"$c".RULE='NA'`
        `uci set ACL.ACL"$c".PBMP='NA'`
        `uci commit`
        echo $c
        ;;

        "del_acl")
        acl_index=$3

        bMEM=`uci show | grep ACL.ACL"$acl_index".RULE`

        if [ "$bMEM" = "" ];then
           echo ""
           exit
        fi

        rule_list=($(uci get ACL.ACL$acl_index.RULE |  sed -e 's/^ *//'  | sed -e 's/ \+/ /g'))

        rule_list_count=${#rule_list[@]}

        if [ "$rule_list_count" = 0 ];then
            exit
        fi

	for (( gi=0; gi < "${rule_list_count}"; gi++ ))
  	do 
            if [ "${rule_list[$gi]}" != "NA" ];then
                `uci delete RULE.RULE${rule_list[$gi]}`
            fi
        done

        `uci delete ACL.ACL"$acl_index"`
        `uci commit`
        ;;

        "bind_acl")
        acl_index=$3
        new_mem=$4

        MEMBER=`uci get ACL.ACL"$acl_index".PBMP`

        if [ "$MEMBER" = "NA" ];then
            old_mem=0
        else
            old_mem=$((2#$MEMBER))
        fi

        IFS=' '
        new_mem_list=($(echo $new_mem))
        new_list_count=${#new_mem_list[@]}

	for (( c=0; c < "${new_list_count}"; c++ ))
  	    do 
            tmp=${new_mem_list[$c]}
            stmp=$((1 << $tmp))
            old_mem=$(($old_mem | $stmp))
	done

        for (( n=$old_mem ; n>0 ; n >>=1 ))
        do 
            bit="$(( n&1))$bit"
        done

        ff=`printf "%s" "$bit"`
        uci set ACL.ACL$acl_index.PBMP="$ff"
        uci commit
#        echo [$ff]
        ;;

        "unbind_acl")
        acl_index=$3
        new_mem=$4

        MEMBER=`uci get ACL.ACL"$acl_index".PBMP`

        if [ "$MEMBER" = "NA" ];then
            old_mem=0
        else
            old_mem=$((2#$MEMBER))
        fi

        IFS=' '
        new_mem_list=($(echo $new_mem))
        new_list_count=${#new_mem_list[@]}

        snew_mem=0
	for (( c=0; c < "${new_list_count}"; c++ ))
  	    do 
            tmp=${new_mem_list[$c]}
            stmp=$((1 << $tmp))
            snew_mem=$(($snew_mem | $stmp))
	done

        rev=$((~$snew_mem))

        ff=$(($rev & $old_mem))

        for (( n=$ff ; n>0 ; n >>=1 ))
        do 
            bit="$(( n&1))$bit"
        done

        fff=`printf "%s" "$bit"`
        if [ "$fff" = "" ];then
            uci set ACL.ACL$acl_index.PBMP="NA"
        else
            uci set ACL.ACL$acl_index.PBMP="$fff"
        fi
        uci commit
#        echo [$fff]
        ;;

        "modify_acl_rule")
        acl_index=$3
        rule_list=$4
        new_mem_list=($(echo $rule_list))

        new_list_count=${#new_mem_list[@]}
        flist=""

	for (( c=0; c < "${new_list_count}"; c++ ))
  	    do 
            tmp=${new_mem_list[$c]}
            flist="$flist"$acl_index"_"$tmp" "
	done
#        echo flist"[$flist]"
        uci set ACL.ACL$acl_index.RULE="$flist"
        uci commit
        ;;

        "add_acl_rule")

        acl_index="${3}"
        rule_index="${4}"

        if [ "$rule_index" = "" ];then
            old_rule_mem=`uci get ACL.ACL$acl_index.RULE | sed -e  's/NA//g' `

            result=`uci get ACL.ACL$acl_index.RULE | sed -e 's/^ *//' | sed -e  's/NA//g' | sed -e 's/ \+/ /g' | sed -e s/"$acl_index"_//g > /tmp/ACL"$acl_index"_RULE_NO_INDEX`
            IFS=$' '
            content=($(cat /tmp/ACL"$acl_index"_RULE_NO_INDEX))
            count=${#content[@]}

            scontent=($(sort <<< "${content[*]}"))

	    for (( c=1; c <= "${count}"; c++ ))
  	    do 

                if [ "${scontent[$(($c-1))]}" != "$c" ];then
                    `uci set ACL.ACL"$acl_index".RULE="$old_rule_mem "$acl_index"_"$c""`
                    ``
                    Add_Default_Rule "$acl_index" "$c"
                    `uci commit`
                    echo $c
                    exit 
                fi
	    done

            `uci set ACL.ACL"$acl_index".RULE="$old_rule_mem "$acl_index"_"$c""`
            Add_Default_Rule "$acl_index" "$c"
            `uci commit`
            echo $c
        else
            old_rule_mem=`uci get ACL.ACL$acl_index.RULE  | sed -e  's/NA//g'  |  sed -e 's/^ *//'  | sed -e 's/ \+/ /g'`

            sub_string="$acl_index"_"$rule_index"

            if test "${old_rule_mem#$sub_string}" != "$old_rule_mem"
            then
                echo ${rule_index}
                ## Already has this rule in ACL ##
            else
                `uci set ACL.ACL"$acl_index".RULE="$old_rule_mem "$acl_index"_"$rule_index""`
                Add_Default_Rule "$acl_index" "$rule_index"
                `uci commit`
                echo ${rule_index}
            fi
        fi 
        ;;



        "del_acl_rule")
        acl_index=$3
        rule_index=$4
        remove="$acl_index"_"$rule_index"

        tmp=`uci show | grep RULE.RULE"$remove"`
        if [ "$tmp" = "" ];then
           exit   
        fi

        old_mem=`uci get ACL.ACL$acl_index.RULE`
        ff=`echo $old_mem | sed -e "s/$remove//g" |  sed -e 's/^ *//' `

        if [ "$ff" = "" ];then
            uci set ACL.ACL$acl_index.RULE="NA"
            uci delete RULE.RULE"$remove"
        else
            uci set ACL.ACL$acl_index.RULE="$ff"
            uci delete RULE.RULE"$remove"
        fi
        
        uci commit
        ;;
        esac
        ;;
*)
help_p
;;

esac

