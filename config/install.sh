#!/bin/sh
#---------------------------------------------------------------------------

ReplaceString(){
    _src=$1
    _des=$2
    _confFile=$3
    sed -i s?$_src?$_des?g  $_confFile
}
 
GetPropertyFromFile(){
    _key=$1
    _confFile=$2
    _value=`awk 'BEGIN { FS="= *"} { if($1=="'$_key'") {print $2}}' $_confFile`
    echo $_value
}
 
runner=`id | awk 'BEGIN { FS="("} { print $2}' | awk 'BEGIN { FS=")"} {print $1}'`
host=`hostname -s`
 
pdir=`dirname $0`
curPath=`cd $pdir ; pwd`

ReplaceString __LAVATOP__ $curPath conf/lsf.conf
ReplaceString __LAVATOP__ $curPath conf/cshrc.jhlava
ReplaceString __LAVATOP__ $curPath conf/profile.jhlava
ReplaceString __LAVATOP__ $curPath bin/jhlava
ReplaceString __HOSTNAME__ $host conf/lsf.cluster.jhlava
ReplaceString __OS__ '!' conf/lsf.cluster.jhlava

if [ "x"$JHLAVA_CLUSTER_NAME = "x" ]; then
	echo -n "Enter the cluster name and press [ENTER]: "
	read JHLAVA_CLUSTER_NAME
fi
if [ "x"$JHLAVA_CLUSTER_NAME = "x" ]; then
	echo "Cluster name can not be null. Exit."
	exit 1
fi

echo -n "Enter jhlava admin username and press [ENTER]: "
read LSFADMIN
if [ "x"$LSFADMIN = "x" ] || [ "$LSFADMIN" = "root" ]; then
        echo "Admin can not be root or null. Exit."
        exit 1
fi 
res=`id $LSFADMIN 2>&1 | awk '{ print index($0, "No such user")}'`
if [ $res != 0 ]; then
	echo "Can not find user $LSFADMIN. Please creat firstly. Exit."
	exit 1
fi
 
if [ "$JHLAVA_CLUSTER_NAME" != "jhlava" ]; then
	mv conf/lsf.cluster.jhlava conf/lsf.cluster.$JHLAVA_CLUSTER_NAME
fi
ReplaceString __CLUSTERNAME__ $JHLAVA_CLUSTER_NAME conf/lsf.shared
#echo clustername:$JHLAVA_CLUSTER_NAME

lsfadmingroup=`id $LSFADMIN | awk 'BEGIN{ FS="[ |(|)]"} { print $5 }'`
ReplaceString __LSFADMIN__ $LSFADMIN conf/lsf.cluster.$JHLAVA_CLUSTER_NAME
#echo lsfadmin:$LSFADMIN $lsfadmingroup
chown $LSFADMIN:$lsfadmingroup -R ./*
chmod 755 bin/jhlava

echo "Install completed."

