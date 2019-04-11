#!/bin/bash

# first make sure we are running from the base directory
if [ ! -e board/renesas/rza1template/rza1template.c ] ; then
  echo "ERROR: You must run this script from the base directory."
  echo "Press enter to continue."
  read dummy
  exit
fi

#defaults
boardname=toaster
companyname=mycompany
devicetype=RZ_A1H

extal=13.33MHz
p1clockspeed=66666666
hasusbxtal=no

scif=SCIF2

hasmmc=no
hassdhi=no
hasnorflash=no
haseth=no

hasi2c=no
hasusbhost=no

hassdram=no
hashyperram=no

while [ "1" == "1" ]
do
	echo 'whiptail --title "Add new BSP Board"  --noitem --menu "Make changes the items below as needed, then select Create BSP.\nYou may use ESC+ESC to cancel." 0 0 0 \' > /tmp/whipcmd.txt
	echo '"         Board Name: $boardname" "" \' >> /tmp/whipcmd.txt
	echo '"       Company Name: $companyname" "" \' >> /tmp/whipcmd.txt
	echo '"        Device Type: $devicetype" "" \' >> /tmp/whipcmd.txt
	echo '"              EXTAL: $extal" "" \' >> /tmp/whipcmd.txt
      if [ "$devicetype" != "RZ_A2M" ] ; then
	echo '"     48MHz USB XTAL: $hasusbxtal" "" \' >> /tmp/whipcmd.txt
      fi
	echo '"     Serial Console: $scif" "" \' >> /tmp/whipcmd.txt
	echo '"     External SDRAM: $hassdram" "" \' >> /tmp/whipcmd.txt
	echo '"  External HyperRAM: $hashyperram" "" \' >> /tmp/whipcmd.txt
	echo '"         eMMC Flash: $hasmmc" "" \' >> /tmp/whipcmd.txt
	echo '"       SD Card Host: $hassdhi" "" \' >> /tmp/whipcmd.txt
      if [ "$devicetype" != "RZ_A2M" ] ; then
	echo '" Parallel NOR Flash: $hasnorflash" "" \' >> /tmp/whipcmd.txt
      fi
	echo '"           Ethernet: $haseth" "" \' >> /tmp/whipcmd.txt
	echo '"                I2C: $hasi2c" "" \' >> /tmp/whipcmd.txt
      if [ "$devicetype" != "RZ_A2M" ] ; then
	echo '"           USB Host: $hasusbhost" "" \' >> /tmp/whipcmd.txt
      fi
	echo '"       [Create BSP]" "" \' >> /tmp/whipcmd.txt
	echo '2> /tmp/answer.txt' >> /tmp/whipcmd.txt

  source /tmp/whipcmd.txt

  #ans=$(head -c 3 /tmp/answer.txt)
  ans=$(cat /tmp/answer.txt)

  # Cancel
  if [ "$ans" == "" ]; then
    exit
    break;
  fi

  ####################################
  # boardname
  ####################################
  echo "$ans" | grep -q "Board Name:"
  if [ "$?" == "0" ] ; then
    whiptail --title "Board Name" --inputbox \
