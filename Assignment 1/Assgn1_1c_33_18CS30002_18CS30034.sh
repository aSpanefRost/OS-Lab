(("$#"!=2||!($2==1||$2==2||$2==3||$2==4)))&&exit||[ ! -f "$1" ]&&exit
cat "$1"|awk -v var="$2" '{print $var}'|sort -f|uniq -ic|sort -nr -k1,1|awk '{print tolower($2),$1}'>>"1c_output_${2}_column.freq"
