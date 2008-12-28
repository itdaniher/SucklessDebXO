# If not running interactively, don't do anything
[ -z "$PS1" ] && return

# don't put duplicate lines in the history. See bash(1) for more options
export HISTCONTROL=ignoredups
# ... and ignore same sucessive entries.
export HISTCONTROL=ignoreboth

# check the window size after each command and, if necessary,
# update the values of LINES and COLUMNS.
shopt -s checkwinsize

# make less more friendly for non-text input files, see lesspipe(1)
[ -x /usr/bin/lesspipe ] && eval "$(lesspipe)"

# set variable identifying the chroot you work in (used in the prompt below)
# set a fancy prompt (non-color, unless we know we "want" color) 
case "$TERM" in
    xterm-color) color_prompt=yes;;
esac

eval "`dircolors -b`"
alias ls="ls -h --color=auto"
alias grep="grep --color=auto"
alias apt="sudo apt-get"

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
PATH=$PATH:$HOME/python
vPATH=$PATH:/usr/games
export PATH
#PATH=$PATH:            

HISTFILESIZE=1000000
export HISTFILESIZE

EDITOR=nano
export EDITOR

screen -xA
cd

#       _           _  _          __
#  ___ | |__   ___ | || |        / _| _   _      ___  _ __  __ _
# / __|| '_ \ / _ \| || | _____ | |_ | | | |    / _ \| '__|/ _` |
# \__ \| | | |  __/| || ||_____||  _|| |_| | _ | (_) | |  | (_| |
# |___/|_| |_|\___||_||_|       |_|   \__,_|(_) \___/|_|   \__, |
#                                                   .bashrc|___/
#


# convert permissions to octal - http://www.shell-fu.org/lister.php?id=205
alias lo='ls -l | sed -e 's/--x/1/g' -e 's/-w-/2/g' -e 's/-wx/3/g' -e 's/r--/4/g' -e 's/r-x/5/g' -e 's/rw-/6/g' -e 's/rwx/7/g' -e 's/---/0/g''


# print a random shell-fu tip - http://www.shell-fu.org/lister.php?id=192
alias shell-fu='links2 -dump "http://www.shell-fu.org/lister.php?random" | grep -A 100 -- ----'


# get an ordered list of subdirectory sizes - http://www.shell-fu.org/lister.php?id=275
alias dux='du -sk ./* | sort -n | awk '\''BEGIN{ pref[1]="K"; pref[2]="M"; pref[3]="G";} { total = total + $1; x = $1; y = 1; while( x > 1024 ) { x = (x + 1023)/1024; y++; } printf("%g%s\t%s\n",int(x*10)/10,pref[y],$2); } END { y = 1; while( total > 1024 ) { total = (total + 1023)/1024; y++; } printf("Total: %g%s\n",int(total*10)/10,pref[y]); }'\'''


# overwrite a file with zeroes - http://www.shell-fu.org/lister.php?id=94
zero() {
  case "$1" in
    "")     echo "Usage: zero "
            return -1;
  esac
  filesize=`wc -c  "$1" | awk '{print $1}'`
  dd if=/dev/zero of=$1 count=$filesize bs=1
}


# keep your home directory organised - http://www.shell-fu.org/lister.php?id=310
export TD="$HOME/temp/`date +'%Y-%m-%d'`"
td(){
    td=$TD
    if [ ! -z "$1" ]; then
        td="$HOME/temp/`date -d "$1 days" +'%Y-%m-%d'`";
    fi
    mkdir -p $td; cd $td
    unset td
}