"Enter a board name.\n"\
"Please use all lower case, no spaces.\n"\
"Numbers are OK as long as the first character\n"\
"is not a number.\n"\
"This string will be used for file names, directory names and in the source code.\n"\
"This name has to be unique.\n"\
"Example: rskrza1" 0 20 \
      2> /tmp/answer.txt

    boardname=$(cat /tmp/answer.txt)

    CHECK=`grep $companyname/$boardname arch/arm/mach-rmobile/Kconfig.rza1`
    if [ "$CHECK" != "" ] ; then
      whiptail --title "Name Conflict" --yesno "ERROR:\n      Board Name=$boardname\n    Company Name=$companyname\n\n This combination has already been used. Do you want to overwrite those files?" 0 0 2> /tmp/answer.txt

      yesno=$?
      if [ "$yesno" == "1" ] ; then
        # no
        boardname=toaster
      fi
    fi

    #check for spaces
    echo "$boardname" | grep -q " "
    if [ "$?" == "0" ] ; then
      whiptail --msgbox "ERROR:\nBoard Name contains spaces ($boardname).\nPlesae try again." 0 0
      boardname=toaster
    fi

    #check for any character other than lowercase letters or numbers
    CHECK=`echo "$boardname" | grep "^[a-z0-9]\+$"`
    if [ "$?" != "0" ] ; then
      whiptail --msgbox "ERROR:\nBoard Name contains characters other than lowercase letters and numbers $CHECK ($boardname).\nPlesae try again." 0 0
      boardname=toaster
    fi

    continue
  fi

  ####################################
  # companyname
  ####################################
  echo "$ans" | grep -q "Company Name:"
  if [ "$?" == "0" ] ; then
    whiptail --title "Board Name" --inputbox \
