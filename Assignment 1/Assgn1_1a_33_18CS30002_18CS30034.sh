gcd(){
    if(($1==0));then
        return $2
    fi
    gcd $(($2%$1)) $1
    return $?
}
oIFS="$IFS"
IFS=,
set -- $1
IFS="$oIFS"
(("$#">9||"$#"<2))&&exit
temp=${1/-/}
for var in "$@";do
    [[ $var =~ [^-?0-9] ]]&&exit
    gcd $temp ${var/-/}
    temp="$?"
done
echo "$temp"
