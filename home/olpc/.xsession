$HOME/conf/xautorun
while true
do
	echo `date +%a\ %m.%d.%y\ %H:%M:%S` :: $(battery-status)
done | dwm || exit