"Enter a company name.\n"\
"Please use all lower case, no spaces.\n"\
"Numbers are OK as long as the first character\n"\
"is not a number.\n"\
"This string will be used as the directory name for the custom boards files.\n"\
"You can use the same company name for multiple boards.\n"\
"\n"\
"Example: acme (will create directory u-boot-2017.05/board/acme/ " 0 20 \
      2> /tmp/answer.txt

    companyname=$(cat /tmp/answer.txt)

    CHECK=`grep $companyname/$boardname arch/arm/mach-rmobile/Kconfig.rza1`
    if [ "$CHECK" != "" ] ; then
      whiptail --title "Name Conflict" --yesno "ERROR:\n      Board Name=$boardname\n    Company Name=$companyname\n\n This combination has already been used. Do you want to overwrite those files?" 0 0 2> /tmp/answer.txt

      yesno=$?
      if [ "$yesno" == "1" ] ; then
        # no
        companyname=mycompany
      fi
    fi

    #check for spaces
    echo "$companyname" | grep -q " "
    if [ "$?" == "0" ] ; then
      whiptail --msgbox "ERROR:\nCompany Name contains spaces ($companyname).\nPlesae try again." 0 0
      companyname=mycompany
    fi

    #check for any character other than lowercase letters or numbers
    CHECK=`echo "$companyname" | grep "^[a-z0-9]\+$"`
    if [ "$?" != "0" ] ; then
      whiptail --msgbox "ERROR:\nCompany Name contains characters other than lowercase letters and numbers $CHECK ($companyname).\nPlesae try again." 0 0
      companyname=mycompany
    fi

    continue
  fi

  ####################################
  # devicetype
  ####################################
  echo "$ans" | grep -q "Device Type:"
  if [ "$?" == "0" ] ; then
    whiptail --title "Device Type" --nocancel --menu "What RZ/A device are you using?" 0 0 0 \
	"RZ_A1H" "(10MB intneral RAM)" \
	"RZ_A1M" "(5MB intneral RAM)" \
	"RZ_A1L" "(3MB intneral RAM)" \
	"RZ_A2M" "(4MB intneral RAM)" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    devicetype=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # scif
  ####################################
  echo "$ans" | grep -q "Serial Console:"
  if [ "$?" == "0" ] ; then
    whiptail --title "Serial Console" --nocancel --menu "What SCIF channel is your serial console on?" 0 0 0 \
	"SCIF0" "TxD0/RxD0" \
	"SCIF1" "TxD1/RxD1" \
	"SCIF2" "TxD2/RxD2" \
	"SCIF3" "TxD3/RxD3" \
	"SCIF4" "TxD4/RxD4" \
	"SCIF5" "TxD5/RxD5" \
	"SCIF6" "TxD6/RxD6" \
	"SCIF7" "TxD7/RxD7" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    scif=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # extal
  ####################################
  echo "$ans" | grep -q "EXTAL:"
  if [ "$?" == "0" ] ; then
    if [ "$devicetype" != "RZ_A2M" ] ; then
      whiptail --title "EXTAL"  --nocancel --menu "What speed is the EXTAL clock?" 0 0 0 \
	"10MHz" "" \
	"12MHz" "" \
	"13.33MHz" "" \
	"none" "(assumes you only have a 48 MHz USB clock)" \
	2> /tmp/answer.txt
    else
      whiptail --title "EXTAL"  --nocancel --menu "What speed is the EXTAL clock?" 0 0 0 \
	"10MHz" "" \
	"12MHz" "" \
	"20MHz" "" \
	"24MHz" "" \
	2> /tmp/answer.txt
    fi

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    extal=$(cat /tmp/answer.txt)

    if [ "$devicetype" == "RZ_A2M" ] ; then
      # RZ/A2
      if [ "$extal" == "10MHz" ] || [ "$extal" == "20MHz" ] ; then
        p1clockspeed=55000000
      fi
      if [ "$extal" == "12MHz" ] || [ "$extal" == "24MHz" ] ; then
        p1clockspeed=66000000
      fi
    else
      # RZ/A1
      if [ "$extal" == "10MHz" ] ; then
        p1clockspeed=50000000
      fi
      if [ "$extal" == "12MHz" ] ; then
        p1clockspeed=64000000
      fi
      if [ "$extal" == "none" ] ; then
        p1clockspeed=64000000
      fi
      if [ "$extal" == "13.33MHz" ] ; then
        p1clockspeed=66666666
      fi
    fi
    continue
  fi


  ####################################
  # hasusbxtal
  ####################################
  echo "$ans" | grep -q "48MHz USB XTAL:"
  if [ "$?" == "0" ] ; then
    whiptail --title "48MHz USB XTAL" --nocancel --menu "Does this board have a 48MHz USB XTAL?" 0 0 0 \
	"yes" "" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hasusbxtal=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # hasmmc
  ####################################
  echo "$ans" | grep -q "eMMC Flash:"
  if [ "$?" == "0" ] ; then

    if [ "$devicetype" != "RZ_A2M" ] ; then
      whiptail --title "eMMC Flash" --nocancel --menu "Does this board have eMMC Flash?" 0 0 0 \
	"yes" "" \
	"no" "" \
	2> /tmp/answer.txt
    else
      whiptail --title "eMMC Flash" --nocancel --menu "Does this board have eMMC Flash?" 0 0 0 \
	"ch0" "SD/MMC-0" \
	"ch1" "SD/MMC-1" \
	"no" "" \
	2> /tmp/answer.txt
    fi

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hasmmc=$(cat /tmp/answer.txt)
    continue
  fi


  ####################################
  # hassdhi
  ####################################
  echo "$ans" | grep -q "SD Card Host:"
  if [ "$?" == "0" ] ; then
    whiptail --title "SD Card Host" --nocancel --menu "Does this board have SD Card Host?" 0 0 0 \
	"ch0" "SDHI-0" \
	"ch1" "SDHI-1" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hassdhi=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # haseth
  ####################################
  echo "$ans" | grep -q "Ethernet:"
  if [ "$?" == "0" ] ; then
    whiptail --title "Ethernet" --nocancel --menu "Does this board have Ethernet?" 0 0 0 \
	"yes" "" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    haseth=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # hasi2c
  ####################################
  echo "$ans" | grep -q "I2C:"
  if [ "$?" == "0" ] ; then
    whiptail --title "I2C" --nocancel --menu "Does this board use I2C (that you plan to access while in u-boot)?" 0 0 0 \
	"yes" "" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hasi2c=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # hasusbhost
  ####################################
  echo "$ans" | grep -q "USB Host:"
  if [ "$?" == "0" ] ; then
    whiptail --title "USB Host" --nocancel --menu "Does this board use USB Host (that you plan to access while in u-boot)?" 0 0 0 \
	"ch0" "USB-0" \
	"ch1" "USB-1" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hasusbhost=$(cat /tmp/answer.txt)
    continue
  fi


  ####################################
  # hasnorflash
  ####################################
  echo "$ans" | grep -q "Parallel NOR Flash:"
  if [ "$?" == "0" ] ; then
    whiptail --title "Parallel NOR Flash" --nocancel --menu "Does this board have Parallel NOR Flash?" 0 0 0 \
	"yes" "" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hasnorflash=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # hassdram
  ####################################
  echo "$ans" | grep -q "External SDRAM:"
  if [ "$?" == "0" ] ; then
    whiptail --title "External SDRAM" --nocancel --menu "Does this board have External SDRAM?" 0 0 0 \
	"yes" "" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hassdram=$(cat /tmp/answer.txt)
    continue
  fi

  ####################################
  # hashyperram
  ####################################
  echo "$ans" | grep -q "External HyperRAM:"
  if [ "$?" == "0" ] ; then
    whiptail --title "External HyperRam" --nocancel --menu "Does this board have External HyperRAM?" 0 0 0 \
	"yes" "" \
	"no" "" \
	2> /tmp/answer.txt

    # ESC was pressed?
    if [ "$?" != "0" ] ; then continue ;  fi

    hashyperram=$(cat /tmp/answer.txt)
    continue
  fi

  # Create BSP
  echo "$ans" | grep -q "Create BSP"
  if [ "$?" == "0" ] ; then

    # Do you want to continue?
    whiptail --title "Create BSP" --yesno \
