#FIX FOR JAVA-DWM FUCKUP
AWT_TOOLKIT=MToolkit
export AWT_TOOLKIT

TERM=xterm
export TERM

#Paths
PATH=""
export PATH
PATH=$PATH:/bin            
PATH=$PATH:/sbin            
PATH=$PATH:/usr/bin            
PATH=$PATH:/usr/sbin                        
PATH=$PATH:/usr/games
PATH=$PATH:/usr/local/bin           
PATH=$PATH:$HOME/bash
PATH=$PATH:/usr/games
export PATH

HISTFILESIZE=1000000
export HISTFILESIZE

EDITOR=nano
export EDITOR

RANDOM=$(cat /dev/urandom|head -n 2)

source ~/.config/suckless/settings

export HISTCONTROL=ignoredups
export HISTCONTROL=ignoreboth

shopt -s checkwinsize

case "$TERM" in
    xterm-color) color_prompt=yes;;
esac

eval "`dircolors -b`"
alias ls="ls -h --color=auto"
alias grep="grep --color=auto"
alias xbs="xoset b"
alias pms="sudo xo-suspend"
alias pgrep="pgrep -lf"

# convert permissions to octal - http://www.shell-fu.org/lister.php?id=205
alias lo='ls -lah | sed -e 's/--x/1/g' -e 's/-w-/2/g' -e 's/-wx/3/g' -e 's/r--/4/g' -e 's/r-x/5/g' -e 's/rw-/6/g' -e 's/rwx/7/g' -e 's/---/0/g''

# If not running interactively, don't do anything else
[ -z "$PS1" ] && return

#export PROMPT_COMMAND='echo -ne "$USER@$HOSTNAME:$PWD"'
export PROMPT_COMMAND='echo -ne "\e]0; $USER@$HOSTNAME:$PWD \a\033k\033\\"'
#export PROMPT_COMMAND='echo -ne "$USER@$HOSTNAME:$PWD \a\033k\033\\"'
#"\033k\033\\" - to help screen
#"\e]0; \u@\h:\w \a" - to set xterm title
export PS1='\u@\h:\w \$ '

for a in $(ls /etc/bash_completion.d/)
do
source "/etc/bash_completion.d/$a"
done

#screen -xAr
