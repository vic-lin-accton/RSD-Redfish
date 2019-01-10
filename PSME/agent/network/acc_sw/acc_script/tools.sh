#!/bin/bash

help()
{
echo "Only support <= 54 ports device application."
echo "PARA1 is add/remove"
echo "PARA2 is port number (port 1~54 )"
echo "PARA3 is original value pbm (HEX 0x1234567890123456 format)"
}

mapping_hexport()
{
# Only support <= 54 ports device application.
# $1 is "add"/"remove" 
# $2 is port number (port 1~54 )
# $3 is original value pbm (HEX 0x1234567890123456 format)

# Check platform.
# Mask on original value of pbm.

#  echo param1 add[$1] param2 port[$2] parma3 orig[$3]

  B1TO49A=`echo -n $3 | tail -c 13`
  B50TO54A=`echo -n $3 | tail -c 18`
  B50TO54=${B50TO54A:0:-13}
  H1TO49="0x$B1TO49A"
  H50TO54="0x$B50TO54"
  N1TO49=$(($H1TO49+0))
  N50TO54=$(($H50TO54+0))


  if [ "$1" = "remap" ];then
 
		if [ "$2" -gt 49 ];then
			SB=$(($2-50))
			SSB=$((1+$SB*4))
                        NSSB=$((1<<$SSB))
			N50TO54=$(($NSSB | $N50TO54))
		else
			shp=$((1 <<  ${2}))
			N1TO49=$((${N1TO49} | ${shp}))
		fi

		PX1TO49=$(printf %013x $N1TO49)
		PX50TO54=$(printf %05x $N50TO54)
		F_PBM="0x$PX50TO54$PX1TO49"
		echo $F_PBM
		exit
  fi  
 

  if [ "$1" = "add" ];then
 
		if [ "$2" -gt 49 ];then
			SB=$(($2-50))
			SSB=$((1+$SB*4))
                        NSSB=$((1<<$SSB))
			N50TO54=$(($NSSB | $N50TO54))
		else
			shp=$((1 <<  ${2}))
			N1TO49=$((${N1TO49} | ${shp}))
		fi

		PX1TO49=$(printf %013x $N1TO49)
		PX50TO54=$(printf %05x $N50TO54)
		F_PBM="0x$PX50TO54$PX1TO49"
		echo $F_PBM
  
  else

		if [ "$2" -gt 49 ];then
			SB=$(($2-50))
			SSB=$((1+$SB*4))
                        NSSB=$((1<<$SSB))
			NSSB=$((~$NSSB))
			N50TO54=$(($NSSB & $N50TO54))
		else
			shp=$((1 <<  ${2}))
			nshp=$((~$shp))
			N1TO49=$((${N1TO49} & ${nshp}))
		fi

		PX1TO49=$(printf %013x $N1TO49)
		PX50TO54=$(printf %05x $N50TO54)
		F_PBM="0x$PX50TO54$PX1TO49"
		echo $F_PBM
  fi 
}

is_port_in_pbm()
{
# Only support <= 54 ports device application.
# $1 is port number (port 1~54 )
# $2 is original value pbm (HEX 0x1234567890123456 format)

# Check platform.
# Mask on original value of pbm.

#  echo param1 add[$1] param2 port[$2] parma3 orig[$3]

  B1TO49A=`echo -n $2 | tail -c 13`
  B50TO54A=`echo -n $2 | tail -c 18`
  B50TO54=${B50TO54A:0:-13}
  H1TO49="0x$B1TO49A"
  H50TO54="0x$B50TO54"
  N1TO49=$(($H1TO49+0))
  N50TO54=$(($H50TO54+0))

  if [ "$1" -gt 49 ];then
	SB=$(($1-50))
	SSB=$((1+$SB*4))
        NSSB=$((1<<$SSB))
	N50TO54=$(($NSSB & $N50TO54))
	if [ "$N50TO54" = 0 ];then
		echo 0	
	else
		echo 1	
	fi
  else
	shp=$((1 <<  ${1}))
	N1TO49=$((${N1TO49} & ${shp}))

	if [ "$N1TO49" = 0 ];then
		echo 0	
	else
		echo 1	
	fi
  fi
}

mapping_bin2hexport()
{
# Only support <= 54 ports device application.
# $1 is "convert" 
# $2 is port number (port 1~54 ) #NOT USE CURRENTLY
# $3 is original value pbm (binary 1100000000000000000000000000000000000000000000000011110 format)

# Check platform.
# Mask on original value of pbm.
# echo param1 add[$1] param2 port[$2] parma3 orig[$3]

  ACL_PBMP=$3
  B1TO49A=`echo -n $ACL_PBMP | tail -c 50`
  B50TO54A=`echo -n $ACL_PBMP | tail -c 55`
  B50TO54=${B50TO54A:0:-50}
  H1TO49=`printf '%x' $((2#$B1TO49A))`
  H1TO49=0x$H1TO49
  H50TO54=`printf '%x' $((2#$B50TO54))`
  H50TO54=0x$H50TO54
  N1TO49=$(($H1TO49+0))
  N50TO54=$(($H50TO54+0))
  F50TO54=0

  if [ "$1" = "convert" ];then
 
		if [ "$N50TO54" -gt 0 ];then
		        for (( c=0; c < 5; c++ ))
        		do
				SN50T54=$(($N50TO54 >> $c))
				SN50T54=$(($SN50T54 & 1))
				if [ $SN50T54 != 0 ];then
					SB=$c
					SSB=$((1+$SB*4))
					NSSB=$((1<<$SSB))
					F50TO54=$(($NSSB | $F50TO54))
				fi
        		done
		fi

		PX1TO49=$(printf %013x $N1TO49)
		PX50TO54=$(printf %05x $F50TO54)
		F_PBM="$PX50TO54$PX1TO49"
		echo $F_PBM
		exit
  fi  
}





#if [ $# -eq 0 ];then
#	help
#       exit 99
#else
#	#return_var=$(mapping_hexport $1 $2 $3)
#	mapping_hexport $1 $2 $3
#	#echo return[$return_var]
#       #is_port_in_pbm $1 $2
#fi
#return_var=$(mapping_bin2hexport convert 48 1111111111111111111111111111111111111111111111111111110 )
#echo return[$return_var]