"We will now create the following files:\n"\
"u-boot-2017.05/board/${companyname}/${boardname}/Kconfig\n"\
"u-boot-2017.05/board/${companyname}/${boardname}/lowlevel_init.S\n"\
"u-boot-2017.05/board/${companyname}/${boardname}/MAINTAINERS\n"\
"u-boot-2017.05/board/${companyname}/${boardname}/Makefile\n"\
"u-boot-2017.05/board/${companyname}/${boardname}/${boardname}.c\n"\
"\n"\
"u-boot-2017.05/configs/${boardname}_defconfig\n"\
"\n"\
"u-boot-2017.05/include/configs/${boardname}.h\n"\
"\n"\
"Do you want to continue?\n" 0 0 2> /tmp/answer.txt

    if [ "$?" == "0" ] ; then
      # yes
      break
    fi
  fi

  # while loop
  continue

done

##########################################################################################
# Done with Menu. Now copy and modify BSP files
##########################################################################################


function remove_section {

  #   /* SECT_XXXX */
  #   blaw
  #   blaw
  #   /* SECT_XXXX_END */

  #  $1 = section name
  #  $2 = file

  START_LINE=SECT_${1}
  END_LINE=SECT_${1}_END

  # Remove lines between /* SECT_XXXX */ and /* SECT_XXXX_END */
  sed -i -e '/'" $START_LINE "'/,/'" $END_LINE "'/d' $2
}

function keep_section {
  #   /* SECT_XXXX */
  #   blaw
  #   blaw
  #   /* SECT_XXXX_END */

  #  $1 = section name
  #  $2 = file

  START_LINE=SECT_${1}
  END_LINE=SECT_${1}_END

  # Remove just line /* SECT_XXXX */
  sed -i -e '/'" $START_LINE "'/,1d' $2

  # Remove just line /* SECT_XXXX_END */
  sed -i -e '/'" $END_LINE "'/,1d' $2
}


# copy over the files
mkdir -p board/${companyname}/${boardname}/

if [ "$devicetype" != "RZ_A2M" ] ; then
  # RZ/A1
  cp -a board/renesas/rza1template/Kconfig board/${companyname}/${boardname}/
  cp -a board/renesas/rza1template/lowlevel_init.S board/${companyname}/${boardname}/
  cp -a board/renesas/rza1template/MAINTAINERS board/${companyname}/${boardname}/
  cp -a board/renesas/rza1template/Makefile board/${companyname}/${boardname}/
  cp -a board/renesas/rza1template/rza1template.c board/${companyname}/${boardname}/${boardname}.c

  cp -a configs/rza1template_defconfig configs/${boardname}_defconfig
  cp -a include/configs/rza1template.h include/configs/${boardname}.h
