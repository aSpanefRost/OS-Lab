if [ -f $1 ];then
  case $1 in
       *.tar.bz2|*.tar.gz|*.bz2|*.tar|*.tbz2|*.tgz|*.Z)tar -xf $1;;
       *.zip)unzip $1;;
       *.gz)guzip $1;;
       *.rar)rar x $1;;
       *.7z)7z x $1;;
       *)echo "Unknown file format: cannot extract";;
  esac
fi
