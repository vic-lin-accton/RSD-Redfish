#!/bin/bash

NETSERVE_PID=`pidof netserve`

if [ "$NETSERVE_PID" = "" ];then
    exit
fi

. /usr/local/bin/psme.sh
. /usr/local/bin/tools.sh

MAX_PORT=`psme.sh get max_port_num`

help_p()
{
reset
echo ""
echo "////////////////////////////////////////////////////////////////////////////////////////////////"
echo "trunk.sh <get> <num/num_index/mem_count/mem_IN> <1~9> <1~$MAX_PORT>" 
echo "trunk.sh <set> <mem/ID_del> <1~9> <1~$MAX_PORT>" 
echo ""
echo "NOTE:"
echo ""
echo "get num                                           : Get total LAG number       "
echo "get num_index  [INDEX ID:1~9]                     : Get LAG number by INDEX    "
echo "get mem_count  [TRUNK ID:1~9]                     : Get LAG ID member count    "
echo "get mem_IN     [TRUNK ID:1~9] [PORT:1~$MAX_PORT]         : Check if Port in LAG ID    "
echo ""
echo "set mem        [TRUNK ID:1~9] [PORT:1~$MAX_PORT]         : Create turnk ID with PORT  "
echo "set ID_del     [TRUNK ID:1~9]                     : Delete turnk ID            "
echo ""
}

DATE=`date '+%Y-%m-%d %H:%M:%S'`
echo [$DATE] [trunk.sh $1 $2 $3 $4 $5 $6] >> /var/log/SDK_CMD_LOG 

if [ $# -eq 0 ];then
        help_p
        exit 99
fi

case "${1}" in

"get")
        case "${2}" in
        
        "num")
                                if [ -f /tmp/LTRUNK ];then
                                    rm  /tmp/LTRUNK
                                fi
                                IFS=$'\n'
 
                                mem_array=($(uci show NETWORK | grep 'switch_trunk' | awk -F'=' '{print $1}' | sed -e 's/NETWORK.Trunk_//g'))
 
                                if [ "$mem_array" = "" ];then
				    echo 0 
                                else
				    TOTAL_COUNT=${#mem_array[@]} 
                                    for (( c=0; c < "${TOTAL_COUNT}"; c++ ))
                                    do
                                        content=${mem_array[$c]}
                                        echo "${content}" >> /tmp/LTRUNK
                                    done
				    echo "$TOTAL_COUNT" 
                                fi
        ;;

        "num_index")
                                INDEX=${3}
                                INDEX=$(($INDEX-1))
                                if [ -f /tmp/LTRUNK ];then
                                    mem_array=($(cat /tmp/LTRUNK))
                                    TOTAL_COUNT=${#mem_array[@]}
                                    if [ "$TOTAL_COUNT" != 0 ];then
    			  	        echo ${mem_array[$INDEX]}
                                    fi
                                fi
 
        ;;

        "mem_count")
                                IFS=$','
                                ID=${3}
                                if [ -f /tmp/TRUNK$ID ];then
                                    rm  /tmp/TRUNK$ID
                                fi
                                HMEMBER=`uci get NETWORK.Trunk_$ID.port`
				TOTAL_COUNT=$MAX_PORT
				MEM_COUNT=0

                                for (( c=1; c <= "${TOTAL_COUNT}"; c++ ))
                                do
            				ISIN=$(is_port_in_pbm $c $HMEMBER)

            				if [ "$ISIN" = 1 ];then
                                        	echo "${c}" >> /tmp/TRUNK$ID
				        	MEM_COUNT=$(($MEM_COUNT + 1))
            				fi
                                done
                                echo $MEM_COUNT

        ;;

        "mem_IN")
                                ID=${3}
                                PORT=${4}

                                if [ -f /tmp/TRUNK$ID ];then
                                  TT=($(cat /tmp/TRUNK$ID))

				  TOTAL_COUNT=${#TT[@]} 
                                  for (( c=0; c < "${TOTAL_COUNT}"; c++ ))
                                  do
                                    content=${TT[$c]}
                                    #echo "content[$content] PORT[$PORT]"
                                    if [ "$content" = "$PORT" ];then
                                       echo 1
                                       exit 
                                    fi
                                  done
                                  echo 0

                                fi
        ;;
 

        esac

        ;;    
"set")
        case "${2}" in
       "mem")
        ID=$3

        NO_MAP_PORT=$4

        if [ $NO_MAP_PORT -gt $MAX_PORT ];then
                echo ""
                exit 99
        fi

        IFS=$','

        ID=`echo $ID | sed -e s/LAG//g`

        org_mem=`uci show | grep NETWORK.Trunk_$ID.port`

        if [ "$org_mem" != "" ];then
            # Already has Trunk ID
            HMEMBER=`uci get NETWORK.Trunk_$ID.port`
            #add/remove port orgvalue
            HMEMBER=$(mapping_hexport add $NO_MAP_PORT $HMEMBER)
        else
            # 1st create Trunk ID
            HMEMBER=$(mapping_hexport add $NO_MAP_PORT 0x000000000000000000)
            RESULT=`uci set NETWORK.Trunk_"$ID"=switch_trunk`
        fi

        RESULT=`acc_sw "trunk add id=$ID pbmp=$HMEMBER"`
        #echo "acc_sw "trunk add id=$ID pbmp=$HMEMBER""
        RESULT=`uci set NETWORK.Trunk_"$ID".port="$HMEMBER"; uci commit`

        ;;

       "ID_del")
        ID=$3
        ID=`echo $ID | sed -e 's/LAG//g'`

        HMEMBER=`uci get NETWORK.Trunk_$ID.port`
        RESULT=`acc_sw "trunk remove id=$ID pbmp=$HMEMBER"`
        RESULT=`uci delete NETWORK.Trunk_"$ID"; uci commit`
        ;;

        esac
        ;;
*)
       help_p
       ;;
esac