else
  # RZ/A2
  cp -a board/renesas/rza2template/Kconfig board/${companyname}/${boardname}/
  cp -a board/renesas/rza2template/lowlevel_init.S board/${companyname}/${boardname}/
  cp -a board/renesas/rza2template/MAINTAINERS board/${companyname}/${boardname}/
  cp -a board/renesas/rza2template/Makefile board/${companyname}/${boardname}/
  cp -a board/renesas/rza2template/rza2template.c board/${companyname}/${boardname}/${boardname}.c

  cp -a configs/rza2template_defconfig configs/${boardname}_defconfig
  cp -a include/configs/rza2template.h include/configs/${boardname}.h
fi

# Convert board name to upper case
boardnameupper=`echo ${boardname} | tr '[:lower:]' '[:upper:]'`


# use sed to change all instances of the board name to the new name
# rzatemplate >> $boardname
# companyname >> $companyname

#Kconfig:
sed -i "s/rzatemplate/$boardname/g"  board/${companyname}/${boardname}/Kconfig
sed -i "s/RZATEMPLATE/$boardnameupper/g"  board/${companyname}/${boardname}/Kconfig
sed -i "s/companyname/$companyname/g"  board/${companyname}/${boardname}/Kconfig


#lowlevel_init.S
# (nothing to change)

#MAINTAINERS
sed -i "s/rzatemplate/$boardname/g"  board/${companyname}/${boardname}/MAINTAINERS
sed -i "s/RZATEMPLATE/$boardnameupper/g"  board/${companyname}/${boardname}/MAINTAINERS
sed -i "s/companyname/$companyname/g"  board/${companyname}/${boardname}/MAINTAINERS

#Makefile
sed -i "s/rzatemplate/$boardname/g"  board/${companyname}/${boardname}/Makefile

#rza1template.c
sed -i "s/rzatemplate/$boardname/g"  board/${companyname}/${boardname}/${boardname}.c

#rza1template_defconfig
sed -i "s/RZATEMPLATE/$boardnameupper/g"  configs/${boardname}_defconfig

#rza1template.h
sed -i "s/RZATEMPLATE/$boardnameupper/g"  include/configs/${boardname}.h


ALREADY_ADDED=`grep "config TARGET_$boardnameupper" arch/arm/mach-rmobile/Kconfig.rza1`
if [ "$ALREADY_ADDED" == "" ] ; then


  if [ "$devicetype" != "RZ_A2M" ] ; then
    # RZ/A1

    #arch/arm/mach-rmobile/Kconfig.rza1
    # (TAG_TARGET)
    #config TARGET_$boardnameupper
    #	bool "$boardname board"
    #	select BOARD_LATE_INIT
    sed -i "s/(TAG_TARGET)/(TAG_TARGET)\nconfig TARGET_$boardnameupper\n\tbool \"$boardname board\"\n\tselect BOARD_LATE_INIT\n/g"  arch/arm/mach-rmobile/Kconfig.rza1

    #arch/arm/mach-rmobile/Kconfig.rza1
    # (TAG_KCONFIG)
    #source "board/$companyname/$boardname/Kconfig"
    sed -i "s:(TAG_KCONFIG):(TAG_KCONFIG)\nsource \"board/$companyname/$boardname/Kconfig\":g"  arch/arm/mach-rmobile/Kconfig.rza1

  else
    # RZ/A2

    #arch/arm/mach-rmobile/Kconfig.rza1
    # (TAG_TARGET_A2)
    #config TARGET_$boardnameupper
    #	bool "$boardname board"
    #	select BOARD_LATE_INIT
    sed -i "s/(TAG_TARGET_A2)/(TAG_TARGET_A2)\nconfig TARGET_$boardnameupper\n\tbool \"$boardname board\"\n\tselect BOARD_LATE_INIT\n/g"  arch/arm/mach-rmobile/Kconfig.rza1

    #arch/arm/mach-rmobile/Kconfig.rza1
    # (TAG_KCONFIG_A2)
    #source "board/$companyname/$boardname/Kconfig"
    sed -i "s:(TAG_KCONFIG_A2):(TAG_KCONFIG_A2)\nsource \"board/$companyname/$boardname/Kconfig\":g"  arch/arm/mach-rmobile/Kconfig.rza1
  fi
fi


####################################
# Modify the config file
####################################

# devicetype
#-----------------------------
if [ "$devicetype" == "RZ_A1H" ] ; then
 echo ""  # keep default setting
elif [ "$devicetype" == "RZ_A1M" ] ; then
  sed -i 's/.*#define CONFIG_SYS_SDRAM_SIZE.*/#define CONFIG_SYS_SDRAM_SIZE		\(5 \* 1024 \* 1024\)/' include/configs/${boardname}.h
elif [ "$devicetype" == "RZ_A1L" ] ; then
  sed -i 's/.*#define CONFIG_SYS_SDRAM_SIZE.*/#define CONFIG_SYS_SDRAM_SIZE		\(3 \* 1024 \* 1024\)/' include/configs/${boardname}.h
fi

# extal
#-----------------------------
if [ "$devicetype" != "RZ_A2M" ] ; then
  sed -i "s/13.33MHz/$extal/" include/configs/${boardname}.h
else
  sed -i "s/24MHz/$extal/" include/configs/${boardname}.h

  if [ "$extal" == "10MHz" ] || [ "$extal" == "12MHz" ] ; then
    sed -i "s/PLL(x88), I:G:B:P1:P0 = 22:11:5.5:2.75:1.375/PLL(x88), I:G:B:P1:P0 = 44:22:11:5.5:2.75/" include/configs/${boardname}.h
  fi

  if [ "$extal" == "10MHz" ] || [ "$extal" == "20MHz" ] ; then
	#/* I  Clock = 528MHz,                           */
    sed -i "s/Clock = 528MHz/Clock = 440MHz/" include/configs/${boardname}.h
	#/* G  Clock = 264MHz                            */
    sed -i "s/G  Clock = 264MHz/G  Clock = 220MHz/" include/configs/${boardname}.h
	#/* B  Clock = 132MHz,                           */
    sed -i "s/Clock = 132MHz/Clock = 110MHz/" include/configs/${boardname}.h
	#/* P1 Clock = 66MHz,                            */
    sed -i "s/Clock = 66MHz/Clock = 55MHz/" include/configs/${boardname}.h
	#/* P0 Clock = 33MHz                             */
    sed -i "s/Clock = 33MHz/Clock = 27.5MHz/" include/configs/${boardname}.h
  fi
fi

# p1clockspeed
#-----------------------------
if [ "$devicetype" != "RZ_A2M" ] ; then
  sed -i "s/66666666/$p1clockspeed/" include/configs/${boardname}.h
else
  sed -i "s/66000000/$p1clockspeed/" include/configs/${boardname}.h
fi

# hasusbxtal
#-----------------------------
if [ "$devicetype" != "RZ_A2M" ] ; then
  #define CONFIG_R8A66597_XTAL		0x0000	/* 0=48MHz USB_X1, 1=12MHz EXTAL*/
  if [ "$hasusbxtal" == "no" ] ; then
    sed -i "s/CONFIG_R8A66597_XTAL		0x0000/CONFIG_R8A66597_XTAL		0x0001/" include/configs/${boardname}.h
  fi
fi

# scif
#-----------------------------
#SCIF_CONSOLE_BASE SCIF2_BASE
sed -i "s/SCIF2_BASE/${scif}_BASE/" include/configs/${boardname}.h


# hasmmc
# RZ/A1 only
#-----------------------------
if [ "$devicetype" != "RZ_A2M" ] ; then
  if [ "$hasmmc" == "no" ] ; then
    remove_section "SH_MMC" include/configs/${boardname}.h

    sed -i "s/.*CONFIG_MMC.*//" configs/${boardname}_defconfig
    echo '# CONFIG_MMC is not set' >> configs/${boardname}_defconfig
  else
    keep_section "SH_MMC" include/configs/${boardname}.h
  fi
fi


# hassdhi
# RZ/A1 only
#-----------------------------
if [ "$devicetype" != "RZ_A2M" ] ; then
  if [ "$hassdhi" == "no" ] ; then
    remove_section "SH_SDHI" include/configs/${boardname}.h
  else
    keep_section "SH_SDHI" include/configs/${boardname}.h
  fi

  if [ "$hassdhi" == "ch0" ] ; then
    # CONFIG_SYS_SH_SDHI1_BASE -> CONFIG_SYS_SH_SDHI0_BASE
    sed -i "s:CONFIG_SYS_SH_SDHI1_BASE:CONFIG_SYS_SH_SDHI0_BASE:" board/${companyname}/${boardname}/${boardname}.c

    # RZ_SDHI_CHANNEL 1 -> RZ_SDHI_CHANNEL 0
    sed -i "s:RZ_SDHI_CHANNEL			1:RZ_SDHI_CHANNEL			0:" include/configs/${boardname}.h
  fi
fi

# hasmmc
# hassdhi
# RZ/A2 only
#-----------------------------
if [ "$devicetype" == "RZ_A2M" ] ; then
  if [ "$hasmmc" == "no" ] && [ "$hassdhi" == "no" ] ; then

    remove_section "SDMMC" include/configs/${boardname}.h
    remove_section "SDMMC" board/${companyname}/${boardname}/${boardname}.c

    sed -i "s/.*CONFIG_MMC.*//" configs/${boardname}_defconfig
    echo '# CONFIG_MMC is not set' >> configs/${boardname}_defconfig
  else
    keep_section "SDMMC" include/configs/${boardname}.h
    keep_section "SDMMC" board/${companyname}/${boardname}/${boardname}.c
  fi

  if [ "$hasmmc" == "ch0" ] || [ "$hassdhi" == "ch0" ] ; then
    keep_section "SDMMC_CH0" board/${companyname}/${boardname}/${boardname}.c
  else
    remove_section "SDMMC_CH0" board/${companyname}/${boardname}/${boardname}.c
  fi

  if [ "$hasmmc" == "ch1" ] || [ "$hassdhi" == "ch1" ] ; then
    keep_section "SDMMC_CH1" board/${companyname}/${boardname}/${boardname}.c
  else
    remove_section "SDMMC_CH1" board/${companyname}/${boardname}/${boardname}.c
  fi
fi


# haseth
#-----------------------------
if [ "$haseth" == "no" ] ; then
  remove_section "ETHERNET" include/configs/${boardname}.h
  remove_section "ETHERNET" board/${companyname}/${boardname}/${boardname}.c

  # Remove these lines from the xxx_defconfig
  # CONFIG_CMD_DHCP=y
  # CONFIG_CMD_MII=y
  # CONFIG_CMD_PING=y
  # CONFIG_CMD_SNTP=y
  sed -i "s/.*CONFIG_CMD_DHCP.*//" configs/${boardname}_defconfig
  sed -i "s/.*CONFIG_CMD_MII.*//" configs/${boardname}_defconfig
  sed -i "s/.*CONFIG_CMD_PING.*//" configs/${boardname}_defconfig
  sed -i "s/.*CONFIG_CMD_SNTP.*//" configs/${boardname}_defconfig
else
  keep_section "ETHERNET" include/configs/${boardname}.h
  keep_section "ETHERNET" board/${companyname}/${boardname}/${boardname}.c
fi

# hasnorflash
#-----------------------------
if [ "$hasnorflash" == "no" ] ; then
  remove_section "NOR_FLASH" include/configs/${boardname}.h

  # Make sure CONFIG_CMD_IMLS is not selected because we didn't set CONFIG_SYS_MAX_FLASH_BANKS
  echo '# CONFIG_CMD_IMLS is not set' >> configs/${boardname}_defconfig
else
    keep_section "NOR_FLASH" include/configs/${boardname}.h
fi

# hasi2c
#-----------------------------
if [ "$hasi2c" == "no" ] ; then
  remove_section "I2C" include/configs/${boardname}.h
  remove_section "I2C" board/${companyname}/${boardname}/${boardname}.c

  # Remove these lines from the xxx_defconfig
  # CONFIG_CMD_I2C=y
  # CONFIG_RZA_RIIC=y
  sed -i "s/.*CONFIG_CMD_I2C.*//" configs/${boardname}_defconfig
  sed -i "s/.*CONFIG_RZA_RIIC.*//" configs/${boardname}_defconfig
else
  keep_section "I2C" include/configs/${boardname}.h
  keep_section "I2C" board/${companyname}/${boardname}/${boardname}.c
fi

# hasusbhost
#-----------------------------
if [ "$hasusbhost" == "no" ] ; then
  remove_section "USB_HOST" include/configs/${boardname}.h

  # Remove these lines from the xxx_defconfig
  # CONFIG_CMD_USB=y
  # CONFIG_USB=y
  # CONFIG_USB_STORAGE=y
  sed -i "s/.*CONFIG_CMD_USB.*//" configs/${boardname}_defconfig
  sed -i "s/.*CONFIG_USB.*//" configs/${boardname}_defconfig
  sed -i "s/.*CONFIG_USB_STORAGE.*//" configs/${boardname}_defconfig
else
  keep_section "USB_HOST" include/configs/${boardname}.h

  if [ "$hasusbhost" == "ch0" ] ; then
    # change default from ch1 to ch0
    #define CONFIG_R8A66597_BASE_ADDR	R8A66597_BASE1
    sed -i "s/CONFIG_R8A66597_BASE_ADDR	R8A66597_BASE1/CONFIG_R8A66597_BASE_ADDR	R8A66597_BASE0/" configs/${boardname}_defconfig
  fi
fi

# hassdram
#-----------------------------
if [ "$hassdram" == "no" ] ; then
  remove_section "SDRAM" board/${companyname}/${boardname}/${boardname}.c
else
  keep_section "SDRAM" board/${companyname}/${boardname}/${boardname}.c
fi

# hashyperram
#-----------------------------
if [ "$hashyperram" == "no" ] ; then
  remove_section "HYPERRAM" board/${companyname}/${boardname}/${boardname}.c
else
  keep_section "HYPERRAM" board/${companyname}/${boardname}/${boardname}.c
fi


###############################
# done
###############################

whiptail --msgbox \
"Complete!"\
"\nPlease note that none of the pin mux settings have been configured yet.\n"\
"You MUST manually edit the file: \n"\
"      board/${companyname}/${boardname}/${boardname}.c\n\n"\
"Also, please review your configuration file and make any necessary changes:\n"\
"      include/configs/${boardname}.h\n\n"\
"To build for your board, please use the following commands:\n"\
"   make ${boardname}_defconfig\n"\
"   make\n\n"\
"Or, if you are using the bsp build environment:\n"\
"   ./build.sh u-boot ${boardname}_defconfig\n"\
"   ./build.sh u-boot\n" 0 0


# open in gedit
CHECK=`which gedit`
if [ "$CHECK" != "" ] ; then
  whiptail --title "Open files" --yesno "Would you like to open the following files now?\n\nboard/${companyname}/${boardname}/${boardname}.c\ninclude/configs/${boardname}.h\n\n" 0 0 2> /tmp/answer.txt

  if [ "$?" == "0" ] ; then
    # yes
    gedit board/${companyname}/${boardname}/${boardname}.c include/configs/${boardname}.h &
  fi
fi

